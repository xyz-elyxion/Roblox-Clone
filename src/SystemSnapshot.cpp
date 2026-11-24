#include "SystemSnapshot.h"

#include <cstdio>
#include <cstring>
#include <cwchar>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <windows.h>
#include <tlhelp32.h>

namespace linear {
namespace anticheat {
namespace {

// Appends lines until the budget runs out. Every fact goes through here, so one
// loud machine cannot make the snapshot larger than the caller asked for.
class Lines {
public:
    explicit Lines(std::size_t maxBytes) : maxBytes_(maxBytes) {}

    bool add(const std::string& line) {
        if (maxBytes_ == 0 || text_.size() + line.size() + 1 > maxBytes_) {
            return false;
        }
        text_ += line;
        text_ += '\n';
        return true;
    }

    bool field(const std::string& key, const std::string& value) {
        return add(key + ": " + value);
    }

    const std::string& text() const { return text_; }

private:
    std::string text_;
    std::size_t maxBytes_ = 0;
};

// Toolhelp has no ...A structures: the SDK declares MODULEENTRY32 and
// PROCESSENTRY32 plus their W forms, and the bare name follows UNICODE. The W
// structures are named here and narrowed, so the snapshot reads the same whatever
// UNICODE the build sets.
std::string narrow(const wchar_t* text) {
    if (text == nullptr || *text == L'\0') {
        return std::string();
    }
    const int length = static_cast<int>(std::wcslen(text));
    const int bytes = WideCharToMultiByte(CP_UTF8, 0, text, length, nullptr, 0, nullptr, nullptr);
    if (bytes <= 0) {
        return std::string();
    }
    std::string result(static_cast<std::size_t>(bytes), '\0');
    WideCharToMultiByte(CP_UTF8, 0, text, length, result.data(), bytes, nullptr, nullptr);
    return result;
}

std::string utcNow() {
    SYSTEMTIME now;
    GetSystemTime(&now);
    char buffer[40] = {0};
    std::snprintf(buffer, sizeof(buffer), "%04u-%02u-%02uT%02u:%02u:%02uZ",
                  static_cast<unsigned>(now.wYear), static_cast<unsigned>(now.wMonth),
                  static_cast<unsigned>(now.wDay), static_cast<unsigned>(now.wHour),
                  static_cast<unsigned>(now.wMinute), static_cast<unsigned>(now.wSecond));
    return std::string(buffer);
}

// RtlGetVersion reports the build Windows is really running. GetVersionEx is
// capped at 6.2 for a program without a manifest for the newer releases, and an
// analysis that says "Windows 8" about Windows 11 is worse than no version.
struct OsVersionInfo {
    ULONG size;
    ULONG major;
    ULONG minor;
    ULONG build;
    ULONG platform;
    wchar_t servicePack[128];
};
using RtlGetVersionFn = LONG(WINAPI*)(OsVersionInfo*);

std::string osDescription() {
    HMODULE ntdll = GetModuleHandleA("ntdll.dll");
    if (ntdll != nullptr) {
        FARPROC address = GetProcAddress(ntdll, "RtlGetVersion");
        RtlGetVersionFn getVersion = nullptr;
        // GetProcAddress hands back a data pointer; copying it into the function
        // pointer is the portable way to call through it without the C4191
        // warning the direct cast earns.
        if (address != nullptr && sizeof(getVersion) == sizeof(address)) {
            std::memcpy(&getVersion, &address, sizeof(getVersion));
        }
        if (getVersion != nullptr) {
            OsVersionInfo info = {};
            info.size = sizeof(info);
            if (getVersion(&info) == 0) {
                return "Windows " + std::to_string(static_cast<unsigned long>(info.major)) + "." +
                       std::to_string(static_cast<unsigned long>(info.minor)) + " build " +
                       std::to_string(static_cast<unsigned long>(info.build));
            }
        }
    }
    return std::string("Windows (version not readable)");
}

std::string architectureName() {
    SYSTEM_INFO info;
    GetNativeSystemInfo(&info);
    switch (info.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: return "x64";
        case PROCESSOR_ARCHITECTURE_INTEL: return "x86";
        case PROCESSOR_ARCHITECTURE_ARM64: return "arm64";
        case PROCESSOR_ARCHITECTURE_ARM: return "arm";
        default: return "unknown";
    }
}

std::string megabytes(unsigned long long bytes) {
    return std::to_string(bytes / (1024ull * 1024ull)) + " MB";
}

std::string processPath() {
    char path[MAX_PATH + 1] = {0};
    const DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);
    if (length == 0 || length > MAX_PATH) {
        return std::string("(not readable)");
    }
    return std::string(path, length);
}

std::string processFacts() {
    const DWORD pid = GetCurrentProcessId();
    DWORD handles = 0;
    if (!GetProcessHandleCount(GetCurrentProcess(), &handles)) {
        handles = 0;
    }

    // How long the client has been up, from the creation time it was given. A
    // client that has been running for ten seconds and already trips a check is a
    // different review from one that has been in a match for an hour.
    long long seconds = 0;
    FILETIME created = {};
    FILETIME exited = {};
    FILETIME kernel = {};
    FILETIME user = {};
    if (GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user)) {
        FILETIME now = {};
        GetSystemTimeAsFileTime(&now);
        ULARGE_INTEGER start;
        start.LowPart = created.dwLowDateTime;
        start.HighPart = created.dwHighDateTime;
        ULARGE_INTEGER current;
        current.LowPart = now.dwLowDateTime;
        current.HighPart = now.dwHighDateTime;
        if (current.QuadPart > start.QuadPart) {
            seconds = static_cast<long long>((current.QuadPart - start.QuadPart) / 10000000ull);
        }
    }

    return "pid " + std::to_string(static_cast<unsigned long>(pid)) + ", up " +
           std::to_string(seconds) + " s, " + std::to_string(static_cast<unsigned long>(handles)) +
           " handles";
}

void appendModules(Lines* lines, const SnapshotLimits& limits) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32,
                                               GetCurrentProcessId());
    if (snapshot == INVALID_HANDLE_VALUE) {
        lines->field("modules", "not readable");
        return;
    }

    MODULEENTRY32W entry;
    entry.dwSize = sizeof(entry);
    std::size_t listed = 0;
    std::size_t skipped = 0;
    lines->add("modules: loaded into this process");
    if (Module32FirstW(snapshot, &entry)) {
        do {
            if (listed >= limits.maxModules) {
                ++skipped;
                continue;
            }
            char line[768] = {0};
            std::snprintf(line, sizeof(line), "  0x%p %llu KB %s", static_cast<void*>(entry.modBaseAddr),
                          static_cast<unsigned long long>(entry.modBaseSize) / 1024ull,
                          narrow(entry.szExePath).c_str());
            if (!lines->add(line)) {
                ++skipped;
                continue;
            }
            ++listed;
        } while (Module32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);

    if (listed == 0) {
        lines->add("  (no module could be read)");
    }
    if (skipped > 0) {
        lines->add("  +" + std::to_string(skipped) + " more (cut by the snapshot limit)");
    }
}

void appendProcesses(Lines* lines, const SnapshotLimits& limits) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        lines->field("processes", "not readable");
        return;
    }

    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    std::size_t listed = 0;
    std::size_t skipped = 0;
    lines->add("processes: running on this machine (pid parent-pid name)");
    if (Process32FirstW(snapshot, &entry)) {
        do {
            // Pid 0 is the idle process; it has no image and no parent worth
            // reading.
            if (entry.th32ProcessID == 0) {
                continue;
            }
            if (listed >= limits.maxProcesses) {
                ++skipped;
                continue;
            }
            char line[512] = {0};
            std::snprintf(line, sizeof(line), "  %lu %lu %s",
                          static_cast<unsigned long>(entry.th32ProcessID),
                          static_cast<unsigned long>(entry.th32ParentProcessID),
                          narrow(entry.szExeFile).c_str());
            if (!lines->add(line)) {
                ++skipped;
                continue;
            }
            ++listed;
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);

    if (listed == 0) {
        lines->add("  (no process could be read)");
    }
    if (skipped > 0) {
        lines->add("  +" + std::to_string(skipped) + " more (cut by the snapshot limit)");
    }
}

}  // namespace

std::string systemSnapshot(const SnapshotLimits& limits) {
    Lines lines(limits.maxBytes);

    lines.add("Linear client system snapshot");
    lines.field("collected", utcNow());
    lines.field("client", processPath());
    lines.field("process", processFacts());
    lines.field("os", osDescription());
    lines.field("architecture", architectureName());

    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD nameLength = MAX_COMPUTERNAME_LENGTH;
    if (GetComputerNameA(computerName, &nameLength)) {
        lines.field("machine", std::string(computerName, nameLength));
    } else {
        lines.field("machine", "(not readable)");
    }

    char userName[256] = {0};
    DWORD userNameLength = sizeof(userName) - 1;
    if (GetUserNameA(userName, &userNameLength)) {
        lines.field("user", std::string(userName, userNameLength));
    } else {
        lines.field("user", "(not readable)");
    }

    MEMORYSTATUSEX memory = {};
    memory.dwLength = sizeof(memory);
    if (GlobalMemoryStatusEx(&memory)) {
        lines.field("memory", megabytes(memory.ullTotalPhys) + " physical, " +
                                  megabytes(memory.ullAvailPhys) + " available");
    } else {
        lines.field("memory", "(not readable)");
    }

    // The lists come last so the facts above survive a cut: a review that has the
    // OS and the process but lost the tail is still usable, and one that only has
    // a list of processes is not.
    appendModules(&lines, limits);
    appendProcesses(&lines, limits);
    return lines.text();
}

}  // namespace anticheat
}  // namespace linear
