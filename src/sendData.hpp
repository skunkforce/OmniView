#pragma once

#include <curl/curl.h>
#include <fmt/format.h>
#include <iostream>

inline bool sendData(const std::string& msg, const std::string& url) {
    CURL* curl;

    // init the winsock stuff (Windows) 
    curl_global_init(CURL_GLOBAL_ALL);

    // get a curl handle 
    curl = curl_easy_init();

    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // specify the POST data 
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, msg.c_str());

        // Perform the request, res will get the return code 
        CURLcode res = curl_easy_perform(curl);

        // Check the request status
        if (res != CURLE_OK) {
            std::cout << "Failed to upload data.\n";
            curl_global_cleanup();
            curl_easy_cleanup(curl);
            return false;
        }

        // cleanup
        curl_easy_cleanup(curl);
        curl_global_cleanup();
        std::cout << "Success!\n"; 
        return true;
    }

    std::cout << "Failed to get a curl handle.\n";
    return false;
}