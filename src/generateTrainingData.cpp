#include "popups.hpp"
#include "sendData.hpp"
#include <fstream>
#include <regex>

void generateTrainingData(bool &open_generate_training_data,
                          const std::optional<OmniscopeSampler> &sampler,
                          std::set<std::string> &savedFileNames) {

  ImGui::OpenPopup("Generate Training Data");
  if (ImGui::BeginPopupModal("Generate Training Data", nullptr,
                             ImGuiWindowFlags_AlwaysAutoResize)) {

    static bool ucw = false; // User Current Waveform
    static bool wff = false; // Waveform From File

    if (ImGui::RadioButton("User Current Waveform", ucw)) {
      ucw = !ucw;
      wff = false;
    }
    static std::string selectedItem{"Devices & Waveforms Menu"};

    if (ucw && !sampler.has_value() && !savedFileNames.size() //&&
        /*!ImGui::IsPopupOpen("Waveforms warning")*/) {
      ImGui::OpenPopup("Waveforms warning",
                       ImGuiPopupFlags_NoOpenOverExistingPopup);
      ucw = false;
    }
    warning_popup("Waveforms warning", "No waveforms were made ");

    if (ucw && (sampler.has_value() || savedFileNames.size()) &&
        (ImGui::BeginCombo("##ComboDevice", selectedItem.c_str()))) {

      static int k{-1};
      size_t i{0};
      bool b{false};
      if (sampler.has_value())
        for (const auto &device : sampler->sampleDevices) {
          b = (k == i);
          if (ImGui::Checkbox(device.first->getId().value().serial.c_str(),
                              &b)) {
            if (b) {
              k = i;
              selectedItem = device.first->getId().value().serial.c_str();
            } else {
              k = -1;
              selectedItem = "Devices & Waveforms Menu";
            }
          }
          i++;
        }
      i = 0;
      if (const size_t sz = savedFileNames.size())
        for (const auto &file : savedFileNames) {
          b = (k == i + sz);
          if (ImGui::Checkbox(file.c_str(), &b)) {
            if (b) {
              k = i + sz;
              selectedItem = file.c_str();
            } else {
              k = -1;
              selectedItem = "Devices & Waveforms Menu";
            }
          }
          ++i;
        }
      ImGui::EndCombo();
    }

    // static ImGui::FileBrowser fileBrowser;
    //  set browser properties
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
        // out of ID, Vin and Mileage, only the last two are required
        std::vector<std::string> FieldsData(numberOfFields);

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

    static std::string fileNameBuf;
    static bool grayFields = false;
    static auto readOnlyFlag{ImGuiInputTextFlags_None};

    auto isWrongFile = [&](const std::filesystem::path &path) {
      if (path.extension() == ".csv") {
        fileNameBuf = path.filename().string();
        savedFileNames.insert(fileNameBuf);
        setVinMileage(path);
        readOnlyFlag = ImGuiInputTextFlags_ReadOnly;
        grayFields = true;
        return false;
      }
      return true;
    };
    fileBrowser.Display();

    if (fileBrowser.HasSelected()) {
      if (isWrongFile(fileBrowser.GetSelected().string()) // &&
          /* !ImGui::IsPopupOpen("Wrong file warning")*/) {
        ImGui::OpenPopup("Wrong file warning",
                         ImGuiPopupFlags_NoOpenOverExistingPopup);
        wff = false;
      }
      fileBrowser.ClearSelected();
    }
    warning_popup("Wrong file warning", "Wrong file type! Try again");

    if (ImGui::RadioButton("Waveform From File", wff)) {
      wff = !wff;
      ucw = false;
    }
    if (wff) {
      if (grayFields) // grey color style
        ImGui::PushStyleColor(ImGuiCol_Text, {.5, .5, .5, 1});

      ImGui::SetNextItemWidth(400); // custom width
      ImGui::InputTextWithHint("##inputLabel", ".csv file", &fileNameBuf,
                               readOnlyFlag);
      if (grayFields)
        ImGui::PopStyleColor(); // remove grey color style

      ImGui::SameLine();
      if (ImGui::Button("Browse"))
        fileBrowser.Open();
    }
    auto vinFilter = [](ImGuiInputTextCallbackData *data) -> int {
      const std::regex chars_regex("[A-HJ-NPR-Z0-9]");
      std::string s;
      // get entered char and save it into string
      s += data->EventChar;
      // strlen is updated when entered char passes the filter
      size_t indx = strlen(VIN);

      if (indx < 17)
        return !std::regex_match(
            s, chars_regex); // return 0 as passed for matched chars
      return 1;              // discard exceeding chars
    };

    static std::string ID;
    ImGui::SetNextItemWidth(400);

    if (grayFields) // greyed out color style
      ImGui::PushStyleColor(ImGuiCol_Text, {.5, .5, .5, 1});

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
    if (grayFields)
      ImGui::PopStyleColor(); // remove grey color style

    std::string msg{ID}; // data to be saved in .csv file
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

    static std::string comment;
    ImGui::InputTextMultiline("Comment", &comment, ImVec2(350, 70),
                              ImGuiInputTextFlags_AllowTabInput);

    auto clearSettings = [&]() {
      ucw = false;
      wff = false;
      ID.clear();
      VIN[0] = 0;
      Mileage.clear();
      fileNameBuf.clear();
      b = c = d = 0;
      comment.clear();
      grayFields = false;
      readOnlyFlag = ImGuiInputTextFlags_None;
      open_generate_training_data = false;
      ImGui::CloseCurrentPopup();
    };

    if (ImGui::Button("Cancel"))
      clearSettings();

    ImGui::SameLine();
    if (ImGui::Button(" Send ")) {
      // example url
      const std::string url{
          "https://raw.githubusercontent.com/skunkforce/omniview/"};
      sendData(msg, url);
      clearSettings();
    }
    ImGui::EndPopup();
  }
}
