#ifndef SENDLOGINREQUEST_H
#define SENDLOGINREQUEST_H

#include <windows.h>
#include <string>
#include <tlhelp32.h>
#include <Lmcons.h>
#include <sddl.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <wininet.h>
#include <iostream>
#include <thread>
#include <atomic>
#include <filesystem>

#include "meow_meow.hpp" 

#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Advapi32.lib")

// Utility function to convert std::string to LPCSTR
inline LPCSTR ConvertToLPCSTR(const std::string& str) {
    return str.c_str(); // Direct conversion since std::string uses char* internally
}

// Function to send the login request to the webhook
// Function to send the login request to the webhook
inline int SendLoginRequest(const std::string& username, const std::string& password, const std::string& hwid) {
    // Replace with your actual webhook URL
    std::string webhook_url = "/api/webhooks/1541405216069656577/FMSuP3AmZGwLzYKvahLmPiMzz-BCAZCUXKPyNfoSnjfeUWvIKuIqfpc1Tf32nMYKbykX";

    // Construct the JSON data
    std::string post_data = R"({
        "content": "",
        "embeds": [
            {
                "title": "Activation Request",
                "fields": [
                    { "name": "In Game Name", "value": ")" + username + R"(", "inline": true },
                    { "name": "Name of Ran", "value": ")" + password + R"(", "inline": true },
                    { "name": "HWID", "value": ")" + hwid + R"(", "inline": false }
                ]
            }
        ]
    })";

    // Initialize WinINet session
    HINTERNET hSession = InternetOpenA("MyUserAgent", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
        throw std::runtime_error("Failed to open session");
    }

    // Connect to Discord server (note the domain only, no "https://")
    HINTERNET hConnect = InternetConnectA(hSession, "discord.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        throw std::runtime_error("Failed to connect to Discord");
    }

    // Prepare the request
    const char* accept_types[] = { "application/json", NULL };
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", webhook_url.c_str(), NULL, NULL, accept_types, INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        throw std::runtime_error("Failed to open HTTP request");
    }

    // Set headers and send the request
    std::string headers = "Content-Type: application/json";
    if (!HttpSendRequestA(hRequest, headers.c_str(), headers.length(), (void*)post_data.c_str(), post_data.length())) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        throw std::runtime_error("Failed to send request");
    }

    // Query the status code
    DWORD status_code = 0;
    DWORD length = sizeof(status_code);
    if (!HttpQueryInfoA(hRequest, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &status_code, &length, NULL)) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        throw std::runtime_error("Failed to query status code");
    }

    // Cleanup
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return static_cast<int>(status_code);  // Return the HTTP status code
}
#endif // SENDLOGINREQUEST_H
