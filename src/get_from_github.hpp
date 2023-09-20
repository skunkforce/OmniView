#pragma once
#include "jasonhandler.hpp"
#include <curl/curl.h>
#include <fmt/format.h>
#include <fstream>
#include <iostream>

bool downloadFileFromGitHub(const std::string &githubUrl,
                            const std::string &localPath) {
  // Initialisieren Sie die libcurl-Bibliothek
  CURL *curl = curl_easy_init();

  if (curl) {
    // Setzen Sie die URL
    curl_easy_setopt(curl, CURLOPT_URL, githubUrl.c_str());

    // Setzen Sie die Schreibfunktion, um die heruntergeladene Datei zu
    // speichern
    FILE *file = fopen(localPath.c_str(), "wb");

    if (file) {
      curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);

      // Führen Sie die Anfrage aus
      CURLcode res = curl_easy_perform(curl);

      // Überprüfen Sie den Anfragestatus
      if (res != CURLE_OK) {
        fmt::print("Fehler beim Herunterladen der Datei: ");
        fclose(file);
        curl_easy_cleanup(curl);
        return false;
      }

      // Datei schließen
      fclose(file);

      // Rückgabe von true bei erfolgreichem Download
      curl_easy_cleanup(curl);
      return true;
    } else {
      fmt::print("Fehler beim Öffnen der lokalen Datei zum Schreiben.");

      curl_easy_cleanup(curl);
      return false;
    }
  }

  // Rückgabe von false bei einem Fehler
  return false;
}

bool update_config_from_github() {

  std::filesystem::create_directories("config");

  const std::string config_url = "https://raw.githubusercontent.com/skunkforce/"
                                 "omniview/master/config/config.json";
  fmt::print("download config file from: {} \n\r", config_url);
  bool downloaded_config =
      downloadFileFromGitHub(config_url, "config/config.json");

  if (downloaded_config) {
    fmt::print("download succeeded\n\r");
  } else {
    fmt::print("download failed\n\r");
  }
}
bool update_language_from_github() {

  std::filesystem::create_directories("languages");

  const std::string lang_url = "https://raw.githubusercontent.com/skunkforce/"
                               "omniview/master/languages/Deutsch.json";
  fmt::print("download language file from: {} \n\r", lang_url);
  bool downloaded_langfiles =
      downloadFileFromGitHub(lang_url, "languages/Deutsch.json");

  if (downloaded_langfiles) {
    fmt::print("download succeeded\n\r");
  } else {
    fmt::print("download failed\n\r");
  }
}