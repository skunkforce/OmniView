// sendData.hpp
#ifndef SENDDATA_HEADER_HPP
#define SENDDATA_HEADER_HPP

#include <curl/curl.h>
#include <fmt/format.h>

inline std::string sendData(const std::string &data) {
  std::string api_message = "empty";

  // init the winsock stuff (Windows)
  curl_global_init(CURL_GLOBAL_ALL);

  // get a curl handle
  CURL *curl = curl_easy_init();

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL,
                     "https://api.auto-intern.de/check-request");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!headers) {
      fmt::println("Failed to append the string!");
      return api_message;
    }

    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // specify the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    // Perform the request, res will get the return code
    CURLcode res = curl_easy_perform(curl);

    // Check the request status
    if (res != CURLE_OK)
      api_message = appLanguage[Key::Upload_failure];
    else
      api_message = appLanguage[Key::Upload_success];
    // always cleanup
    curl_easy_cleanup(curl);
    curl_slist_free_all(headers); // free the list again
  } else
    fmt::println("Failed to get a curl handle.");

  curl_global_cleanup();
  return api_message;
}
#endif