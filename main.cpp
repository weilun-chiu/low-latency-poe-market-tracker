#include <iostream>
#include <string>
#include "items.h"
#include <curl/curl.h>

extern unsigned char items_json[];
extern unsigned int items_json_len;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

int main() {
    CURL* curl = curl_easy_init();
    if (curl) {
        // Set the headers
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        // API won't accept the request without the User-Agent
        headers = curl_slist_append(headers, "User-Agent: low-latency-poe-market-tracker/1.0");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        std::string json_data(reinterpret_cast<char*>(items_json), items_json_len);
        std::cout << json_data << std::endl;
        std::string response;

        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, "https://www.pathofexile.com/api/trade/search/Standard");

        // Set the POST data
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data.c_str());

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);

        // Display the response
        std::cout << "Response: " << response << std::endl;
    }

    return 0;
}