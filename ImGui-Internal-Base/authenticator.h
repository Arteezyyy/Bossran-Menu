#define CURL_STATICLIB

#include <iostream>
#include <sddl.h>
#include <winhttp.h>
#include <vector>
#include <string>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "Normaliz.lib")
#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Wldap32.lib")
#pragma comment(lib, "Crypt32.lib")
#pragma comment(lib, "advapi32.lib")

// Callback function for handling data received from the HTTP request
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

// Function to download content from a given URL
std::string DownloadContentFromUrl(const std::wstring& url) {
    std::string response;

    HINTERNET hSession = WinHttpOpen(L"A WinHTTP Example Program/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {
        std::cerr << "Can't initialize WinHTTP!" << std::endl;
        return response;
    }

    // Parse the URL to extract host and path
    size_t hostEnd = url.find(L"/", 8);
    std::wstring host = url.substr(8, hostEnd - 8);
    std::wstring path = url.substr(hostEnd);

    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(),
        INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        std::cerr << "Can't connect to server!" << std::endl;
        return response;
    }

    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),
        NULL, WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        std::cerr << "Can't open HTTP request!" << std::endl;
        return response;
    }

    BOOL bResults = WinHttpSendRequest(hRequest,
        WINHTTP_NO_ADDITIONAL_HEADERS, 0,
        WINHTTP_NO_REQUEST_DATA, 0,
        0, 0);

    if (bResults) {
        bResults = WinHttpReceiveResponse(hRequest, NULL);
    }

    DWORD dwSize = 0;
    DWORD dwDownloaded = 0;

    if (bResults) {
        do {
            dwSize = 0;
            if (!WinHttpQueryDataAvailable(hRequest, &dwSize)) {
                std::cerr << "Error in WinHttpQueryDataAvailable." << std::endl;
                break;
            }

            auto pszOutBuffer = new char[dwSize + 1];
            if (!pszOutBuffer) {
                std::cerr << "Out of memory" << std::endl;
                dwSize = 0;
            }
            else {
                ZeroMemory(pszOutBuffer, dwSize + 1);
                if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded)) {
                    std::cerr << "Error in WinHttpReadData." << std::endl;
                }
                else {
                    response.append(pszOutBuffer, dwDownloaded);
                }
                delete[] pszOutBuffer;
            }
        } while (dwSize > 0);
    }
    else {
        std::cerr << "Error in WinHttpSendRequest/WinHttpReceiveResponse." << std::endl;
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);

    return response;
}

// Function to check if the target HWID is present in the content from Dropbox
bool IsHWIDPresent(const std::wstring& dropboxURL, const std::string& targetHWID) {
    std::string pasteContent = DownloadContentFromUrl(dropboxURL);
    // Check if the target HWID is present in the paste content
    return (pasteContent.find(targetHWID) != std::string::npos);
}

// Function to validate HWID using Dropbox
bool isHWIDValidated = false;
bool IsHWIDValid(const std::wstring& dropboxURL, const std::string& targetHWID) {
    // Check the flag to determine if validation has already been performed
    if (isHWIDValidated) {
        return true;
    }

    // Perform HWID validation
    if (IsHWIDPresent(dropboxURL, targetHWID)) {
        isHWIDValidated = true;  // Set the flag to true after successful validation
        return true;
    }

    return false;
}

std::string GetComputerSid() {
    try {
        HANDLE hToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            throw std::exception("OpenProcessToken failed");
        }

        DWORD bufferSize = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &bufferSize);

        if (bufferSize == 0) {
            throw std::exception("GetTokenInformation failed");
        }

        std::vector<BYTE> buffer(bufferSize);
        PTOKEN_USER pTokenUser = reinterpret_cast<PTOKEN_USER>(buffer.data());

        if (!GetTokenInformation(hToken, TokenUser, pTokenUser, bufferSize, &bufferSize)) {
            throw std::exception("GetTokenInformation failed");
        }

        CloseHandle(hToken);

        CHAR* sidString;
        if (!ConvertSidToStringSidA(pTokenUser->User.Sid, &sidString)) {
            throw std::exception("ConvertSidToStringSidA failed");
        }

        std::string result(sidString);
        LocalFree(sidString);

        return result;
    }
    catch (const std::exception& ex) {
        std::cerr << "Error getting computer SID: " << ex.what() << std::endl;
    }

    return "";
}

void CopyToClipboard(const std::string& text) {
    if (OpenClipboard(NULL)) {
        EmptyClipboard();

        HGLOBAL hClipboardData;
        size_t size = (text.length() + 1) * sizeof(wchar_t);

        // Allocate global memory for the Unicode text
        hClipboardData = GlobalAlloc(GMEM_DDESHARE, size);

        if (hClipboardData != NULL) {
            // Lock the global memory and get a pointer to it
            wchar_t* pBuffer = static_cast<wchar_t*>(GlobalLock(hClipboardData));

            if (pBuffer != NULL) {
                // Convert narrow-character string to wide-character string
                size_t converted;
                mbstowcs_s(&converted, pBuffer, size, text.c_str(), _TRUNCATE);

                // Unlock the global memory
                GlobalUnlock(hClipboardData);

                // Set the clipboard data with the allocated memory
                SetClipboardData(CF_UNICODETEXT, hClipboardData);
            }
        }

        // Close the clipboard
        CloseClipboard();
    }
}