// #include "OmniscopeCommunication.hpp"
// clang-format off
#include <boost/asio.hpp>
//clang-format on
#include "../ai_omniscope-v2-communication_sw/src/OmniscopeSampler.hpp"
#include "apihandler.hpp"
#include "create_training_data.hpp"
#include "get_from_github.hpp"
#include "saves_popup.hpp"
#include "jasonhandler.hpp"
#include "settingspopup.hpp"
#include <ImGuiInstance/ImGuiInstance.hpp>
#include <algorithm>
#include <fmt/chrono.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fstream>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>
#include <set>
#include <thread>
#include <cmake_git_version/version.hpp>
// clang-format off
#include <imfilebrowser.h>
// clang-format on
#include "sendData.hpp"
#include <regex>

// include style

#include "menus/Style.hpp"

// include menus

#include "menus/DevicesMenu.cpp"
#include "menus/SideBarMenu.cpp"

// Load Languages
static std::vector<std::string>
getAvailableLanguages(std::string const &languageFolder) {
  std::vector<std::string> languages;
  for (auto const &entry :
       std::filesystem::directory_iterator(languageFolder)) {
    if (entry.is_regular_file()) {
      std::string extension = entry.path().extension().string();
      if (extension == ".json") {
        std::string filename = entry.path().filename().stem().string();
        languages.push_back(filename);
      }
    }
  }
  return languages;
}

static void set_button_style_to(nlohmann::json const &config,
                                std::string const &name) {
  ImGui::PushStyleColor(
      ImGuiCol_Button,
      ImVec4(load_json<Color>(config, "button", name, "normal")));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonHovered,
      ImVec4(load_json<Color>(config, "button", name, "hover")));
  ImGui::PushStyleColor(
      ImGuiCol_ButtonActive,
      ImVec4(load_json<Color>(config, "button", name, "active")));
}

static void load_settings(nlohmann::json const &config) {
  ImGui::PushStyleColor(ImGuiCol_MenuBarBg,
                        ImVec4(load_json<Color>(config, "menubar", "main")));
  ImGui::PushStyleColor(ImGuiCol_PopupBg,
                        ImVec4(load_json<Color>(config, "menubar", "popup")));
  ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(load_json<Color>(
                                           config, "text", "color", "normal")));
  ImGui::PushStyleColor(ImGuiCol_WindowBg,
                        ImVec4(load_json<Color>(config, "window", "color")));
  set_button_style_to(config, "standart");
}

// ###########################################################################
// ############# INT MAIN BEGINN #############################################
// ###########################################################################

int main() {

  // ImGuiIO &io = ImGui::GetIO();
  // io.FontGlobalScale = load_json<float>(config, "text", "scale");

  // Loading the config and language files
  nlohmann::json config;
  const std::string configpath = "config/config.json";
  if (std::filesystem::exists(configpath)) {
    fmt::print("found config.json\n\r");
  } else {
    fmt::print("Did not find config.json.\n Download from Github\n\r");
    update_config_from_github();
  }
  config = load_json_file(configpath);
  if (std::filesystem::exists(
          load_json<std::string>(config, ("languagepath")))) {
    fmt::print("found language: {}\n\r",
               load_json<std::string>(config, ("language")));
  } else {
    fmt::print("Did not find {}.\n Download from Github\n\r",
               load_json<std::string>(config, ("language")));
    update_language_from_github();
  }

  constexpr ImVec2 toolBtnSize = ImVec2(200, 100); // toolbar buttons size
  constexpr ImVec2 btnSize = ImVec2(0, 0);         // other buttons size

  std::vector<std::string> availableLanguages =
      getAvailableLanguages(load_json<std::string>(config, ("languagepath")));

  nlohmann::json language =
      load_json_file(load_json<std::string>(config, "languagepath") +
                     load_json<std::string>(config, "language") + ".json");

  // static constexpr std::size_t captureDataReserve = 1 << 26;

  // Creating a device manager
  OmniscopeDeviceManager deviceManager{};
  std::vector<std::shared_ptr<OmniscopeDevice>>
      devices; // = deviceManager.getDevices(VID, PID);
  // auto newDevices = devices;
  std::map<Omniscope::Id, std::array<float, 3>> colorMap;

  //   auto startTimepoint = std::chrono::system_clock::now();
  auto now = std::chrono::system_clock::now();
  std::time_t now_time_t = std::chrono::system_clock::to_time_t(now);
  std::tm now_tm = *std::gmtime(&now_time_t);

  double xmax_paused{0};
  static bool open_settings = false;
  static bool open_generate_training_data = false;
  static bool upload_success = false;
  static bool flagPaused = true;
  static bool flagDataNotSaved = true;
  static ImVec2 mainMenuBarSize;
  std::optional<OmniscopeSampler> sampler{};
  std::map<Omniscope::Id, std::vector<std::pair<double, double>>> captureData;

  std::string path;
  path.resize(256);

  // creating the plot function

  auto addPlots = [&, firstRun = std::set<std::string>{}](
                      auto const &name, auto const &plots,
                      auto axesSetup) mutable {
    auto const plotRegion = ImGui::GetContentRegionAvail();
    if (ImPlot::BeginPlot(name, plotRegion)) {
      double x_min = std::numeric_limits<double>::max();
      double x_max = std::numeric_limits<double>::min();

      for (auto const &plot : plots) {
        if (!plot.second.empty()) {
          x_min = std::min(x_min, plot.second.front().first);
          x_max = std::max(x_max, plot.second.back().first);
        }
      }

      axesSetup(x_min, x_max);

      auto const limits = [&]() {
        if (!firstRun.contains(name)) {
          firstRun.insert(name);
          return ImPlotRect(x_min, x_max, 0, 0);
        }
        return ImPlot::GetPlotLimits();
      }();
      auto addPlot = [&](auto const &plot) {
        if (!plot.second.empty()) {
          auto const start = [&]() {
            auto p =
                std::lower_bound(plot.second.begin(), plot.second.end(),
                                 std::pair<double, double>{limits.X.Min, 0});
            if (p != plot.second.begin()) {
              return p - 1;
            }
            return p;
          }();

          auto const end = [&]() {
            auto p =
                std::upper_bound(start, plot.second.end(),
                                 std::pair<double, double>{limits.X.Max, 0});
            if (p != plot.second.end()) {
              return p + 1;
            }
            return p;
          }();
          std::size_t const stride = [&]() -> std::size_t {
            auto const s = std::distance(start, end) / (plotRegion.x * 2.0);
            if (1 >= s) {
              return 1;
            }
            return static_cast<std::size_t>(s);
          }();

          ImPlot::PlotLine(
              fmt::format("{}-{}", plot.first.type, plot.first.serial).c_str(),
              std::addressof(start->first), std::addressof(start->second),
              static_cast<std::size_t>(std::distance(start, end)) / stride, 0,
              0, 2 * sizeof(double) * stride);
        }
      };

      for (auto const &plot : plots) {
        ImPlot::SetNextLineStyle(ImVec4{colorMap[plot.first][0],
                                        colorMap[plot.first][1],
                                        colorMap[plot.first][2], 1.0f});
        addPlot(plot);
      }

      ImPlot::EndPlot();
    }
  };

  // one extra space for '\0' character and another
  // for one past the last accepted input character
  static char vinBuffer[19];
  auto vinFilter = [](ImGuiInputTextCallbackData *data) -> int {
    const std::regex chars_regex("[A-HJ-NPR-Z0-9]");
    std::string s;
    // get entered char and save it into string
    s += data->EventChar;
    // strlen is updated when entered char passes the filter
    size_t indx = strlen(vinBuffer) + 1;

    if (indx >= 1 && indx <= 17)
      return !std::regex_match(
          s, chars_regex); // return 0 as passed for matched chars
    return 1;              // discard exceeding chars
  };

  // creating the render function -->
  // Only the minimally necessary code should reside directly in this function;
  // everything else should be structured and externalized into .hpp files.
  auto render = [&]() {
    load_settings(config);
    ImGui::SetupImGuiStyle(false, 0.99f);
    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
    ImGui::Begin("OmniScopev2 Data Capture Tool", nullptr,
                 /*ImGuiWindowFlags_NoTitleBar*/ ImGuiWindowFlags_NoDecoration |
                     ImGuiWindowFlags_NoResize /*ImGuiWindowFlags_NoMove*/); //

    // ############################ Menu bar ##############################
    //  main menu -->will be changed to SideBarmenu.hpp --> seperate file

    // --> Initiliazing the language --> seperate file
    std::string analyse_data{"analyse data"};
    std::string create_training_data{"create training data"};
    // replace space chars with new line
    std::replace(analyse_data.begin(), analyse_data.end(), ' ', '\n');
    std::replace(create_training_data.begin(), create_training_data.end(), ' ',
                 '\n');

    // CREATE A SIDEBARMENU

    SetSideBarMenu(language, availableLanguages, config, configpath,
                   open_settings, sampler, devices, deviceManager, captureData,
                   flagPaused, open_generate_training_data, mainMenuBarSize,
                   colorMap);

    /*
if (ImGui::BeginMenu(
        load_json<std::string>(language, "menubar", "view", "label")
            .c_str())) {
  ImGui::EndMenu();
}*/

    // EndSideBarMenu

    // ############################ Live Capture
    // ##############################

    ImGui::SetCursorPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.18f,
                               ImGui::GetIO().DisplaySize.y * 0.06f));

    ImGui::BeginChild("Live Capture",
                      ImVec2(ImGui::GetIO().DisplaySize.x * 0.82f,
                             ImGui::GetIO().DisplaySize.y * 0.65f));
    if (sampler.has_value())
      if (!flagPaused)
        sampler->copyOut(captureData);

    float optimal_buttonstripe_height = toolBtnSize.y * 1.1;
    if (toolBtnSize.y < (ImGui::GetTextLineHeightWithSpacing() * 1.1))
      optimal_buttonstripe_height = ImGui::GetTextLineHeightWithSpacing() * 1.1;

    ImGui::BeginChild("Buttonstripe", ImVec2(-1, optimal_buttonstripe_height),
                      false, ImGuiWindowFlags_NoScrollbar);

    // ############################ Popup Save
    // ##############################
    if (ImGui::BeginPopupModal("Save recorded data", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      saves_popup(config, language, captureData, now, now_time_t, now_tm, path,
                  flagDataNotSaved, devices);

      // ImGui::PopStyleColor();

      ImGui::EndPopup();
    }

    // ############################ Popup Save warning
    // ##############################
    if (ImGui::BeginPopupModal("Save warning", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      ImGui::Text("No devices are available ...");

      if (ImGui::Button("Close"))
        ImGui::CloseCurrentPopup();

      ImGui::EndPopup();
    }

    // ############################ Popup Reset
    // ##############################
    if (ImGui::BeginPopupModal("Reset?", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      ImGui::Text("The measurement was not saved!\n"
                  "Would you like to save it before deleting it?\n");
      if (ImGui::Button("Continue deletion", btnSize)) {
        sampler.reset();
        devices.clear();
        deviceManager.clearDevices();
        captureData.clear();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ImGui::Button("Back", btnSize)) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::SetNextWindowPos(ImVec2(0, 100));
    ImGui::SetNextWindowSize(ImVec2(0, 800));
    if (flagPaused) {
      if (ImGui::BeginPopupModal("Creation of learning data set", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoMove)) {
        ImGui::SetItemDefaultFocus();

        popup_create_training_data_select(config, language, upload_success);
        ImGui::EndPopup();
      }
      if (upload_success == true) {
        ImGui::OpenPopup("upload_success");
      }
      if (ImGui::BeginPopupModal("upload_success", nullptr,
                                 ImGuiWindowFlags_AlwaysAutoResize |
                                     ImGuiWindowFlags_NoSavedSettings |
                                     ImGuiWindowFlags_NoMove)) {
        ImGui::TextUnformatted(
            load_json<std::string>(language, "training", "upload_success")
                .c_str());
        if (ImGui::Button(
                load_json<std::string>(language, "button", "ok").c_str())) {
          ImGui::CloseCurrentPopup();
          upload_success = false;
        }

        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
      }
      // ######################## Buttonstripe
      // ################################

      // Start only if devices are available, otherwise search for devices
      if (!devices.empty()) {
        // ############################ Start Button
        // ##############################
        if (!sampler.has_value()) {
          set_button_style_to(config, "start");
          if (ImGui::Button(
                  load_json<std::string>(language, "button", "start").c_str(),
                  toolBtnSize)) {
            sampler.emplace(deviceManager, devices);
            flagPaused = false;
            flagDataNotSaved = true;
          }
          ImGui::PopStyleColor(3);
        }
      }
      // set_button_style_to(config, "standart");
    } else {
      // ############################ Stop Button
      // ##############################
      set_button_style_to(config, "stop");
      if (ImGui::Button(
              load_json<std::string>(language, "button", "stop").c_str(),
              toolBtnSize))
        flagPaused = true;

      ImGui::PopStyleColor(3);
    }
    if (flagPaused) {
      ImGui::SameLine();

      // Start/reset the measurement when the measurement is paused,
      // followed by a query as to whether the old data should be saved
      if (sampler.has_value()) {
        ImGui::SameLine();
        set_button_style_to(config, "start");
        if (ImGui::Button("Continue", toolBtnSize)) {
          flagPaused = false;
          flagDataNotSaved = true;
        }
        ImGui::PopStyleColor(3);
        ImGui::SameLine();

        set_button_style_to(config, "stop");
        if (ImGui::Button("Reset", toolBtnSize)) {
          if (flagDataNotSaved) {
            ImGui::OpenPopup("Reset?");
          } else {
            sampler.reset();
            devices.clear();
            deviceManager.clearDevices();
            captureData.clear();
            flagPaused = true;
          }
        }
        ImGui::PopStyleColor(3);
      }
      ImGui::SameLine();

      // gray out "Save" button when pop-up is open
      const bool pushStyle = ImGui::IsPopupOpen("Save recorded data");

      if (pushStyle)
        ImGui::PushStyleColor(
            ImGuiCol_Text,
            load_json<Color>(config, "text", "color", "inactive"));
      if (ImGui::Button(
              load_json<std::string>(language, "button", "save").c_str(),
              toolBtnSize)) {
        if (!devices.size())
          ImGui::OpenPopup("Save warning");
        else
          ImGui::OpenPopup("Save recorded data");
      }

      if (pushStyle)
        ImGui::PopStyleColor();

      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));

      ImGui::Button(analyse_data.c_str(), toolBtnSize);
      ImGui::PopStyleColor();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "normal"));
      ImGui::SameLine();

      // ############################ Button create trainings data
      // ##############################
      if (ImGui::Button(create_training_data.c_str(), toolBtnSize)) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(0, 0));
        ImGui::OpenPopup("Creation of learning data set");
      }
      ImGui::PopStyleColor();
    } else {
      ImGui::SameLine();
      ImGui::PushStyleColor(
          ImGuiCol_Text, load_json<Color>(config, "text", "color", "inactive"));
      ImGui::Button(load_json<std::string>(language, "button", "save").c_str(),
                    toolBtnSize);

      ImGui::SameLine();
      ImGui::Button(analyse_data.c_str(), toolBtnSize);
      ImGui::SameLine();
      ImGui::Button(create_training_data.c_str(), toolBtnSize);
      ImGui::PopStyleColor();
    }
    ImGui::EndChild();
    // ############################ Settings Menu
    // ##############################
    std::string settingstitle =
        load_json<std::string>(language, "settings", "title");
    if (open_settings == true) {
      ImGui::OpenPopup(settingstitle.c_str());
      open_settings = false;
    }
    if (ImGui::BeginPopupModal(settingstitle.c_str(), nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      ImGui::SetItemDefaultFocus();
      popup_settings(config, language, configpath);
      ImGui::EndPopup();
    }

    // ############################ Generate training data Menu
    // ##############################
    if (open_generate_training_data) {
      ImGui::OpenPopup("Generate Training Data");
      open_generate_training_data = false;
    }

    if (ImGui::BeginPopupModal("Generate Training Data", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
      static int a = 0;
      ImGui::RadioButton("User current Waveform", &a, 0);
      ImGui::RadioButton("Waveform from File", &a, 1);

      static char ID[10];
      static char milage[10];
      ImGui::SetNextItemWidth(300); // custom width
      ImGui::InputTextWithHint("ID", "Enter ID(optional)", ID,
                               IM_ARRAYSIZE(ID));
      ImGui::SetNextItemWidth(300);
      ImGui::InputTextWithHint("VIN", "Enter VIN", vinBuffer,
                               IM_ARRAYSIZE(vinBuffer),
                               ImGuiInputTextFlags_CharsUppercase |
                                   ImGuiInputTextFlags_CharsNoBlank |
                                   ImGuiInputTextFlags_CallbackCharFilter,
                               // callback function to filter each character
                               // before putting it into the buffer
                               vinFilter);
      ImGui::SetNextItemWidth(300);
      ImGui::InputTextWithHint("milage", "Enter milage", milage,
                               IM_ARRAYSIZE(milage));

      std::string msg{ID};
      // have each entry on a new line
      msg += '\n';
      msg += vinBuffer;
      msg += '\n';
      msg += milage;

      ImGui::SeparatorText("Reason-for-investigation");
      static int b = 0;
      ImGui::RadioButton("Maintenance", &b, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Fault", &b, 1);

      ImGui::SeparatorText("Electrical-Consumers");
      static int c = 0;
      ImGui::RadioButton("Off", &c, 0);
      ImGui::SameLine();
      ImGui::RadioButton("On", &c, 1);

      ImGui::SeparatorText("Assessment");
      static int d = 0;
      ImGui::RadioButton("Normal", &d, 0);
      ImGui::SameLine();
      ImGui::RadioButton("Anomaly", &d, 1);

      static char comment[16];
      ImGui::InputTextMultiline("Comment", comment, IM_ARRAYSIZE(comment),
                                ImVec2(250, 70),
                                ImGuiInputTextFlags_AllowTabInput);

      // VCDS Auto-Scan (file-path or drag&drop)

      if (ImGui::Button("Cancel"))
        ImGui::CloseCurrentPopup();

      ImGui::SameLine();
      if (ImGui::Button(" Send ")) {
        // example url
        const std::string url{
            "https://raw.githubusercontent.com/skunkforce/omniview/"};
        sendData(msg, url);
      }
      ImGui::EndPopup();
    }

    ImGui::EndChild();

    // ############################ addPlots("Recording the data", ...)
    // ##############################
    SetMainWindowStyle();

    ImGui::BeginChild("Live Capture", ImVec2(-1, 615));
    addPlots("Recording the data", captureData,
             [&sampler, &xmax_paused](auto /*x_min*/, auto x_max) {
               if (!flagPaused) {

                 ImPlot::SetupAxes("x [Data points]", "y [ADC Value]",
                                   ImPlotAxisFlags_AutoFit,
                                   ImPlotAxisFlags_AutoFit);
                 ImPlot::SetupAxisLimits(ImAxis_X1, x_max - 7500, x_max + 7500,
                                         ImGuiCond_Always);
               } else {
                 xmax_paused = x_max;
                 ImPlot::SetupAxes("x [Seconds]", "y [Volts]");
                 // x and y axes ranges: [0, 10], [-10, 200]
                 ImPlot::SetupAxesLimits(0, 10, -10, 200);
                 // make specific values/ticks on Y-axis
                 ImPlot::SetupAxisTicks(ImAxis_Y1, -10, 200, 22, nullptr, true);
               }
             });

    ImGui::EndChild();
    ImGui::SetupImGuiStyle(false, 0.99f);

    // Create Devices Menu at the bottom of the programm

    SetDevicesMenu(colorMap, sampler, devices);

    ImGui::SameLine();
    ImGui::End();
    ImGui::PopStyleColor(7);
  };

  ImGuiInstance window{1280, 760,
                       fmt::format("{} {}", CMakeGitVersion::Target::Name,
                                   CMakeGitVersion::Project::Version)};

  while (window.run(render)) {
  }
  return 0;
}