#include "popups.hpp"
#include "sendData.hpp"
#include <algorithm>
#include <fstream>
#include <ranges>
#include <regex>

void generateTrainingData(
    bool &open_generate_training_data,
    const std::vector<std::shared_ptr<OmniscopeDevice>> &devices,
    const std::vector<std::string> &savedFileNames) {

  const size_t devicesSz{devices.size()};

  ImGui::OpenPopup("Generate Training Data");

  if (ImGui::BeginPopupModal("Generate Training Data", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    // Have address of bool for std::vector
    struct BoolWrapper {
      BoolWrapper() : b(false) {}
      BoolWrapper(bool _b) : b(_b) {}
      bool b;
    };

    // devices checkboxes buffer
    static std::vector<BoolWrapper> dvcCheckedArr(devicesSz, false);
    // Update devices size for new connections during run-time
    dvcCheckedArr.resize(devicesSz);

    // file names checkboxes buffer
    static std::vector<BoolWrapper> flnmCheckedArr(devicesSz, false);
    // Update files names checked array size for new connections during run-time
    flnmCheckedArr.resize(devicesSz);

    // User Current Waveform
    static bool ucw = false;
    // User Current Waveform Warning Popup
    static bool ucwWPop = false;

    if (ImGui::RadioButton("User Current Waveform", ucw))
      ucw = !ucw;
    if (ucw && !devicesSz)
      ucwWPop = true;
    else if (ucw && devicesSz) {
      if (devicesSz != savedFileNames.size())
        fmt::println("size of devices and waveforms don't match!");
      else if (ImGui::BeginCombo("##ComboDevice", "Devices & Waveforms Menu")) {
        for (size_t i = 0; i < devicesSz; i++) {
          ImGui::Checkbox(devices[i]->getId().value().serial.c_str(),
                          &(dvcCheckedArr[i].b));
          ImGui::Checkbox(savedFileNames[i].c_str(), &(flnmCheckedArr[i].b));
        }
        ImGui::EndCombo();
      }
    }
    if (ucwWPop) {
      warning_popup(ucwWPop, "No waveforms were made");
      ucw = false;
    }

    static ImGui::FileBrowser fileBrowser;
    // set browser properties
    fileBrowser.SetTitle("Searching for .csv files");
    // fileBrowser.SetTypeFilters({".csv"});

    // one extra space for '\0' character
    static char VIN[18];
    static std::string Mileage = "";
    auto setVinMileage = [&](const std::filesystem::path &filename) {
      std::ifstream file(filename, std::ios::binary);
      if (!file.is_open())
        fmt::println("Failed to open file {}", filename);
      else {
        // only vin/fin and mileage are required here
        std::string measurStr{}, vinStr{};
        file >> measurStr >> vinStr >> Mileage;
        vinStr.erase(std::next(vinStr.end(), -1)); // remove the last comma

        // convert letters to uppercase
        std::ranges::transform(vinStr, vinStr.begin(), [](unsigned char ch) {
          return std::toupper(ch);
        });
        vinStr.copy(VIN, vinStr.size());
      }
    };

    // Waveform From File
    static bool wff = false;
    static bool wrongFile = false;
    static std::string fileNameBuf = "";

    auto isWrongFile = [setVinMileage](const std::filesystem::path &path) {
      if (path.extension() == ".csv") {
        fileNameBuf = path.filename().string();
        setVinMileage(path);
        return false;
      }
      return true;
    };

    fileBrowser.Display();

    if (fileBrowser.HasSelected()) {
      wrongFile = isWrongFile(fileBrowser.GetSelected().string());
      fileBrowser.ClearSelected();
    }

    if (ImGui::RadioButton("Waveform From File", wff))
      wff = !wff;
    if (wff) {
      ImGui::SetNextItemWidth(400); // custom width
      ImGui::InputTextWithHint("##inputLabel", ".csv file", &fileNameBuf);
      ImGui::SameLine();
      if (ImGui::Button("Browse"))
        fileBrowser.Open();
    }

    if (wrongFile) {
      warning_popup(wrongFile, "Wrong file type");
      wff = false;
    }

    auto vinFilter = [](ImGuiInputTextCallbackData *data) -> int {
      const std::regex chars_regex("[A-HJ-NPR-Z0-9]");
      std::string s;
      // get entered char and save it into string
      s += data->EventChar;
      // strlen is updated when entered char passes the filter
      size_t indx = strlen(VIN);

      if (indx >= 0 && indx < 17)
        return !std::regex_match(
            s, chars_regex); // return 0 as passed for matched chars
      return 1;              // discard exceeding chars
    };

    static std::string ID = "";
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("ID", "Set your ID in settings", &ID);
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("VIN", "Enter VIN", VIN, IM_ARRAYSIZE(VIN),
                             ImGuiInputTextFlags_CharsUppercase |
                                 ImGuiInputTextFlags_CharsNoBlank |
                                 ImGuiInputTextFlags_CallbackCharFilter,
                             // callback function to filter each character
                             // before putting it into the buffer
                             vinFilter);
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("Mileage", "Enter Mileage", &Mileage);

    std::string msg{ID};
    // have each entry on a new line
    msg += '\n';
    msg += VIN;
    msg += '\n';
    msg += Mileage;

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

    if (ImGui::Button("Cancel")) {
      ImGui::CloseCurrentPopup();
      open_generate_training_data = false;
    }

    ImGui::SameLine();
    if (ImGui::Button(" Send ")) {
      // example url
      const std::string url{
          "https://raw.githubusercontent.com/skunkforce/omniview/"};
      sendData(msg, url);
      ImGui::CloseCurrentPopup();
      open_generate_training_data = false;
    }
    ImGui::EndPopup();
  }
}
