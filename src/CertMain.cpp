// LinearCert: puts the project's signing certificate where Windows looks for it.
//
// Signing each executable embeds the certificate, but Windows still asks who the publisher is
// until that certificate is in the trust stores this program writes to: ROOT, so a signature
// chain ending in it is accepted, and TrustedPublisher, so a signed program starts without an
// "unknown publisher" prompt. Per-user needs no administrator; --machine does.
//
// With no arguments it installs, because a double-click in the package folder is how most
// people meet it; see consoleIsOurs for why it then waits for Enter.

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
        "Started with no arguments it installs, which is what double-clicking it does.\n"
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

// Explorer gives a console program a console of its own and closes it the moment the program
// exits, so a double-clicked LinearCert.exe had already lost its output by the time anyone
// looked at it: installing looked like it did nothing, and pressing it again looked the same
// because the certificate was already there. A console with exactly one process on it is a
// console that exists only for this run, so keep it open; a shell that started the program
// keeps its own window and needs no help.
bool consoleIsOurs() {
    DWORD processes[4] = {};
    const DWORD count = GetConsoleProcessList(
        processes, static_cast<DWORD>(sizeof(processes) / sizeof(processes[0])));
    return count == 1;
}

// Every way out goes through here, so a failure stays on screen as long as a success does.
int finish(int code) {
    if (consoleIsOurs()) {
        std::printf("\nPress Enter to close this window.\n");
        std::fflush(stdout);
        int key = 0;
        do {
            key = std::getchar();
        } while (key != EOF && key != '\n');
    }
    return code;
}

}  // namespace

int main(int argc, char** argv) {
    std::string command = "install";
    std::string file;
    bool machine = false;

    if (argc <= 1) {
        std::printf("LinearCert - making Windows trust the Linear signing certificate.\n\n");
    }

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "install" || argument == "uninstall" || argument == "status") {
            command = argument;
        } else if (argument == "--machine") {
            machine = true;
        } else if (argument == "--file") {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "--file needs a path.\n");
                return finish(2);
            }
            file = argv[++i];
        } else if (argument == "--help" || argument == "-h" || argument == "/?") {
            usage();
            return finish(0);
        } else {
            std::fprintf(stderr, "Unknown argument: %s\n\n", argument.c_str());
            usage();
            return finish(2);
        }
    }

    const std::string path =
        file.empty() ? linear::certificate::defaultPath(executablePath()) : file;
    if (path.empty()) {
        std::fprintf(stderr, "Could not work out where this program is, so pass --file.\n");
        return finish(1);
    }

    std::string error;
    if (command == "install") {
        int alreadyThere = 0;
        int storeCount = 0;
        linear::certificate::targets(&storeCount);
        if (!linear::certificate::install(path, machine, &alreadyThere, &error)) {
            std::fprintf(stderr, "Could not install %s: %s\n", path.c_str(), error.c_str());
            if (machine) {
                std::fprintf(stderr,
                             "--machine needs an elevated shell: open PowerShell or Command\n"
                             "Prompt as administrator and run this again.\n");
            }
            return finish(1);
        }
        if (alreadyThere >= storeCount) {
            std::printf("%s was already trusted for %s; nothing to do.\n", path.c_str(),
                        machine ? "every account on this machine" : "your account");
        } else {
            std::printf("Trusted %s for %s.\n", path.c_str(),
                        machine ? "every account on this machine" : "your account");
        }
        std::printf("Programs signed with it now start without \"unknown publisher\".\n");
        if (!machine) {
            std::printf("Run this again with --machine from an administrator shell to trust it\n"
                        "for every account on this machine.\n");
        }
        return finish(0);
    }

    if (command == "uninstall") {
        int removed = 0;
        if (!linear::certificate::remove(path, machine, &removed, &error)) {
            std::fprintf(stderr, "Could not remove %s: %s\n", path.c_str(), error.c_str());
            return finish(1);
        }
        std::printf(removed == 0 ? "Was not trusted; nothing to remove.\n"
                                 : "Removed the certificate from %d store(s).\n",
                    removed);
        return finish(0);
    }

    bool trusted = false;
    if (!linear::certificate::isTrusted(path, machine, &trusted, &error)) {
        std::fprintf(stderr, "Could not read %s: %s\n", path.c_str(), error.c_str());
        return finish(1);
    }
    std::printf("%s is %s (%s).\n", path.c_str(), trusted ? "trusted" : "not trusted",
                machine ? "machine" : "current user");
    return finish(trusted ? 0 : 1);
}
