#pragma once
#include <iostream>
#include <string>
#include <nlohmann/json.hpp>
#include <curl/curl.h>

using json = nlohmann::json;

class IpToCountryResolver
{
public:
    static std::string Resolve(const uint32_t& ipAddress) {
        if (!ipAddress)
            return "";

        const std::string ipAddressStr = std::to_string((ipAddress >> 24) & 0xFF) + "." +
            std::to_string((ipAddress >> 16) & 0xFF) + "." +
            std::to_string((ipAddress >> 8) & 0xFF) + "." +
            std::to_string(ipAddress & 0xFF);

        if (ipAddressStr.empty())
            return "";

        const std::string url = "http://ip-api.com/json/" + ipAddressStr;

        CURL* curl = curl_easy_init();
        if (!curl)
            return "";

        std::string response;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 2000L); // Set 2-second timeout

        try {
            CURLcode res = curl_easy_perform(curl);
            if (res != CURLE_OK) {
                curl_easy_cleanup(curl);
                return "";
            }
        }
        catch (...) {
            curl_easy_cleanup(curl);
            return "";
        }

        curl_easy_cleanup(curl);

        json json_response = json::parse(response);
        return json_response.value("country", "");
    }

private:
    static size_t writeCallback(void* contents, size_t size, size_t nmemb, void* userp) {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
};