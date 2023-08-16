#include <iostream>
#include <string>
#include "items.h"
#include <curl/curl.h>
#include <vector>
#include <numeric>

extern unsigned char items_json[];
extern unsigned int items_json_len;

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

// Helper function to trim whitespace from both ends of a string
std::string trim(const std::string &str) {
    size_t start = str.find_first_not_of(" \t\r\n\"");
    size_t end = str.find_last_not_of(" \t\r\n\"");
    if (start == std::string::npos || end == std::string::npos)
        return "";
    return str.substr(start, end - start + 1);
}

std::string findFieldValue(const std::string& jsonString, const std::string& field) {
    std::string result;
    
    size_t pos = jsonString.find("\"" + field + "\":\"");
    if (pos != std::string::npos) {
        pos += field.length() + 4; // Move the position to the beginning of the value
        
        size_t endPos = jsonString.find("\"", pos);
        if (endPos != std::string::npos) {
            result = jsonString.substr(pos, endPos - pos);
        }
    }
    
    return result;
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
        // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);


        // Perform the request
        CURLcode res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Display the response
        std::cout << "Response: " << response << std::endl;

        // get id
        // get first 5 results, once you get a id, send a request to ask data
        // url: https://www.pathofexile.com/api/trade/fetch/RESULT_LINES_HERE?query=ID_HERE
        std::string id, resultPart;
        std::vector<std::string> resultArray;

        size_t idPos = response.find("\"id\":\"");
        if (idPos != std::string::npos) {
            size_t idStart = idPos + 6;
            size_t idEnd = response.find("\"", idStart);
            id = response.substr(idStart, idEnd - idStart);
        }

        size_t resultPos = response.find("\"result\":[");
        if (resultPos != std::string::npos) {
            size_t resultStart = resultPos + 10;
            size_t resultEnd = response.find("]", resultStart);
            resultPart = response.substr(resultStart, resultEnd - resultStart);
        }

        // Split resultPart into individual elements
        size_t i = 0;
        while (!resultPart.empty()) {
            size_t commaPos = resultPart.find(",");
            if (commaPos != std::string::npos) {
                std::string element = trim(resultPart.substr(0, commaPos));
                resultArray.push_back(element);
                resultPart = resultPart.substr(commaPos + 1);
            } else {
                std::string element = trim(resultPart);
                resultArray.push_back(element);
                break;
            }
            ++i;
            if ( i == 1 ) break;
        }

        std::cout << "ID: " << id << std::endl;
        std::cout << "First 5 results:" << std::endl;
        for (size_t i = 0; i < std::min(resultArray.size(), size_t(1)); ++i) {
            std::cout << resultArray[i] << std::endl;
        }
        std::string delimiter = ",";
        std::string joinedString = std::accumulate(resultArray.begin(), resultArray.end(), std::string(),
            [&delimiter](const std::string& a, const std::string& b) -> std::string {
                return a.empty() ? b : a + delimiter + b;
        });

        // https://www.pathofexile.com/api/trade/fetch/RESULT_LINES_HERE?query=ID_HERE
        curl_easy_reset(curl);

        std::string url = "https://www.pathofexile.com/api/trade/fetch/" + joinedString + "?query=" + id;
        std::cout << "Request url: " << url << '\n'; 
        std::string response2;

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response2);
        struct curl_slist *headers2 = NULL;
        // API won't accept the request without the User-Agent
        headers2 = curl_slist_append(headers2, "User-Agent: low-latency-poe-market-tracker/1.0");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers2);

        CURLcode res2 = curl_easy_perform(curl);

        if (res2 != CURLE_OK) {
            std::cerr << "Curl error: " << curl_easy_strerror(res2) << std::endl;
        } 
        // else {
        //     std::cout << "Response 2: " << response2 << std::endl;
        // }

        std::string whisper = findFieldValue(response2, "whisper");
        if (!whisper.empty()) {
            std::cout << whisper << std::endl; // Print the "whisper" field
        } else {
            std::cerr << "Whisper field not found." << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }

    return 0;
}