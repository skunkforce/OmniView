// sendData.hpp

#ifndef SENDDATA_HEADER_HPP
#define SENDDATA_HEADER_HPP

#include "jasonhandler.hpp"
#include <curl/curl.h>
#include <fmt/format.h>

inline std::string sendData(nlohmann::json const &config,
                            const nlohmann::json &myJson) {
  std::string api_message = "empty";

  CURL *curl;
  CURLcode res;

  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();

  if (curl) {
    std::string url = load_json<std::string>(config, "api", "url");

    curl_mime *mime = curl_mime_init(curl);
    curl_mimepart *part;

    part = curl_mime_addpart(mime);
    curl_mime_name(part, "measured_data");
    curl_mime_data(part, myJson.dump().c_str(), CURL_ZERO_TERMINATED);

    struct curl_slist *header_list = NULL;
    header_list = curl_slist_append(
        header_list, load_json<std::string>(config, "api", "header1").c_str());
    header_list = curl_slist_append(
        header_list, load_json<std::string>(config, "api", "header2").c_str());

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, header_list);
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);

    // Sending the request
    res = curl_easy_perform(curl);

    // Check for errors
    if (res != CURLE_OK)
      api_message = appLanguage[Key::Upload_failure];
    else
      api_message = appLanguage[Key::Upload_success];

    // Clean up
    curl_easy_cleanup(curl);
    curl_slist_free_all(header_list);
  } else {
    fmt::println("cURL initialization failed! ");
  }

  // Cleaning up cURL
  curl_global_cleanup();

  return api_message;
}

#endif