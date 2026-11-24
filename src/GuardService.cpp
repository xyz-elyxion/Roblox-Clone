// LinearGuard: the anti-cheat that runs as a service.
//
// LinearAntiCheat is compiled into Client.exe because its integrity, anti-debug
// and injection checks only mean something inside the process they protect. That
// also means a client that is patched, frozen or simply not started takes the
// checks with it. This program is the other side of that: a Windows service,
// started by the service manager as LocalSystem, that watches the game from
// outside.
//
// What it can see from there is what does not need to be in the process: which
// programs are running on the machine, and where the modules loaded into the
// client came from. What it finds goes into
// %ProgramData%\Linear\LinearGuard.log in the same "[stamp] [DETECTION] ..." shape
// the client already tails (see include/ClientCheatReport.h), so the client files
// those lines against the signed-in account and the service needs no token of its
// own. A client that stops reporting cannot hide that either: the snapshot says
// whether the service is running, and how long ago it last beat.
//
//   LinearGuard.exe --install      register and start the service (administrator)
//   LinearGuard.exe --uninstall    stop and remove it
//   LinearGuard.exe --status       what the snapshot would say about it
//   LinearGuard.exe --run          run the checks in this console, once a cycle
//   (no flags)                     started by the service manager; not by hand
//
// It cannot see the player's desktop: a service runs in session 0, where there are
// no windows to enumerate and no user to imitate. Window-title checks therefore
// stay in the client, and this program never pretends to make them.

#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tlhelp32.h>

#include "GuardChecks.h"
#include "GuardStatus.h"

namespace {

const wchar_t* const kServiceName = L"LinearGuard";
const wchar_t* const kServiceDisplay = L"Linear anti-cheat guard";
const wchar_t* const kServiceDescription =
    L"Watches the Linear client from outside the game: programs running beside it and the "
    L"modules loaded into it. Writes what it finds where the client reports it.";

// One pass every few seconds: often enough to catch a tool opened mid-match, and
// cheap enough that a service on a player's machine is not a background load.
constexpr DWORD kCycleMilliseconds = 5000;

SERVICE_STATUS_HANDLE g_statusHandle = nullptr;
SERVICE_STATUS g_status = {};
HANDLE g_stopEvent = nullptr;
bool g_consoleStop = false;
std::vector<std::string> g_reported;

// How this guard was started, written into its heartbeat: "service" when the
// service manager runs it, "user" when it was started at logon instead. The
// client reports which, because the two have different privileges.
const char* g_mode = "service";

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

std::string lowerPath(const std::string& text) {
    std::string result = text;
    for (char& c : result) {
        if (c == '/') {
            c = '\\';
        } else if (c >= 'A' && c <= 'Z') {
            c = static_cast<char>(c - 'A' + 'a');
        }
    }
    return result;
}

std::string folderOf(const std::string& path) {
    const std::size_t slash = path.find_last_of('\\');
    return slash == std::string::npos ? std::string() : path.substr(0, slash);
}

bool folderContains(const std::string& folder, const std::string& path) {
    if (folder.empty() || path.empty()) {
        return false;
    }
    const std::string lowerFolder = lowerPath(folder);
    const std::string lowerPathValue = lowerPath(path);
    return lowerPathValue.size() > lowerFolder.size() &&
           lowerPathValue.compare(0, lowerFolder.size(), lowerFolder) == 0 &&
           lowerPathValue[lowerFolder.size()] == '\\';
}

std::size_t fileSize(const std::string& path) {
    WIN32_FILE_ATTRIBUTE_DATA attributes = {};
    if (!GetFileAttributesExA(path.c_str(), GetFileExInfoStandard, &attributes)) {
        return 0;
    }
    return (static_cast<std::size_t>(attributes.nFileSizeHigh) << 32) |
           static_cast<std::size_t>(attributes.nFileSizeLow);
}

// The log the client reads, written with the same line shape it already parses.
// A full log starts again rather than growing; the client's tail sees a shorter
// file and reads it from the top, which it already handles for rotation.
void writeDetection(const std::string& message) {
    const std::string path = linear::guard::logPath();
    if (path.empty()) {
        return;
    }
    const std::string line = linear::guard::detectionLine(utcNow(), message);
    const std::size_t current = fileSize(path);
    if (current > 0 && !linear::guard::fitsInLog(current, line.size())) {
        std::ofstream truncate(path.c_str(), std::ios::binary | std::ios::trunc);
        truncate.close();
    }
    std::ofstream file(path.c_str(), std::ios::binary | std::ios::app);
    file << line << "\n";
}

// The heartbeat the client reads: fresh means the service is alive, old or absent
// means something stopped it. Nothing else about the service is trusted by the
// client, and the client is not trusted by the site.
void writeState() {
    const std::string path = linear::guard::statePath();
    if (path.empty()) {
        return;
    }
    std::ofstream file(path.c_str(), std::ios::binary | std::ios::trunc);
    file << "running " << utcNow() << " pid " << GetCurrentProcessId() << " mode=" << g_mode
         << "\n";
}

std::string imagePathOf(unsigned long pid) {
    HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE,
                                 static_cast<DWORD>(pid));
    if (process == nullptr) {
        return std::string();
    }
    wchar_t buffer[1024] = {0};
    DWORD length = static_cast<DWORD>(sizeof(buffer) / sizeof(buffer[0]));
    std::string path;
    if (QueryFullProcessImageNameW(process, 0, buffer, &length)) {
        path = narrow(buffer);
    }
    CloseHandle(process);
    return path;
}

std::vector<unsigned long> clientProcesses() {
    std::vector<unsigned long> found;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return found;
    }
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (entry.th32ProcessID == 0) {
                continue;
            }
            if (lowerPath(narrow(entry.szExeFile)) == "client.exe") {
                found.push_back(static_cast<unsigned long>(entry.th32ProcessID));
            }
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
    return found;
}

void scanPrograms(std::vector<std::string>* messages) {
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }
    PROCESSENTRY32W entry;
    entry.dwSize = sizeof(entry);
    if (Process32FirstW(snapshot, &entry)) {
        do {
            if (entry.th32ProcessID == 0 ||
                entry.th32ProcessID == static_cast<DWORD>(GetCurrentProcessId())) {
                continue;
            }
            const std::string name = narrow(entry.szExeFile);
            if (!linear::guard::isCheatToolName(name)) {
                continue;
            }
            messages->push_back("Process " + name + " (pid " +
                                std::to_string(static_cast<unsigned long>(entry.th32ProcessID)) +
                                ") is running on this machine");
        } while (Process32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

// The modules a client has loaded, narrowed to the ones that stand out: not one of
// the overlays every machine has, not from the client's own folder, and loaded
// from a place the logged-in user can write to.
void scanModules(unsigned long pid, std::vector<std::string>* messages) {
    const std::string clientFolder = folderOf(imagePathOf(pid));
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, pid);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return;
    }

    MODULEENTRY32W entry;
    entry.dwSize = sizeof(entry);
    if (Module32FirstW(snapshot, &entry)) {
        do {
            const std::string name = narrow(entry.szModule);
            if (linear::guard::isExpectedModule(name)) {
                continue;
            }
            const std::string path = narrow(entry.szExePath);
            if (folderContains(clientFolder, path)) {
                continue;
            }
            if (!linear::guard::isUserWritablePath(path)) {
                continue;
            }
            messages->push_back(
                "Module " + path +
                " was loaded into the game from a folder any program can write to");
        } while (Module32NextW(snapshot, &entry));
    }
    CloseHandle(snapshot);
}

void runCycle() {
    writeState();

    std::vector<std::string> messages;
    for (unsigned long pid : clientProcesses()) {
        scanModules(pid, &messages);
    }
    scanPrograms(&messages);

    // A detection is written once per service lifetime. The client dedupes too,
    // but a tool left open would otherwise rewrite the same line every cycle and
    // push everything else out of a bounded log.
    constexpr std::size_t kMaxReported = 64;
    for (const std::string& message : messages) {
        bool seen = false;
        for (const std::string& reported : g_reported) {
            if (reported == message) {
                seen = true;
                break;
            }
        }
        if (seen || g_reported.size() >= kMaxReported) {
            continue;
        }
        g_reported.push_back(message);
        writeDetection(message);
        std::printf("  guard: %s\n", message.c_str());
    }
}

void reportState(DWORD state, DWORD waitHint = 0) {
    g_status.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    g_status.dwCurrentState = state;
    g_status.dwWin32ExitCode = NO_ERROR;
    g_status.dwServiceSpecificExitCode = 0;
    g_status.dwWaitHint = waitHint;
    g_status.dwCheckPoint = (state == SERVICE_START_PENDING || state == SERVICE_STOP_PENDING) ? 1 : 0;
    g_status.dwControlsAccepted =
        state == SERVICE_RUNNING ? (SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN) : 0;
    if (g_statusHandle != nullptr) {
        SetServiceStatus(g_statusHandle, &g_status);
    }
}

void WINAPI controlHandler(DWORD control) {
    if (control == SERVICE_CONTROL_STOP || control == SERVICE_CONTROL_SHUTDOWN) {
        reportState(SERVICE_STOP_PENDING, kCycleMilliseconds * 2);
        if (g_stopEvent != nullptr) {
            SetEvent(g_stopEvent);
        }
        return;
    }
    if (control == SERVICE_CONTROL_INTERROGATE) {
        reportState(g_status.dwCurrentState);
    }
}

void WINAPI serviceMain(DWORD, LPWSTR*) {
    g_statusHandle = RegisterServiceCtrlHandlerW(kServiceName, controlHandler);
    if (g_statusHandle == nullptr) {
        return;
    }
    reportState(SERVICE_START_PENDING, kCycleMilliseconds * 2);

    g_stopEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr);
    if (g_stopEvent == nullptr) {
        reportState(SERVICE_STOPPED);
        return;
    }

    // A folder it cannot create is worth reporting but not worth stopping for:
    // the checks still run, and the snapshot says the guard cannot write.
    std::string folderError;
    if (!linear::guard::ensureFolder(&folderError)) {
        std::fprintf(stderr, "LinearGuard: %s\n", folderError.c_str());
    }

    reportState(SERVICE_RUNNING);
    while (WaitForSingleObject(g_stopEvent, kCycleMilliseconds) == WAIT_TIMEOUT) {
        runCycle();
    }

    reportState(SERVICE_STOPPED);
    CloseHandle(g_stopEvent);
    g_stopEvent = nullptr;
    g_statusHandle = nullptr;
}

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        g_consoleStop = true;
        return TRUE;
    }
    return FALSE;
}

bool installService(std::string* error) {
    SC_HANDLE manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);
    if (manager == nullptr) {
        const DWORD code = GetLastError();
        if (error != nullptr) {
            *error = code == ERROR_ACCESS_DENIED
                         ? "installing the guard needs an administrator: run it from an "
                           "elevated command prompt"
                         : "could not open the service manager (Windows error " +
                               std::to_string(code) + ")";
        }
        return false;
    }

    wchar_t image[MAX_PATH] = {0};
    if (GetModuleFileNameW(nullptr, image, MAX_PATH) == 0) {
        if (error != nullptr) {
            *error = "could not read this executable's path";
        }
        CloseServiceHandle(manager);
        return false;
    }
    const std::wstring command = L"\"" + std::wstring(image) + L"\" --service";

    SC_HANDLE service = CreateServiceW(
        manager, kServiceName, kServiceDisplay, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS,
        SERVICE_AUTO_START, SERVICE_ERROR_NORMAL, command.c_str(), nullptr, nullptr, nullptr,
        L"LocalSystem", nullptr);

    if (service == nullptr) {
        const DWORD code = GetLastError();
        if (code == ERROR_SERVICE_EXISTS) {
            service = OpenServiceW(manager, kServiceName, SERVICE_ALL_ACCESS);
        }
        if (service == nullptr) {
            if (error != nullptr) {
                *error = "could not register the service (Windows error " +
                         std::to_string(code) + ")";
            }
            CloseServiceHandle(manager);
            return false;
        }
    }

    // The description is what the Services window shows; a failure here does not
    // stop the guard from running.
    SERVICE_DESCRIPTIONW description = {};
    description.lpDescription = const_cast<LPWSTR>(kServiceDescription);
    ChangeServiceConfig2W(service, SERVICE_CONFIG_DESCRIPTION, &description);

    const BOOL started = StartServiceW(service, 0, nullptr);
    const DWORD startError = started ? ERROR_SUCCESS : GetLastError();
    CloseServiceHandle(service);
    CloseServiceHandle(manager);

    if (!started && startError != ERROR_SERVICE_ALREADY_RUNNING) {
        if (error != nullptr) {
            *error = "the service is registered but would not start (Windows error " +
                     std::to_string(startError) + ")";
        }
        return false;
    }
    return true;
}

bool uninstallService(std::string* error) {
    SC_HANDLE manager = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_CONNECT);
    if (manager == nullptr) {
        if (error != nullptr) {
            *error = "removing the guard needs an administrator (Windows error " +
                     std::to_string(GetLastError()) + ")";
        }
        return false;
    }

    SC_HANDLE service = OpenServiceW(manager, kServiceName,
                                     SERVICE_STOP | SERVICE_QUERY_STATUS | DELETE);
    if (service == nullptr) {
        const DWORD code = GetLastError();
        CloseServiceHandle(manager);
        if (code == ERROR_SERVICE_DOES_NOT_EXIST) {
            if (error != nullptr) {
                *error = "the guard is not installed";
            }
            return false;
        }
        if (error != nullptr) {
            *error = "could not open the service (Windows error " + std::to_string(code) + ")";
        }
        return false;
    }

    SERVICE_STATUS status = {};
    if (ControlService(service, SERVICE_CONTROL_STOP, &status)) {
        // Wait for the stop to be reported: DeleteService fails with
        // ERROR_SERVICE_MARKED_FOR_DELETE otherwise and leaves it half removed.
        for (int attempt = 0; attempt < 40; ++attempt) {
            Sleep(250);
            if (!QueryServiceStatus(service, &status)) {
                break;
            }
            if (status.dwCurrentState == SERVICE_STOPPED) {
                break;
            }
        }
    }

    const BOOL removed = DeleteService(service);
    const DWORD removeError = removed ? ERROR_SUCCESS : GetLastError();
    CloseServiceHandle(service);
    CloseServiceHandle(manager);

    if (!removed && removeError != ERROR_SERVICE_MARKED_FOR_DELETE) {
        if (error != nullptr) {
            *error = "could not remove the service (Windows error " +
                     std::to_string(removeError) + ")";
        }
        return false;
    }
    return true;
}

void printUsage(const char* program) {
    std::printf(
        "LinearGuard - the anti-cheat that watches the game from outside it\n"
        "\n"
        "  %s --install      register it with the service manager and start it\n"
        "                    (needs an elevated command prompt)\n"
        "  %s --uninstall    stop it and remove it\n"
        "  %s --status       what the client's snapshot says about it\n"
        "  %s --run          run the checks here, once every %lu seconds, until Ctrl+C\n"
        "                    (--background starts it without a console)\n"
        "\n"
        "The guard watches every running Client.exe from outside the game: cheat\n"
        "tools running on the machine, and modules loaded into the client from a\n"
        "folder any program can write to. What it finds goes into\n"
        "%%ProgramData%%\\Linear\\LinearGuard.log, which the client reads and files\n"
        "against the signed-in account.\n"
        "\n"
        "A machine where the player cannot elevate the service runs the same program\n"
        "from the user's Run key instead: --run --background, with the user's rights.\n",
        program, program, program, program, static_cast<unsigned long>(kCycleMilliseconds / 1000));
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> arguments;
    for (int i = 1; i < argc; ++i) {
        arguments.push_back(argv[i]);
    }
    const auto has = [&arguments](const char* flag) {
        for (const std::string& argument : arguments) {
            if (argument == flag) {
                return true;
            }
        }
        return false;
    };

    if (has("--help") || has("-h") || has("/?")) {
        printUsage(argv[0]);
        return 0;
    }

    // A flag this build does not know is a typo, not a mode: say so instead of
    // falling through to the service manager, which would wait forever for one.
    static const char* const known[] = {"--install", "--uninstall", "--status", "--run",
                                        "--service", "--background"};
    for (const std::string& argument : arguments) {
        bool recognised = false;
        for (const char* flag : known) {
            if (argument == flag) {
                recognised = true;
                break;
            }
        }
        if (!recognised) {
            std::fprintf(stderr, "LinearGuard: unknown option \"%s\". Use --help.\n",
                         argument.c_str());
            return 1;
        }
    }

    std::string error;
    if (has("--install")) {
        if (!installService(&error)) {
            std::fprintf(stderr, "LinearGuard: %s\n", error.c_str());
            return 1;
        }
        std::printf("LinearGuard: installed and started. The snapshot will say \"%s\".\n",
                    linear::guard::serviceFacts().c_str());
        return 0;
    }

    if (has("--uninstall")) {
        if (!uninstallService(&error)) {
            std::fprintf(stderr, "LinearGuard: %s\n", error.c_str());
            return 1;
        }
        std::printf("LinearGuard: removed.\n");
        return 0;
    }

    if (has("--status")) {
        std::printf("guard: %s\n", linear::guard::serviceFacts().c_str());
        std::printf("log:   %s\n", linear::guard::logPath().c_str());
        return 0;
    }

    if (has("--run")) {
        // The logon start-up uses this: no console window, and one guard per
        // session, because Windows can start the same value twice.
        const bool background = has("--background");
        if (background) {
            FreeConsole();
        }

        HANDLE single = CreateMutexW(nullptr, TRUE, L"Local\\LinearGuard");
        const DWORD mutexState = GetLastError();
        if (single == nullptr || mutexState == ERROR_ALREADY_EXISTS) {
            if (single != nullptr) {
                CloseHandle(single);
            }
            if (!background) {
                std::printf("LinearGuard: already running in this session.\n");
            }
            return 0;
        }

        g_mode = "user";
        SetConsoleCtrlHandler(consoleHandler, TRUE);
        if (!linear::guard::ensureFolder(&error)) {
            if (!background) {
                std::fprintf(stderr, "LinearGuard: %s\n", error.c_str());
            }
            CloseHandle(single);
            return 1;
        }
        if (!background) {
            std::printf("LinearGuard: watching as your user; Ctrl+C stops it.\n");
        }
        while (!g_consoleStop) {
            runCycle();
            Sleep(kCycleMilliseconds);
        }
        CloseHandle(single);
        return 0;
    }

    // Started by the service manager. Started by hand with no flag it comes back
    // with ERROR_FAILED_SERVICE_CONTROLLER_CONNECT, which is why --run exists.
    SERVICE_TABLE_ENTRYW table[] = {
        {const_cast<LPWSTR>(kServiceName), serviceMain},
        {nullptr, nullptr},
    };
    if (!StartServiceCtrlDispatcherW(table)) {
        std::fprintf(stderr,
                     "LinearGuard: not started by the service manager. Use --install to "
                     "register it, or --run to watch from this console.\n");
        return 1;
    }
    return 0;
}
