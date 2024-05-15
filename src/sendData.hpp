// sendData.hpp
#ifndef SENDDATA_HEADER_HPP
#define SENDDATA_HEADER_HPP

#include <curl/curl.h>
#include <fmt/format.h>

inline size_t cb_write(void *contents, size_t size, size_t nmemb,
                       void *stream) {
  ((std::string *)stream)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

inline std::string sendData(const std::string &data) {
  std::string api_message = "empty";
  curl_global_init(CURL_GLOBAL_ALL); // init the winsock stuff (Windows)
  CURL *curl = curl_easy_init();  // get a curl handle

  if (curl) {
    curl_easy_setopt(curl, CURLOPT_URL, "https://api.auto-intern.de/analyze");

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    if (!headers) {
      fmt::println("Failed to append the string!");
      return api_message;
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers); 
    // specify the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    // save the response from the server into a string
    std::string response;
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, cb_write);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    // Perform the request and check the returned status
    if (curl_easy_perform(curl) != CURLE_OK)
      fmt::println("{}", appLanguage[Key::Upload_failure]);
    else 
      api_message = response;

    curl_slist_free_all(headers); // free the list again
    curl_easy_cleanup(curl);      // always cleanup
  } else
    fmt::println("Failed to get a curl handle.");

  curl_global_cleanup();
  return api_message;
}
#endif