#pragma once
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

bool update_yourself_from_github() {
  std::filesystem::create_directories("config");
  std::filesystem::create_directories("languages");

  bool downloaded_config = downloadFileFromGitHub(
      "https://github.com/skunkforce/omniview/blob/master/config/config.json",
      "config/config.json");
  bool dowloaded_langfiles =
      downloadFileFromGitHub("https://github.com/skunkforce/omniview/blob/"
                             "master/languages/Deutsch.json",
                             "languages/Deutsch.json");
}
