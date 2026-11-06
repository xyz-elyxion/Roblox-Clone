// The installer and update launcher.
//
// Two jobs, one executable: it is what the Windows installer puts behind the
// Play shortcut, and it is what a machine uses when there is no installer at
// all. On every start it asks the release server whether the copy it is sitting
// in is current, downloads only the files that changed, checks each one against
// the SHA-256 in the manifest, and then starts the game. A machine that is
// offline, or that has a newer build than the server knows about, still starts
// the game: being unable to check is not a reason to refuse to play.
//
// Commands:
//   (none)            check and apply, then exit
//   --launch          the same, then start the program named in update.json
//   check             say whether an update exists; exit 10 if it does
//   update            check and apply
//   install           fill an empty (or existing) folder from a manifest URL
//   manifest          write manifest.json for a built folder, ready to publish
//   uninstall         remove the files the installed manifest lists, keep data
//
// Publishing a release is therefore two commands: package the build, then
//   RBLXUpdater.exe manifest --dir dist\rblx-windows-release --version 1.2.0
// and serve that folder over HTTP.

#include "HttpClient.h"
#include "Updater.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#ifdef _WIN32
#include <direct.h>
#include <process.h>
#else
#include <unistd.h>
#endif

namespace {

namespace fs = std::filesystem;
using rblx::update::Config;
using rblx::update::FileEntry;
using rblx::update::Manifest;
using rblx::update::Plan;

struct Options {
    std::string command;
    std::string directory;
    std::string manifestUrl;
    std::string version;
    std::string channel = "stable";
    bool launch = false;
    bool quiet = false;
};

void printError(const std::string& text) { std::cerr << "RBLXUpdater: " << text << std::endl; }
void print(const Options& options, const std::string& text) {
    if (!options.quiet) std::cout << text << std::endl;
}

void usage() {
    std::cout << "RBLXUpdater - installs and updates RBLX Clone\n\n"
                 "  RBLXUpdater [--launch]        apply any update, then start the game\n"
                 "  RBLXUpdater check             report whether an update exists (exit 10 if so)\n"
                 "  RBLXUpdater update            apply any update and exit\n"
                 "  RBLXUpdater install --manifest <url> [--dir <folder>]\n"
                 "  RBLXUpdater manifest --dir <folder> --version <version> [--channel stable]\n"
                 "  RBLXUpdater uninstall [--dir <folder>]\n\n"
                 "Options:\n"
                 "  --dir <folder>        the install folder, default: the one this exe is in\n"
                 "  --manifest <url>      manifest.json to use instead of update.json\n"
                 "  --version <version>   version to record when writing a manifest\n"
                 "  --channel <name>      channel to record when writing a manifest\n"
                 "  --launch              start the program named in update.json afterwards\n"
                 "  --quiet               only print warnings and errors\n";
}

std::string executableDirectory(const char* argv0) {
    std::error_code error;
    const fs::path executable = fs::weakly_canonical(fs::absolute(fs::path(argv0 ? argv0 : "."), error), error);
    if (error || !executable.has_parent_path()) return std::string(".");
    return executable.parent_path().string();
}

// Everything before the last slash: the folder the files are served from.
std::string baseUrlOf(const std::string& url) {
    const std::size_t slash = url.find_last_of('/');
    if (slash == std::string::npos) return url;
    return url.substr(0, slash);
}

bool readValue(int argc, char** argv, int index, const std::string& flag, std::string* out) {
    if (index >= argc) {
        printError(flag + " needs a value");
        return false;
    }
    *out = argv[index];
    return true;
}

// Returns false when the arguments are unusable or help was asked for; `command`
// says which.
bool parseOptions(int argc, char** argv, Options* options) {
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--dir") {
            if (!readValue(argc, argv, ++i, "--dir", &options->directory)) return false;
        } else if (argument == "--manifest") {
            if (!readValue(argc, argv, ++i, "--manifest", &options->manifestUrl)) return false;
        } else if (argument == "--version") {
            if (!readValue(argc, argv, ++i, "--version", &options->version)) return false;
        } else if (argument == "--channel") {
            if (!readValue(argc, argv, ++i, "--channel", &options->channel)) return false;
        } else if (argument == "--launch") {
            options->launch = true;
        } else if (argument == "--quiet") {
            options->quiet = true;
        } else if (argument == "--help" || argument == "-h" || argument == "help") {
            options->command = "help";
            return false;
        } else if (argument.rfind("--", 0) == 0) {
            printError("unknown option " + argument);
            return false;
        } else if (options->command.empty()) {
            options->command = argument;
        } else {
            printError("unexpected argument " + argument);
            return false;
        }
    }
    return true;
}

bool fetch(const std::string& url, std::string* body, std::string* error) {
    const rblx::http::Response response =
        rblx::http::request("GET", url, std::string(), std::string(), 120000);
    if (!response.ok()) {
        if (!response.error.empty()) {
            *error = response.error + " (" + url + ")";
        } else {
            *error = "the server answered " + std::to_string(response.status) + " for " + url;
        }
        return false;
    }
    *body = response.body;
    return true;
}

bool fetchManifest(const std::string& url, Manifest* manifest, std::string* error) {
    std::string body;
    if (!fetch(url, &body, error)) return false;
    return rblx::update::parseManifest(body, manifest, error);
}

// The file is checked before it is written, and written through a rename, so a
// connection that dies halfway leaves nothing that looks installed.
bool downloadEntry(const fs::path& staging, const std::string& baseUrl, const FileEntry& entry,
                   std::string* error) {
    const std::string url = rblx::update::joinUrl(baseUrl, entry.path);
    if (url.empty()) {
        *error = "could not build a download address for " + entry.path;
        return false;
    }

    std::string body;
    if (!fetch(url, &body, error)) return false;

    if (entry.size > 0 && static_cast<long long>(body.size()) != entry.size) {
        *error = entry.path + " is " + std::to_string(body.size()) + " bytes, the manifest says " +
                 std::to_string(entry.size);
        return false;
    }
    const std::string digest = rblx::update::sha256Text(body);
    if (digest != entry.sha256) {
        *error = entry.path + " does not match its SHA-256 (expected " + entry.sha256 + ", got " +
                 digest + "); the download was corrupted or the release changed under it";
        return false;
    }

    return rblx::update::writeFileAtomically((staging / fs::path(entry.path)).string(), body, error);
}

// Folders emptied by a removed file would otherwise stay behind forever.
void removeEmptyDirectories(const std::string& directory) {
    std::error_code error;
    const fs::path root = fs::weakly_canonical(directory, error);
    if (error) return;

    std::vector<fs::path> folders;
    for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, error),
         end;
         it != end; it.increment(error)) {
        if (error) break;
        if (it->is_directory(error)) folders.push_back(it->path());
    }

    // Deepest first, so a folder that only held folders also disappears.
    std::sort(folders.begin(), folders.end(),
              [](const fs::path& a, const fs::path& b) { return a.string().size() > b.string().size(); });
    for (const fs::path& folder : folders) {
        std::error_code ignored;
        if (fs::is_empty(folder, ignored)) fs::remove(folder, ignored);
    }
}

// Brings `directory` to the release the manifest describes. `install` means
// there is no local record to diff against, so every file is fetched.
bool sync(const Options& options, const std::string& manifestUrl, bool install,
          Manifest* applied, std::string* error) {
    Manifest target;
    if (!fetchManifest(manifestUrl, &target, error)) return false;

    const fs::path root(options.directory);
    const std::string installedPath = (root / "manifest.json").string();

    Manifest installed;
    if (!install) {
        std::string ignored;
        rblx::update::readManifestFile(installedPath, &installed, &ignored);
        if (!installed.version.empty() &&
            rblx::update::compareVersions(target.version, installed.version) < 0) {
            print(options, "installed " + installed.version + " is newer than the published " +
                               target.version + "; keeping the installed copy");
            if (applied) *applied = installed;
            return true;
        }
    }

    const Plan plan = rblx::update::planUpdate(target, installed, options.directory);
    if (plan.upToDate() && !installed.version.empty()) {
        print(options, "up to date (" + target.version + ")");
        if (applied) *applied = target;
        return true;
    }

    const std::string baseUrl = baseUrlOf(manifestUrl);
    const fs::path staging = root / ".update-staging";
    std::error_code ignored;
    fs::remove_all(staging, ignored);
    fs::create_directories(staging, ignored);

    print(options, std::string(installed.version.empty() ? "installing " : "updating to ") +
                        target.version + ": " + std::to_string(plan.download.size()) +
                        " file(s), " + std::to_string(plan.downloadBytes / (1024 * 1024)) +
                        " MB to download");

    for (const FileEntry& entry : plan.download) {
        if (!downloadEntry(staging, baseUrl, entry, error)) {
            fs::remove_all(staging, ignored);
            return false;
        }
        print(options, "  fetched " + entry.path);
    }

    for (const FileEntry& entry : plan.download) {
        std::string resolved;
        if (!rblx::update::resolveUnder(options.directory, entry.path, &resolved)) {
            *error = "refusing to write outside the install folder: " + entry.path;
            return false;
        }
        const fs::path staged = staging / fs::path(entry.path);
        if (!rblx::update::applyStagedFile(staged.string(), resolved, error)) return false;
    }

    for (const std::string& path : plan.remove) {
        std::string resolved;
        if (!rblx::update::resolveUnder(options.directory, path, &resolved)) continue;
        fs::remove(resolved, ignored);
        print(options, "  removed " + path);
    }

    removeEmptyDirectories(options.directory);

    if (!rblx::update::writeManifestFile(installedPath, target, error)) return false;
    fs::remove_all(staging, ignored);

    print(options, std::string(installed.version.empty() ? "installed " : "updated to ") +
                        target.version + " in " + options.directory);
    if (applied) *applied = target;
    return true;
}

int checkCommand(const Options& options, const std::string& manifestUrl) {
    Manifest target;
    std::string error;
    if (!fetchManifest(manifestUrl, &target, &error)) {
        printError(error);
        return 1;
    }

    Manifest installed;
    std::string ignored;
    rblx::update::readManifestFile((fs::path(options.directory) / "manifest.json").string(), &installed,
                                   &ignored);

    const int comparison = rblx::update::compareVersions(target.version, installed.version);
    if (installed.version.empty()) {
        std::cout << "not installed; the published release is " << target.version << std::endl;
        return 10;
    }
    if (comparison > 0) {
        const Plan plan = rblx::update::planUpdate(target, installed, options.directory);
        std::cout << "an update is available: " << installed.version << " -> " << target.version << " ("
                  << plan.download.size() << " file(s), " << plan.downloadBytes / (1024 * 1024)
                  << " MB)" << std::endl;
        return 10;
    }
    std::cout << "up to date (" << installed.version << ")" << std::endl;
    return 0;
}

int manifestCommand(const Options& options) {
    if (options.version.empty()) {
        printError("manifest needs --version, for example --version 1.2.0");
        return 1;
    }

    Manifest manifest;
    manifest.version = options.version;
    manifest.channel = options.channel;

    std::error_code error;
    if (!fs::is_directory(options.directory, error)) {
        printError("no folder at " + options.directory);
        return 1;
    }

    std::vector<std::string> paths;
    for (fs::recursive_directory_iterator it(options.directory,
                                             fs::directory_options::skip_permission_denied, error),
         end;
         it != end; it.increment(error)) {
        if (error) {
            printError("could not list " + options.directory + ": " + error.message());
            return 1;
        }
        if (!it->is_regular_file(error)) continue;

        const std::string relative = fs::relative(it->path(), options.directory, error).generic_string();
        if (error) {
            printError("could not describe " + it->path().string());
            return 1;
        }
        if (!rblx::update::isPackagedFile(relative)) continue;
        paths.push_back(relative);
    }

    // Sorted, so two runs over the same folder produce the same document and a
    // release can be compared line by line.
    std::sort(paths.begin(), paths.end());
    long long total = 0;
    for (const std::string& relative : paths) {
        std::string digestError;
        const std::string path = (fs::path(options.directory) / fs::path(relative)).string();
        FileEntry entry;
        entry.path = relative;
        entry.sha256 = rblx::update::sha256File(path, &digestError);
        if (entry.sha256.empty()) {
            printError(digestError);
            return 1;
        }
        const auto size = fs::file_size(path, error);
        if (error) {
            printError("could not measure " + path);
            return 1;
        }
        entry.size = static_cast<long long>(size);
        total += entry.size;
        manifest.files.push_back(entry);
    }

    const std::string manifestPath = (fs::path(options.directory) / "manifest.json").string();
    std::string writeError;
    if (!rblx::update::writeManifestFile(manifestPath, manifest, &writeError)) {
        printError(writeError);
        return 1;
    }

    std::cout << "wrote " << manifestPath << ": version " << manifest.version << ", "
              << manifest.files.size() << " file(s), " << total / (1024 * 1024) << " MB" << std::endl;
    return 0;
}

int uninstallCommand(const Options& options) {
    const fs::path root(options.directory);
    Manifest installed;
    std::string error;
    if (!rblx::update::readManifestFile((root / "manifest.json").string(), &installed, &error)) {
        printError(error);
        return 1;
    }

    int removed = 0;
    std::error_code ignored;
    for (const FileEntry& entry : installed.files) {
        std::string resolved;
        if (!rblx::update::resolveUnder(options.directory, entry.path, &resolved)) continue;
        if (fs::remove(resolved, ignored)) ++removed;
    }

    removeEmptyDirectories(options.directory);
    fs::remove(root / "manifest.json", ignored);
    fs::remove(root / "update.json", ignored);

    std::cout << "removed " << removed << " file(s) from " << options.directory
              << "; worlds, saves and account data are left in place" << std::endl;
    return 0;
}

bool launchProgram(const std::string& directory, const std::string& program, std::string* error) {
    const fs::path path = fs::path(directory) / fs::path(program);
    if (!fs::exists(path)) {
        *error = "there is no " + path.string() + " to start";
        return false;
    }

#ifdef _WIN32
    _chdir(directory.c_str());
    const std::string command = path.string();
    if (_spawnl(_P_NOWAIT, command.c_str(), command.c_str(), static_cast<char*>(nullptr)) == -1) {
        *error = "could not start " + command;
        return false;
    }
#else
    const pid_t pid = fork();
    if (pid < 0) {
        *error = "could not start " + path.string();
        return false;
    }
    if (pid == 0) {
        if (chdir(directory.c_str()) != 0) _exit(127);
        execl(path.c_str(), path.c_str(), static_cast<char*>(nullptr));
        _exit(127);
    }
#endif
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    if (!parseOptions(argc, argv, &options)) {
        usage();
        return options.command == "help" ? 0 : 2;
    }
    if (options.directory.empty()) options.directory = executableDirectory(argv[0]);

    const std::string command = options.command;
    if (command == "manifest") return manifestCommand(options);
    if (command == "uninstall") return uninstallCommand(options);

    const bool installing = command == "install";
    if (!command.empty() && command != "install" && command != "update" && command != "check") {
        printError("unknown command " + command);
        usage();
        return 2;
    }

    const std::string configPath = (fs::path(options.directory) / "update.json").string();
    Config config;
    std::string error;
    if (options.manifestUrl.empty()) {
        if (!rblx::update::readConfigFile(configPath, &config, &error)) {
            printError(error + "; run: RBLXUpdater install --manifest <url>");
            return 1;
        }
    } else {
        std::string ignored;
        rblx::update::readConfigFile(configPath, &config, &ignored);
        config.manifestUrl = options.manifestUrl;
    }

    if (command == "check") return checkCommand(options, config.manifestUrl);

    Manifest applied;
    const bool updated = sync(options, config.manifestUrl, installing, &applied, &error);
    if (!updated) {
        // Being unable to update is not a reason to refuse to play: say what
        // happened and start the game anyway.
        if (options.launch) {
            printError(error);
            printError("starting the installed copy unchanged");
        } else {
            printError(error);
            return 1;
        }
    }

    if (installing || !options.manifestUrl.empty()) {
        std::string writeError;
        if (!rblx::update::writeConfigFile(configPath, config, &writeError)) {
            printError(writeError);
            if (!options.launch) return 1;
        }
    }

    if (!options.launch) return 0;
    if (config.launch.empty()) config.launch = "Client.exe";
    if (!launchProgram(options.directory, config.launch, &error)) {
        printError(error);
        return 1;
    }
    return 0;
}
