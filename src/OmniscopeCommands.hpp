#pragma once

#include <aglio/type_descriptor.hpp>
#include <array>
#include <variant>

namespace Omniscope {

struct Version {
    std::uint8_t major{};
    std::uint8_t minor{};
    std::uint8_t patch{};
};

struct Id {
    std::string   serial;
    std::string   type;
    std::uint32_t sampleRate;
    Version       hwVersion;
    Version       swVersion;
};

struct StartOfFrame {
    std::uint16_t sof_and_change_index;
};

struct MeasureData {
    std::uint8_t              packageCounter;
    StartOfFrame              sof;
    std::vector<std::int16_t> data;
};

struct SetRgb {
    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
};

struct Ping {};
struct GetId {};
struct Start {};
struct Stop {};

using UCToHostTypes = std::variant<Id, MeasureData>;
using HostToUCTypes = std::variant<GetId, Ping, Start, Stop, SetRgb>;
}   // namespace Omniscope

#include "TypeDescriptor_OmniscopeCommands.hpp"
