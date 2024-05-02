// languages.hpp

#ifndef LANGUAGES_HEADER_HPP
#define LANGUAGES_HEADER_HPP

#include <map>

enum class Key {
  Known_Car,
  New_Car,
  Additional_Information,
  Devices_Menu,
  Select_Devices,
  Save_Recorded_Data,
  Select_Storage_Location,
  Browse,
  Menu,
  LanOption,
  English,
  Settings,
  Reset,
  Reset_q,
  Measure_not_saved,
  Version,
  Diagnostics,
  Compression,
  Anlyz_crnt_waveform,
  Gnrt_trning_data,
  Timing_Belt,
  Fuel_Delivery_Pump,
  Common_Rail_Pressure,
  Help,
  HelpLink,
  Dvc_search,
  Save,
  Save_warning,
  No_dvc_available,
  AnalyzeData,
  Crt_trng_data,
  Back,
  Start,
  Stop,
  Continue,
  Continue_del,
  Devices_found,
  Device_List,
  Usr_curnt_wave,
  Wv_from_file,
  WvForms_warning,
  No_wave_made,
  Wrong_file_warning,
  Wrong_file_type,
  Gn_trng_data_pop,
  Data_upload,
  Upload_success,
  Upload_failure,
  OK,
  Send,
  Nothing_to_send,
  No_slct_to_upld,
  Maintenance,
  On,
  Off,
  Invst_reason,
  Fault,
  Electr_consumer,
  Assessment,
  Anomaly,
  Comment,
  Csv_file,
  Set_id_in_setting,
  Measurement,
  Enter_measurement,
  Enter_vin,
  Mileage,
  Enter_mileage,
  Battery_measure,
  Attitude,
  x_axis_label,
  y_axis_label,
  Record_Data,
  German
};

inline const std::map<Key, const char *> englishLan{
    {Key::Known_Car, "Known Cars"},
    {Key::New_Car, "New_Car"},
    {Key::Additional_Information, "Additional_Information"},
    {Key::Devices_Menu, "Devices Menu"},
    {Key::Select_Devices, "Select Devices"},
    {Key::Save_Recorded_Data, "Save recorded Data"},
    {Key::Select_Storage_Location, "Select the storage location"},
    {Key::Browse, "Browse"},
    {Key::Menu, "Menu"},
    {Key::LanOption, "Language Option"},
    {Key::English, "English"},
    {Key::Settings, "Settings"},
    {Key::Reset, "Reset"},
    {Key::Reset_q, "Reset?"},
    {Key::Measure_not_saved, "The measurement was not saved!\n"
                             "Would you like to save it before deleting it?\n"},
    {Key::Version, "Version"},
    {Key::Diagnostics, "Diagnostics"},
    {Key::Compression, "Compression"},
    {Key::Anlyz_crnt_waveform, "Analyze current waveform"},
    {Key::Gnrt_trning_data, "Generate training data"},
    {Key::Timing_Belt, "Timing-Belt"},
    {Key::Fuel_Delivery_Pump, "Fuel-Delivery-Pump"},
    {Key::Common_Rail_Pressure, "Common-Rail-Pressure"},
    {Key::Help, "Help"},
    {Key::HelpLink, "Click on the button to go to the help page"},
    {Key::Dvc_search, "Search for Devices"},
    {Key::Save, "Save"},
    {Key::Save_warning, "Save warning! "},
    {Key::No_dvc_available, "No devices are available ..."},
    {Key::AnalyzeData, "Analyze\nData"},
    {Key::Crt_trng_data, "Create\nTraining\nData"},
    {Key::Back, "Back"},
    {Key::Start, "Start"},
    {Key::Stop, "Stop"},
    {Key::Continue, "Continue"},
    {Key::Continue_del, "Continue Deletion?"},
    {Key::Devices_found, "Devices found"},
    {Key::Device_List, "Devicelist"},
    {Key::Usr_curnt_wave, "User Current Waveform"},
    {Key::Wv_from_file, "Waveform From File"},
    {Key::WvForms_warning, "Waveforms Warning! "},
    {Key::No_wave_made, "No waveforms were made! "},
    {Key::Wrong_file_warning, "Wrong file warning! "},
    {Key::Wrong_file_type, "Wrong file type! Try again!"},
    {Key::Gn_trng_data_pop, "Generate Training Data"},
    {Key::Data_upload, "Uploading data"},
    {Key::Upload_success, "File uploaded successfully!"},
    {Key::Upload_failure, "File upload failed!"},
    {Key::OK, "OK"},
    {Key::Send, "Send"},
    {Key::Nothing_to_send, "Nothing to send."},
    {Key::No_slct_to_upld, "Nothing is selected to upload!"},
    {Key::Maintenance, "Maintenance"},
    {Key::On, "On"},
    {Key::Off, "Off"},
    {Key::Invst_reason, "Reason-for-investigation"},
    {Key::Fault, "Fault"},
    {Key::Electr_consumer, "Electrical-Consumers"},
    {Key::Assessment, "Assessment"},
    {Key::Anomaly, "Anomaly"},
    {Key::Comment, "Comment"},
    {Key::Csv_file, ".csv file"},
    {Key::Set_id_in_setting, "Set your ID in settings"},
    {Key::Measurement, "Measurement"},
    {Key::Enter_measurement, "Enter Measurement"},
    {Key::Enter_vin, "Enter VIN"},
    {Key::Mileage, "Mileage"},
    {Key::Enter_mileage, "Enter Mileage"},
    {Key::Battery_measure, "Battery measurement"},
    {Key::Attitude, "Attitude"},
    {Key::x_axis_label, "x [Seconds]"},
    {Key::y_axis_label, "y [Volts]"},
    {Key::Record_Data, "Recording the data"},
    {Key::German, "German"}};

inline const std::map<Key, const char *> germanLan{
    {Key::Device_List, "Geräteliste"},
    {Key::Known_Car, "Fahrzeugauswahl"},
    {Key::New_Car, "Neues Fahrzeug"},
    {Key::Additional_Information, "Weiterführende Information"},
    {Key::Devices_Menu, "Geräteliste"},
    {Key::Select_Devices, "Geräteauswahl"},
    {Key::Save_Recorded_Data, "Speichern der aufgenommenen Daten"},
    {Key::Select_Storage_Location, "Wählen Sie den Speicherort"},
    {Key::Browse, "Durchsuchen"},
    {Key::Menu, "Menü"},
    {Key::LanOption, "Sprachoption"},
    {Key::English, "Englisch"},
    {Key::Settings, "Einstellungen"},
    {Key::Reset, "Zurücksetzen"},
    {Key::Reset_q, "Zurücksetzen?"},
    {Key::Measure_not_saved, "Die Messung wurde nicht gespeichert!\n"
                  "Möchten Sie es speichern, bevor Sie es löschen?\n"},
    {Key::Version, "Ausführung"},
    {Key::Diagnostics, "Diagnose"},
    {Key::Compression, "Kompression"},
    {Key::Anlyz_crnt_waveform, "Aktuelle Wellenform analysieren"},
    {Key::Gnrt_trning_data, "Trainingsdaten generieren"},
    {Key::Timing_Belt, "Zahnriemen"},
    {Key::Fuel_Delivery_Pump, "Kraftstoff-Förderpumpe"},
    {Key::Common_Rail_Pressure, "Common-Rail-Druck"},
    {Key::Help, "Hilfe"},
    {Key::HelpLink, "Klicken Sie auf den Button um zur Hilfeseite zu gelangen"},
    {Key::Dvc_search, "Suche Geräte"},
    {Key::Save, "Speichern"},
    {Key::Save_warning, "Speicherwarnung!"},
    {Key::No_dvc_available, "Es sind keine Geräte verfügbar ..."},
    {Key::AnalyzeData, "Daten\nAnalysieren"},
    {Key::Crt_trng_data, "Erstellen\nSie\nTrainingsdaten"},
    {Key::Back, "Zurück"},
    {Key::Start, "Start"},
    {Key::Stop, "Stoppen"},
    {Key::Continue, "Weiter"},
    {Key::Continue_del, "Mit dem Löschen fortfahren?"},
    {Key::Devices_found, "Geräte"},
    {Key::Usr_curnt_wave, "Aktuelle Wellenform Des Benutzers"},
    {Key::Wv_from_file, "Wellenform Aus Datei"},
    {Key::WvForms_warning, "Warnung vor Wellenformen!"},
    {Key::No_wave_made, "Es wurden keine Wellenformen erstellt!"},
    {Key::Wrong_file_warning, "Warnung vor falscher Datei!"},
    {Key::Wrong_file_type, "Falscher Dateityp! Versuchen Sie es erneut!"},
    {Key::Gn_trng_data_pop, "Trainingsdaten Generieren"},
    {Key::Data_upload, "Daten hochladen"},
    {Key::Upload_success, "Datei erfolgreich hochgeladen! "},
    {Key::Upload_failure, "Das Hochladen der Datei ist fehlgeschlagen!"},
    {Key::OK, "OK"},
    {Key::Send, "Senden"},
    {Key::Nothing_to_send, "Nichts zu senden"},
    {Key::No_slct_to_upld, "Es ist nichts zum Hochladen ausgewählt!"},
    {Key::Maintenance, "Wartung"},
    {Key::On, "Ein"},
    {Key::Off, "Aus"},
    {Key::Invst_reason, "Grund-für-die-Untersuchung"},
    {Key::Fault, "Fehler"},
    {Key::Electr_consumer, "Elektrische-Verbraucher"},
    {Key::Assessment, "Bewertung"},
    {Key::Anomaly, "Anomalie"},
    {Key::Comment, "Kommentar"},
    {Key::Csv_file, ".csv Datei"},
    {Key::Set_id_in_setting, "Legen Sie Ihre ID in den Einstellungen fest"},
    {Key::Measurement, "Messung"},
    {Key::Enter_measurement, "Geben Sie die Messung ein"},
    {Key::Enter_vin, "Geben Sie VIN ein"},
    {Key::Mileage, "Kilometerstand"},
    {Key::Enter_mileage, "Geben Sie den Kilometerstand ein"},
    {Key::Battery_measure, "Batteriemessung"},
    {Key::Attitude, "Einstellung"},
    {Key::x_axis_label, "x [Sekunden]"},
    {Key::y_axis_label, "y [Volt]"},
    {Key::Record_Data, "Datenaufnahme"},
    {Key::German, "Deutsch"}};

inline auto appLanguage = englishLan;
#endif