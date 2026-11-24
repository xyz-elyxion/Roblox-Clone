#include "GuardChecks.h"

namespace linear {
namespace guard {
namespace {

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

std::string fileNameOf(const std::string& path) {
    const std::size_t slash = path.find_last_of("\\/");
    return slash == std::string::npos ? path : path.substr(slash + 1);
}

}  // namespace

const std::vector<std::string>& cheatToolNames() {
    // A tool that reads or writes another process's memory while the game runs.
    // The list is names, not hashes: renaming an executable is one step, and this
    // is a signal for a review, not a proof on its own. The same judgement is
    // made by the scan inside the client, so both agree on what is a cheat tool.
    static const std::vector<std::string> names = {
        "cheatengine-x86_64.exe",
        "cheatengine-i386.exe",
        "cheatengine.exe",
        "x64dbg.exe",
        "x32dbg.exe",
        "windbg.exe",
        "ollydbg.exe",
        "ida.exe",
        "ida64.exe",
        "processhacker.exe",
        "systeminformer.exe",
        "scylla.exe",
        "extremeinjector.exe",
        "frida-server.exe",
        "hxd.exe",
        "reshacker.exe",
    };
    return names;
}

bool isCheatToolName(const std::string& nameOrPath) {
    const std::string name = lowerPath(fileNameOf(nameOrPath));
    if (name.empty()) {
        return false;
    }
    for (const std::string& known : cheatToolNames()) {
        if (known == name) {
            return true;
        }
    }
    return false;
}

bool isUserWritablePath(const std::string& path) {
    const std::string lower = lowerPath(path);
    if (lower.empty()) {
        return false;
    }
    // A user profile, its temporary folders, and the places a browser or a chat
    // program drops a file. Program Files and Windows are absent on purpose:
    // something loaded from there needed an installer or an administrator.
    static const char* const roots[] = {
        "\\appdata\\",
        "\\temp\\",
        "\\tmp\\",
        "\\downloads\\",
        "\\desktop\\",
        "\\documents\\",
        "\\users\\public\\",
        "\\$recycle.bin\\",
        "\\perflogs\\",
    };
    for (const char* root : roots) {
        if (lower.find(root) != std::string::npos) {
            return true;
        }
    }
    return false;
}

bool isExpectedModule(const std::string& fileName) {
    const std::string name = lowerPath(fileNameOf(fileName));
    // LinearGame.dll ships beside the client; the Discord hooks are loaded into
    // every game process that runs while Discord is open.
    static const char* const expected[] = {
        "lineargame.dll",
        "discordhook64.dll",
        "discordhook32.dll",
        "discord_hook",
    };
    for (const char* known : expected) {
        if (name == known) {
            return true;
        }
    }
    return name.find("discord_hook") != std::string::npos;
}

std::string detectionLine(const std::string& stamp, const std::string& message) {
    std::string text = "[" + stamp + "] [DETECTION] ";
    for (char c : message) {
        const unsigned char byte = static_cast<unsigned char>(c);
        text.push_back(byte < 0x20 || byte == 0x7f ? ' ' : c);
    }
    while (!text.empty() && text.back() == ' ') {
        text.pop_back();
    }
    return text;
}

bool fitsInLog(std::size_t currentBytes, std::size_t lineBytes, std::size_t limit) {
    if (lineBytes >= limit) {
        return false;
    }
    return currentBytes <= limit - lineBytes - 1;
}

}  // namespace guard
}  // namespace linear
