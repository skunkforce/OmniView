#include "popups.hpp"
#include "sendData.hpp"
#include <fstream>
#include <regex>

void generateTrainingData(
    bool &open_generate_training_data,
    const std::vector<std::shared_ptr<OmniscopeDevice>> &devices,
    std::set<std::string> &savedFileNames) {

  const size_t devicesSz{devices.size()};
  ImGui::OpenPopup("Generate Training Data");

  if (ImGui::BeginPopupModal("Generate Training Data", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    // // devices checkboxes buffer
    // static std::vector<BoolWrapper> dvcCheckedArr(devicesSz, false);
    // // Update devices size for new connections during run-time
    // dvcCheckedArr.resize(devicesSz);

    // file names checkboxes buffer
    // static std::vector<BoolWrapper> flnmCheckedArr(savedFileNames.size(),
    //                                                false);
    // // Update files names checked array size for new connections during
    // run-time flnmCheckedArr.resize(savedFileNames.size());

    static bool ucw = false;     // User Current Waveform
    static bool wff = false;     // Waveform From File
    static bool ucwWPop = false; // User Current Waveform Warning Popup

    if (ImGui::RadioButton("User Current Waveform", ucw)) {
      ucw = !ucw;
      wff = false;
    }
    static std::string selectedItem{"Devices & Waveforms Menu"};
    if (ucw && !devicesSz && !savedFileNames.size())
      ucwWPop = true;
    else if (ucw && (devicesSz || savedFileNames.size()) &&
             (ImGui::BeginCombo("##ComboDevice", selectedItem.c_str()))) {

      static int k{-1};
      size_t i{0};
      for (; i < devicesSz; i++) {
        ImGui::PushID(i);
        bool b = (k == i);
        if (ImGui::Checkbox(devices[i]->getId().value().serial.c_str(), &b)) {
          if (b) {
            k = i;
            selectedItem = devices[i]->getId().value().serial.c_str();
          } else {
            k = -1;
            selectedItem = "Devices & Waveforms Menu";
          }
        }
        ImGui::PopID();
      }
      i = 0;
      static const size_t sz{savedFileNames.size()};
      for (auto it = savedFileNames.begin(); it != savedFileNames.end();
           it++, ++i) {
        ImGui::PushID(i + sz);
        bool b = (k == i + sz);
        if (ImGui::Checkbox(it->c_str(), &b)) {
          if (b) {
            k = i + sz;
            selectedItem = it->c_str();
          } else {
            k = -1;
            selectedItem = "Devices & Waveforms Menu";
          }
        }
        ImGui::PopID();
      }
      ImGui::EndCombo();
    }
    if (ucwWPop) {
      warning_popup(ucwWPop, "No waveforms were made  ");
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
        constexpr size_t numberOfFields{3};
        // out of three fields ID, Vin and Mileage, only the last two are
        // required
        std::vector<std::string> FieldsData(numberOfFields, "");

        // read fields data from file
        for (size_t i = 0; i < numberOfFields;) {
          char ch;
          std::string temp{};
          while (file >> std::noskipws >> ch) {
            if (ch == ',' || ch == '\n') {
              FieldsData[i] = temp;
              i++;
              break;
            }
            temp += ch;
          }
        }
        FieldsData[1].copy(VIN, FieldsData[1].size());
        Mileage = FieldsData[2];
      }
    };

    static bool wrongFile = false;
    static bool readOnly = false;
    static std::string fileNameBuf = "";

    auto isWrongFile = [&](const std::filesystem::path &path) {
      if (path.extension() == ".csv") {
        std::string filename = path.filename().string();
        fileNameBuf = filename;
        savedFileNames.insert(filename);
        setVinMileage(path);
        readOnly = true;
        return false;
      }
      return true;
    };

    fileBrowser.Display();
    ImGuiInputTextFlags_ readOnlyFlag{ImGuiInputTextFlags_None};
    if (readOnly)
      readOnlyFlag = ImGuiInputTextFlags_ReadOnly;

    if (fileBrowser.HasSelected()) {
      wrongFile = isWrongFile(fileBrowser.GetSelected().string());
      fileBrowser.ClearSelected();
    }

    if (ImGui::RadioButton("Waveform From File", wff)) {
      wff = !wff;
      ucw = false;
    }
    if (wff) {
      ImGui::SetNextItemWidth(400); // custom width
      ImGui::InputTextWithHint("##inputLabel", ".csv file", &fileNameBuf,
                               readOnlyFlag);
      ImGui::SameLine();
      if (ImGui::Button("Browse"))
        fileBrowser.Open();
    }

    if (wrongFile) {
      warning_popup(wrongFile, "Wrong file type  ");
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
    ImGui::InputTextWithHint("ID", "Set your ID in settings", &ID,
                             ImGuiInputTextFlags_ReadOnly);
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint(
        "VIN", "Enter VIN", VIN, IM_ARRAYSIZE(VIN),
        ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank |
            ImGuiInputTextFlags_CallbackCharFilter | readOnlyFlag,
        // callback function to filter each character
        // before putting it into the buffer
        vinFilter);
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextWithHint("Mileage", "Enter Mileage", &Mileage,
                             readOnlyFlag);

    std::string msg{ID};
    // have each entry on a new line
    msg += ",";
    msg += VIN;
    msg += ",";
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
                              ImVec2(350, 70),
                              ImGuiInputTextFlags_AllowTabInput);

    auto clearStuff = [&]() {
      readOnly = false;
      ID.clear();
      VIN[0] = 0;
      Mileage.clear();
      fileNameBuf.clear();
      ImGui::CloseCurrentPopup();
      open_generate_training_data = false;
      ucw = false;
    };

    if (ImGui::Button("Cancel"))
      clearStuff();

    ImGui::SameLine();
    if (ImGui::Button(" Send ")) {
      // example url
      const std::string url{
          "https://raw.githubusercontent.com/skunkforce/omniview/"};
      sendData(msg, url);
      clearStuff();
    }
    ImGui::EndPopup();
  }
}
