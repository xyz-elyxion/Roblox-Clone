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
#include "InstallUi.h"
#include "ReleaseSignature.h"
#include "Updater.h"
#include "WebJson.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

#include <direct.h>
#include <process.h>

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
    // The signing key file for `sign-manifest`; never part of a release.
    std::string signingKey;
    bool launch = false;
    bool quiet = false;
    // A window instead of the console; --no-ui forces the console.
    bool ui = false;
    bool noUi = false;
};

// Where a windowed run sends its lines. Empty means the console, which is what
// every script, test and packaging step uses.
std::function<void(const std::string&)> g_progress;
std::function<void(const std::string&)> g_problem;
std::function<bool()> g_cancelled;
// Bytes downloaded and the size of the whole download, for the window's bar.
std::function<void(long long, long long)> g_bytes;

bool cancelled() { return g_cancelled && g_cancelled(); }

void printError(const std::string& text) {
    if (g_problem) {
        g_problem(text);
        return;
    }
    std::cerr << "RBLXUpdater: " << text << std::endl;
}

void print(const Options& options, const std::string& text) {
    if (g_progress) {
        g_progress(text);
        return;
    }
    if (!options.quiet) std::cout << text << std::endl;
}

void usage() {
    std::cout << "RBLXUpdater - installs and updates RBLX Clone\n\n"
                 "  RBLXUpdater [--launch]        apply any update, then start the game\n"
                 "  RBLXUpdater check             report whether an update exists (exit 10 if so)\n"
                 "  RBLXUpdater update            apply any update and exit\n"
                 "  RBLXUpdater install --manifest <url> [--dir <folder>]\n"
                 "  RBLXUpdater manifest --dir <folder> --version <version> [--channel stable]\n"
                 "  RBLXUpdater sign-manifest --dir <folder> --key <keyfile>\n"
                 "  RBLXUpdater uninstall [--dir <folder>]\n\n"
                 "Options:\n"
                 "  --dir <folder>        the install folder, default: the one this exe is in\n"
                 "  --manifest <url>      manifest.json to use instead of update.json\n"
                 "  --version <version>   version to record when writing a manifest\n"
                 "  --channel <name>      channel to record when writing a manifest\n"
                 "  --key <file>          release signing key (modulus, exponent, privateExponent)\n"
                 "  --launch              start the program named in update.json afterwards\n"
                 "  --ui                  show the work in a window (default for --launch)\n"
                 "  --no-ui               never show a window, even if --ui is implied\n"
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
        } else if (argument == "--key") {
            if (!readValue(argc, argv, ++i, "--key", &options->signingKey)) return false;
        } else if (argument == "--launch") {
            options->launch = true;
        } else if (argument == "--ui") {
            options->ui = true;
        } else if (argument == "--no-ui") {
            options->noUi = true;
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

bool fetch(const std::string& url, std::string* body, std::string* error,
           const rblx::http::ProgressCallback& onProgress = rblx::http::ProgressCallback(),
           int* status = nullptr) {
    // g_cancelled makes the download itself interruptible, not just the space
    // between two files: a large file or an unreachable server would otherwise
    // leave the window's Cancel button with nothing to act on for two minutes.
    const rblx::http::Response response =
        rblx::http::request("GET", url, std::string(), std::string(), 120000, onProgress,
                            g_cancelled);
    if (status) *status = response.status;
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
}// Asking the release server ends three ways. "Nothing published" is not a
// failure: the website serves /updates/ out of the folder releases are copied
// into, so a fresh copy of this project has nothing there yet. The updater then
// keeps the copy it has and the game still starts.
enum class ManifestFetch { Ok, NotPublished, Failed };

// A copy with a pinned key installs only a manifest that key signed. A copy with
// no key is a development install reading a release folder on this machine, so
// it may only use a loopback URL: that is the one case where nobody on the
// network can rewrite the answer, and it keeps a fresh checkout working.
bool manifestIsTrusted(const std::string& url, const Config& config, const Manifest& manifest,
                       std::string* error) {
    if (!config.signatureModulus.empty()) {
        return rblx::update::verifyManifestSignature(manifest, config.signatureModulus,
                                                     config.signatureExponent, error);
    }

    const std::size_t scheme = url.find("://");
    std::string host = url.substr(scheme == std::string::npos ? 0 : scheme + 3);
    const std::size_t slash = host.find('/');
    if (slash != std::string::npos) host = host.substr(0, slash);
    const std::size_t colon = host.find(':');
    if (colon != std::string::npos) host = host.substr(0, colon);
    if (host == "127.0.0.1" || host == "localhost" || host == "[::1]" || host == "::1") {
        return true;
    }

    if (error) {
        *error = "this copy has no release signing key, so " + url +
                 " is not trusted. Sign the release and put the key it prints into "
                 "update.json as signatureKey, or point this copy at a loopback release folder.";
    }
    return false;
}

ManifestFetch fetchManifest(const std::string& url, const Config& config, Manifest* manifest,
                            std::string* error) {
    std::string body;
    int status = 0;
    if (!fetch(url, &body, error, rblx::http::ProgressCallback(), &status)) {
        if (status == 404) return ManifestFetch::NotPublished;
        return ManifestFetch::Failed;
    }
    if (!rblx::update::parseManifest(body, manifest, error)) return ManifestFetch::Failed;
    if (!manifestIsTrusted(url, config, *manifest, error)) return ManifestFetch::Failed;
    return ManifestFetch::Ok;
}

// The file is checked before it is written, and written through a rename, so a
// connection that dies halfway leaves nothing that looks installed.
bool downloadEntry(const fs::path& staging, const std::string& baseUrl, const FileEntry& entry,
                   const rblx::http::ProgressCallback& onProgress, std::string* error) {
    const std::string url = rblx::update::joinUrl(baseUrl, entry.path);
    if (url.empty()) {
        *error = "could not build a download address for " + entry.path;
        return false;
    }

    std::string body;
    if (!fetch(url, &body, error, onProgress)) return false;

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
bool sync(const Options& options, const Config& config, bool install,
          Manifest* applied, std::string* error) {
    const std::string& manifestUrl = config.manifestUrl;
    Manifest target;
    // Say what is being contacted before the request: a slow or unreachable
    // server otherwise leaves the window showing nothing at all.
    print(options, "checking " + manifestUrl + " for a newer release");
    const ManifestFetch fetched = fetchManifest(manifestUrl, config, &target, error);
    if (fetched == ManifestFetch::NotPublished) {
        // Installing from scratch has nothing to copy, so a missing release is
        // still an error there; an existing copy simply stays as it is.
        if (install) {
            *error = "nothing is published at " + manifestUrl + " to install from";
            return false;
        }
        print(options, "nothing is published at " + manifestUrl + " yet; keeping this copy");
        return true;
    }
    if (fetched == ManifestFetch::Failed) {
        if (cancelled()) *error = "cancelled; the installed copy is unchanged";
        return false;
    }

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

    // The bar follows the bytes that actually arrive, not a guess: the manifest
    // says how much these files weigh, and the client reports each chunk.
    long long downloaded = 0;
    if (g_bytes) g_bytes(0, plan.downloadBytes);

    for (const FileEntry& entry : plan.download) {
        // Only between files: the file being fetched is finished, so the install
        // is left as it was rather than half replaced.
        if (cancelled()) {
            fs::remove_all(staging, ignored);
            *error = "cancelled; the installed copy is unchanged";
            return false;
        }

        long long thisFile = 0;
        const rblx::http::ProgressCallback onProgress = [&](long long received, long long) {
            if (received > thisFile) thisFile = received;
            if (g_bytes) g_bytes(downloaded + thisFile, plan.downloadBytes);
        };
        if (!downloadEntry(staging, baseUrl, entry, onProgress, error)) {
            fs::remove_all(staging, ignored);
            if (cancelled()) *error = "cancelled; the installed copy is unchanged";
            return false;
        }
        downloaded += thisFile > 0 ? thisFile : entry.size;
        if (g_bytes) g_bytes(downloaded, plan.downloadBytes);
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

int checkCommand(const Options& options, const Config& config) {
    const std::string& manifestUrl = config.manifestUrl;
    Manifest target;
    std::string error;
    const ManifestFetch fetched = fetchManifest(manifestUrl, config, &target, &error);
    if (fetched == ManifestFetch::NotPublished) {
        // Exit 0 is "nothing to install": a monitor that polls this must not be
        // woken up by a release server that has not published anything.
        std::cout << "nothing is published at " << manifestUrl << " yet" << std::endl;
        return 0;
    }
    if (fetched == ManifestFetch::Failed) {
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
    std::string error;
    if (!rblx::update::buildManifestFromDirectory(options.directory, options.version,
                                                  options.channel, &manifest, &error)) {
        printError(error);
        return 1;
    }

    long long total = 0;
    for (const FileEntry& entry : manifest.files) total += entry.size;

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

// Signs the manifest this folder already has. The private exponent stays in the
// key file named by --key and never enters the release; the public half is
// printed so the packaging step can pin it in update.json.
int signManifestCommand(const Options& options) {
    if (options.signingKey.empty()) {
        printError("sign-manifest needs --key <file>");
        return 1;
    }

    const std::string manifestPath = (fs::path(options.directory) / "manifest.json").string();
    Manifest manifest;
    std::string error;
    if (!rblx::update::readManifestFile(manifestPath, &manifest, &error)) {
        printError(error);
        return 1;
    }

    std::string keyText;
    if (!rblx::update::readTextFile(options.signingKey, &keyText)) {
        printError("could not read the signing key at " + options.signingKey);
        return 1;
    }
    const rblx::web::Json key = rblx::web::Json::parse(keyText, &error);
    if (!key.isObject()) {
        printError("the signing key must be JSON with modulus, exponent and privateExponent");
        return 1;
    }
    const std::string modulus = key.member("modulus").asString();
    const std::string exponent = key.member("exponent").asString("10001");
    if (!rblx::update::isValidRsaKey(modulus, exponent, &error)) {
        printError("the signing key: " + error);
        return 1;
    }

    const std::string payload = rblx::update::manifestSigningPayload(manifest);
    std::string signature;
    if (!rblx::update::signRsaSha256(payload.data(), payload.size(), modulus,
                                     key.member("privateExponent").asString(), &signature,
                                     &error)) {
        printError("could not sign the manifest: " + error);
        return 1;
    }

    manifest.signature = signature;
    if (!rblx::update::writeManifestFile(manifestPath, manifest, &error)) {
        printError(error);
        return 1;
    }
    std::cout << "signed " << manifestPath << " (version " << manifest.version << ")\n"
              << "modulus=" << modulus << '\n'
              << "exponent=" << exponent << std::endl;
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

    _chdir(directory.c_str());
    const std::string command = path.string();
    if (_spawnl(_P_NOWAIT, command.c_str(), command.c_str(), static_cast<char*>(nullptr)) == -1) {
        *error = "could not start " + command;
        return false;
    }
    return true;
}

// Applies whatever the release server has, records the settings when this run
// supplied them, and starts the game when asked to. The console path and the
// window both come through here, so the two cannot drift apart.
int applyRelease(Options& options, const Config& config, const std::string& configPath,
                 bool installing, std::string* error) {
    Manifest applied;
    const bool updated = sync(options, config, installing, &applied, error);
    if (!updated) {
        // Being unable to update is not a reason to refuse to play: say what
        // happened and start the game anyway.
        printError(*error);
        if (!options.launch) return 1;
        printError("starting the installed copy unchanged");
    }

    if (installing || !options.manifestUrl.empty()) {
        std::string writeError;
        if (!rblx::update::writeConfigFile(configPath, config, &writeError)) {
            printError(writeError);
            if (!options.launch) return 1;
        }
    }

    if (!options.launch) return 0;
    const std::string program = config.launch.empty() ? "Client.exe" : config.launch;
    if (!launchProgram(options.directory, program, error)) {
        printError(*error);
        return 1;
    }
    return 0;
}

// Runs the update in a window and starts the game from its Play button. Returns
// -1 when no window could be created, so the caller can use the console.
int windowed(Options& options, bool installing) {
    rblx::ui::Options window;
    window.title = "RBLX Clone";
    window.heading = "RBLX Clone";
    window.detail =
        "Checks for a new release, downloads only the files that changed, then starts the game.";
    window.showFolder = false;
    window.autoStart = true;

    const std::vector<std::string> actions{"Play", "Close"};

    const rblx::ui::Result result = rblx::ui::run(window, actions, [&](rblx::ui::Report& report) {
        Options run = options;
        run.launch = false;  // the window's Play button starts the game
        run.quiet = false;

        g_progress = [&report](const std::string& text) { report.line(text); };
        g_problem = [&report](const std::string& text) { report.problem(text); };
        g_cancelled = [&report]() { return report.cancelled(); };
        g_bytes = [&report](long long done, long long total) {
            report.progressBytes(done, total);
        };

        const std::string configPath = (fs::path(run.directory) / "update.json").string();
        Config config;
        std::string error;
        bool ok = false;
        if (run.manifestUrl.empty() && !rblx::update::readConfigFile(configPath, &config, &error)) {
            printError(error + "; run: RBLXUpdater install --manifest <url>");
        } else {
            if (!run.manifestUrl.empty()) {
                std::string ignored;
                rblx::update::readConfigFile(configPath, &config, &ignored);
                config.manifestUrl = run.manifestUrl;
            }
            ok = applyRelease(run, config, configPath, installing, &error) == 0;
        }

        g_progress = nullptr;
        g_problem = nullptr;
        g_cancelled = nullptr;
        g_bytes = nullptr;
        return ok;
    });

    if (!result.shown) return -1;
    if (result.action != "Play") return result.ok ? 0 : 1;

    Config config;
    std::string ignored;
    rblx::update::readConfigFile((fs::path(options.directory) / "update.json").string(), &config,
                                 &ignored);
    std::string error;
    const std::string program = config.launch.empty() ? "Client.exe" : config.launch;
    if (!launchProgram(options.directory, program, &error)) {
        printError(error);
        return 1;
    }
    return 0;
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
    if (command == "sign-manifest") return signManifestCommand(options);
    if (command == "uninstall") return uninstallCommand(options);

    const bool installing = command == "install";
    if (!command.empty() && command != "install" && command != "update" && command != "check") {
        printError("unknown command " + command);
        usage();
        return 2;
    }

    // The Play shortcut runs `--launch` with nothing else, and a person pressed
    // it: show the update in a window. `--ui` asks for the window explicitly, and
    // every other command stays on the console because a script is driving it.
    // This runs before the settings are read, so a missing update.json is
    // explained in the window rather than on a console nobody can see.
    if (!options.noUi && (options.ui || (command.empty() && options.launch))) {
        const int status = windowed(options, installing);
        if (status >= 0) return status;
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

    if (command == "check") return checkCommand(options, config);

    return applyRelease(options, config, configPath, installing, &error);
}
