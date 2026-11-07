#include "HardwareId.h"

#include <cstdio>
#include <string>

#include "WebCrypto.h"

#  ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#  endif
#  include <winsock2.h>
#  include <windows.h>

namespace rblx {
namespace hwid {
namespace {

// What identifies this machine, kept in the process.
//
// Windows: the serial of the volume the system directory sits on, plus the
// computer name. Both survive a game reinstall, a new account and a new game
// folder, which is exactly the point - the two things a banned player reaches for
// first all leave these alone.
std::string machineInputs() {
    std::string inputs;
    char systemDirectory[MAX_PATH + 1] = {0};
    if (GetSystemDirectoryA(systemDirectory, MAX_PATH) > 0) {
        char root[4] = {0};
        root[0] = systemDirectory[0];
        root[1] = ':';
        root[2] = '\\';
        char volumeName[MAX_PATH + 1] = {0};
        char fileSystemName[MAX_PATH + 1] = {0};
        DWORD serialNumber = 0;
        DWORD maxComponentLength = 0;
        DWORD fileSystemFlags = 0;
        if (GetVolumeInformationA(root, volumeName, MAX_PATH, &serialNumber, &maxComponentLength,
                                  &fileSystemFlags, fileSystemName, MAX_PATH)) {
            inputs += "vol=" + std::to_string(static_cast<unsigned long>(serialNumber)) + ";";
        }
    }
    char computerName[MAX_COMPUTERNAME_LENGTH + 1] = {0};
    DWORD nameLength = MAX_COMPUTERNAME_LENGTH;
    if (GetComputerNameA(computerName, &nameLength)) {
        inputs += "host=";
        inputs.append(computerName, nameLength);
    }
    if (inputs.empty()) {
        // A machine that answers neither question still gets a stable id for the
        // life of the process rather than a hash of nothing, which every such
        // machine would share.
        inputs = "unknown-machine";
    }
    return inputs;
}

}  // namespace

std::string hashInputs(const std::string& inputs) {
    // The prefix versions the scheme: changing how the inputs are gathered can
    // then change the digest instead of silently matching an old ban.
    const std::string digest = rblx::web::hexEncode(rblx::web::sha256("rblx-hwid:v1:" + inputs));
    return digest.substr(0, kHardwareIdLength);
}

std::string machineId() {
    static const std::string id = hashInputs(machineInputs());
    return id;
}

}  // namespace hwid
}  // namespace rblx
