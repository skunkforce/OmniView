#pragma once

#include "jasonhandler.hpp"
#include <curl/curl.h>
#include <imgui.h>
#include <nlohmann/json.hpp>
#include <nlohmann/json_fwd.hpp>

inline std::string send_to_api(nlohmann::json const &config, std::string const &file,
                        std::string const &vin, std::string const &scantype) {
  std::string api_message = "empty";
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    std::string url =
        load_json<std::string>(config, "api", "url") + "?" +
        load_json<std::string>(config, "api", "input1") + "=" + vin + "&" +
        load_json<std::string>(config, "api", "input2") + "=" + scantype;

    // Hinzufügen der Daten als Formfelder
    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    // Datei

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, file.c_str());

    struct curl_slist *header_list = NULL;
    header_list = curl_slist_append(
        header_list, load_json<std::string>(config, "api", "header1").c_str());
    header_list = curl_slist_append(
        header_list, load_json<std::string>(config, "api", "header2").c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    // Senden der Anfrage

    res = curl_easy_perform(curl);

    // Überprüfung auf Fehler

    if (res != CURLE_OK) {
      api_message =
          fmt::format("Fehler beim Hochladen der Datei:{}\n\r Fehler:{} \n\r ",
                      file, curl_easy_strerror(res));
    } else {
      api_message =
          fmt::format("Hochladen der Datei:{}\n\r erfolgreich \n\r ", file);
    }
    // Aufräumen
    curl_easy_cleanup(curl);
    curl_slist_free_all(header_list);
  } else {
    // implement failed curl routine
  }

  // Aufräumen von cURL

  curl_global_cleanup();

  return api_message;
}

inline std::string send_to_api(nlohmann::json const &config, std::string const &file,
                        std::string const &vin, std::string const &scantype,
                        nlohmann::json const &additionalData) {
  std::string api_message = "empty";
  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    std::string url =
        load_json<std::string>(config, "api", "url") + "?" +
        load_json<std::string>(config, "api", "input1") + "=" + vin + "&" +
        load_json<std::string>(config, "api", "input2") + "=" + scantype;

    // Hinzufügen der Daten als Formfelder
    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    // Datei
    part = curl_mime_addpart(mime);
    curl_mime_name(part, "file");
    curl_mime_filedata(part, file.c_str());

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "additional_metadata");
    curl_mime_data(part, additionalData.dump().c_str(), CURL_ZERO_TERMINATED);

    struct curl_slist *header_list = NULL;
    header_list = curl_slist_append(
        header_list, load_json<std::string>(config, "api", "header1").c_str());
    header_list = curl_slist_append(
        header_list, load_json<std::string>(config, "api", "header2").c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    // Senden der Anfrage

    res = curl_easy_perform(curl);

    // Überprüfung auf Fehler

    if (res != CURLE_OK) {
      api_message =
          fmt::format("Fehler beim Hochladen der Datei:{}\n\r Fehler:{} \n\r ",
                      file, curl_easy_strerror(res));
    } else {
      api_message =
          fmt::format("Hochladen der Datei:{}\n\r erfolgreich \n\r ", file);
    }
    // Aufräumen
    curl_easy_cleanup(curl);
    curl_slist_free_all(header_list);
  } else {
    // implement failed curl routine
  }

  // Aufräumen von cURL

  curl_global_cleanup();

  return api_message;
}
