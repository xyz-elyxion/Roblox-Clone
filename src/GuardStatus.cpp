#include "GuardStatus.h"

#include <cstdio>
#include <fstream>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

#include "Environment.h"

namespace linear {
namespace guard {
namespace {

const wchar_t* const kServiceName = L"LinearGuard";

// Older than this and the service is not keeping its own heartbeat, which is
// worth saying even when the service manager still calls it running.
constexpr long long kStaleHeartbeatSeconds = 30;

long long secondsSince(const FILETIME& when) {
    FILETIME now = {};
    GetSystemTimeAsFileTime(&now);
    ULARGE_INTEGER start;
    start.LowPart = when.dwLowDateTime;
    start.HighPart = when.dwHighDateTime;
    ULARGE_INTEGER current;
    current.LowPart = now.dwLowDateTime;
    current.HighPart = now.dwHighDateTime;
    if (current.QuadPart <= start.QuadPart) {
        return 0;
    }
    return static_cast<long long>((current.QuadPart - start.QuadPart) / 10000000ull);
}

// Which way the guard was started, read from the heartbeat it writes: "service"
// when the service manager runs it, "user" when the machine starts it at logon.
// Empty when there is no heartbeat, or the last one is old enough that the guard
// is not running any more - a dead guard must not be reported as alive.
std::string heartbeatMode() {
    const std::string path = statePath();
    if (path.empty()) {
        return std::string();
    }
    WIN32_FILE_ATTRIBUTE_DATA attributes = {};
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes) ||
        secondsSince(attributes.ftLastWriteTime) > kStaleHeartbeatSeconds) {
        return std::string();
    }
    std::ifstream file(path.c_str());
    std::string line;
    if (!std::getline(file, line)) {
        return std::string();
    }
    const std::size_t at = line.find("mode=");
    if (at == std::string::npos) {
        return std::string();
    }
    std::string mode = line.substr(at + 5);
    while (!mode.empty() && (mode.back() == '\r' || mode.back() == ' ')) {
        mode.pop_back();
    }
    if (mode == "service" || mode == "user") {
        return mode;
    }
    return std::string();
}

std::string heartbeatAge() {
    const std::string path = statePath();
    if (path.empty()) {
        return std::string("no heartbeat file");
    }
    WIN32_FILE_ATTRIBUTE_DATA attributes = {};
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes)) {
        return std::string("no heartbeat file");
    }
    const long long seconds = secondsSince(attributes.ftLastWriteTime);
    if (seconds > kStaleHeartbeatSeconds) {
        return "last heartbeat " + std::to_string(seconds) + " s ago, which is stale";
    }
    return "last heartbeat " + std::to_string(seconds) + " s ago";
}

}  // namespace

std::string folder() {
    const std::string base = linear::environment::get("ProgramData");
    if (base.empty()) {
        return std::string();
    }
    return base + "\\Linear";
}

std::string logPath() {
    const std::string base = folder();
    return base.empty() ? std::string() : base + "\\LinearGuard.log";
}

std::string statePath() {
    const std::string base = folder();
    return base.empty() ? std::string() : base + "\\LinearGuard.state";
}

bool ensureFolder(std::string* error) {
    const std::string base = folder();
    if (base.empty()) {
        if (error != nullptr) {
            *error = "ProgramData is not readable, so there is nowhere to keep the guard's files";
        }
        return false;
    }
    if (CreateDirectoryA(base.c_str(), nullptr)) {
        return true;
    }
    const DWORD code = GetLastError();
    if (code == ERROR_ALREADY_EXISTS) {
        return true;
    }
    if (error != nullptr) {
        *error = "Could not create " + base + " (Windows error " + std::to_string(code) + ")";
    }
    return false;
}

bool installed() {
    SC_HANDLE manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (manager == nullptr) {
        return false;
    }
    SC_HANDLE service = OpenServiceW(manager, kServiceName, SERVICE_QUERY_STATUS);
    const bool found = service != nullptr;
    if (service != nullptr) {
        CloseServiceHandle(service);
    }
    CloseServiceHandle(manager);
    return found;
}

bool running() {
    SC_HANDLE manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (manager == nullptr) {
        return false;
    }
    SC_HANDLE service = OpenServiceW(manager, kServiceName, SERVICE_QUERY_STATUS);
    bool isRunning = false;
    if (service != nullptr) {
        SERVICE_STATUS status = {};
        isRunning = QueryServiceStatus(service, &status) != FALSE &&
                    status.dwCurrentState == SERVICE_RUNNING;
        CloseServiceHandle(service);
    }
    CloseServiceHandle(manager);
    return isRunning;
}

std::string serviceFacts() {
    const char* const unknown = "state unknown (the service manager is not readable)";
    SC_HANDLE manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (manager == nullptr) {
        return std::string(unknown);
    }

    SC_HANDLE service = OpenServiceW(manager, kServiceName, SERVICE_QUERY_STATUS);
    if (service == nullptr) {
        const DWORD code = GetLastError();
        CloseServiceHandle(manager);
        if (code == ERROR_SERVICE_DOES_NOT_EXIST) {
            const std::string mode = heartbeatMode();
            if (!mode.empty()) {
                return "running (" + mode + ", started at logon), " + heartbeatAge();
            }
            return std::string("not installed");
        }
        return std::string("installed, not visible to this account (Windows error ") +
               std::to_string(code) + ")";
    }

    SERVICE_STATUS status = {};
    std::string facts;
    if (!QueryServiceStatus(service, &status)) {
        facts = "state unreadable (Windows error " + std::to_string(GetLastError()) + ")";
    } else if (status.dwCurrentState == SERVICE_RUNNING) {
        facts = "running (service), " + heartbeatAge();
    } else if (status.dwCurrentState == SERVICE_START_PENDING) {
        facts = "starting";
    } else if (status.dwCurrentState == SERVICE_STOP_PENDING) {
        facts = "stopping";
    } else if (const std::string mode = heartbeatMode(); !mode.empty()) {
        // The service is not running, but something is still writing its
        // heartbeat: this machine starts the same guard at logon instead.
        facts = "running (" + mode + ", started at logon), " + heartbeatAge();
    } else {
        facts = "installed, not running";
    }

    CloseServiceHandle(service);
    CloseServiceHandle(manager);
    return facts;
}

}  // namespace guard
}  // namespace linear
