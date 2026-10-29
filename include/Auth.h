#pragma once

#include <cstdlib>
#include <sstream>
#include <string>

#include "HttpClient.h"

#ifdef _WIN32
    #include <shellapi.h>
    #include <windows.h>
#endif

// Client-side calls to the website backend. The transport lives in
// HttpClient.cpp so the same code runs on Windows and POSIX; this class only
// builds the JSON and picks the endpoints.
class Auth {
public:
    static std::string jsonEscape(const std::string& value) {
        std::ostringstream escaped;
        for (char c : value) {
            unsigned char uc = static_cast<unsigned char>(c);
            switch (c) {
                case '\\': escaped << "\\\\"; break;
                case '"': escaped << "\\\""; break;
                case '\b': escaped << "\\b"; break;
                case '\f': escaped << "\\f"; break;
                case '\n': escaped << "\\n"; break;
                case '\r': escaped << "\\r"; break;
                case '\t': escaped << "\\t"; break;
                default:
                    if (uc < 0x20) {
                        escaped << "\\u00";
                        const char* hex = "0123456789abcdef";
                        escaped << hex[(uc >> 4) & 0x0f] << hex[uc & 0x0f];
                    } else {
                        escaped << c;
                    }
            }
        }
        return escaped.str();
    }

    static std::string httpPost(const std::string& url, const std::string& jsonData, const std::string& bearerToken = "") {
        const rblx::http::Response response = rblx::http::request("POST", url, jsonData, bearerToken);
        if (!response.ok()) {
            return "";
        }
        return response.body;
    }

    static std::string httpGet(const std::string& url, const std::string& bearerToken = "") {
        const rblx::http::Response response = rblx::http::request("GET", url, "", bearerToken);
        if (!response.ok()) {
            return "";
        }
        return response.body;
    }

    static std::string login(const std::string& username, const std::string& password, const std::string& serverUrl = "http://localhost:3000") {
        std::ostringstream json;
        json << "{\"username\":\"" << jsonEscape(username) << "\",\"password\":\"" << jsonEscape(password) << "\"}";
        return httpPost(serverUrl + "/api/login", json.str());
    }

    static std::string verifyToken(const std::string& token, const std::string& serverUrl = "http://localhost:3000") {
        return httpPost(serverUrl + "/api/verify", "{}", token);
    }

    static std::string getAvatar(const std::string& token, const std::string& serverUrl = "http://localhost:3000") {
        return httpGet(serverUrl + "/api/avatar", token);
    }

    // Opens the website's page in the user's browser.
    static void openBrowser(const std::string& url) {
#ifdef _WIN32
        const std::wstring wurl(url.begin(), url.end());
        ShellExecuteW(nullptr, L"open", wurl.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
#elif defined(__APPLE__)
        const std::string command = "open '" + url + "' >/dev/null 2>&1 &";
        std::system(command.c_str());
#else
        const std::string command = "xdg-open '" + url + "' >/dev/null 2>&1 &";
        std::system(command.c_str());
#endif
    }
};
