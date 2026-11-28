#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Part.h"
#include "Raycaster.h"
#include "Skybox.h"
#include "PhysicsWorld.h"
#include "ShadowMap.h"
#include "Character.h"
#include "Network.h"
#include "UdpTransport.h"
#include "WorldLoader.h"
#include "Auth.h"
#include "Avatar.h"
#include "UiScale.h"
#include "ClientShared.h"
#include "Audio.h"
#include "SoundBank.h"
#include "Obfuscated.h"
#include "WebJson.h"
#include <linearmath/gtx/euler_angles.hpp>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>

#include <deque>
#include <memory>
#include <vector>
#include <map>
#include <iostream>
#include <string>
#include <algorithm>
#include <atomic>
#include <cmath>
#include <cstring>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <csignal>
#include <exception>
#include <io.h>
#include <share.h>
#include <thread>
#include <mutex>
#include <iomanip>

// The file picker is the only part of the client that needs a native window
// handle, so the Win32 headers stay behind this guard.
#define LINEARWIN_EXPOSE_NATIVE_WIN32
#include <LINEARWIN/linearwinnative.h>
#include <commdlg.h>

#include "linearui.h"
#include "linearui_impl_linearwin.h"
#include "linearui_impl_opengl3.h"

#if defined(LINEAR_ANTICHEAT)
// Vendored from UltimateAntiCheat and renamed to LinearAntiCheat (AGPL-3.0, see
// LinearAntiCheat/LICENSE). It is linked into this executable on purpose: the
// integrity, anti-debug and injection checks only mean something inside the
// process they protect.
//
// Every way of starting it is non-fatal. A machine where a check cannot run
// (an unexpected parent process, a locked-down install) still gets a playable
// client and a line in LinearAntiCheat.log explaining why.
#include "Core/AntiCheatLib.hpp"
#include "Common/Settings.hpp"

namespace {
    std::unique_ptr<Settings> g_antiCheatSettings;
    std::unique_ptr<AntiCheat> g_antiCheat;

    void startAntiCheat() {
        try {
            // The ways this client normally gets started: the website launcher,
            // the desktop shell, and a terminal while developing. The library
            // compares the real parent process against this list.
            //
            // Masked at compile time: this list is the one thing in the client
            // worth knowing before attacking it (launch from an allowed parent),
            // and it should not fall out of `strings Client.exe`.
            const std::list<std::wstring> allowedParents = {
                LINEAR_OBF_W(L"WebApp.exe"), LINEAR_OBF_W(L"node.exe"),
                LINEAR_OBF_W(L"explorer.exe"), LINEAR_OBF_W(L"cmd.exe"),
                LINEAR_OBF_W(L"powershell.exe"), LINEAR_OBF_W(L"pwsh.exe"),
                LINEAR_OBF_W(L"bash.exe")
            };

            g_antiCheatSettings = std::make_unique<Settings>(
                "127.0.0.1", static_cast<unsigned short>(5445),
                /*bNetworkingEnabled*/ false,   // no anti-cheat server is running
                /*bEnforceSecureBoot*/ false,
                /*bEnforceDSE*/ false,
                /*bEnforceNoKDbg*/ false,
                /*bUseAntiDebugging*/ true,
                /*bCheckIntegrity*/ true,
                /*bCheckThreads*/ true,
                /*bCheckHypervisor*/ false,
                /*bRequireRunAsAdministrator*/ false,  // a game client should not need admin
                /*bUsingDriver*/ false,   // no kernel-mode driver ships with this build
                L"",
                allowedParents,
                /*bEnableLogging*/ true,
                LINEAR_OBF("LinearAntiCheat.log")
            );

            g_antiCheat = std::make_unique<AntiCheat>(g_antiCheatSettings.get());
            std::cout << LINEAR_OBF("LinearAntiCheat started.") << std::endl;
        } catch (const std::exception& e) {
            std::cout << LINEAR_OBF("LinearAntiCheat did not start: ") << e.what() << std::endl;
            g_antiCheat.reset();
        } catch (...) {
            std::cout << LINEAR_OBF("LinearAntiCheat did not start.") << std::endl;
            g_antiCheat.reset();
        }
    }

    void stopAntiCheat() {
        if (g_antiCheat) {
            g_antiCheat->Destroy();
            g_antiCheat.reset();
        }
        g_antiCheatSettings.reset();
    }
}
#endif

// The client sends its own output to client_log.txt, so the console it was started
// from stays blank. A failure before the window opens then looks like the program doing
// nothing at all, with the reason sitting in a file nobody thinks to open first. Startup
// milestones and failures are written to the console as well, through its own handle.
void sayToConsole(const std::string& message) {
    std::cout << message << std::endl;

    FILE* console = nullptr;
    if (fopen_s(&console, "CONOUT$", "w") != 0 || console == nullptr) {
        return;
    }
    std::fputs(message.c_str(), console);
    std::fputc('\n', console);
    std::fflush(console);
    std::fclose(console);
}

// Every program here reads its data from the directory it was started in, which is not
// always obvious: an executable opened from inside a .zip runs from a temporary folder
// that holds nothing else. Naming it is what turns "it does not start" into a reason.
std::string workingDirectory() {
    std::error_code ignored;
    const std::filesystem::path path = std::filesystem::current_path(ignored);
    return ignored ? std::string(".") : path.string();
}

// A launch that shows nothing at all is the one failure this program cannot report on:
// stdout goes into client_log.txt from inside main, and the console writes need a console
// the process may not have, so a stop in the loader, in a TLS callback or in a static
// initialiser leaves no evidence anywhere - the window opens on an empty prompt and that
// is all anyone sees. This appends one line per stage to startup_trace.txt in the working
// directory, using only the Win32 calls that are safe that early, so the next report says
// how far the process got. It is a handful of short lines per run, plus one a second
// while the first frames are drawn; delete the file freely.

// The last stage reached, for the crash report below. A fixed buffer on purpose:
// the report may be written while the process is already faulting, which is no time
// to allocate.
char g_lastStartupStage[160] = "before the first trace line";

void traceStartup(const char* stage) {
    HANDLE trace = CreateFileA("startup_trace.txt", FILE_APPEND_DATA,
                               FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, nullptr);
    if (trace == INVALID_HANDLE_VALUE) {
        return;
    }
    std::snprintf(g_lastStartupStage, sizeof(g_lastStartupStage), "%s", stage);
    DWORD written = 0;
    SetFilePointer(trace, 0, nullptr, FILE_END);
    WriteFile(trace, stage, static_cast<DWORD>(lstrlenA(stage)), &written, nullptr);
    WriteFile(trace, "\r\n", 2, &written, nullptr);
    CloseHandle(trace);
}

// For the stages whose text is built at run time; the const char* overload above stays
// the one every fixed message uses.
void traceStartup(const std::string& stage) {
    traceStartup(stage.c_str());
}

namespace {
// Static initialisers run after every TLS callback and before main, so this line being
// present is what tells "the process got as far as its own code" from "it stopped in the
// loader, before anything of ours ran".
struct StartupTrace {
    StartupTrace() { traceStartup("1/10 static initialisers ran"); }
};
const StartupTrace g_startupTrace;
}

// Keeps a fatal error readable when the client stops before it can play. The reason goes
// to the console and to the log whichever way it was started, but a client started by the
// website is given no console at all: it simply exits, and the player is left looking at a
// window that opened and closed. A message box is the one thing such a process can still
// show, and it is the difference between "it closed" and the sentence that says why.
// With a console, `pause` keeps the same text on screen instead.
void pauseBeforeExit(const std::string& reason) {
    sayToConsole(reason);
    const std::string logLocation = "The full log is in client_log.txt and "
                                    "client_error_log.txt in " +
                                    workingDirectory() + ".";
    sayToConsole(logLocation);

    if (GetConsoleWindow() != nullptr) {
        system("pause");
        return;
    }
    MessageBoxA(nullptr, (reason + "\n\n" + logLocation).c_str(), "Linear Client",
                MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND);
}

// Appends text to crash.txt. Fixed buffers and Win32 calls only, because this can run
// while the process is already ending.
void appendToCrashFile(const char* text) {
    HANDLE file = CreateFileA("crash.txt", FILE_APPEND_DATA,
                              FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL, nullptr);
    if (file == INVALID_HANDLE_VALUE) {
        return;
    }
    SetFilePointer(file, 0, nullptr, FILE_END);
    DWORD written = 0;
    WriteFile(file, text, static_cast<DWORD>(std::strlen(text)), &written, nullptr);
    CloseHandle(file);
}

// Writes crash.txt beside the logs. A crash that Windows or the anti-cheat handles
// on its own closes the window and leaves client_log.txt ending mid-line: the code,
// the module and the last stage below are what turn "it closed" into a call site.
// Win32 calls and one fixed buffer only, because this runs after the process has
// already faulted.
void writeCrashReport(const char* kind, unsigned long code, const void* address,
                      const char* detail) {
    char module[MAX_PATH] = "unknown";
    unsigned long long offset = 0;
    if (address != nullptr) {
        MEMORY_BASIC_INFORMATION info{};
        if (VirtualQuery(address, &info, sizeof(info)) != 0 && info.AllocationBase != nullptr) {
            if (GetModuleFileNameA(static_cast<HMODULE>(info.AllocationBase), module,
                                   sizeof(module)) == 0) {
                std::snprintf(module, sizeof(module), "unknown");
            }
            offset = static_cast<unsigned long long>(static_cast<const char*>(address) -
                                                     static_cast<const char*>(info.AllocationBase));
        }
    }

    char text[1024];
    std::snprintf(text, sizeof(text),
                  "%s\r\ncode: 0x%08lX\r\naddress: %s+0x%llX\r\n"
                  "last stage: %s\r\ndetail: %s\r\n\r\n",
                  kind, code, module, offset, g_lastStartupStage,
                  detail != nullptr ? detail : "");

    appendToCrashFile(text);
}

LPTOP_LEVEL_EXCEPTION_FILTER g_previousExceptionFilter = nullptr;
DWORD g_mainThreadId = 0;

LONG WINAPI reportUnhandledException(EXCEPTION_POINTERS* info) {
    if (info != nullptr && info->ExceptionRecord != nullptr) {
        writeCrashReport("unhandled exception", info->ExceptionRecord->ExceptionCode,
                         info->ExceptionRecord->ExceptionAddress, "");
    }
    // The anti-cheat installs its own filter from the loader, and letting it run
    // keeps its log line and Windows Error Reporting: this report is an addition.
    if (g_previousExceptionFilter != nullptr &&
        g_previousExceptionFilter != reportUnhandledException) {
        return g_previousExceptionFilter(info);
    }
    return EXCEPTION_CONTINUE_SEARCH;
}

// std::terminate runs for an exception nobody caught in a thread, which is silent
// from the outside: the window closes and the reason only ever reached stderr.
void reportTerminate() {
    const char* detail = "no active exception";
    std::string message;
    if (std::current_exception()) {
        try {
            std::rethrow_exception(std::current_exception());
        } catch (const std::exception& error) {
            message = error.what();
        } catch (...) {
            message = "non-standard exception";
        }
        if (!message.empty()) {
            detail = message.c_str();
        }
    }
    writeCrashReport("std::terminate", 0, nullptr, detail);
    std::abort();
}

// Which thread called abort(), and from where.
//
// abort() is the one ending that runs no exception handler: Windows reports the process as
// a fast fail (0xC0000409, subcode 7), so both the filter above and the terminate handler
// are skipped and crash.txt stays empty. abort() raises SIGABRT first, and a signal handler
// does run, so the report is written from here. Nothing below may allocate - it runs on the
// thread that is already ending the process.
void reportAbort(int) {
    char text[4096];
    const int headerLength = std::snprintf(
        text, sizeof(text),
        "abort (a fast fail: 0xC0000409, subcode 7)\r\nthread: %lu (main thread %lu)\r\n"
        "last stage: %s\r\nframes:\r\n",
        GetCurrentThreadId(), g_mainThreadId, g_lastStartupStage);

    size_t used = 0;
    if (headerLength > 0) {
        used = std::min(static_cast<size_t>(headerLength), sizeof(text) - 1);
    }

    void* frames[24] = {};
    const USHORT frameCount = CaptureStackBackTrace(0, 24, frames, nullptr);

    for (USHORT i = 0; i < frameCount && used + 128 < sizeof(text); ++i) {
        HMODULE module = nullptr;
        char moduleName[MAX_PATH] = "unknown";
        unsigned long long offset = 0;
        if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                                   GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                               static_cast<LPCSTR>(frames[i]), &module) != 0 &&
            module != nullptr) {
            if (GetModuleFileNameA(module, moduleName, sizeof(moduleName)) == 0) {
                std::snprintf(moduleName, sizeof(moduleName), "unknown");
            }
            offset = static_cast<unsigned long long>(
                static_cast<const char*>(frames[i]) - reinterpret_cast<const char*>(module));
        }
        const int length = std::snprintf(text + used, sizeof(text) - used, "  %s+0x%llX\r\n",
                                         moduleName, offset);
        if (length <= 0) {
            break;
        }
        used += std::min(static_cast<size_t>(length), sizeof(text) - used - 1);
    }

    appendToCrashFile(text);
}

void installCrashReporting() {
    g_previousExceptionFilter = SetUnhandledExceptionFilter(reportUnhandledException);
    std::set_terminate(reportTerminate);

    // The signal handler is the only hook a fast fail leaves: it runs before the process is
    // ended, where no exception handler gets a turn.
    g_mainThreadId = GetCurrentThreadId();
    std::signal(SIGABRT, reportAbort);
}

// Sends one stream to a log file with a share mode that lets anybody else read it.
//
// freopen_s() opens the file with the CRT's default sharing, and a client that is
// wedged holds that handle open: opening client_log.txt in an editor, or reading it
// from the website, then fails with "being used by another process" at exactly the
// moment the log is worth reading. _fsopen() takes the share mode, and _SH_DENYNO
// lets another process read the file while this one keeps writing it. The descriptor
// is put on the stream's own, so printf and iostream need no change. Best effort: on
// failure the stream keeps its current destination and the caller reports it.
bool redirectStreamToLog(const char* path, FILE* stream) {
    FILE* log = _fsopen(path, "w", _SH_DENYNO);
    if (log == nullptr) {
        return false;
    }
    const bool redirected = _dup2(_fileno(log), _fileno(stream)) == 0;
    fclose(log); // the duplicate stays open; this handle is only the way to the file
    return redirected;
}

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(linearmath::vec3(0.0f, 5.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

#include "linearstb_image.h"

// Texture Loader
unsigned int loadTexture(const char* path) {
    unsigned int textureID = 0;
    
    // Only asks whether the file can be read: linearstbi_load does the reading.
    // An ifstream is that question without the CRT's deprecated fopen.
    if (!std::ifstream(path, std::ios::binary)) {
        std::cout << "Texture file not found: " << path << std::endl;
        return 0;
    }

    int width, height, nrComponents;
    unsigned char *data = linearstbi_load(path, &width, &height, &nrComponents, 0);
    if (!data) {
        std::cout << "Texture failed to load at path: " << path << " - linearstbi_load returned NULL" << std::endl;
        return 0;
    }
    
    if (width == 0 || height == 0) {
        std::cout << "Texture has invalid size: " << width << "x" << height << std::endl;
        linearstbi_image_free(data);
        return 0;
    }
    
    std::cout << "Loading texture: " << path << " - Size: " << width << "x" << height << ", Components: " << nrComponents << std::endl;

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    GLenum format;
    unsigned char* finalData = data;
    bool converted = false;
    
    // Convert 2-component (grayscale + alpha) to RGBA for easier shader handling
    if (nrComponents == 2) {
        finalData = new unsigned char[width * height * 4];
        for (int i = 0; i < width * height; i++) {
            unsigned char gray = data[i * 2];
            unsigned char alpha = data[i * 2 + 1];
            // Convert grayscale to RGB (all channels same)
            finalData[i * 4] = gray;     // R
            finalData[i * 4 + 1] = gray; // G
            finalData[i * 4 + 2] = gray; // B
            finalData[i * 4 + 3] = alpha; // A
        }
        format = GL_RGBA;
        converted = true;
    } else if (nrComponents == 1)
        format = GL_RED;
    else if (nrComponents == 3)
        format = GL_RGB;
    else if (nrComponents == 4)
        format = GL_RGBA;
    else {
        std::cout << "Unsupported texture format: " << nrComponents << " components" << std::endl;
        linearstbi_image_free(data);
        glDeleteTextures(1, &textureID);
        return 0;
    }

    // Check for OpenGL errors before creating texture
    GLenum glError = glGetError();
    if (glError != GL_NO_ERROR) {
        std::cout << "OpenGL error before texture creation: " << glError << std::endl;
    }
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, finalData);
    
    glError = glGetError();
    if (glError != GL_NO_ERROR) {
        std::cout << "OpenGL error creating texture: " << glError << std::endl;
        linearstbi_image_free(data);
        if (converted) {
            delete[] finalData;
        }
        glDeleteTextures(1, &textureID);
        return 0;
    }
    
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Free original data
    linearstbi_image_free(data);
    // Free converted data if we created it
    if (converted) {
        delete[] finalData;
    }
    
    // Verify texture was created correctly
    GLint checkWidth = 0, checkHeight = 0;
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &checkWidth);
    glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &checkHeight);
    
    if (checkWidth != width || checkHeight != height) {
        std::cout << "WARNING: Texture size mismatch! Expected " << width << "x" << height 
                  << " but got " << checkWidth << "x" << checkHeight << std::endl;
    }

    return textureID;
}

// State
bool isCursorLocked = false; // Start unlocked for login
float deltaTime = 0.0f;
float lastFrame = 0.0f;
unsigned int faceTextureId = 0;
const std::string DEFAULT_FACE_ID = "classic";
const std::vector<std::pair<std::string, const char*>> FACE_TEXTURE_FILES = {
    {"classic", "face.png"},
    {"happy", "face-happy.png"},
    {"surprised", "face-surprised.png"},
    {"smirk", "face-smirk.png"},
    {"wink", "face-wink.png"}
};
std::map<std::string, unsigned int> faceTextureIds;

GameState currentState = MENU_LOADING;

// The saved sign-in is checked once the loading screen is on screen; this is the
// token that check will use and where it came from.
std::string pendingSessionToken;
bool pendingSessionFromArguments = false;

// Network State
SOCKET clientSocket = INVALID_SOCKET;
std::unique_ptr<linear::net::UdpSession> clientSession;
bool gameConnectionLost = false;
// Keeps a failing client from sending the same report on every frame.
bool clientProblemReported = false;
bool awaitingWelcome = false;
std::chrono::steady_clock::time_point welcomeDeadline;
int myPlayerId = -1;
bool isConnected = false;
char myUsername[32] = "Guest";
// Coins the signed-in account owns, as last reported by the website.
int accountCoins = 0;

// Health and the respawn timer belong to the server; these only mirror what its
// PLAYER_HEALTH packets said so the HUD can draw them and input can stop while
// the character is waiting to come back.
bool playerIsDead = false;
float respawnCountdown = 0.0f;

// Driving game state, mirrored from the server's VEHICLE_STATE packets.
int vehiclePartIndex = -1;
int vehicleDriverId = -1;
float vehicleSpeed = 0.0f;
char serverIP[32] = "127.0.0.1";
int serverPort = 7777;
char authToken[256] = "";
char loginUsername[32] = "";
char loginPassword[64] = "";
char webServerUrl[128] = "http://localhost:3000";
const char* DEFAULT_WEB_SERVER_URL = "http://localhost:3000";
const char* DEFAULT_GAME_SERVER_IP = "127.0.0.1";
bool showLoginError = false;
char loginErrorMsg[256] = "";
bool showConnectionUI = false; // Track if connection UI should be shown
bool autoConnectOnStart = false;
int launchGameId = 0;
bool showDebugGui = false;
bool escapeMenuOpen = false;
EscapeMenuTab escapeMenuTab = ESCAPE_TAB_PEOPLE;
bool shiftLockEnabled = false;
bool invertMouseY = false;
bool showChatHud = true;
bool showPlayerListHud = true;
bool waitingForWebLogin = false;
SOCKET authListenSocket = INVALID_SOCKET;
SOCKET authClientSocket = INVALID_SOCKET;
int authCallbackPort = 0;
std::string authCallbackBuffer;
char chatInput[256] = "";


std::map<int, Character*> remotePlayers;
std::map<int, std::string> playerNames;
std::map<int, int> playerScores;
std::vector<PlayerInfo> playerList;
std::vector<char> clientReceiveBuffer;
std::vector<ChatLine> chatLines;


std::vector<DiscoverGame> discoverGames;
bool discoverLoaded = false;
std::string discoverError;

Character* myCharacter = nullptr;
AvatarConfig currentAvatar;

bool isKnownFaceId(const std::string& faceId) {
    return faceId == "classic" ||
           faceId == "happy" ||
           faceId == "surprised" ||
           faceId == "smirk" ||
           faceId == "wink";
}

std::string normalizeFaceId(const std::string& faceId) {
    return isKnownFaceId(faceId) ? faceId : DEFAULT_FACE_ID;
}

unsigned int textureForFace(const std::string& faceId) {
    const std::string safeFaceId = normalizeFaceId(faceId);
    auto face = faceTextureIds.find(safeFaceId);
    if (face != faceTextureIds.end()) {
        return face->second;
    }

    auto defaultFace = faceTextureIds.find(DEFAULT_FACE_ID);
    return defaultFace != faceTextureIds.end() ? defaultFace->second : 0;
}

void loadFaceTextures() {
    faceTextureIds.clear();
    for (const auto& face : FACE_TEXTURE_FILES) {
        unsigned int textureId = loadTexture(face.second);
        if (textureId > 0) {
            faceTextureIds[face.first] = textureId;
            std::cout << "Face Texture Loaded: " << face.first << " (ID: " << textureId << ")" << std::endl;
        }
    }

    faceTextureId = textureForFace(DEFAULT_FACE_ID);
    if (faceTextureId == 0) {
        std::cout << "WARNING: Face textures failed to load! Check if face PNG files exist and are valid." << std::endl;
    }
}

std::string jsonStringField(const std::string& json, const std::string& name) {
    size_t pos = json.find("\"" + name + "\"");
    if (pos == std::string::npos) return {};

    pos = json.find(":", pos);
    if (pos == std::string::npos) return {};

    pos = json.find("\"", pos);
    if (pos == std::string::npos) return {};

    size_t end = json.find("\"", pos + 1);
    if (end == std::string::npos || end <= pos + 1) return {};

    return json.substr(pos + 1, end - pos - 1);
}

// Reads a numeric field ("coins":42) out of a small JSON response.
int jsonIntField(const std::string& json, const std::string& field) {
    const std::string needle = "\"" + field + "\":";
    const size_t pos = json.find(needle);
    if (pos == std::string::npos) return 0;
    return std::atoi(json.c_str() + pos + needle.size());
}

void copyToBuffer(char* destination, size_t destinationSize, const std::string& value) {
    if (!destination || destinationSize == 0) return;
    std::memset(destination, 0, destinationSize);
    std::memcpy(destination, value.c_str(), std::min(destinationSize - 1, value.size()));
}

std::string automaticWebServerUrl() {
    if (std::strlen(webServerUrl) == 0) {
        copyToBuffer(webServerUrl, sizeof(webServerUrl), DEFAULT_WEB_SERVER_URL);
    }
    return std::string(webServerUrl);
}

const char* automaticGameServerIp() {
    if (std::strlen(serverIP) == 0) {
        copyToBuffer(serverIP, sizeof(serverIP), DEFAULT_GAME_SERVER_IP);
    }
    return serverIP;
}

// Tells the website why this client could not play, so the reason shows up in
// the admin panel instead of only in client_log.txt on this machine. Best
// effort on purpose: a client that is already failing must not hang or crash
// because the website is slow, and the player still sees the message locally.
void reportClientProblem(const std::string& stage, const std::string& reason) {
    if (reason.empty()) {
        return;
    }

    const std::string body =
        "{\"gameId\":" + std::to_string(launchGameId) +
        ",\"stage\":\"" + Auth::jsonEscape(stage) +
        "\",\"reason\":\"" + Auth::jsonEscape(reason) +
        "\",\"serverAddress\":\"" +
        Auth::jsonEscape(std::string(automaticGameServerIp()) + ":" +
                         std::to_string(serverPort)) +
        "\"}";

    Auth::httpPost(automaticWebServerUrl() + "/api/client-reports", body, authToken);
}

#if defined(LINEAR_ANTICHEAT)
#include "ClientCheatReport.h"
#include "GuardStatus.h"
#include "HardwareId.h"
#include "SystemSnapshot.h"

namespace {
    std::unique_ptr<linear::anticheat::LogTail> detectionTail;
    std::vector<std::string> reportedDetections;
    std::chrono::steady_clock::time_point lastDetectionPoll =
        std::chrono::steady_clock::time_point::min();

    // The website answers a reported detection with what it cost. A restricted
    // answer means the session is over, which is posted back from a worker
    // thread and acted on by the render thread in the main loop.
    std::atomic<bool> punishmentPending{false};
    std::mutex punishmentMutex;
    std::string punishmentMessage;

    // Posts one filed detection with the snapshot that explains it. Two readers
    // produce detections - this process's log and the guard service's - so the
    // request body and the thread that carries it are written once.
    void postDetection(const std::string& flag, const std::string& filed,
                       const linear::anticheat::SnapshotSubject& subject) {
        const std::string url = automaticWebServerUrl() + "/api/client-detections";
        const std::string token = authToken;
        std::thread([url, flag, filed, subject, token]() {
            const std::string body =
                "{\"flag\":\"" + flag + "\",\"detail\":\"" + Auth::jsonEscape(filed) +
                "\",\"hardwareId\":\"" + Auth::jsonEscape(linear::hwid::machineId()) +
                "\",\"snapshot\":\"" +
                Auth::jsonEscape(linear::anticheat::systemSnapshot(
                    linear::anticheat::SnapshotLimits(), subject)) +
                "\"}";
            const std::string reply = Auth::httpPost(url, body, token);
            if (reply.empty()) {
                return;
            }
            std::string parseError;
            const linear::web::Json answer = linear::web::Json::parse(reply, &parseError);
            if (!answer.member("restricted").asBool(false)) {
                return;
            }
            // The account is suspended or banned now. The player is told why
            // and taken out of the game rather than left playing until the
            // token happens to expire.
            {
                std::lock_guard<std::mutex> lock(punishmentMutex);
                punishmentMessage = answer.member("message").asString();
            }
            punishmentPending = true;
        }).detach();
    }

    // What the guard service found, from the log it writes outside this process.
    // A check that runs there survives a client that has been frozen or patched,
    // so its lines are filed the same way and keep their own flag: the admin
    // console shows whether a detection came from inside the game or beside it.
    // The file not existing is the normal state of a machine without the service.
    void pollGuardDetections() {
        static std::unique_ptr<linear::anticheat::LogTail> guardTail;
        if (!guardTail) {
            guardTail = std::make_unique<linear::anticheat::LogTail>();
        }

        std::vector<std::string> lines;
        const std::string path = linear::guard::logPath();
        if (path.empty() || !guardTail->poll(path, &lines)) {
            return;
        }

        for (const std::string& line : lines) {
            const std::string detail = linear::anticheat::detectionDetail(line);
            if (detail.empty()) {
                continue;
            }

            // The service writes one line per finding, already deduplicated on
            // its side; the client still remembers it, so a service restarted
            // mid-session does not file the same tool twice.
            const std::string key = "guard\n" + detail;
            bool alreadySent = false;
            for (const std::string& sent : reportedDetections) {
                if (sent == key) {
                    alreadySent = true;
                    break;
                }
            }
            if (alreadySent || reportedDetections.size() >= 64) {
                continue;
            }
            reportedDetections.push_back(key);
            postDetection("guard", detail, linear::anticheat::subjectFromDetection(detail));
        }
    }

    // The anti-cheat library writes its detections to LinearAntiCheat.log and
    // nowhere else: its own transport is switched off, and it speaks a protocol
    // no server here hosts. This reads what it appended and files each detection
    // against the signed-in account, so tampering reaches an admin instead of
    // sitting on the cheater's own disk. It runs on the render thread but only
    // every few seconds, and the request itself is handed to another thread, so
    // a slow website cannot stall a frame.
    void pollAntiCheatDetections() {
        if (std::strlen(authToken) == 0) {
            // A guest has no account to file a detection against; the log is
            // still on disk for the player to read.
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        if (lastDetectionPoll != std::chrono::steady_clock::time_point::min() &&
            now - lastDetectionPoll < std::chrono::seconds(5)) {
            return;
        }
        lastDetectionPoll = now;

        // The service first: its lines come from outside this process, so a
        // client that has stopped its own checks still reports them.
        pollGuardDetections();

        if (!detectionTail) {
            detectionTail = std::make_unique<linear::anticheat::LogTail>();
        }

        std::vector<std::string> lines;
        if (!detectionTail->poll(LINEAR_OBF("LinearAntiCheat.log"), &lines)) {
            return;
        }

        for (const std::string& line : lines) {
            const std::string detail = linear::anticheat::detectionDetail(line);
            if (detail.empty()) {
                continue;
            }

            // WebApp.exe starts this client and keeps the handle it waits on, so
            // the library's handle scan reports the project's own launcher as an
            // intruder on a clean machine. That is a normal launch, not tampering.
            // Discord's overlay is loaded into the client without the player
            // asking for it and hooks the frame it presents; that is not cheating
            // and the account must not climb the ladder over it.
            if (linear::anticheat::namesOverlayModules(detail)) {
                std::cout << LINEAR_OBF("Ignored a detection about a Discord module.") << std::endl;
                continue;
            }

            if (linear::anticheat::namesOurOwnProcesses(detail)) {
                std::cout << LINEAR_OBF("Ignored a detection about this project's own processes.")
                          << std::endl;
                continue;
            }

            // One handle scan writes the handle, the process behind it and the
            // scan's summary as three detections. They are filed as the one event
            // they describe, so a single scan cannot climb the whole ladder.
            const std::string filed = linear::anticheat::canonicalDetection(detail);

            // The library repeats a detection every time its check runs, so the
            // same detection is only sent once. The guard's findings share the
            // list, so their keys are marked apart from these.
            const std::string key = "anticheat\n" + filed;
            bool alreadySent = false;
            for (const std::string& sent : reportedDetections) {
                if (sent == key) {
                    alreadySent = true;
                    break;
                }
            }
            if (alreadySent || reportedDetections.size() >= 64) {
                continue;
            }
            reportedDetections.push_back(key);

            // The detection is what fired; the snapshot is the machine it fired
            // on. They travel together so a reviewer reads the check's label
            // beside the modules and processes that explain it. What the line
            // named travels in the snapshot: `filed` has the family label the
            // ladder counts, and the raw line has the module or process that
            // tripped it, which is what a review acts on. The snapshot is built
            // on the worker thread: walking every process costs a frame and the
            // render thread is drawing.
            const linear::anticheat::SnapshotSubject subject =
                linear::anticheat::subjectFromDetection(detail);
            postDetection("anticheat", filed, subject);
        }
    }
}  // namespace
#endif

void addChatLine(const std::string& username, const std::string& message, bool system) {
    if (message.empty()) return;
    // The in-game chat is not drawn on the menu a failed join returns to, so a
    // server that explains itself would still look silent. Mirror every line to
    // the console that client_log.txt captures.
    std::cout << (system ? "[system] " : "[chat] ") << username << ": " << message << std::endl;
    chatLines.push_back({username, message, system});
    if (chatLines.size() > 80) {
        chatLines.erase(chatLines.begin(), chatLines.begin() + static_cast<std::ptrdiff_t>(chatLines.size() - 80));
    }
}

std::string urlEncode(const std::string& value) {
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex;
    for (unsigned char c : value) {
        if (std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            encoded << c;
        } else {
            encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(c);
            encoded << std::setfill(' ');
        }
    }
    return encoded.str();
}

std::string urlDecode(const std::string& value) {
    std::string decoded;
    for (size_t i = 0; i < value.size(); ++i) {
        if (value[i] == '+' ) {
            decoded.push_back(' ');
        } else if (value[i] == '%' && i + 2 < value.size()) {
            const std::string hex = value.substr(i + 1, 2);
            char* end = nullptr;
            long character = std::strtol(hex.c_str(), &end, 16);
            if (end && *end == '\0') {
                decoded.push_back(static_cast<char>(character));
                i += 2;
            } else {
                decoded.push_back(value[i]);
            }
        } else {
            decoded.push_back(value[i]);
        }
    }
    return decoded;
}

std::string queryParam(const std::string& request, const std::string& name) {
    const size_t requestStart = request.find("GET ");
    if (requestStart == std::string::npos) return {};
    const size_t pathStart = requestStart + 4;
    const size_t pathEnd = request.find(' ', pathStart);
    if (pathEnd == std::string::npos) return {};

    const std::string path = request.substr(pathStart, pathEnd - pathStart);
    const size_t queryStart = path.find('?');
    if (queryStart == std::string::npos) return {};

    const std::string query = path.substr(queryStart + 1);
    const std::string prefix = name + "=";
    size_t pos = 0;
    while (pos <= query.size()) {
        const size_t next = query.find('&', pos);
        const std::string pair = query.substr(pos, next == std::string::npos ? std::string::npos : next - pos);
        if (pair.rfind(prefix, 0) == 0) {
            return urlDecode(pair.substr(prefix.size()));
        }
        if (next == std::string::npos) break;
        pos = next + 1;
    }
    return {};
}

bool parseJsonColorArray(const std::string& json, const std::string& name, linearmath::vec3& color) {
    size_t pos = json.find("\"" + name + "\":[");
    if (pos == std::string::npos) return false;

    pos = json.find("[", pos);
    if (pos == std::string::npos) return false;

    size_t end = json.find("]", pos);
    if (end == std::string::npos || end <= pos) return false;

    std::string colorStr = json.substr(pos + 1, end - pos - 1);
    size_t comma1 = colorStr.find(",");
    size_t comma2 = colorStr.find(",", comma1 + 1);
    if (comma1 == std::string::npos || comma2 == std::string::npos) return false;

    try {
        color.r = std::stof(colorStr.substr(0, comma1));
        color.g = std::stof(colorStr.substr(comma1 + 1, comma2 - comma1 - 1));
        color.b = std::stof(colorStr.substr(comma2 + 1));
        return true;
    } catch (...) {
        return false;
    }
}

void fetchAvatarForToken(const std::string& token, const std::string& serverUrl) {
    std::string avatarResponse = Auth::getAvatar(token, serverUrl);
    if (!avatarResponse.empty() && avatarResponse.find("\"success\":true") != std::string::npos) {
        parseJsonColorArray(avatarResponse, "headColor", currentAvatar.headColor);
        parseJsonColorArray(avatarResponse, "torsoColor", currentAvatar.torsoColor);
        parseJsonColorArray(avatarResponse, "leftArmColor", currentAvatar.leftArmColor);
        parseJsonColorArray(avatarResponse, "rightArmColor", currentAvatar.rightArmColor);
        parseJsonColorArray(avatarResponse, "leftLegColor", currentAvatar.leftLegColor);
        parseJsonColorArray(avatarResponse, "rightLegColor", currentAvatar.rightLegColor);
        currentAvatar.faceId = normalizeFaceId(jsonStringField(avatarResponse, "faceId"));
        currentAvatar.hatId = jsonStringField(avatarResponse, "hatId");
        currentAvatar.saveToFile("avatar.txt");
    }
}

// Returns false when the token is unusable. `tokenRejected` separates a token
// the website refused (expired, signed with an old secret, account deleted) from
// a website that simply could not be reached: only the first one is a reason to
// sign the player out.
bool applyAuthSession(const std::string& token, const std::string& fallbackUsername, const std::string& serverUrl, bool saveToken, std::string& error, bool* tokenRejected = nullptr) {
    if (tokenRejected) {
        *tokenRejected = false;
    }
    if (token.empty()) {
        error = "No token came back from the website.";
        return false;
    }

    const linear::http::Response verify =
        linear::http::request("POST", serverUrl + "/api/verify", "{}", token);
    if (verify.status == 401 || verify.status == 403) {
        error = "The website no longer accepts this login, so it was signed out.";
        if (tokenRejected) {
            *tokenRejected = true;
        }
        return false;
    }
    if (!verify.ok()) {
        error = "Could not reach the website to check the login.";
        return false;
    }

    const std::string& verifyResponse = verify.body;
    if (verifyResponse.find("\"success\":true") == std::string::npos) {
        error = "The website login could not be verified.";
        return false;
    }

    const std::string verifiedUsername = jsonStringField(verifyResponse, "username");
    accountCoins = jsonIntField(verifyResponse, "coins");
    copyToBuffer(authToken, sizeof(authToken), token);
    copyToBuffer(myUsername, sizeof(myUsername), verifiedUsername.empty() ? fallbackUsername : verifiedUsername);

    if (saveToken) {
        std::ofstream tokenFile("auth_token.txt");
        if (tokenFile.is_open()) {
            tokenFile << token;
        }
    }

    fetchAvatarForToken(token, serverUrl);
    return true;
}

// Forgets a login the website refused: the saved token is deleted, the in-memory
// token is cleared and the player is put back on the sign-in screen instead of
// being left in the menu with a token that will fail on every request.
void signOutInvalidToken(const std::string& reason) {
    authToken[0] = '\0';
    accountCoins = 0;
    copyToBuffer(myUsername, sizeof(myUsername), "Guest");
    loginUsername[0] = '\0';
    loginPassword[0] = '\0';
    showLoginError = true;
    copyToBuffer(loginErrorMsg, sizeof(loginErrorMsg), reason);
    currentState = MENU_AUTH_CHOICE;
    showConnectionUI = false;
    std::remove("auth_token.txt");
    addChatLine("System", reason, true);
}

void closeAuthCallback() {
    if (authClientSocket != INVALID_SOCKET) {
        closesocket(authClientSocket);
        authClientSocket = INVALID_SOCKET;
    }
    if (authListenSocket != INVALID_SOCKET) {
        closesocket(authListenSocket);
        authListenSocket = INVALID_SOCKET;
    }
    waitingForWebLogin = false;
    authCallbackPort = 0;
    authCallbackBuffer.clear();
}

bool startAuthCallback(std::string& loginUrl, std::string& error) {
    closeAuthCallback();

    for (int port = 39170; port < 39190; ++port) {
        SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (s == INVALID_SOCKET) continue;

        sockaddr_in addr;
        std::memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_port = htons(static_cast<u_short>(port));
        inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

        if (bind(s, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) == SOCKET_ERROR) {
            closesocket(s);
            continue;
        }

        if (listen(s, 1) == SOCKET_ERROR) {
            closesocket(s);
            continue;
        }

        Network::setNonBlocking(s);
        authListenSocket = s;
        authCallbackPort = port;
        waitingForWebLogin = true;
        const std::string redirect = "http://127.0.0.1:" + std::to_string(port) + "/auth";
        loginUrl = automaticWebServerUrl() + "/login?client=player&redirect=" + urlEncode(redirect);
        return true;
    }

    error = "Could not open a local login callback port.";
    return false;
}

void sendAuthCallbackResponse(SOCKET socket, bool success) {
    const char* body = success
        ? "<!doctype html><title>Linear Player</title><h1>Signed in</h1><p>You can return to the player.</p>"
        : "<!doctype html><title>Linear Player</title><h1>Login failed</h1><p>Please return to the player and try again.</p>";
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nContent-Length: "
             << std::strlen(body) << "\r\nConnection: close\r\n\r\n" << body;
    const std::string payload = response.str();
    send(socket, payload.c_str(), static_cast<int>(payload.size()), 0);
}

void pollAuthCallback() {
    if (!waitingForWebLogin || authListenSocket == INVALID_SOCKET) return;

    if (authClientSocket == INVALID_SOCKET) {
        sockaddr_in clientAddr;
        SocketLength clientLen = sizeof(clientAddr);
        SOCKET accepted = accept(authListenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientLen);
        if (accepted == INVALID_SOCKET) return;
        Network::setNonBlocking(accepted);
        authClientSocket = accepted;
    }

    char buffer[2048];
    int received = recv(authClientSocket, buffer, sizeof(buffer), 0);
    if (received == SOCKET_ERROR) {
        const int error = WSAGetLastError();
        if (error == WSAEWOULDBLOCK) return;
        closeAuthCallback();
        return;
    }
    if (received <= 0) return;

    authCallbackBuffer.append(buffer, buffer + received);
    if (authCallbackBuffer.find("\r\n\r\n") == std::string::npos) return;

    std::string error;
    const std::string token = queryParam(authCallbackBuffer, "token");
    const std::string username = queryParam(authCallbackBuffer, "username");
    const bool success = applyAuthSession(token, username, automaticWebServerUrl(), true, error);
    sendAuthCallbackResponse(authClientSocket, success);
    if (success) {
        showLoginError = false;
        currentState = MENU_PLAY_CHOICE;
        showConnectionUI = false;
        addChatLine("System", "Signed in as " + std::string(myUsername) + ".", true);
    } else {
        showLoginError = true;
        copyToBuffer(loginErrorMsg, sizeof(loginErrorMsg), error);
    }
    closeAuthCallback();
}


std::map<int, NetworkTransform> targetTransforms;
std::map<int, NetworkTransform> startTransforms;
float interpolationTime = 0.1f; // 100ms buffer

std::string openFileDialog(LINEARWINwindow* window);









// Drains the socket into the session, appends every completed frame to the
// receive buffer, and returns how many bytes it added. Nothing is appended
// until a message is whole, so the parser below never sees half a packet.





void renderDebugWindow(const LinearUIIO& io) {
    if (!showDebugGui) return;

    LinearUI::Begin("Debug Info", &showDebugGui);
    LinearUI::Text("FPS: %.1f", io.Framerate);
    LinearUI::Separator();

    if (currentState == ONLINE || currentState == OFFLINE) {
        LinearUI::Text("=== CAMERA ===");
        LinearUI::Text("Position: (%.2f, %.2f, %.2f)", camera.Position.x, camera.Position.y, camera.Position.z);
        LinearUI::Text("Yaw: %.2f", camera.Yaw);
        LinearUI::Text("Pitch: %.2f", camera.Pitch);
        LinearUI::Separator();

        if (myCharacter) {
            LinearUI::Text("=== PLAYER (Local) ===");
            linearmath::vec3 pos = myCharacter->getPosition();
            LinearUI::Text("Position: (%.2f, %.2f, %.2f)", pos.x, pos.y, pos.z);
            LinearUI::Text("Internal Yaw: %.2f", myCharacter->debugCurrentYaw);
            LinearUI::Text("Target Yaw: %.2f", myCharacter->debugTargetYaw);
            LinearUI::Text("Move Dir: (%.2f, %.2f, %.2f)",
                myCharacter->debugMoveDir.x,
                myCharacter->debugMoveDir.y,
                myCharacter->debugMoveDir.z);

            if (currentState == ONLINE && isConnected) {
                LinearUI::Separator();
                LinearUI::Text("=== NETWORK ===");
                LinearUI::Text("Sending RotationY: %.2f", myCharacter->debugCurrentYaw);
                LinearUI::Text("Player ID: %d", myPlayerId);
            }

            if (!remotePlayers.empty()) {
                LinearUI::Separator();
                LinearUI::Text("=== REMOTE PLAYERS ===");
                for (auto& [id, remoteChar] : remotePlayers) {
                    LinearUI::Text("Player %d:", id);
                    linearmath::vec3 remotePos = remoteChar->getPosition();
                    LinearUI::Text("  Pos: (%.2f, %.2f, %.2f)", remotePos.x, remotePos.y, remotePos.z);
                    LinearUI::Text("  Yaw: %.2f", remoteChar->debugCurrentYaw);
                    LinearUI::Text("  Target: %.2f", remoteChar->debugTargetYaw);
                }
            }
        }
    }
    LinearUI::End();
}


// Reads the public game list straight from the website so the player can browse
// and join without being sent to a browser first.












// The pause menu: dim the game, release the mouse and offer the things
// you would expect from pressing Esc in a game - resume, reset, settings, leave.

// File picker for opening a world file.
//
// Only Windows has a native dialog here. Elsewhere the caller keeps whatever
// path was passed on the command line and the menu reports that there is no
// picker, which beats a silently empty result.
std::string openFileDialog(LINEARWINwindow* window) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = linearwinGetWin32Window(window);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "World Files\0*.world\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}

// The air of the world this client is in. The server sends it on join, because a
// client is only ever given parts and never the .world file they were read from.
// A density of 0 means the world has no fog.
linearmath::vec3 fogColor(0.0f);
float fogDensity = 0.0f;

void copyArgument(char* destination, size_t destinationSize, const char* value) {
    if (!destination || destinationSize == 0 || !value) return;
    // snprintf always terminates, and strncpy is deprecated as C4996 on Windows.
    std::snprintf(destination, destinationSize, "%s", value);
}

void parseClientArguments(int argc, char** argv) {
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i] ? argv[i] : "";
        if (arg == "--server" && i + 1 < argc) {
            copyArgument(serverIP, sizeof(serverIP), argv[++i]);
        } else if (arg == "--port" && i + 1 < argc) {
            int parsed = std::atoi(argv[++i]);
            if (parsed > 0 && parsed <= 65535) {
                serverPort = parsed;
            }
        } else if (arg == "--token" && i + 1 < argc) {
            copyArgument(authToken, sizeof(authToken), argv[++i]);
            currentState = MENU_PLAY_CHOICE;
        } else if (arg == "--web" && i + 1 < argc) {
            copyArgument(webServerUrl, sizeof(webServerUrl), argv[++i]);
        } else if (arg == "--game" && i + 1 < argc) {
            launchGameId = std::max(0, std::atoi(argv[++i]));
        } else if (arg == "--connect") {
            autoConnectOnStart = true;
            currentState = MENU_PLAY_CHOICE;
        }
    }
}

bool connectToConfiguredServer() {
    clientSocket = Network::createUdpSocket();
    if (clientSocket == INVALID_SOCKET) {
        showLoginError = true;
        std::snprintf(loginErrorMsg, sizeof(loginErrorMsg), "%s", "Could not create network socket.");
        return false;
    }

    sockaddr_in serverAddr;
    std::memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) != 1) {
        showLoginError = true;
        std::snprintf(loginErrorMsg, sizeof(loginErrorMsg), "%s", "Invalid server address.");
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        return false;
    }
    serverAddr.sin_port = htons(static_cast<u_short>(serverPort));

    if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        showLoginError = true;
        std::snprintf(loginErrorMsg, sizeof(loginErrorMsg), "%s", "Connection failed.");
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        return false;
    }

    isConnected = true;
    currentState = ONLINE;
    showConnectionUI = false;
    Network::setNonBlocking(clientSocket);
    clientSession = std::make_unique<linear::net::UdpSession>();

    PacketConnect pkt;
    std::memset(&pkt, 0, sizeof(pkt));
    pkt.protocolRevision = PacketProtocol::kProtocolRevision;
    pkt.packetLayout = PacketProtocol::packetLayoutSignature();

    if (std::strlen(authToken) > 0) {
        std::snprintf(pkt.username, sizeof(pkt.username), "%s", myUsername);
        std::snprintf(pkt.authToken, sizeof(pkt.authToken), "%s", authToken);
    } else {
        std::snprintf(pkt.username, sizeof(pkt.username), "%s", "Guest");
    }

    if (!sendClientPacket(PacketProtocol::buildStructPacket(PacketType::CONNECT, pkt), true)) {
        showLoginError = true;
        std::snprintf(loginErrorMsg, sizeof(loginErrorMsg), "%s", "Failed to send connect packet.");
        isConnected = false;
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
        currentState = MENU_PLAY_CHOICE;
        showConnectionUI = true;
        return false;
    }

    awaitingWelcome = true;
    welcomeDeadline = std::chrono::steady_clock::now() +
                      std::chrono::milliseconds(static_cast<long long>(kGameConnectionTimeoutMs));

    std::cout << "Connected to " << serverIP << ":" << serverPort;
    if (launchGameId > 0) {
        std::cout << " for game " << launchGameId;
    }
    std::cout << ". Waiting for world state from server..." << std::endl;
    return true;
}

// The cursor mode is not touched here: applyCursorCapture() owns it, from the state
// the player is in, so shift lock and a held right button both arrive with isCursorLocked
// already set and neither can be undone between two mouse moves.
void mouse_callback(LINEARWINwindow*, double xposIn, double yposIn) {
    if (escapeMenuOpen || LinearUI::GetIO().WantCaptureMouse) return;
    if (!isCursorLocked) return; // Looking around is captured: not a free cursor

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 
    if (invertMouseY) {
        yoffset = -yoffset;
    }

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Runs on the first frame after the loading screen is up. A rejected token is
// signed out; a website that cannot be reached keeps the token but still lands on
// the sign-in page, which is the screen that can ask again.
void restorePendingSession() {
    const std::string token = pendingSessionToken;
    const bool fromArguments = pendingSessionFromArguments;
    pendingSessionToken.clear();
    pendingSessionFromArguments = false;

    std::string authError;
    bool tokenRejected = false;
    if (applyAuthSession(token, "Player", automaticWebServerUrl(), false, authError,
                         &tokenRejected)) {
        currentState = MENU_PLAY_CHOICE;
        return;
    }
    if (tokenRejected) {
        signOutInvalidToken(authError);
        return;
    }
    if (!fromArguments) {
        authToken[0] = '\0';
    }
    currentState = MENU_AUTH_CHOICE;
    addChatLine("System", authError, true);
}

int main(int argc, char** argv) {
    // Before anything is redirected, so this reaches the console even if the log cannot be
    // opened: a run that prints nothing at all stopped with the loader or in a static
    // initialiser rather than in anything the client does. sayToConsole() writes to the
    // console's own handle as well, so a redirected stdout does not swallow it.
    traceStartup("2/10 main entered");
    sayToConsole("Client starting.");

    // Redirect stdout/stderr to files for debugging. These are best effort: if the
    // file cannot be opened the console stays the destination and the run continues,
    // which is why the result is read. The files are shared so they can be read while
    // the client is still running; see redirectStreamToLog.
    if (!redirectStreamToLog("client_log.txt", stdout))
        std::cout << "warning: could not redirect stdout to client_log.txt" << std::endl;
    if (!redirectStreamToLog("client_error_log.txt", stderr))
        std::cerr << "warning: could not redirect stderr to client_error_log.txt" << std::endl;
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    // After the redirect, so the report and the log land in the same folder, and
    // before the anti-cheat, so it is in place for the whole run.
    installCrashReporting();

#if defined(LINEAR_ANTICHEAT)
    sayToConsole("Starting the anti-cheat...");
    traceStartup("3/10 starting the anti-cheat");
    startAntiCheat();
    traceStartup("4/10 the anti-cheat returned");
    sayToConsole("Anti-cheat started.");
#endif

    try {
        sayToConsole("Starting Client...");
        
        if (!std::ifstream("shaders/vertex.glsl")) {
            const std::string reason =
                "CRITICAL ERROR: shaders/vertex.glsl is not in the client's working "
                "directory (" + workingDirectory() +
                "). Keep the 'shaders' folder next to the executable, and extract the "
                "package before running it rather than opening Client.exe from inside a "
                ".zip.";
            reportClientProblem("startup", reason);
            pauseBeforeExit(reason);
            return -1;
        }

        Window window(SCR_WIDTH, SCR_HEIGHT, "Linear Client");
        sayToConsole("Window Created.");
        // The window and its OpenGL context take the video driver, which can be the
        // slowest thing in a cold start. A trace that ends on the line above stopped
        // before there was a window at all; one that ends here stopped in the setup
        // below, which is a different bug.
        traceStartup("5/10 window and OpenGL context created");

        // A machine with no output device still has a game to play, so audio
        // failing is a line in the log and silence, never a reason to stop.
        std::string audioError;
        if (linear::audio::init(&audioError)) {
            std::cout << "Audio ready: " << linear::audio::kSampleRate << " Hz mono" << std::endl;
        } else {
            std::cout << "Audio unavailable: " << audioError << std::endl;
        }
        linearwinSetCursorPosCallback(window.getNativeWindow(), mouse_callback);
        
        // Setup LinearUI Standard Init
        LINEARUI_CHECKVERSION();
        LinearUI::CreateContext();
        LinearUIIO& io = LinearUI::GetIO(); (void)io;
        applyModernPlayerStyle();
        const float uiScale = UiScale::Apply(window.getNativeWindow());
        LinearUI_ImplLinearWin_InitForOpenGL(window.getNativeWindow(), true);
        LinearUI_ImplOpenGL3_Init("#version 330");
        sayToConsole("LinearUI Initialized.");
        traceStartup("6/10 audio and UI ready");

        Renderer renderer;
        renderer.init();
        sayToConsole("Renderer Initialized.");
        // The shaders are compiled here, in the driver, and one the driver refuses
        // can take as long as a driver decision does.
        traceStartup("7/10 renderer and shaders ready");

        // Skybox skybox; 
        // skybox.init();
        Skybox skybox;
        skybox.init();
        std::cout << "Skybox Initialized." << std::endl;

        PhysicsWorld physicsWorld;
        physicsWorld.init();
        std::cout << "PhysicsWorld Initialized." << std::endl;
        
        ShadowMap shadowMap;
        shadowMap.init();
        std::cout << "ShadowMap Initialized." << std::endl;
        traceStartup("8/10 sky, physics and shadow map ready");

        std::deque<Part> parts;

        // Server part index -> index in `parts`. The two are the same number
        // until a character appends its own parts to this deque, so they drift
        // apart right after the join. A part the script builds later still has
        // to land where the renderer and the physics world can find it.
        std::vector<int> serverPartToClient;

        // One place that turns a PartData packet into the part this client
        // renders, so a snapshot entry and a part built later cannot disagree.
        const auto partFromData = [](const PartData& data) {
            Part p;
            p.shape = (ShapeType)data.shape;
            p.name = std::string(data.name);
            p.parentIndex = data.parentIndex;
            p.isFolder = (data.flags & 1) != 0;
            p.isCamera = (data.flags & 2) != 0;
            p.isSpawn = (data.flags & 4) != 0;
            p.deleted = (data.flags & 8) != 0;
            p.position = data.position;
            p.size = data.size;
            p.color = data.color;
            p.rotation = data.rotation;
            p.transparency = data.transparency;
            p.reflectance = data.reflectance;
            p.anchored = (data.physicsFlags & 1) != 0;
            p.canCollide = (data.physicsFlags & 2) != 0;
            p.mass = data.mass;
            p.material = data.material < kMaterialCount
                             ? static_cast<Material>(data.material)
                             : Material::Plastic;
            p.texture = PacketProtocol::fixedString(data.texture, sizeof(data.texture));
            return p;
        };

        traceStartup("starting Winsock");
        Network::init();
        std::cout << "Network Initialized." << std::endl;
        traceStartup("Winsock ready");
        
        // Load saved token on startup
        std::string savedToken;
        std::ifstream tokenFile("auth_token.txt");
        if (tokenFile.is_open()) {
            std::getline(tokenFile, savedToken);
            tokenFile.close();
        }

        parseClientArguments(argc, argv);
        const bool hasLaunchToken = std::strlen(authToken) > 0;

        // Load saved avatar (will be fetched from server when logged in)
        currentAvatar.loadFromFile("avatar.txt");

        // Verifying a saved sign-in is an HTTP call that may use its whole
        // timeout, and a client that shows nothing until it answers looks hung.
        // The check waits for the first loading frame instead, so the window and
        // the word "Loading" are on screen while it runs.
        if (hasLaunchToken || !savedToken.empty()) {
            pendingSessionToken = hasLaunchToken ? std::string(authToken) : savedToken;
            pendingSessionFromArguments = hasLaunchToken;
        } else {
            // Nothing saved to check, so there is nothing to wait for.
            currentState = MENU_AUTH_CHOICE;
        }

        traceStartup("loading face textures");
        loadFaceTextures();
        traceStartup("face textures loaded");

        if (autoConnectOnStart) {
            traceStartup("sending join request");
            std::string connectError;
            if (!connectToServer(connectError)) {
                showLoginError = true;
                copyToBuffer(loginErrorMsg, sizeof(loginErrorMsg), connectError);
            }
        }

        traceStartup(autoConnectOnStart ? "9/10 join request sent, still to get a window frame"
                                        : "9/10 client settings loaded");

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        std::cout << "Starting Main Loop..." << std::endl;
        traceStartup("10/10 entering the main loop");

        int frameCount = 0;
        while (!window.shouldClose()) {
            // 1. Poll Events
            window.pollEvents();
            applyCursorCapture();

            // Windows draws a window whose thread has stopped pumping its messages as
            // "Not Responding", and it never says where the thread went. The trace says
            // whether the loop itself is still running, which is what tells a stop
            // during startup from one in a later frame. Bounded to the first twenty
            // seconds so a long game session does not keep growing the file.
            static int loopTraceCount = 0;
            static auto lastLoopTrace = std::chrono::steady_clock::now();
            if (loopTraceCount < 20 &&
                std::chrono::steady_clock::now() - lastLoopTrace >= std::chrono::seconds(1)) {
                lastLoopTrace = std::chrono::steady_clock::now();
                ++loopTraceCount;
                traceStartup("loop alive after " + std::to_string(loopTraceCount) + " s");
            }

            float currentFrame = static_cast<float>(linearwinGetTime());
            deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            static bool escapeWasDown = false;
            const bool escapeDown = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_ESCAPE) == LINEARWIN_PRESS;
            if (escapeDown && !escapeWasDown) {
                const bool inGame = currentState == ONLINE || currentState == OFFLINE;
                if (inGame && escapeMenuOpen) {
                    closeEscapeMenu();
                } else if (inGame) {
                    openEscapeMenu();
                }
            }
            escapeWasDown = escapeDown;

            // One pass of the loop has drawn and presented the loading screen, so
            // the check that can block for seconds may run now.
            static int loadingFrames = 0;
            if (currentState == MENU_LOADING && !pendingSessionToken.empty()) {
                if (loadingFrames++ >= 1) restorePendingSession();
            }

            // 2. LinearUI NewFrame
            LinearUI_ImplOpenGL3_NewFrame();
            LinearUI_ImplLinearWin_NewFrame();
            LinearUI::NewFrame();

            if (frameCount < 5) std::cout << "Frame " << frameCount << " LinearUI NewFrame" << std::endl;

            // 3. Logic / UI
            pollAuthCallback();
#if defined(LINEAR_ANTICHEAT)
            // Anti-cheat detections are filed against the signed-in account, so
            // tampering shows up on the site and not only in a local log.
            pollAntiCheatDetections();

            // A detection the website acted on ends the session here, so an
            // account that has just been suspended or banned is not left playing.
            if (punishmentPending.exchange(false)) {
                std::string reason;
                {
                    std::lock_guard<std::mutex> lock(punishmentMutex);
                    reason = punishmentMessage;
                }
                if (reason.empty()) {
                    reason = "The anti-cheat detected tampering, so this session ended.";
                }
                disconnectFromServerOnly();
                currentState = MENU_AUTH_CHOICE;
                addChatLine("System", reason, true);
            }
#endif

            static bool wasDebugToggleDown = false;
            const bool debugToggleDown = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_F3) == LINEARWIN_PRESS;
            if (debugToggleDown && !wasDebugToggleDown) {
                showDebugGui = !showDebugGui;
            }
            wasDebugToggleDown = debugToggleDown;

            static bool shiftWasDown = false;
            const bool shiftDown = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_LEFT_SHIFT) == LINEARWIN_PRESS ||
                                   linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_RIGHT_SHIFT) == LINEARWIN_PRESS;
            if (shiftDown && !shiftWasDown && !escapeMenuOpen && !io.WantCaptureKeyboard &&
                (currentState == ONLINE || currentState == OFFLINE)) {
                setShiftLockEnabled(!shiftLockEnabled);
            }
            shiftWasDown = shiftDown;

            renderDebugWindow(io);
            if (currentState == MENU_AUTH_CHOICE || currentState == MENU) {
                renderAuthMenu(uiScale);
            } else if (currentState == MENU_PLAY_CHOICE) {
                renderPlayMenu(uiScale);
            } else if (currentState == MENU_GAMES) {
                renderGamesMenu(uiScale);
            } else if (currentState == MENU_LOADING) {
                renderLoadingScreen(uiScale);
            }
            if (currentState != ONLINE && currentState != OFFLINE) {
                escapeMenuOpen = false;
                escapeMenuTab = ESCAPE_TAB_PEOPLE;
            }
            renderGameHud(parts, physicsWorld, uiScale);
            renderEscapeMenu(parts, physicsWorld, uiScale);

            if (currentState != ONLINE && currentState != OFFLINE) {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glDrawBuffer(GL_BACK);
                glReadBuffer(GL_BACK);
                glViewport(0, 0, window.getWidth(), window.getHeight());
                glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                LinearUI::Render();
                LinearDrawData* drawData = LinearUI::GetDrawData();
                if (drawData) {
                    LinearUI_ImplOpenGL3_RenderDrawData(drawData);
                }
                window.swapBuffers();
                frameCount++;
                continue;
            }

            // ... Network & Physics Logic (Keep logic running)
            // Footsteps are the client's own: the server has no reason to know a
            // character is walking, and a packet would put them a round trip
            // behind the feet.
            if (myCharacter != nullptr) {
                // The jump is an edge, not a state: isJumping stays true for the
                // whole hop, and a sound per frame would be a drone.
                static bool wasJumping = false;
                if (myCharacter->isJumping && !wasJumping) {
                    if (const linear::audio::Clip* jump =
                            linear::audio::sound(linear::audio::SoundId::Jump)) {
                        linear::audio::play(*jump, 0.4f, false);
                    }
                }
                wasJumping = myCharacter->isJumping;
            }
            if (myCharacter != nullptr && !myCharacter->isJumping &&
                linearmath::length(myCharacter->debugMoveDir) > 0.1f) {
                static double lastFootstep = 0.0;
                const double nowSeconds = linearwinGetTime();
                if (nowSeconds - lastFootstep >= 0.42) {
                    lastFootstep = nowSeconds;
                    if (const linear::audio::Clip* step =
                            linear::audio::sound(linear::audio::SoundId::Footstep)) {
                        linear::audio::play(*step, 0.35f, false);
                    }
                }
            }
            if (isConnected && clientSocket != INVALID_SOCKET) {
                int bytesReceived = receiveGameDatagrams();
                
                if (bytesReceived > 0) {
                    std::vector<char> completeFrames;
                    while (true) {
                        PacketProtocol::ParsedPacket frame;
                        PacketProtocol::ParseStatus status = PacketProtocol::tryReadFrame(clientReceiveBuffer, frame);
                        if (status == PacketProtocol::ParseStatus::NeedMoreData) {
                            break;
                        }
                        if (status == PacketProtocol::ParseStatus::Invalid) {
                            std::cerr << "Disconnected after receiving malformed packet from server" << std::endl;
                            isConnected = false;
                            closesocket(clientSocket);
                            clientSocket = INVALID_SOCKET;
                            currentState = MENU_AUTH_CHOICE;
                            showConnectionUI = false;
                            clientReceiveBuffer.clear();
                            break;
                        }

                        completeFrames.insert(
                            completeFrames.end(),
                            clientReceiveBuffer.begin(),
                            clientReceiveBuffer.begin() + static_cast<std::ptrdiff_t>(frame.frameSize)
                        );
                        PacketProtocol::consumeFrame(clientReceiveBuffer, frame.frameSize);
                    }

                    char* packetBuffer = completeFrames.empty() ? nullptr : completeFrames.data();
                    int bytesToProcess = static_cast<int>(completeFrames.size());

                    // Limit logging to prevent spam
                    static int logCounter = 0;
                    if (logCounter++ % 60 == 0) {
                        std::cout << "Recv bytes: " << bytesReceived << std::endl;
                    }
                    int offset = 0;
                    while (offset < bytesToProcess) {
                        // Both sides signed: sizeof is compared as the int it fits in,
                        // so the check raises no signed/unsigned warning of its own.
                        if (offset + static_cast<int>(sizeof(PacketHeader)) > bytesToProcess)
                            break; // Safety check
                        
                        PacketHeader* header = (PacketHeader*)(packetBuffer + offset);
                        uint32_t packetSize = sizeof(PacketHeader) + header->size;
                        
                        // Safety check: ensure entire packet fits in buffer
                        if (packetSize > static_cast<uint32_t>(bytesToProcess - offset)) {
                            std::cerr << "Warning: Incomplete packet, waiting for more data" << std::endl;
                            break; // Wait for more data
                        }
                        
                        // Limit logging to prevent spam
                        if (logCounter % 60 == 0) {
                            std::cout << "Packet Type: " << (int)header->type << " Size: " << header->size << std::endl;
                        }

                        if (header->type == PacketType::WELCOME) {
                            if (header->size < sizeof(PacketWelcome)) {
                                // A server built before the handshake carried a fingerprint
                                // sends the shorter, older welcome. Left alone this looks
                                // like ten seconds of silence and then a silent trip back
                                // to the menu, so say what actually happened.
                                std::cerr << "The game server sent a " << header->size
                                          << "-byte welcome but this client expects "
                                          << sizeof(PacketWelcome)
                                          << ". Rebuild Server.exe and Client.exe from the same source."
                                          << std::endl;
                                reportClientProblem(
                                    "join",
                                    "The game server answered with an older handshake, so the "
                                    "server binary was built from different source than this "
                                    "client. Rebuild both from the same source.");
                                addChatLine("System", "That game server is an older build. Rebuild the server and client together.", true);
                                awaitingWelcome = false;
                                disconnectFromServerOnly();
                                currentState = MENU_AUTH_CHOICE;
                                offset += packetSize;
                                continue;
                            }
                            PacketWelcome* pkt = (PacketWelcome*)(packetBuffer + offset + sizeof(PacketHeader));
                             if (pkt->protocolRevision != PacketProtocol::kProtocolRevision ||
                                 pkt->packetLayout != PacketProtocol::packetLayoutSignature()) {
                                 // A build from another revision parses every later packet
                                 // as garbage, so stop here and say why instead.
                                 std::cerr << "Protocol mismatch: the server speaks "
                                           << pkt->protocolRevision << "/" << pkt->packetLayout
                                           << ", this client speaks "
                                           << PacketProtocol::kProtocolRevision << "/"
                                           << PacketProtocol::packetLayoutSignature()
                                           << ". Rebuild Client.exe and Server.exe from the same source."
                                           << std::endl;
                                 addChatLine("System", "The server was built from a different version. Rebuild the client and server together.", true);
                                 awaitingWelcome = false;
                                 disconnectFromServerOnly();
                                 currentState = MENU_PLAY_CHOICE;
                                 offset += packetSize;
                                 continue;
                             }
                             myPlayerId = pkt->playerId;
                             awaitingWelcome = false;
                             std::cout << "Joined with ID: " << myPlayerId << std::endl;
                             
                             // Don't create character yet - wait for WORLD_STATE to load all parts first
                             // Character will be created after WORLD_STATE is received.
                             // The cursor stays free until the player looks around or
                             // turns shift lock on; applyCursorCapture() decides that.
                        }
                        else if (header->type == PacketType::SOUND) {
                            if (header->size < sizeof(PacketSound)) {
                                std::cerr << "ERROR: Invalid SOUND packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketSound* pkt =
                                (PacketSound*)(packetBuffer + offset + sizeof(PacketHeader));
                            const std::string name =
                                PacketProtocol::fixedString(pkt->name, sizeof(pkt->name));
                            const linear::audio::Clip* clip = linear::audio::soundByName(name);
                            if (clip == nullptr) {
                                // A server built from other source can ask for a
                                // sound this client does not have; silence and a
                                // line is the whole of the disagreement.
                                std::cerr << "Ignoring unknown sound '" << name << "'"
                                          << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            float volume = pkt->volume;
                            float pan = 0.0f;
                            if (pkt->spatial != 0 && myCharacter != nullptr) {
                                const linearmath::vec3 offset =
                                    pkt->position - myCharacter->getPosition();
                                const float distance = linearmath::length(offset);
                                volume *= linear::audio::attenuation(distance);
                                // Which ear it is nearer is decided by the view,
                                // not by the body: the player's ears turn with the
                                // camera, so a sound behind the head lands on the
                                // side it would really be heard on. A sound at the
                                // ear itself has no side to be on.
                                if (distance > 0.001f) {
                                    const linearmath::vec3 direction =
                                        offset * (1.0f / distance);
                                    pan = std::clamp(linearmath::dot(direction, camera.Right),
                                                     -1.0f, 1.0f);
                                }
                            }
                            linear::audio::play(*clip, volume, false, pan);
                        }
                        else if (header->type == PacketType::PLAYER_SCORE) {
                            if (header->size < sizeof(PacketPlayerScore)) {
                                std::cerr << "ERROR: Invalid PLAYER_SCORE packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketPlayerScore* pkt =
                                (PacketPlayerScore*)(packetBuffer + offset + sizeof(PacketHeader));
                            // The server owns the score; this only remembers what
                            // it was last told, for the player list.
                            playerScores[pkt->playerId] = pkt->score;
                        }
                        else if (header->type == PacketType::TELEPORT) {
                            if (header->size < sizeof(PacketPlayerState)) {
                                std::cerr << "ERROR: Invalid TELEPORT packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketPlayerState* pkt = (PacketPlayerState*)(packetBuffer + offset + sizeof(PacketHeader));
                            if (pkt->playerId == myPlayerId && myCharacter) {
                                // A correction moves the character, not the player's
                                // hands: the cursor stays captured through it.
                                myCharacter->teleportTo(pkt->position);
                            }
                        }
                        // ... (rest of packet handlers)
                        else if (header->type == PacketType::PLAYER_JOIN) {
                            if (header->size < sizeof(PacketPlayerJoin)) {
                                std::cerr << "ERROR: Invalid PLAYER_JOIN packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketPlayerJoin* pkt = (PacketPlayerJoin*)(packetBuffer + offset + sizeof(PacketHeader));
                            
                            // Validate player ID
                            if (pkt->playerId < 0 || pkt->playerId > 1000) {
                                std::cerr << "ERROR: Invalid player ID in PLAYER_JOIN: " << pkt->playerId << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            
                            if (pkt->playerId != myPlayerId) {
                                std::cout << "Remote Player Joined: " << pkt->username << " (ID: " << pkt->playerId << ")" << std::endl;
                                playerNames[pkt->playerId] = pkt->username;
                                
                                // Only create if doesn't exist (prevent duplicate/crash)
                                if (remotePlayers.count(pkt->playerId) == 0) {
                                    // Create Remote Character with error handling
                                    linearmath::vec3 spawnPos(0, 10, 0);
                                    for(const auto& p : parts) { 
                                        if(p.isSpawn) {
                                            spawnPos = p.position + linearmath::vec3(0, 5, 0);
                                            break;
                                        }
                                    }
                                    
                                    std::cout << "Creating remote character for player " << pkt->playerId << std::endl;
                                    
                                    // Safety check: ensure parts list is valid
                                    if (parts.empty()) {
                                        std::cerr << "ERROR: Parts list is empty, cannot create character" << std::endl;
                                        offset += packetSize;
                                        continue;
                                    }
                                    
                                    // Default avatar colors
                                    linearmath::vec3 headColor(0.8f, 0.6f, 0.4f);
                                    linearmath::vec3 torsoColor(0.2f, 0.4f, 0.8f);
                                    linearmath::vec3 leftArmColor(0.8f, 0.6f, 0.4f);
                                    linearmath::vec3 rightArmColor(0.8f, 0.6f, 0.4f);
                                    linearmath::vec3 leftLegColor(0.2f, 0.6f, 0.2f);
                                    linearmath::vec3 rightLegColor(0.2f, 0.6f, 0.2f);
                                    std::string faceId = DEFAULT_FACE_ID;
                                    std::string hatId;
                                    
                                    // Find player in playerList for avatar colors
                                    bool foundAvatar = false;
                                    for (const auto& playerInfo : playerList) {
                                        if (playerInfo.playerId == pkt->playerId) {
                                            headColor = linearmath::vec3(playerInfo.headColor[0], playerInfo.headColor[1], playerInfo.headColor[2]);
                                            torsoColor = linearmath::vec3(playerInfo.torsoColor[0], playerInfo.torsoColor[1], playerInfo.torsoColor[2]);
                                            leftArmColor = linearmath::vec3(playerInfo.leftArmColor[0], playerInfo.leftArmColor[1], playerInfo.leftArmColor[2]);
                                            rightArmColor = linearmath::vec3(playerInfo.rightArmColor[0], playerInfo.rightArmColor[1], playerInfo.rightArmColor[2]);
                                            leftLegColor = linearmath::vec3(playerInfo.leftLegColor[0], playerInfo.leftLegColor[1], playerInfo.leftLegColor[2]);
                                            rightLegColor = linearmath::vec3(playerInfo.rightLegColor[0], playerInfo.rightLegColor[1], playerInfo.rightLegColor[2]);
                                            faceId = normalizeFaceId(PacketProtocol::fixedString(playerInfo.faceId, sizeof(playerInfo.faceId)));
                                            hatId = PacketProtocol::fixedString(playerInfo.hatId, sizeof(playerInfo.hatId));
                                            foundAvatar = true;
                                            break;
                                        }
                                    }
                                    
                                    // If avatar not found in player list, use defaults (will be updated when player list arrives)
                                    if (!foundAvatar) {
                                        std::cout << "Avatar data not found for player " << pkt->playerId << ", using defaults (will update when player list arrives)" << std::endl;
                                    }
                                    
                                    // Double-check parts list is still valid and not empty
                                    if (parts.empty()) {
                                        std::cerr << "ERROR: Parts list became empty before character creation for player " << pkt->playerId << std::endl;
                                        offset += packetSize;
                                        continue;
                                    }
                                    
                                    Character* remoteChar = nullptr;
                                    try {
                                        remoteChar = new Character(spawnPos, &physicsWorld, &parts,
                                            headColor, torsoColor, leftArmColor, rightArmColor, leftLegColor, rightLegColor);
                                        
                                        if (remoteChar) {
                                            remoteChar->setFaceTexture(textureForFace(faceId));
                                            // The hat is drawn from the same catalog as our
                                            // own, and an unknown id draws nothing.
                                            remoteChar->setHat(hatId);
                                            remoteChar->setRemoteControlled(true);
                                            remotePlayers[pkt->playerId] = remoteChar;
                                            std::cout << "Successfully created remote character for player " << pkt->playerId << " with avatar colors" << std::endl;
                                        } else {
                                            std::cerr << "ERROR: Character creation returned null for player " << pkt->playerId << std::endl;
                                        }
                                    } catch (const std::bad_alloc& e) {
                                        std::cerr << "ERROR: Memory allocation failed creating remote character for player " << pkt->playerId << ": " << e.what() << std::endl;
                                    } catch (const std::exception& e) {
                                        std::cerr << "ERROR: Exception creating remote character for player " << pkt->playerId << ": " << e.what() << std::endl;
                                    } catch (...) {
                                        std::cerr << "ERROR: Unknown exception creating remote character for player " << pkt->playerId << std::endl;
                                    }
                                } else {
                                    std::cout << "Warning: Remote character for player " << pkt->playerId << " already exists, skipping creation" << std::endl;
                                }
                            } else {
                                std::cout << "Received PLAYER_JOIN for myself (ID: " << pkt->playerId << "), ignoring" << std::endl;
                            }
                        }
                        else if (header->type == PacketType::PLAYER_LEAVE) {
                            if (header->size < sizeof(PacketPlayerLeave)) {
                                std::cerr << "ERROR: Invalid PLAYER_LEAVE packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketPlayerLeave* pkt = (PacketPlayerLeave*)(packetBuffer + offset + sizeof(PacketHeader));
                            
                            // Safety check: don't remove local player's character (shouldn't be in remotePlayers anyway)
                            if (pkt->playerId == myPlayerId) {
                                std::cout << "Received PLAYER_LEAVE for myself (ID: " << pkt->playerId << "), ignoring" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            
                            if (remotePlayers.count(pkt->playerId)) {
                                Character* c = remotePlayers[pkt->playerId];
                                if (c) {
                                    // Use the Character's method to get its folder index
                                    int playerFolderIndex = c->getFolderIndex();
                                    
                                    std::vector<int> partsToRemove;
                                    
                                    // Add the folder itself
                                    if (playerFolderIndex >= 0 && playerFolderIndex < parts.size()) {
                                        // Verify this is actually a Player folder
                                        if (parts[playerFolderIndex].name == "Player" && parts[playerFolderIndex].isFolder) {
                                            partsToRemove.push_back(playerFolderIndex);
                                            
                                            // Find all parts with this folder as parent
                                            for (int j = 0; j < parts.size(); ++j) {
                                                if (parts[j].parentIndex == playerFolderIndex && !parts[j].deleted) {
                                                    partsToRemove.push_back(j);
                                                }
                                            }
                                        } else {
                                            std::cerr << "WARNING: getFolderIndex returned invalid folder for player " << pkt->playerId << std::endl;
                                        }
                                    }
                                    
                                    // Mark parts as deleted instead of removing (prevents index shifts)
                                    for (int idx : partsToRemove) {
                                        if (idx >= 0 && idx < parts.size()) {
                                            Part& p = parts[idx];
                                            if (p.deleted) continue; // Already deleted
                                            
                                            // Remove from physics world if it has a body
                                            if (p.physicsBody) {
                                                physicsWorld.removePart(&p);
                                            }
                                            // Mark as deleted instead of erasing
                                            p.deleted = true;
                                            p.transparency = 1.0f; // Make invisible
                                            p.anchored = true; // Stop physics
                                            p.canCollide = false;
                                        }
                                    }
                                    
                                    delete c; // Properly delete the character
                                }
                                remotePlayers.erase(pkt->playerId);
                                playerNames.erase(pkt->playerId);
                                std::cout << "Player " << pkt->playerId << " left, removed character and parts" << std::endl;
                            } else {
                                std::cout << "Player " << pkt->playerId << " left, but no remote character found (already removed?)" << std::endl;
                            }
                        }
                        else if (header->type == PacketType::PLAYER_STATE) {
                            if (header->size < sizeof(PacketPlayerState)) {
                                std::cerr << "ERROR: Invalid PLAYER_STATE packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketPlayerState* pkt = (PacketPlayerState*)(packetBuffer + offset + sizeof(PacketHeader));
                            if (pkt->playerId != myPlayerId && remotePlayers.count(pkt->playerId)) {
                                try {
                                    Character* remoteChar = remotePlayers[pkt->playerId];
                                    if (remoteChar) {
                                        remoteChar->setRemoteState(pkt->position, pkt->rotationY, pkt->isWalking, pkt->isJumping);
                                        remoteChar->health = pkt->health;
                                    }
                                } catch (...) {
                                    std::cerr << "ERROR: Exception updating remote character state for player " << pkt->playerId << std::endl;
                                }
                            }
                        }
                        else if (header->type == PacketType::VEHICLE_STATE) {
                            if (header->size < sizeof(PacketVehicleState)) {
                                std::cerr << "ERROR: Invalid VEHICLE_STATE packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketVehicleState* pkt = (PacketVehicleState*)(packetBuffer + offset + sizeof(PacketHeader));
                            const int previousDriver = vehicleDriverId;
                            vehiclePartIndex = pkt->partIndex;
                            vehicleDriverId = pkt->driverId;
                            vehicleSpeed = pkt->speed;

                            // The car is as authoritative as any other world part,
                            // so it is placed exactly where the server says.
                            if (pkt->partIndex >= 0 && pkt->partIndex < (int)parts.size()) {
                                Part& car = parts[pkt->partIndex];
                                car.position = pkt->position;
                                car.rotation.y = pkt->yaw;
                                if (car.physicsBody) {
                                    auto* body = (linearphysics::RigidBody*)car.physicsBody;
                                    body->setType(linearphysics::BodyType::KINEMATIC);
                                    body->setTransform(linearphysics::Transform(
                                        linearphysics::Vector3(pkt->position.x, pkt->position.y, pkt->position.z),
                                        linearphysics::Quaternion::fromEulerAngles(0.0f, linearmath::radians(pkt->yaw), 0.0f)));
                                    body->setLinearVelocity(linearphysics::Vector3(0, 0, 0));
                                    body->setAngularVelocity(linearphysics::Vector3(0, 0, 0));
                                }
                            }

                            if (pkt->driverId == myPlayerId && previousDriver != myPlayerId) {
                                addChatLine("System", "Driving: W/S throttle and brake, A/D steer, E to get out.", true);
                            }
                        }
                        else if (header->type == PacketType::PLAYER_HEALTH) {
                            if (header->size < sizeof(PacketPlayerHealth)) {
                                std::cerr << "ERROR: Invalid PLAYER_HEALTH packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            PacketPlayerHealth* pkt = (PacketPlayerHealth*)(packetBuffer + offset + sizeof(PacketHeader));
                            const float maxHealth = pkt->maxHealth > 0.0f ? pkt->maxHealth : 100.0f;
                            if (pkt->playerId == myPlayerId && myCharacter) {
                                myCharacter->health = pkt->health;
                                myCharacter->maxHealth = maxHealth;
                                myCharacter->serverControlsHealth = true;

                                playerIsDead = pkt->respawnSeconds > 0;
                                respawnCountdown = static_cast<float>(pkt->respawnSeconds);
                                // The server is holding the character until the timer
                                // ends, so the mouse is handed back for the countdown:
                                // playerIsDead is what applyCursorCapture() reads, and
                                // it captures again by itself when the respawn clears it.
                            } else if (remotePlayers.count(pkt->playerId) != 0) {
                                Character* remoteChar = remotePlayers[pkt->playerId];
                                if (remoteChar) {
                                    remoteChar->health = pkt->health;
                                    remoteChar->maxHealth = maxHealth;
                                }
                            }
                        }
                        else if (header->type == PacketType::CHAT) {
                            if (header->size < sizeof(PacketChat)) {
                                std::cerr << "ERROR: Invalid CHAT packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }

                            PacketChat* pkt = (PacketChat*)(packetBuffer + offset + sizeof(PacketHeader));
                            const std::string username = PacketProtocol::fixedString(pkt->username, sizeof(pkt->username));
                            const std::string message = PacketProtocol::fixedString(pkt->message, sizeof(pkt->message));
                            addChatLine(username.empty() ? "Player" : username, message);
                        }
                        else if (header->type == PacketType::PLAYER_LIST) {
                            // Player list from server
                            if (header->size < sizeof(uint32_t)) {
                                std::cerr << "ERROR: Invalid PLAYER_LIST packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            
                            // Read player count
                            uint32_t playerCount = *(uint32_t*)(packetBuffer + offset + sizeof(PacketHeader));
                            const uint32_t expectedSize = static_cast<uint32_t>(
                                sizeof(uint32_t) + playerCount * sizeof(PlayerInfo));
                            
                            if (header->size != expectedSize) {
                                std::cerr << "ERROR: PLAYER_LIST size mismatch. Expected " << expectedSize 
                                          << ", got " << header->size << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            
                            playerList.clear();
                            if (playerCount > 0 && playerCount < 100) { // Safety limit
                                PlayerInfo* players = (PlayerInfo*)(packetBuffer + offset + sizeof(PacketHeader) + sizeof(uint32_t));
                                for (uint32_t i = 0; i < playerCount; i++) {
                                    playerList.push_back(players[i]);
                                    playerNames[players[i].playerId] = players[i].username;
                                    // The server names a player from the account its join
                                    // token belongs to, and this client sends its join
                                    // request before that token has been checked: its own
                                    // row was labelled "Guest" for the whole session while
                                    // the site, the chat and every detection filed against
                                    // the account used the real name. The player list is
                                    // where the two meet, so take the server's answer.
                                    if (players[i].playerId == myPlayerId) {
                                        const std::string selfName = PacketProtocol::fixedString(
                                            players[i].username, sizeof(players[i].username));
                                        if (!selfName.empty()) {
                                            copyToBuffer(myUsername, sizeof(myUsername), selfName);
                                        }
                                    }
                                    const std::string faceId = normalizeFaceId(PacketProtocol::fixedString(players[i].faceId, sizeof(players[i].faceId)));
                                    
                                    // Apply avatar colors to remote character if it exists
                                    if (remotePlayers.count(players[i].playerId) > 0) {
                                        Character* remoteChar = remotePlayers[players[i].playerId];
                                        if (remoteChar) {
                                            // setHat keeps its part and only resizes it, so a
                                            // repeated player list does not leak parts.
                                            remoteChar->setHat(PacketProtocol::fixedString(
                                                players[i].hatId, sizeof(players[i].hatId)));
                                        }
                                        // Update part colors
                                        if (remoteChar && !parts.empty()) {
                                            try {
                                                int folderIndex = remoteChar->getFolderIndex();
                                                if (folderIndex >= 0 && folderIndex < (int)parts.size()) {
                                                    // Find character parts and update colors
                                                    for (size_t j = 0; j < parts.size(); j++) {
                                                        Part& p = parts[j];
                                                        if (p.parentIndex == folderIndex && !p.deleted) {
                                                            if (p.name == "Head") {
                                                                p.color = linearmath::vec3(players[i].headColor[0], players[i].headColor[1], players[i].headColor[2]);
                                                                p.textureId = textureForFace(faceId);
                                                            } else if (p.name == "Torso") {
                                                                p.color = linearmath::vec3(players[i].torsoColor[0], players[i].torsoColor[1], players[i].torsoColor[2]);
                                                            } else if (p.name == "LeftArm") {
                                                                p.color = linearmath::vec3(players[i].leftArmColor[0], players[i].leftArmColor[1], players[i].leftArmColor[2]);
                                                            } else if (p.name == "RightArm") {
                                                                p.color = linearmath::vec3(players[i].rightArmColor[0], players[i].rightArmColor[1], players[i].rightArmColor[2]);
                                                            } else if (p.name == "LeftLeg") {
                                                                p.color = linearmath::vec3(players[i].leftLegColor[0], players[i].leftLegColor[1], players[i].leftLegColor[2]);
                                                            } else if (p.name == "RightLeg") {
                                                                p.color = linearmath::vec3(players[i].rightLegColor[0], players[i].rightLegColor[1], players[i].rightLegColor[2]);
                                                            }
                                                        }
                                                    }
                                                    std::cout << "Updated avatar colors for player " << players[i].playerId << std::endl;
                                                }
                                            } catch (...) {
                                                std::cerr << "ERROR: Exception updating avatar colors for player " << players[i].playerId << std::endl;
                                            }
                                        }
                                    }
                                }
                                std::cout << "Received player list: " << playerCount << " players" << std::endl;
                            }
                        }
                        else if (header->type == PacketType::WORLD_STATE) {
                            // Complete world state from server - load all parts
                            if (header->size < sizeof(uint32_t)) {
                                std::cerr << "ERROR: Invalid WORLD_STATE packet size" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            
                            // Read part count
                            uint32_t partCount = *(uint32_t*)(packetBuffer + offset + sizeof(PacketHeader));
                            const uint32_t expectedSize = static_cast<uint32_t>(
                                sizeof(uint32_t) + partCount * sizeof(PartData));
                            
                            if (header->size != expectedSize) {
                                std::cerr << "ERROR: WORLD_STATE size mismatch. Expected " << expectedSize 
                                          << " but got " << header->size << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            
                            // CRITICAL: Delete all characters BEFORE resetting physics world
                            // Otherwise character physics bodies will be invalid
                            std::cout << "WORLD_STATE received - cleaning up before reset" << std::endl;
                            
                            if (myCharacter) {
                                std::cout << "Deleting my character before physics reset" << std::endl;
                                delete myCharacter;
                                myCharacter = nullptr;
                            }
                            
                            for (auto& [id, remoteChar] : remotePlayers) {
                                if (remoteChar) {
                                    std::cout << "Deleting remote character " << id << " before physics reset" << std::endl;
                                    delete remoteChar;
                                }
                            }
                            remotePlayers.clear();
                            
                            // Clear existing parts
                            std::cout << "Resetting physics world" << std::endl;
                            physicsWorld.reset();
                            parts.clear();
                            serverPartToClient.clear();
                            std::cout << "Physics world reset complete" << std::endl;
                            
                            // Read all parts
                            PartData* partData = (PartData*)(packetBuffer + offset + sizeof(PacketHeader) + sizeof(uint32_t));
                            
                            for (uint32_t i = 0; i < partCount; ++i) {
                                PartData& data = partData[i];
                                
                                Part p;
                                p.shape = (ShapeType)data.shape;
                                p.name = std::string(data.name);
                                p.parentIndex = data.parentIndex;
                                
                                // Unpack flags
                                p.isFolder = (data.flags & 1) != 0;
                                p.isCamera = (data.flags & 2) != 0;
                                p.isSpawn = (data.flags & 4) != 0;
                                
                                p.position = data.position;
                                p.size = data.size;
                                p.color = data.color;
                                p.rotation = data.rotation;
                                p.transparency = data.transparency;
                                p.reflectance = data.reflectance;
                                
                                // Unpack physics flags
                                p.anchored = (data.physicsFlags & 1) != 0;
                                p.canCollide = (data.physicsFlags & 2) != 0;
                                
                                p.mass = data.mass;
                                p.material = data.material < kMaterialCount
                                                 ? static_cast<Material>(data.material)
                                                 : Material::Plastic;
                                p.texture = PacketProtocol::fixedString(data.texture, sizeof(data.texture));
                                // Bit 3 marks a part a script destroyed before this
                                // client joined: the snapshot keeps its index so the
                                // parts after it keep theirs, but it is not a room.
                                p.deleted = (data.flags & 8) != 0;
                                
                                serverPartToClient.push_back(static_cast<int>(parts.size()));
                                parts.push_back(p);
                                
                                // Add to physics world if not a folder, and not a
                                // part the hotel has already taken away.
                                if (!p.isFolder && !p.deleted) {
                                    physicsWorld.addPart(&parts.back());
                                }
                            }
                            
                            std::cout << "Loaded complete world state from server: " << partCount << " parts" << std::endl;
                            {
                                // The game view hides stdout, so a client that renders an
                                // empty world and a client that never got the snapshot used
                                // to look identical. Say which one it was in the chat panel.
                                addChatLine("System",
                                            "World loaded from the game server: " +
                                                std::to_string(partCount) + " parts",
                                            false);
                                if (partCount == 0) {
                                    addChatLine("System",
                                                "The game server started with an empty world, so there is "
                                                "nothing to stand on. Restart it with its world file and check "
                                                "its log.",
                                                true);
                                }
                            }
                            
                            // Now that world is loaded, create the character
                            if (myPlayerId != -1 && !myCharacter) {
                                // Find spawn
                                linearmath::vec3 spawnPos(0, 10, 0);
                                for(const auto& p : parts) { 
                                    if(p.isSpawn) {
                                        spawnPos = p.position + linearmath::vec3(0, 5, 0);
                                        break;
                                    }
                                }
                                
                                std::cout << "Creating Character at: " << spawnPos.x << ", " << spawnPos.y << ", " << spawnPos.z << std::endl;
                                myCharacter = new Character(spawnPos, &physicsWorld, &parts,
                                    currentAvatar.headColor, currentAvatar.torsoColor,
                                    currentAvatar.leftArmColor, currentAvatar.rightArmColor,
                                    currentAvatar.leftLegColor, currentAvatar.rightLegColor);
                                
                                // Immediately update character to position all parts correctly
                                if (myCharacter) {
                                    myCharacter->serverControlsHealth =
                                        (currentState == ONLINE);
                                    myCharacter->setFaceTexture(textureForFace(currentAvatar.faceId));
                                    myCharacter->setHat(currentAvatar.hatId);
                                    myCharacter->update(0.0f, nullptr, false);
                                }
                            }
                        }
                        else if (header->type == PacketType::WORLD_UPDATE) {
                            if (header->size % sizeof(PartUpdate) != 0) {
                                std::cerr << "ERROR: Invalid WORLD_UPDATE packet size (not multiple of PartUpdate)" << std::endl;
                                offset += packetSize;
                                continue;
                            }
                            int numUpdates = header->size / sizeof(PartUpdate);
                            PartUpdate* updates = (PartUpdate*)(packetBuffer + offset + sizeof(PacketHeader));
                            float now = (float)linearwinGetTime();
                            for(int i=0; i<numUpdates; ++i) {
                                PartUpdate& up = updates[i];
                                // The server numbers its own parts; a character's
                                // parts sit after the world's in this deque, so ask
                                // the map rather than trusting the number.
                                int updateIndex = up.partIndex;
                                if (updateIndex >= 0 &&
                                    updateIndex < static_cast<int>(serverPartToClient.size()) &&
                                    serverPartToClient[updateIndex] >= 0) {
                                    updateIndex = serverPartToClient[updateIndex];
                                }
                                if (updateIndex >= 0 && updateIndex < static_cast<int>(parts.size())) {
                                    Part& p = parts[updateIndex];
                                    
                                    // SAFETY: Validate server position
                                    if (std::isnan(up.position.x) || std::isnan(up.position.y) || std::isnan(up.position.z) ||
                                        std::isinf(up.position.x) || std::isinf(up.position.y) || std::isinf(up.position.z)) {
                                        continue; // Skip invalid updates
                                    }
                                    
                                    // Check if this is a player part (don't apply server updates to player parts)
                                    bool isPlayerPart = false;
                                    if (p.parentIndex >= 0 && p.parentIndex < parts.size()) {
                                        if (parts[p.parentIndex].name == "Player") {
                                            isPlayerPart = true;
                                        }
                                    }
                                    
                                    // Server is fully authoritative for world parts
                                    if (!isPlayerPart && p.physicsBody) {
                                        linearphysics::RigidBody* body = (linearphysics::RigidBody*)p.physicsBody;
                                        
                                        // CRITICAL: Make world parts KINEMATIC immediately
                                        // This prevents ANY local physics simulation
                                        body->setType(linearphysics::BodyType::KINEMATIC);
                                        
                                        // Apply server position IMMEDIATELY - server is source of truth
                                        linearphysics::Vector3 serverPos(up.position.x, up.position.y, up.position.z);
                                        linearphysics::Quaternion serverRot = linearphysics::Quaternion::fromEulerAngles(
                                            linearmath::radians(up.rotation.x),
                                            linearmath::radians(up.rotation.y),
                                            linearmath::radians(up.rotation.z)
                                        );
                                        
                                        linearphysics::Transform serverTransform(serverPos, serverRot);
                                        body->setTransform(serverTransform);
                                        body->setLinearVelocity(linearphysics::Vector3(0, 0, 0));
                                        body->setAngularVelocity(linearphysics::Vector3(0, 0, 0));
                                        
                                        // Update part position for rendering - server is truth
                                        p.position = up.position;
                                        p.rotation = up.rotation;
                                    }
                                    
                                    // Store for continuous updates (apply every frame)
                                    NetworkTransform target; 
                                    target.position = up.position; 
                                    target.rotation = up.rotation; 
                                    target.timestamp = now;
                                    targetTransforms[updateIndex] = target;
                                }
                            }
                        }
                        
                        // How the world's air looks. Sent with the welcome, so
                        // it is here before the first part is drawn.
                        if (header->type == PacketType::WORLD_ENVIRONMENT) {
                            if (header->size != sizeof(PacketWorldEnvironment)) {
                                std::cerr << "ERROR: Invalid WORLD_ENVIRONMENT packet size" << std::endl;
                            } else {
                                const PacketWorldEnvironment* environment =
                                    (const PacketWorldEnvironment*)(packetBuffer + offset + sizeof(PacketHeader));
                                fogColor = linearmath::vec3(linearmath::clamp(environment->fogColor, linearmath::vec3(0.0f), linearmath::vec3(1.0f)));
                                fogDensity = std::clamp(environment->fogDensity, 0.0f, 0.2f);
                            }
                        }

                        // Parts the script built for this player. The join
                        // snapshot only describes the world as it was then, so a
                        // hotel that builds its rooms as the player walks sends
                        // each new one on its own.
                        if (header->type == PacketType::PART_STATE) {
                            if (header->size != sizeof(PacketPartState)) {
                                std::cerr << "ERROR: Invalid PART_STATE packet size" << std::endl;
                            } else {
                                const PacketPartState* pkt =
                                    (const PacketPartState*)(packetBuffer + offset + sizeof(PacketHeader));
                                const int serverIndex = pkt->partIndex;
                                const Part built = partFromData(pkt->data);

                                int clientIndex = -1;
                                if (serverIndex >= 0 &&
                                    serverIndex < static_cast<int>(serverPartToClient.size())) {
                                    clientIndex = serverPartToClient[serverIndex];
                                }

                                if (clientIndex >= 0 && clientIndex < static_cast<int>(parts.size())) {
                                    // A part that replaced one at the same index.
                                    Part& existing = parts[clientIndex];
                                    if (existing.physicsBody) {
                                        physicsWorld.removePart(&existing);
                                    }
                                    existing = built;
                                    if (!existing.isFolder && !existing.deleted) {
                                        physicsWorld.addPart(&existing);
                                    }
                                } else if (serverIndex >= 0) {
                                    while (static_cast<int>(serverPartToClient.size()) <= serverIndex) {
                                        serverPartToClient.push_back(-1);
                                    }
                                    serverPartToClient[serverIndex] = static_cast<int>(parts.size());
                                    parts.push_back(built);
                                    if (!built.isFolder && !built.deleted) {
                                        physicsWorld.addPart(&parts.back());
                                    }
                                }
                            }
                        }

                        // A room the script took away behind the player. Its index
                        // stays taken so the rooms in front keep theirs.
                        if (header->type == PacketType::PART_REMOVE) {
                            if (header->size != sizeof(PacketPartRemove)) {
                                std::cerr << "ERROR: Invalid PART_REMOVE packet size" << std::endl;
                            } else {
                                const PacketPartRemove* pkt =
                                    (const PacketPartRemove*)(packetBuffer + offset + sizeof(PacketHeader));
                                const int serverIndex = pkt->partIndex;
                                if (serverIndex >= 0 &&
                                    serverIndex < static_cast<int>(serverPartToClient.size())) {
                                    const int clientIndex = serverPartToClient[serverIndex];
                                    if (clientIndex >= 0 && clientIndex < static_cast<int>(parts.size())) {
                                        Part& gone = parts[clientIndex];
                                        if (gone.physicsBody) {
                                            physicsWorld.removePart(&gone);
                                        }
                                        gone.deleted = true;
                                        gone.transparency = 1.0f;
                                        gone.anchored = true;
                                        gone.canCollide = false;
                                    }
                                }
                            }
                        }

                        offset += packetSize; // Use calculated packetSize
                    }
                } 
                else if (gameConnectionLost) {
                    const bool neverJoined = awaitingWelcome;
                    disconnectFromServerOnly();
                    currentState = MENU_AUTH_CHOICE;
                    addChatLine("System",
                                neverJoined
                                    ? "The game server did not answer. Check that it is running."
                                    : "Lost connection to the game server.",
                                true);
                }
                // Send state with safety checks
                if (isConnected && clientSocket != INVALID_SOCKET && myCharacter && myPlayerId != -1) {
                    try {
                        PacketPlayerState state;
                        state.playerId = myPlayerId;
                        state.position = myCharacter->getPosition();
                        // Send the actual current yaw (internalYaw) which smoothly interpolates to target
                        // This ensures smooth rotation even during diagonal movement
                        state.rotationY = myCharacter->debugCurrentYaw;
                        state.isWalking = (linearmath::length(myCharacter->debugMoveDir) > 0.1f);
                        // Remote players need this to play the airborne pose;
                        // it used to be hard-coded false, so everyone else only
                        // ever saw the walk cycle.
                        state.isJumping = myCharacter->isJumping;
                        state.health = myCharacter->health;
                        // The moment this position belongs to, so the server can
                        // judge speed by the time the player moved instead of by
                        // when the datagram was read. Packets that bunch up after
                        // a loss then stop looking like a speed hack.
                        state.clientTimeMs = static_cast<uint32_t>(steadyMilliseconds());
                        if (!sendPlayerStateToServer(state)) {
                            std::cout << "Disconnected while sending player state." << std::endl;
                            isConnected = false;
                            closesocket(clientSocket);
                            clientSocket = INVALID_SOCKET;
                            currentState = MENU_AUTH_CHOICE;
                            showConnectionUI = false;
                            clientReceiveBuffer.clear();
                        }
                    } catch (...) {
                        std::cerr << "ERROR: Exception getting character state" << std::endl;
                    }
                }
            }

            // The countdown is drawn locally between the server's packets; the
            // server still decides when the respawn actually happens.
            if (respawnCountdown > 0.0f) {
                respawnCountdown = std::max(0.0f, respawnCountdown - deltaTime);
                if (respawnCountdown <= 0.0f) {
                    playerIsDead = false;
                }
            }

            if (myCharacter) {
                myCharacter->faceCameraYaw = shiftLockEnabled;
                myCharacter->cameraShoulderOffset = shiftLockEnabled ? 1.5f : 0.0f;
            }

            // While seated the client sends throttle and steer instead of walking,
            // and follows the car the server is moving.
            const bool drivingVehicle =
                vehicleDriverId == myPlayerId && vehiclePartIndex >= 0 &&
                vehiclePartIndex < static_cast<int>(parts.size()) && myCharacter != nullptr;

            static bool vehicleKeyDown = false;
            const bool vehicleKey = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_E) == LINEARWIN_PRESS;
            if (vehicleKey && !vehicleKeyDown && isConnected && !io.WantCaptureKeyboard &&
                !escapeMenuOpen) {
                toggleVehicle();
            }
            vehicleKeyDown = vehicleKey;

            if (drivingVehicle) {
                static float vehicleInputTimer = 0.0f;
                vehicleInputTimer += deltaTime;
                if (vehicleInputTimer >= 1.0f / 20.0f) {
                    vehicleInputTimer = 0.0f;
                    const auto pressed = [&](int key) {
                        return linearwinGetKey(window.getNativeWindow(), key) == LINEARWIN_PRESS;
                    };
                    const float throttle = (pressed(LINEARWIN_KEY_W) ? 1.0f : 0.0f) -
                                           (pressed(LINEARWIN_KEY_S) ? 1.0f : 0.0f);
                    const float steer = (pressed(LINEARWIN_KEY_D) ? 1.0f : 0.0f) -
                                        (pressed(LINEARWIN_KEY_A) ? 1.0f : 0.0f);
                    sendVehicleInput(throttle, steer);
                }

                Part& car = parts[vehiclePartIndex];
                myCharacter->teleportTo(car.position + linearmath::vec3(0.0f, 1.8f, 0.0f));
                myCharacter->update(deltaTime, nullptr, false);

                // Chase camera: behind the car along its heading, looking at it.
                // The car's yaw uses the same convention as Camera::Yaw.
                const float yawRad = linearmath::radians(car.rotation.y);
                const linearmath::vec3 forward(std::cos(yawRad), 0.0f, std::sin(yawRad));
                camera.Position = car.position - forward * 13.0f + linearmath::vec3(0.0f, 6.5f, 0.0f);
                camera.LookAt(car.position + linearmath::vec3(0.0f, 1.5f, 0.0f));
            }

            if (!drivingVehicle && (isConnected || currentState == OFFLINE) && myCharacter &&
                !io.WantCaptureKeyboard && !escapeMenuOpen && !playerIsDead) {
                 try {
                     bool fwd = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_W) == LINEARWIN_PRESS;
                     bool bwd = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_S) == LINEARWIN_PRESS;
                     bool l = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_A) == LINEARWIN_PRESS;
                     bool r = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_D) == LINEARWIN_PRESS;
                     bool j = linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_SPACE) == LINEARWIN_PRESS;
                     float zoom = 0.0f;
                     if (linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_UP) == LINEARWIN_PRESS) zoom = 1.0f;
                     if (linearwinGetKey(window.getNativeWindow(), LINEARWIN_KEY_DOWN) == LINEARWIN_PRESS) zoom = -1.0f;
                     myCharacter->processInput(fwd, bwd, l, r, j, camera, zoom, deltaTime);
                 } catch (...) {
                     std::cerr << "ERROR: Exception processing character input" << std::endl;
                 }
            }
            // Update local character with safety checks (the car drives it while
            // the player is seated)
            if (myCharacter && !drivingVehicle) {
                try {
                    myCharacter->update(deltaTime, &camera, false);
                } catch (...) {
                    std::cerr << "ERROR: Exception updating local character" << std::endl;
                    delete myCharacter;
                    myCharacter = nullptr;
                }
            }
            
            // Update remote characters with safety checks
            for (auto it = remotePlayers.begin(); it != remotePlayers.end();) {
                auto& [id, remoteChar] = *it;
                if (remoteChar) {
                    try {
                        remoteChar->update(deltaTime, nullptr, false);
                        ++it;
                    } catch (...) {
                        std::cerr << "ERROR: Exception updating remote character " << id << ", removing" << std::endl;
                        delete remoteChar;
                        it = remotePlayers.erase(it);
                    }
                } else {
                    it = remotePlayers.erase(it);
                }
            }
            
            if (currentState == OFFLINE) {
                // Offline: Full physics simulation
                physicsWorld.update(deltaTime);
            } else if (currentState == ONLINE && isConnected) {
                // Online: Server is fully authoritative
                // FIRST: Ensure ALL world parts are KINEMATIC (even if no recent updates)
                for (auto& p : parts) {
                    if (p.deleted) continue; // Skip deleted parts
                    
                    // Check if this is a player part
                    bool isPlayerPart = false;
                    if (p.parentIndex >= 0 && p.parentIndex < parts.size()) {
                        if (parts[p.parentIndex].name == "Player") {
                            isPlayerPart = true;
                        }
                    }
                    
                    // Force world parts to KINEMATIC - server controls them
                    if (!isPlayerPart && p.physicsBody) {
                        linearphysics::RigidBody* body = (linearphysics::RigidBody*)p.physicsBody;
                        if (body->getType() != linearphysics::BodyType::KINEMATIC) {
                            body->setType(linearphysics::BodyType::KINEMATIC);
                        }
                    }
                }
                
                // SECOND: Apply server updates EVERY FRAME - keep parts in sync
                float currentTime = (float)linearwinGetTime();
                for (auto it = targetTransforms.begin(); it != targetTransforms.end();) {
                    int index = it->first;
                    NetworkTransform& target = it->second;
                    
                    float age = currentTime - target.timestamp;
                    if (age > 1.0f) {
                        it = targetTransforms.erase(it);
                        continue;
                    }
                    
                    if (index >= 0 && index < parts.size()) {
                        Part& p = parts[index];
                        if (p.deleted) continue; // Skip deleted parts
                        
                        // Only apply to world parts (not player character parts)
                        bool isPlayerPart = false;
                        if (p.parentIndex >= 0 && p.parentIndex < parts.size()) {
                            if (parts[p.parentIndex].name == "Player") {
                                isPlayerPart = true;
                            }
                        }
                        
                        // Server is fully authoritative for world parts
                        if (!isPlayerPart && p.physicsBody) {
                            linearphysics::RigidBody* body = (linearphysics::RigidBody*)p.physicsBody;
                            
                            // FORCE KINEMATIC - server controls this completely
                            body->setType(linearphysics::BodyType::KINEMATIC);
                            
                            // Apply server position DIRECTLY every frame
                            linearphysics::Vector3 serverPos(target.position.x, target.position.y, target.position.z);
                            linearphysics::Quaternion serverRot = linearphysics::Quaternion::fromEulerAngles(
                                linearmath::radians(target.rotation.x),
                                linearmath::radians(target.rotation.y),
                                linearmath::radians(target.rotation.z)
                            );
                            
                            linearphysics::Transform serverTransform(serverPos, serverRot);
                            body->setTransform(serverTransform);
                            body->setLinearVelocity(linearphysics::Vector3(0, 0, 0));
                            body->setAngularVelocity(linearphysics::Vector3(0, 0, 0));
                            
                            // Update part position for rendering - server is source of truth
                            p.position = target.position;
                            p.rotation = target.rotation;
                        }
                    }
                    ++it;
                }
                
                // THIRD: Run physics AFTER applying server updates
                // PhysicsWorld::update() will skip KINEMATIC parts, so server positions stay
                physicsWorld.update(deltaTime);
            } else {
                // Menu state - no physics
            }

            // 4. Render Scene
            // Shadow Pass
            linearmath::vec3 lightPos(20.0f, 50.0f, 20.0f);
            linearmath::mat4 lightSpaceMatrix = shadowMap.getLightSpaceMatrix(lightPos);

            if (!parts.empty()) {
                shadowMap.bindForWriting();
                for (const auto& part : parts) {
                    if (!part.deleted) {
                        renderer.drawPartShadow(part, lightSpaceMatrix);
                    }
                }
                shadowMap.unbind();
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glDrawBuffer(GL_BACK);
            glReadBuffer(GL_BACK);
            glViewport(0, 0, window.getWidth(), window.getHeight());
            glClearColor(0.53f, 0.81f, 0.92f, 1.0f); // Sky Blue
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            linearmath::mat4 view = camera.GetViewMatrix();
            linearmath::mat4 projection = camera.GetProjectionMatrix((float)window.getWidth(), (float)window.getHeight());

            // Draw Skybox
            glDepthMask(GL_FALSE);
            skybox.draw(view, projection);
            glDepthMask(GL_TRUE);

            // The game view hides stdout, so a machine whose driver refused the
            // part shader showed nothing but the sky with no hint why. Say it
            // once, in the panel the player can actually see.
            static bool reportedPartShader = false;
            if (!reportedPartShader && renderer.shader && !renderer.shader->usable &&
                (currentState == ONLINE || currentState == OFFLINE)) {
                reportedPartShader = true;
                addChatLine("System",
                            "This GPU refused the 3D shader, so only the sky can be drawn. "
                            "Driver said: " + renderer.shader->buildLog.substr(0, 300),
                            true);
            }

            // Once per game, report where the view is actually pointed. An empty
            // screen is either a camera looking at nothing or geometry that never
            // reached the GPU, and the two are impossible to tell apart by eye.
            static bool reportedView = false;
            if (!reportedView && myCharacter && (currentState == ONLINE || currentState == OFFLINE)) {
                reportedView = true;
                int liveParts = 0;
                for (const auto& p : parts) {
                    if (!p.deleted) ++liveParts;
                }
                const linearmath::vec3 playerPos = myCharacter->getPosition();
                addChatLine("System",
                            "View: camera (" + std::to_string((int)camera.Position.x) + ", " +
                                std::to_string((int)camera.Position.y) + ", " +
                                std::to_string((int)camera.Position.z) + ") yaw " +
                                std::to_string((int)camera.Yaw) + ", player (" +
                                std::to_string((int)playerPos.x) + ", " +
                                std::to_string((int)playerPos.y) + ", " +
                                std::to_string((int)playerPos.z) + "), " +
                                std::to_string(liveParts) + " parts",
                            false);
            }

            // Bind Skybox Texture for Parts Reflection (Texture Unit 0)
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, skybox.getTextureID());
            
            if (renderer.shader) {
                 renderer.shader->use();
                 renderer.shader->setInt("skybox", 0); 
                 renderer.shader->setVec3("lightPos", lightPos);
                 renderer.shader->setVec3("fogColor", fogColor);
                 renderer.shader->setFloat("fogDensity", fogDensity);
            }

            // Only render if we have parts and camera is valid
            if (!parts.empty() && window.getWidth() > 0 && window.getHeight() > 0) {
                for (const auto& part : parts) {
                    if (!part.deleted) {
                        renderer.drawPart(part, view, projection, lightSpaceMatrix, shadowMap.depthMap, faceTextureId);
                    }
                }
            }
            
            // Cleanup texture state for LinearUI
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

            // 5. LinearUI Render
            LinearUI::Render();
            
            // 6. Render Draw Data
            LinearDrawData* drawData = LinearUI::GetDrawData();
            if (drawData) {
                LinearUI_ImplOpenGL3_RenderDrawData(drawData);
            } else {
                if (frameCount < 5) std::cout << "LinearUI DrawData is NULL" << std::endl;
            }

            // 7. Swap Buffers
            window.swapBuffers();
            
            frameCount++;
        }

        // The loop ends only when the window was asked to close, so this line is what
        // separates "the player closed it" from a run that stopped on its own.
        traceStartup("main loop ended, shutting down");

        // The window is destroyed last, by the destructor of `window`, so all of
        // the shutdown below used to run behind a window that had stopped
        // pumping messages. Windows paints that as "Linear Client is not
        // responding" and the player has to kill the process, even though it was
        // busy shutting down. Hide the window as soon as the close is asked for,
        // so the close happens at once and the rest of the work (the anti-cheat's
        // threads, Winsock, the physics world) finishes out of sight.
        const auto shutdownStart = std::chrono::steady_clock::now();
        auto shutdownMs = [&shutdownStart]() {
            return std::chrono::duration_cast<std::chrono::milliseconds>(
                       std::chrono::steady_clock::now() - shutdownStart)
                .count();
        };
        linearwinHideWindow(window.getNativeWindow());
        linearwinPollEvents();
        std::cout << "Shutting down." << std::endl;

        // Sockets must not outlive the Winsock session that owns them, and the
        // game socket and the login callback socket are both still open here.
        disconnectFromServerOnly();
        closeAuthCallback();
        std::cout << "Sockets closed after " << shutdownMs() << " ms." << std::endl;

#if defined(LINEAR_ANTICHEAT)
        // stopAntiCheat() joins the anti-cheat's threads. A thread can be inside a
        // driver or a certificate call that never returns, and then the join never
        // returns either: the window is already hidden by this point, so the process
        // sits in the background holding client_log.txt, responds to nothing, and
        // Task Manager's End task has no window to close. Wait a bounded time and then
        // end the process outright, which is what End task would have done anyway.
        std::atomic<bool> antiCheatStopped{false};
        std::thread stopper([&antiCheatStopped]() {
            stopAntiCheat();
            antiCheatStopped.store(true, std::memory_order_release);
        });
        stopper.detach();

        const auto antiCheatDeadline =
            std::chrono::steady_clock::now() + std::chrono::seconds(4);
        while (!antiCheatStopped.load(std::memory_order_acquire) &&
               std::chrono::steady_clock::now() < antiCheatDeadline) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        if (!antiCheatStopped.load(std::memory_order_acquire)) {
            std::cout << "Anti-cheat did not stop within 4 s; ending the process."
                      << std::endl;
            std::cout.flush();
            ExitProcess(0);
        }
        std::cout << "Anti-cheat stopped after " << shutdownMs() << " ms." << std::endl;
#endif
        Network::cleanup();
        std::cout << "Winsock closed after " << shutdownMs() << " ms." << std::endl;
        return 0;
    } catch (const std::exception& e) {
        const std::string reason = std::string("CRITICAL ERROR: ") + e.what();
        traceStartup("client stopped: " + reason);
        reportClientProblem("startup", reason);
        pauseBeforeExit(reason);
        return -1;
    } catch (...) {
        const std::string reason = "CRITICAL ERROR: the client stopped with an unknown error.";
        traceStartup("client stopped: " + reason);
        reportClientProblem("startup", reason);
        pauseBeforeExit(reason);
        return -1;
    }
}
