// LinearCert: puts the project's signing certificate where Windows looks for it.
//
// Signing each executable embeds the certificate, but Windows still asks who the publisher is
// until that certificate is in the trust stores this program writes to: ROOT, so a signature
// chain ending in it is accepted, and TrustedPublisher, so a signed program starts without an
// "unknown publisher" prompt. Per-user needs no administrator; --machine does.
//
// Double-clicked, it draws the installer's window instead of a console: the window reports what
// this machine already thinks of the certificate and offers the one action that changes it -
// trust it, or stop trusting it - so removing the trust is a button rather than a command
// somebody has to know about. The arguments are unchanged, because they are what the packaging
// scripts and CI call, and a machine that cannot make a window falls back to them.

#include "CertificateTrust.h"
#include "InstallUi.h"

#include <windows.h>

#include <cstdio>
#include <functional>
#include <string>
#include <vector>

namespace {

// Where a windowed run sends its lines. Empty means the console, which is what
// every argument-driven run uses.
std::function<void(const std::string&)> g_report;
std::function<void(const std::string&)> g_problem;

void say(const std::string& text) {
    if (g_report) {
        g_report(text);
        return;
    }
    std::printf("%s\n", text.c_str());
}

void complain(const std::string& text) {
    if (g_problem) {
        g_problem(text);
        return;
    }
    std::fprintf(stderr, "%s\n", text.c_str());
}

void usage() {
    std::printf(
        "LinearCert - trust (or stop trusting) the Linear signing certificate\n"
        "\n"
        "  LinearCert [--ui] [--file <certificate.cer>]\n"
        "  LinearCert install [--machine] [--file <certificate.cer>]\n"
        "  LinearCert uninstall [--machine] [--file <certificate.cer>]\n"
        "  LinearCert status [--machine] [--file <certificate.cer>]\n"
        "\n"
        "Started with no arguments it opens a window that says whether this machine trusts the\n"
        "certificate and offers to add it or remove it, which is what double-clicking it does.\n"
        "Any argument means a script, so the console is used; --ui asks for the window anyway\n"
        "and --no-ui refuses it.\n"
        "\n"
        "Without --machine the certificate goes in your own stores, which needs no\n"
        "administrator. --machine writes to the machine stores and needs an elevated shell.\n"
        "Without --file it looks for certs\\linear-codesign.cer beside this program.\n"
        "\n"
        "Skip it and the signed programs still run, but Windows asks before starting one and\n"
        "an antivirus has no publisher to check the signature against.\n");
}

struct Options {
    std::string command = "install";
    std::string file;
    bool machine = false;
    bool ui = false;
    bool noUi = false;
    bool help = false;
};

bool parseOptions(int argc, char** argv, Options* options) {
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "install" || argument == "uninstall" || argument == "status") {
            options->command = argument;
        } else if (argument == "--machine") {
            options->machine = true;
        } else if (argument == "--file") {
            if (i + 1 >= argc) {
                complain("--file needs a path.");
                return false;
            }
            options->file = argv[++i];
        } else if (argument == "--ui") {
            options->ui = true;
        } else if (argument == "--no-ui") {
            options->noUi = true;
        } else if (argument == "--help" || argument == "-h" || argument == "/?") {
            options->help = true;
        } else {
            complain("Unknown argument: " + argument);
            usage();
            return false;
        }
    }
    return true;
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
// keeps its own window and needs no help. A run that drew a window has freed its console and
// gets nothing to wait for.
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

const char* scopeName(bool machine) {
    return machine ? "every account on this machine" : "your account";
}

int install(const std::string& path, bool machine) {
    int alreadyThere = 0;
    int storeCount = 0;
    linear::certificate::targets(&storeCount);
    std::string error;
    if (!linear::certificate::install(path, machine, &alreadyThere, &error)) {
        complain("Could not install " + path + ": " + error);
        if (machine) {
            complain("--machine needs an elevated shell: open PowerShell or Command Prompt as");
            complain("administrator and run this again.");
        }
        return 1;
    }
    if (alreadyThere >= storeCount) {
        say(path + " was already trusted for " + scopeName(machine) + "; nothing to do.");
    } else {
        say("Trusted " + path + " for " + scopeName(machine) + ".");
    }
    say("Programs signed with it now start without \"unknown publisher\".");
    if (!machine) {
        say("Run this again with --machine from an administrator shell to trust it for every");
        say("account on this machine.");
    }
    return 0;
}

int remove(const std::string& path, bool machine) {
    int removed = 0;
    std::string error;
    if (!linear::certificate::remove(path, machine, &removed, &error)) {
        complain("Could not remove " + path + ": " + error);
        return 1;
    }
    if (removed == 0) {
        say(std::string("Was not trusted; nothing to remove."));
    } else {
        say("Removed the certificate from " + std::to_string(removed) + " store(s).");
    }
    return 0;
}

int status(const std::string& path, bool machine) {
    bool trusted = false;
    std::string error;
    if (!linear::certificate::isTrusted(path, machine, &trusted, &error)) {
        complain("Could not read " + path + ": " + error);
        return 1;
    }
    say(path + " is " + (trusted ? "trusted" : "not trusted") + " (" +
        (machine ? "machine" : "current user") + ").");
    return trusted ? 0 : 1;
}

// Runs the trust change in the installer's window, which the setup, the updater and the server
// setup already draw with. Returns -1 when no window could be created - a headless machine, a
// build agent, a driver with no context - so the caller can do the same work on the console.
int windowed(const std::string& path, bool machine) {
    bool trusted = false;
    std::string error;
    if (!linear::certificate::isTrusted(path, machine, &trusted, &error)) {
        // Without knowing the state there is no sensible button to offer, and the error belongs
        // somewhere it can be read: the console path prints it with the same wording.
        return -1;
    }

    linear::ui::Options window;
    window.title = "Linear certificate";
    // The window always acts on what the stores say now, so the button it opens with is the
    // action that changes something: trusting an untrusted certificate, or removing a trusted
    // one. A button that would do nothing is a button that looks broken.
    if (trusted) {
        window.heading = "Windows trusts the Linear certificate";
        window.detail =
            "Programs signed with it start without asking who published them. Removing the "
            "certificate brings that prompt back and touches nothing else. SmartScreen's "
            "reputation check is separate from this and is not changed by it.";
    } else {
        window.heading = "Trust the Linear certificate";
        window.detail =
            "Programs signed with this certificate ask who published them until Windows trusts "
            "it. Trusting it here covers your account and needs no administrator.";
    }
    if (!machine) {
        window.detail += " From an administrator shell, --machine covers every account.";
    }
    window.startLabel = trusted ? "Stop trusting it" : "Trust it";
    window.cancelLabel = trusted ? "Keep it" : "Close";
    // Nothing to type: the certificate is the one beside this program, and the scope is the one
    // the run was started with.
    window.showFolder = false;
    window.showShortcuts = false;

    const std::vector<std::string> actions{"Close"};
    const linear::ui::Result result =
        linear::ui::run(window, actions, [&](linear::ui::Report& report) {
            g_report = [&report](const std::string& text) { report.line(text); };
            g_problem = [&report](const std::string& text) { report.problem(text); };

            const bool ok = trusted ? (remove(path, machine) == 0) : (install(path, machine) == 0);

            g_report = nullptr;
            g_problem = nullptr;
            return ok;
        });

    if (!result.shown) return -1;
    if (result.cancelled) return 0;
    return result.ok ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parseOptions(argc, argv, &options)) return finish(2);
    if (options.help) {
        usage();
        return finish(0);
    }

    const std::string path =
        options.file.empty() ? linear::certificate::defaultPath(executablePath()) : options.file;
    if (path.empty()) {
        complain("Could not work out where this program is, so pass --file.");
        return finish(1);
    }

    // No arguments at all means somebody double-clicked the program, so it opens a window. Any
    // argument means a script, which wants the console, unless --ui says otherwise. A window
    // that cannot be created falls back to the console, which is what the scripts use anyway.
    if (!options.noUi && (options.ui || argc == 1)) {
        const int shown = windowed(path, options.machine);
        if (shown >= 0) return shown;
    }

    if (argc <= 1) {
        std::printf("LinearCert - making Windows trust the Linear signing certificate.\n\n");
    }

    if (options.command == "install") return finish(install(path, options.machine));
    if (options.command == "uninstall") return finish(remove(path, options.machine));
    return finish(status(path, options.machine));
}
