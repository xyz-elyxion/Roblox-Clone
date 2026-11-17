// LinearCert: puts the project's signing certificate where Windows looks for it.
//
// Signing each executable embeds the certificate, but Windows still asks who the publisher is
// until that certificate is in the trust stores this program writes to: ROOT, so a signature
// chain ending in it is accepted, and TrustedPublisher, so a signed program starts without an
// "unknown publisher" prompt. Per-user needs no administrator; --machine does.

#include "CertificateTrust.h"

#include <windows.h>

#include <cstdio>
#include <string>

namespace {

void usage() {
    std::printf(
        "LinearCert - trust (or stop trusting) the Linear signing certificate\n"
        "\n"
        "  LinearCert install [--machine] [--file <certificate.cer>]\n"
        "  LinearCert uninstall [--machine] [--file <certificate.cer>]\n"
        "  LinearCert status [--machine] [--file <certificate.cer>]\n"
        "\n"
        "Without --machine the certificate goes in your own stores, which needs no\n"
        "administrator. --machine writes to the machine stores and needs an elevated shell.\n"
        "Without --file it looks for certs\\linear-codesign.cer beside this program.\n"
        "\n"
        "Skip it and the signed programs still run, but Windows asks before starting one and\n"
        "an antivirus has no publisher to check the signature against.\n");
}

std::string executablePath() {
    wchar_t wide[MAX_PATH * 4] = {};
    const DWORD length =
        GetModuleFileNameW(nullptr, wide, static_cast<DWORD>(sizeof(wide) / sizeof(wide[0])));
    if (length == 0) {
        return std::string();
    }
    const int bytes = WideCharToMultiByte(CP_ACP, 0, wide, static_cast<int>(length), nullptr, 0,
                                          nullptr, nullptr);
    std::string path(static_cast<std::size_t>(bytes), '\0');
    WideCharToMultiByte(CP_ACP, 0, wide, static_cast<int>(length), path.data(), bytes, nullptr,
                        nullptr);
    return path;
}

}  // namespace

int main(int argc, char** argv) {
    std::string command = "install";
    std::string file;
    bool machine = false;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "install" || argument == "uninstall" || argument == "status") {
            command = argument;
        } else if (argument == "--machine") {
            machine = true;
        } else if (argument == "--file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "--file needs a path.\n");
                return 2;
            }
            file = argv[++i];
        } else if (argument == "--help" || argument == "-h" || argument == "/?") {
            usage();
            return 0;
        } else {
            std::fprintf(stderr, "Unknown argument: %s\n\n", argument.c_str());
            usage();
            return 2;
        }
    }

    const std::string path =
        file.empty() ? linear::certificate::defaultPath(executablePath()) : file;
    if (path.empty()) {
        std::fprintf(stderr, "Could not work out where this program is, so pass --file.\n");
        return 1;
    }

    std::string error;
    if (command == "install") {
        int alreadyThere = 0;
        if (!linear::certificate::install(path, machine, &alreadyThere, &error)) {
            std::fprintf(stderr, "Could not install %s: %s\n", path.c_str(), error.c_str());
            if (machine) {
                std::fprintf(stderr, "--machine needs an elevated shell (Run as administrator).\n");
            }
            return 1;
        }
        if (alreadyThere > 0) {
            std::printf("Already trusted in %d store(s); the rest were written.\n", alreadyThere);
        }
        std::printf("Trusted %s (%s).\n", path.c_str(), machine ? "machine" : "current user");
        return 0;
    }

    if (command == "uninstall") {
        int removed = 0;
        if (!linear::certificate::remove(path, machine, &removed, &error)) {
            std::fprintf(stderr, "Could not remove %s: %s\n", path.c_str(), error.c_str());
            return 1;
        }
        std::printf(removed == 0 ? "Was not trusted; nothing to remove.\n"
                                 : "Removed the certificate from %d store(s).\n",
                    removed);
        return 0;
    }

    bool trusted = false;
    if (!linear::certificate::isTrusted(path, machine, &trusted, &error)) {
        std::fprintf(stderr, "Could not read %s: %s\n", path.c_str(), error.c_str());
        return 1;
    }
    std::printf("%s is %s (%s).\n", path.c_str(), trusted ? "trusted" : "not trusted",
                machine ? "machine" : "current user");
    return trusted ? 0 : 1;
}
