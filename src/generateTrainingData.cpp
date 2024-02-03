#include "popups.hpp"
#include "sendData.hpp"
#include <iostream>
#include <regex>
#include <sstream>

void generateTrainingData(
    bool &open_generate_training_data,
    const std::vector<std::shared_ptr<OmniscopeDevice>> &devices,
    const std::vector<std::string> &savedFileNames) {

  const size_t devicesSz{devices.size()};

  //  one extra space for '\0' character and another
  //  for one past the last accepted input character
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
    static std::vector<BoolWrapper> dvcChackedArr(devicesSz, false);
    dvcChackedArr.resize(devicesSz);

    // file names checkboxes buffer
    static std::vector<BoolWrapper> flnmChackedArr(devicesSz, false);
    flnmChackedArr.resize(devicesSz);

    std::stringstream ss;

    static bool ucw = false;
    static bool wUcwPop = false;
    if (ImGui::RadioButton("User current Waveform", ucw))
      ucw = !ucw;
    if (ucw && !devices.size())
      wUcwPop = true;
    else if (ucw && devices.size()) {
      if (ImGui::BeginCombo("##ComboDevice", "Devices Menu")) {
        for (size_t i = 0; i < devicesSz; i++) {
          ss << devices[i]->getId().value().serial;
          ImGui::Checkbox(ss.str().c_str(), &((dvcChackedArr.begin() + i)->b));
          ss.str(std::string());
        }
        ImGui::EndCombo();
      }
    }
    if (wUcwPop) {
      save_warning_popup(wUcwPop, "No Waveforms were made");
      ucw = false;
    }

    static bool wff = false;
    static bool wWffPop = false;
    if (ImGui::RadioButton("Waveform from File", wff))
      wff = !wff;
    if (wff && !savedFileNames.size())
      wWffPop = true;
    else if (wff && savedFileNames.size()) {
      if (ImGui::BeginCombo("##ComboFile", "Waveforms Menu")) {
        for (size_t i = 0; i < savedFileNames.size(); i++)
          ImGui::Checkbox(savedFileNames[i].c_str(),
                          &((flnmChackedArr.begin() + i)->b));
        ImGui::EndCombo();
      }
    }
    if (wWffPop) {
      save_warning_popup(wWffPop, "No files were saved during measurement");
      wff = false;
    }

    static char ID[10];
    static char milage[10];
    ImGui::SetNextItemWidth(300); // custom width
    ImGui::InputTextWithHint("ID", "Enter ID(optional)", ID, IM_ARRAYSIZE(ID));
    ImGui::SetNextItemWidth(300);
    ImGui::InputTextWithHint(
        "VIN", "Enter VIN", vinBuffer, IM_ARRAYSIZE(vinBuffer),
        ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank |
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
