#pragma once

#include <cstdlib>
#include <sstream>
#include <string>

#include "HttpClient.h"
#include "WebCaptcha.h"

#ifdef _WIN32
    // windows.h must come first: shellapi.h uses types (HDROP and friends) that
    // it does not declare itself, so including it alone breaks every
    // translation unit that reaches this header.
    #include <windows.h>
    #include <shellapi.h>
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

    // The client only reads a few scalar fields out of a response, so these sit
    // here instead of pulling a second JSON parser into every target.
    static std::string jsonStringField(const std::string& json, const std::string& field) {
        const std::string needle = "\"" + field + "\":\"";
        const std::size_t pos = json.find(needle);
        if (pos == std::string::npos) {
            return "";
        }
        const std::size_t begin = pos + needle.size();
        const std::size_t end = json.find('"', begin);
        if (end == std::string::npos) {
            return "";
        }
        return json.substr(begin, end - begin);
    }

    static std::string jsonNumberField(const std::string& json, const std::string& field) {
        const std::string needle = "\"" + field + "\":";
        const std::size_t pos = json.find(needle);
        if (pos == std::string::npos) {
            return "";
        }
        std::size_t end = pos + needle.size();
        const std::size_t begin = end;
        while (end < json.size() && (std::isdigit(static_cast<unsigned char>(json[end])) || json[end] == '-')) {
            ++end;
        }
        return json.substr(begin, end - begin);
    }

    // The account endpoints only answer a solved captcha. A program cannot read
    // the image one, so this asks for the machine challenge and finds a nonce
    // whose sha256 digest starts with the difficulty the server asked for.
    static std::string login(const std::string& username, const std::string& password, const std::string& serverUrl = "http://localhost:3000") {
        std::string captchaFields;
        const std::string challenge = httpGet(serverUrl + "/api/captcha?kind=machine");
        const std::string captchaId = jsonStringField(challenge, "id");
        const int difficulty = std::atoi(jsonNumberField(challenge, "difficulty").c_str());
        std::string nonce;
        if (!captchaId.empty() && difficulty > 0 &&
            rblx::web::solveCaptchaPow(captchaId, difficulty, &nonce)) {
            std::ostringstream captcha;
            captcha << ",\"captchaId\":\"" << jsonEscape(captchaId) << "\",\"captchaNonce\":\""
                    << nonce << "\"";
            captchaFields = captcha.str();
        }

        std::ostringstream json;
        json << "{\"username\":\"" << jsonEscape(username) << "\",\"password\":\"" << jsonEscape(password) << "\""
             << captchaFields << "}";
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
        (void)std::system(command.c_str());
#else
        const std::string command = "xdg-open '" + url + "' >/dev/null 2>&1 &";
        (void)std::system(command.c_str());
#endif
    }
};
