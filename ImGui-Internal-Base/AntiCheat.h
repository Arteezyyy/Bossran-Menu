#pragma once
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
#include <shlobj.h>
#include <atlimage.h>
#include <thread>



#pragma comment(lib, "wininet.lib")
#pragma comment(lib, "Advapi32.lib")

// Function to retrieve the current URL

class AntiCheat {
public:
    static void Check();
    static std::string GetComputerSID();
    static std::string GetIPAddress();
    static void CaptureScreenshot(const std::string& filePath);
    static std::string PostMultipartRequest(const std::string& url, const std::string& hwid, const std::string& filePath);
    static std::string PostPilotMonitoringWebhook(const std::string& url, const std::string& message);

private:
    
    static bool IsScanButton(HWND hwnd);
    static BOOL CALLBACK EnumChildProc(HWND hwndChild, LPARAM lParam);

};


std::atomic<bool> submitted{ false };
std::atomic<bool> shouldExit{ false };

std::string AntiCheat::PostPilotMonitoringWebhook(const std::string& url, const std::string& message) {
    // Open internet session
    HINTERNET hSession = InternetOpenA("AntiCheat", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) return "Failed to open internet session";

    // Connect to Discord server
    HINTERNET hConnect = InternetConnectA(hSession, "discord.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        return "Failed to connect to Discord";
    }

    // Open HTTP POST request to the webhook URL
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", url.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return "Failed to open HTTP request";
    }

    // Prepare JSON payload for pilot monitoring message
    std::string jsonPayload = R"({
        "content": ")" + message + R"("
    })";

    std::string headers = "Content-Type: application/json\r\n";

    // Send HTTP request with JSON body
    BOOL result = HttpSendRequestA(hRequest, headers.c_str(), headers.length(), (LPVOID)jsonPayload.c_str(), jsonPayload.length());

    if (!result) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return "Failed to send HTTP request";
    }

    // Read response
    char responseBuffer[1024];
    DWORD bytesRead;
    std::string response;
    while (InternetReadFile(hRequest, responseBuffer, sizeof(responseBuffer), &bytesRead) && bytesRead > 0) {
        response.append(responseBuffer, bytesRead);
    }

    // Cleanup handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return response;
}

std::string AntiCheat::PostMultipartRequest(const std::string& url, const std::string& hwid, const std::string& filePath) {
    // Initialize the internet session
    HINTERNET hSession = InternetOpenA("AntiCheat", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hSession) {
        return "Failed to open internet session: " + std::to_string(GetLastError());
    }

    // Connect to the server
    HINTERNET hConnect = InternetConnectA(hSession, "discord.com", INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        InternetCloseHandle(hSession);
        return "Failed to connect to Discord: " + std::to_string(GetLastError());
    }

    // Open the HTTP request
    HINTERNET hRequest = HttpOpenRequestA(hConnect, "POST", url.c_str(), NULL, NULL, NULL, INTERNET_FLAG_SECURE | INTERNET_FLAG_RELOAD, 0);
    if (!hRequest) {
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return "Failed to open HTTP request: " + std::to_string(GetLastError());
    }

    // Define boundary and headers
    std::string boundary = "------------------------" + std::to_string(GetTickCount64());
    std::string headers = "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";

    // Construct the multipart body
    std::ostringstream body;

    // JSON part with detection report
    std::string jsonPart = R"({
        "content": "",
        "embeds": [ 
            {
                "title": "Detection Report",
                "fields": [
                    { "name": ":desktop:  HWID:", "value": ")" + hwid + R"(", "inline": true },
                    { "name": ":globe_with_meridians:  IP Address:", "value": ")" + GetIPAddress() + R"(", "inline": false }
                ],
                "image": {
                     "url": "https://share.creavite.co/66ff6d94d5095e1d66297165.gif"
                }
            }
        ]
    })";

    // Add JSON data to multipart body (first part)
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"payload_json\"\r\n\r\n";
    body << jsonPart << "\r\n";

    // Attach the screenshot file (second part)
    body << "--" << boundary << "\r\n";
    body << "Content-Disposition: form-data; name=\"file1\"; filename=\"Yume.png\"\r\n";
    body << "Content-Type: image/png\r\n\r\n";

    // Read the screenshot file data
    std::ifstream screenshotFile(filePath, std::ios::binary);
    if (!screenshotFile.is_open()) {
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return "Failed to open screenshot file: " + filePath;
    }

    std::vector<BYTE> fileData((std::istreambuf_iterator<char>(screenshotFile)), std::istreambuf_iterator<char>());
    screenshotFile.close();

    // Write screenshot file data to body
    body.write(reinterpret_cast<const char*>(fileData.data()), fileData.size());
    body << "\r\n";

    // End the boundary
    body << "--" << boundary << "--\r\n";

    std::string bodyStr = body.str();

    // Include Content-Length header
    headers += "Content-Length: " + std::to_string(bodyStr.length()) + "\r\n\r\n";

    // Send the HTTP request with headers and body
    BOOL result = HttpSendRequestA(hRequest, headers.c_str(), headers.length(), (LPVOID)bodyStr.c_str(), bodyStr.length());
    if (!result) {
        DWORD error = GetLastError();
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hSession);
        return "Failed to send request: " + std::to_string(error);
    }

    // Read the response
    char responseBuffer[1024];
    DWORD bytesRead;
    std::string response;
    while (InternetReadFile(hRequest, responseBuffer, sizeof(responseBuffer), &bytesRead) && bytesRead > 0) {
        response.append(responseBuffer, bytesRead);
    }

    // Clean up handles
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hSession);

    return response;
}


std::string GetPicturesDirectory() {
    PWSTR path = NULL;
    if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_Pictures, 0, NULL, &path))) {
        std::wstring widePath(path);
        CoTaskMemFree(path);
        return std::string(widePath.begin(), widePath.end());
    }
    return "";
}

void AntiCheat::CaptureScreenshot(const std::string& filePath) {
    // Get the device context of the entire screen
    HDC hScreenDC = GetDC(NULL);
    HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

    // Get screen width and height
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    // Create a compatible bitmap
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreenDC, screenWidth, screenHeight);

    // Select the compatible bitmap into the memory DC
    HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

    // Bit block transfer from screen to memory DC
    BitBlt(hMemoryDC, 0, 0, screenWidth, screenHeight, hScreenDC, 0, 0, SRCCOPY);

    // Restore the old bitmap
    SelectObject(hMemoryDC, hOldBitmap);

    // Cleanup
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);

    // Save the bitmap to a file
    IStream* stream = NULL;
    HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &stream);
    if (FAILED(hr)) {
        std::cerr << "Failed to create stream" << std::endl;
        return;
    }

    CImage image;
    image.Attach(hBitmap);
    image.Save(stream, Gdiplus::ImageFormatPNG);

    ULARGE_INTEGER liSize;
    IStream_Size(stream, &liSize);
    DWORD len = liSize.LowPart;

    IStream_Reset(stream);
    std::vector<BYTE> buf(len);
    IStream_Read(stream, buf.data(), len);
    stream->Release();

    std::fstream fi;
    fi.open(filePath, std::fstream::binary | std::fstream::out);
    fi.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    fi.close();

    // Cleanup
    DeleteObject(hBitmap);
}

std::string AntiCheat::GetIPAddress() {
    std::string ipStr = "Unknown";

    // Initialize WinHTTP
    HINTERNET hSession = WinHttpOpen(L"WinHTTP Example/1.0",
        WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
        WINHTTP_NO_PROXY_NAME,
        WINHTTP_NO_PROXY_BYPASS, 0);

    if (hSession) {
        HINTERNET hConnect = WinHttpConnect(hSession, L"api.ipify.org",
            INTERNET_DEFAULT_HTTP_PORT, 0); // Use a different, simpler service for IP lookup

        if (hConnect) {
            HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", L"/?format=json",
                NULL, WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                0);

            if (hRequest) {
                BOOL bResults = WinHttpSendRequest(hRequest,
                    WINHTTP_NO_ADDITIONAL_HEADERS, 0,
                    WINHTTP_NO_REQUEST_DATA, 0,
                    0, 0);

                if (bResults) {
                    bResults = WinHttpReceiveResponse(hRequest, NULL);

                    if (bResults) {
                        DWORD dwSize = 0;
                        DWORD dwDownloaded = 0;
                        LPSTR pszOutBuffer;
                        std::string response;

                        do {
                            dwSize = 0;
                            if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                                break;

                            pszOutBuffer = new char[dwSize + 1];
                            if (!pszOutBuffer) {
                                dwSize = 0;
                                break;
                            }

                            ZeroMemory(pszOutBuffer, dwSize + 1);

                            if (!WinHttpReadData(hRequest, (LPVOID)pszOutBuffer,
                                dwSize, &dwDownloaded)) {
                                delete[] pszOutBuffer;
                                break;
                            }

                            response.append(pszOutBuffer, dwDownloaded);
                            delete[] pszOutBuffer;

                        } while (dwSize > 0);

                        // Parse JSON response to extract IP address
                        size_t ipStart = response.find("\"ip\":\"");
                        if (ipStart != std::string::npos) {
                            ipStart += 6; // Move past "ip":"
                            size_t ipEnd = response.find("\"", ipStart);
                            if (ipEnd != std::string::npos) {
                                ipStr = response.substr(ipStart, ipEnd - ipStart);
                            }
                        }
                    }
                }
                WinHttpCloseHandle(hRequest);
            }
            WinHttpCloseHandle(hConnect);
        }
        WinHttpCloseHandle(hSession);
    }

    return ipStr;
}

std::string AntiCheat::GetComputerSID() {
    try {
        HANDLE hToken;
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
            throw std::exception("OpenProcessToken failed");
        }

        DWORD bufferSize = 0;
        GetTokenInformation(hToken, TokenUser, nullptr, 0, &bufferSize);
        if (bufferSize == 0) throw std::exception("GetTokenInformation failed");

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

std::wstring ConvertToWString(const TCHAR* charArray) {
    int size_needed = MultiByteToWideChar(CP_ACP, 0, charArray, -1, NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_ACP, 0, charArray, -1, &wstrTo[0], size_needed);
    return wstrTo;
}

bool AntiCheat::IsScanButton(HWND hwnd) {
    WCHAR windowText[256];
    if (GetWindowTextW(hwnd, windowText, sizeof(windowText) / sizeof(WCHAR))) {
        std::wstring text(windowText);
        return
            text.find(L" \\-\\ ") != std::wstring::npos ||
            text.find(L"남자 아이") != std::wstring::npos ||
            text.find(L"여자 아이") != std::wstring::npos ||
            text.find(L"首次扫描") != std::wstring::npos ||
            text.find(L"再次担描") != std::wstring::npos ||
            text.find(L"GZ 1stScan") != std::wstring::npos ||
            text.find(L"GZ NxtScan") != std::wstring::npos ||
            text.find(L"FIRST ") != std::wstring::npos ||
            text.find(L"NEXT ") != std::wstring::npos ||
            text.find(L"Next Scan") != std::wstring::npos ||
            text.find(L"First Scan") != std::wstring::npos ||
            text.find(L"bossb View") != std::wstring::npos ||
            text.find(L"Memory View") != std::wstring::npos ||
            text.find(L"扫描") != std::wstring::npos ||
            text.find(L"scan") != std::wstring::npos || // Generic scan term
            text.find(L"扫描") != std::wstring::npos;   // Generic scan term in Chinese


    }
    return false;
}

BOOL CALLBACK AntiCheat::EnumChildProc(HWND hwndChild, LPARAM) {
    if (IsScanButton(hwndChild)) {
        std::string hwid = GetComputerSID();
        CaptureScreenshot("data\gui\server.png");
        std::string ipAddress = GetIPAddress(); // Implement this function
        std::string response = PostMultipartRequest(
            "/api/webhooks/1349293813893562408/CYiL96RRLkvxGZgLhn9TaaoUKqQ0rvE8ZZfoPMM_1597s0MIgDeVWJqV89KJqDE-JP4A",
            hwid,
            "data\gui\server.png");



        HANDLE hProcess = GetCurrentProcess();
        TerminateProcess(hProcess, 0);
    }
    return TRUE; // Continue enumeration
}

void AntiCheat::Check() {
    // Create a snapshot of the processes
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32 processEntry = {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &processEntry)) {
        do {
            // Open the process to query its window titles
            HWND hwnd = NULL;
            while ((hwnd = FindWindowEx(NULL, hwnd, NULL, NULL)) != NULL) {
                TCHAR windowTitle[256];
                GetWindowText(hwnd, windowTitle, sizeof(windowTitle) / sizeof(TCHAR));

                std::wstring windowTitleWStr = ConvertToWString(windowTitle);

                // Check for specific window titles or debugger-related modules

                if (windowTitleWStr.find(L"Crossfire") != std::wstring::npos ||
                    windowTitleWStr.find(L"Select a process to open") != std::wstring::npos ||
                    windowTitleWStr.find(L"This list shows all the found addresses that matched your last scan") != std::wstring::npos ||
                    windowTitleWStr.find(L"Delete all addresses from the list") != std::wstring::npos ||
                    windowTitleWStr.find(L"Copy all selected items to the address list") != std::wstring::npos ||
                    windowTitleWStr.find(L"Codelist and pause") != std::wstring::npos ||
                    windowTitleWStr.find(L"Type the name of the process you want to automatically open. Note: Only works when NO process has been opened yet") != std::wstring::npos ||
                    windowTitleWStr.find(L"B?? V 0.0.1") != std::wstring::npos ||
                    windowTitleWStr.find(L"N?? ?1.1") != std::wstring::npos ||
                    windowTitleWStr.find(L"-.exe") != std::wstring::npos ||
                    windowTitleWStr.find(L"? ??? ?? ?? ???") != std::wstring::npos ||
                    windowTitleWStr.find(L"Memory View") != std::wstring::npos ||
                    windowTitleWStr.find(L"|난 한국어 조금 할수 있어요") != std::wstring::npos ||
                    windowTitleWStr.find(L"Console Engine detected") != std::wstring::npos ||
                    windowTitleWStr.find(L"Crossfire") != std::wstring::npos ||
                    windowTitleWStr.find(L"Auto Suspender v0.1 beta") != std::wstring::npos ||
                    windowTitleWStr.find(L"Mellow Suspender") != std::wstring::npos ||
                    windowTitleWStr.find(L"olaf_007") != std::wstring::npos ||
                    windowTitleWStr.find(L"ZACK ENGINE 6.7") != std::wstring::npos ||
                    windowTitleWStr.find(L"Address") != std::wstring::npos ||
                    windowTitleWStr.find(L"Memory View") != std::wstring::npos ||
                    windowTitleWStr.find(L"Copy to clipboard") != std::wstring::npos ||
                    windowTitleWStr.find(L"Goto Address") != std::wstring::npos ||
                    windowTitleWStr.find(L"Enable/Disable speedhack. (No hotkey)") != std::wstring::npos ||
                    windowTitleWStr.find(L"Open a cheat table or unprotected cetrainer") != std::wstring::npos ||
                    windowTitleWStr.find(L"Save your current data") != std::wstring::npos ||
                    windowTitleWStr.find(L"address") != std::wstring::npos ||
                    windowTitleWStr.find(L"cheatengine-x86_64.exe") != std::wstring::npos ||
                    windowTitleWStr.find(L"Attach debugger to process") != std::wstring::npos ||
                    windowTitleWStr.find(L"auto attaching first time") != std::wstring::npos ||
                    windowTitleWStr.find(L"attached to process") != std::wstring::npos ||
                    windowTitleWStr.find(L"not attached to process") != std::wstring::npos ||
                    windowTitleWStr.find(L"You will need to run the dissect code routine first before this window is usable. Run it now?") != std::wstring::npos ||
                    windowTitleWStr.find(L"suspicious_dll_name.dll") != std::wstring::npos ||
                    windowTitleWStr.find(L"ollydbg.exe") != std::wstring::npos ||
                    windowTitleWStr.find(L"ida.exe") != std::wstring::npos ||
                    windowTitleWStr.find(L"memoryscanner.exe") != std::wstring::npos ||
                    windowTitleWStr.find(L"bossb engine") != std::wstring::npos) {


                    std::string hwid = GetComputerSID();
                    CaptureScreenshot("data\gui\server.png");
                    std::string ipAddress = GetIPAddress(); // Implement this function
                    std::string response = PostMultipartRequest(
                        "/api/webhooks/1541405372835954810/UHyjKHOWXA8G34LTlW1_4mzqfqY-4VBnpbBqddbWp2OgNuZS1xCw9ZL2pjC-9PR6o5w-",
                        hwid,
                        "data\gui\server.png");



                    HANDLE hProcess = GetCurrentProcess();
                    TerminateProcess(hProcess, 0);
                    return;

                }

                // Enumerate child windows
                EnumChildWindows(hwnd, EnumChildProc, 0);
            }

        } while (Process32Next(hSnapshot, &processEntry));
    }

    CloseHandle(hSnapshot);
}