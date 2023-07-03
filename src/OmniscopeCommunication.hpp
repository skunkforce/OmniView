#pragma once

#include <aglio/packager.hpp>
#include <algorithm>
#include <atomic>
#include <boost/asio.hpp>
#include <boost/crc.hpp>
#include <condition_variable>
#include <cstdint>
#include <filesystem>
#include <fmt/format.h>
#include <iterator>
#include <map>
#include <memory>
#include <regex>
#include <span>
#include <thread>
#include <tuple>
#include <vector>

namespace Omniscope {

struct PacketCrc {
    using type = std::uint16_t;
    static type calc(std::span<std::byte const> s) {
        boost::crc_ccitt_type crc;
        crc.process_bytes(s.data(), s.size());
        return crc.checksum();
    }
};
using Packager    = aglio::Packager<aglio::CrcConfig<PacketCrc>>;
using Data_t      = std::int16_t;
using Package_t   = std::tuple<std::uint16_t, std::uint16_t, std::array<Data_t, 25>>;
using SOFData_t   = std::pair<std::uint16_t, Data_t>;
using TimedData_t = std::pair<std::uint64_t, Data_t>;

struct Device {
    std::string    type;
    std::string    serial;
    std::string    port;
    //constexpr bool operator==(Device const&) const  = default;
    //constexpr auto operator<=>(Device const&) const = default;
    bool operator==(Device const&) const  = default;
    auto operator<=>(Device const&) const = default;
};

inline auto queryDevices() {
    std::vector<Device> devices;

    std::regex const  regex{"usb-Auto-Intern_GmbH_([[:alnum:]]+)_([[:alnum:]]+)-if[[:digit:]]{2}"};
    std::string const path{"/dev/serial/by-id"};

    if(std::filesystem::exists(path)) {
        for(auto const& dir_entry : std::filesystem::directory_iterator{path}) {
            if(dir_entry.is_character_file()) {
                std::smatch match;
                std::string filename = dir_entry.path().filename().string();
                std::regex_match(filename, match, regex);
                if(match.size() == 3) {
                    devices.push_back(
                      Device{match[1].str(), match[2].str(), dir_entry.path().string()});
                }
            }
        }
    }

    std::sort(devices.begin(), devices.end());

    return devices;
}

struct DeviceRunner {
private:
    struct SerialDevice {
        static constexpr std::size_t PackageReserve = 512;
        static constexpr std::size_t SamplesReserve = PackageReserve * 32;
        static constexpr std::size_t ReadSize       = 4096;
        static constexpr std::size_t ReadReserve    = ReadSize * 4;
        static constexpr std::size_t NumRecvBuffers = 16;

        std::vector<std::vector<std::byte>> recvBuffers;
        std::size_t                         currentRecvBuffer{};
        std::size_t                         currentReadBuffer{};

        std::vector<std::byte> parseBuffer;

        std::vector<Package_t> packages;
        std::vector<SOFData_t> samples;

        std::size_t       packageCnt{};
        std::uint16_t     lastCnt{};
        bool              first{true};
        bool              hasError{false};
        std::atomic<bool> hasFatalError{false};

        std::mutex&              mutex;
        std::condition_variable& cv;

        Device                   dev;
        boost::asio::serial_port serialPort;

        SerialDevice(
          boost::asio::io_context& ioc,
          Device const&            dev_,
          std::mutex&              mutex_,
          std::condition_variable& cv_)
          : mutex{mutex_}
          , cv{cv_}
          , dev{dev_}
          , serialPort{ioc, dev.port} {
            serialPort.set_option(boost::asio::serial_port_base::baud_rate{115200});
            serialPort.set_option(boost::asio::serial_port_base::character_size{8});
            serialPort.set_option(
              boost::asio::serial_port_base::parity{boost::asio::serial_port_base::parity::none});
            serialPort.set_option(boost::asio::serial_port_base::stop_bits{
              boost::asio::serial_port_base::stop_bits::one});
            serialPort.set_option(boost::asio::serial_port_base::flow_control{
              boost::asio::serial_port_base::flow_control::none});

            recvBuffers.resize(NumRecvBuffers);
            for(auto& recvBuffer : recvBuffers) {
                recvBuffer.reserve(ReadReserve);
            }
            parseBuffer.reserve(ReadReserve * NumRecvBuffers * 2);
            packages.reserve(PackageReserve);
            samples.reserve(SamplesReserve);
            start_async_read_some();
        }

        void extractPackages() {
            while(!parseBuffer.empty()) {
                auto const package = Packager::template unpack<Package_t>(parseBuffer);
                if(package) {
                    ++packageCnt;
                    //throw away packages because of USB buffer may had old packages
                    if(8 > packageCnt) {
                        continue;
                    }

                    if(
                      static_cast<decltype(lastCnt)>(lastCnt + 1) != std::get<0>(*package)
                      && !first)
                    {
                        hasError = true;
                        fmt::print(stderr, "Error in {}-{}: lost package\n", dev.type, dev.serial);
                    }

                    lastCnt = std::get<0>(*package);
                    first   = false;
                    packages.push_back(*package);
                } else {
                    break;
                }
            }
        }

        void unpackPackages() {
            for(auto const& package : packages) {
                std::uint16_t const startSof{
                  static_cast<std::uint16_t>(std::get<1>(package) & 0x07FF)};
                std::uint16_t const sofChange{
                  static_cast<std::uint16_t>((std::get<1>(package) & 0xF800) >> 11)};

                for(std::uint16_t i{}; auto const& v : std::get<2>(package)) {
                    if(i >= sofChange) {
                        samples.push_back(std::make_pair((startSof + 1) % 2048, v));
                    } else {
                        samples.push_back(std::make_pair(startSof, v));
                    }
                    ++i;
                }
            }
            packages.clear();
        }

        void start_async_read_some() {
            auto&      buffer  = recvBuffers[currentRecvBuffer];
            auto const oldSize = buffer.size();
            buffer.resize(oldSize + ReadSize);

            serialPort.async_read_some(
              boost::asio::buffer(buffer.data() + oldSize, ReadSize),
              [this](auto const& ec, auto s) { read_some_callback(ec, s); });
        }

        void swap_buffer() {
            std::lock_guard<std::mutex> lock{mutex};

            currentRecvBuffer = (currentRecvBuffer + 1) % NumRecvBuffers;

            cv.notify_all();
        }

        void copy_buffer() {
            std::lock_guard<std::mutex> lock{mutex};

            while(currentRecvBuffer != currentReadBuffer) {
                auto& buffer = recvBuffers[currentReadBuffer];
                std::copy(buffer.begin(), buffer.end(), std::back_inserter(parseBuffer));
                buffer.clear();
                currentReadBuffer = (currentReadBuffer + 1) % NumRecvBuffers;
            }
        }

        void
        read_some_callback(boost::system::error_code const& error, std::size_t bytes_transferred) {
            auto& buffer = recvBuffers[currentRecvBuffer];
            if(!error) {
                buffer.resize(buffer.size() - (ReadSize - bytes_transferred));
                if(buffer.size() > ReadSize * 2) {
                    swap_buffer();
                }

                start_async_read_some();
            } else {
                hasFatalError = true;
                fmt::print(stderr, "Error in {}-{}: {}\n", dev.type, dev.serial, error.message());
            }
        }

        void parse() {
            copy_buffer();
            extractPackages();
            unpackPackages();
        }
    };

    static constexpr std::size_t alignedSamplesReserve = 1 << 24;

    boost::asio::io_context                    ioc;
    std::mutex                                 mutex;
    std::mutex                                 copyOutMutex;
    std::condition_variable                    cv;
    std::atomic<bool>                          stop{false};
    std::vector<std::unique_ptr<SerialDevice>> devices;

    std::map<Device, std::vector<std::pair<double, double>>> alignedSamples;

    bool firstAlign{true};

    std::jthread readThread;
    std::jthread parseThread;

    void alignSamples() {
        std::size_t const minSamples
          = std::min_element(
              devices.begin(),
              devices.end(),
              [](auto const& a, auto const& b) { return a->samples.size() < b->samples.size(); })
              ->get()
              ->samples.size();

        if(128 > minSamples) {
            return;
        }

        if(firstAlign == true) {
            auto const deviceWithHighestSOF
              = std::max_element(devices.begin(), devices.end(), [](auto const& a, auto const& b) {
                    return a->samples.front().first < b->samples.front().first;
                });

            std::uint16_t const sofToDeleteTo
              = ((**deviceWithHighestSOF).samples.front().first + 1) % 2048;

            for(auto& device : devices) {
                device->samples.erase(
                  device->samples.begin(),
                  std::find_if(device->samples.begin(), device->samples.end(), [&](auto const& v) {
                      return v.first == sofToDeleteTo;
                  }));
            }
            firstAlign = false;
            return;
        }

        for(auto& device : devices) {
            auto& inSamples = device->samples;
            auto  end       = std::next(inSamples.begin(), static_cast<long>(minSamples));
            {
                std::lock_guard<std::mutex> lock(copyOutMutex);
                auto&                       outSamples    = alignedSamples[device->dev];
                double                      lastTimeValue = [&]() {
                    if(outSamples.empty()) {
                        return -1.0;
                    } else {
                        return outSamples.back().first;
                    }
                }();
                std::transform(
                  inSamples.begin(),
                  end,
                  std::back_inserter(outSamples),
                  [&](auto const& sample) {
                      lastTimeValue += 1.0;
                      return std::pair<double, double>{lastTimeValue, sample.second};
                  });
            }
            inSamples.erase(inSamples.begin(), end);
        }
    }

    void readRunner(std::stop_token stoken) {
        while(ioc.run_one() && !stoken.stop_requested() && !stop) {
        }
    }

    void parseRunner(std::stop_token stoken) {
        while(!stoken.stop_requested()) {
            for(auto& device : devices) {
                device->parse();
                if(device->hasFatalError) {
                    stop = true;
                    return;
                }
            }
            alignSamples();
            std::unique_lock<std::mutex> lock{mutex};
            cv.wait_for(lock, std::chrono::milliseconds{1});
        }
    }

    std::vector<std::unique_ptr<SerialDevice>>
    make_serial_devices(std::vector<Device> const& devices_) {
        std::vector<std::unique_ptr<SerialDevice>> ret;
        ret.reserve(devices.size());

        for(auto const& dev : devices_) {
            ret.push_back(std::make_unique<SerialDevice>(ioc, dev, mutex, cv));
        }
        return ret;
    }

    std::map<Device, std::vector<std::pair<double, double>>>
    initSamplesBuffer(std::vector<Device> const& devices_) {
        std::map<Device, std::vector<std::pair<double, double>>> ret;

        for(auto const& dev : devices_) {
            ret[dev].reserve(alignedSamplesReserve);
        }
        return ret;
    }

public:
    DeviceRunner(std::vector<Device> const& devices_)
      : devices{make_serial_devices(devices_)}
      , alignedSamples{initSamplesBuffer(devices_)}
      , readThread{std::bind_front(&DeviceRunner::readRunner, this)}
      , parseThread{std::bind_front(&DeviceRunner::parseRunner, this)} {}

    bool copyOut(std::map<Device, std::vector<std::pair<double, double>>>& outMap) {
        std::lock_guard<std::mutex> lock(copyOutMutex);
        if(stop) {
            return false;
        }
        for(auto& [dev, inData] : alignedSamples) {
            auto& outData = outMap[dev];
            outData.reserve((outData.size() + inData.size()) * 10);

            auto end = [&]() {
                if(inData.empty()) {
                    return inData.begin();
                }
                return std::next(inData.end(), -1);
            }();

            std::copy(inData.begin(), end, std::back_inserter(outData));
            inData.erase(inData.begin(), end);
        }
        return true;
    }
};

}   // namespace Omniscope
