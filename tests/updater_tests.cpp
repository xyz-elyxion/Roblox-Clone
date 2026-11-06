// The updater's decisions, checked without a network or a release server: how
// versions order, what a manifest may name, which files a plan touches, and that
// a replaced file is only gone once its replacement is in position.

#include "Updater.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;
int failures = 0;

void check(bool ok, const std::string& what) {
    std::printf("%s %s\n", ok ? "ok  " : "FAIL", what.c_str());
    if (!ok) ++failures;
}

void writeFile(const fs::path& path, const std::string& text) {
    fs::create_directories(path.parent_path());
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    file << text;
}

std::string readFile(const fs::path& path) {
    std::ifstream file(path, std::ios::binary);
    return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
}

// A clean folder per test, so one failure cannot look like another.
fs::path makeScratch(const std::string& name) {
    const fs::path root = fs::temp_directory_path() / ("rblx-updater-tests-" + name);
    std::error_code error;
    fs::remove_all(root, error);
    fs::create_directories(root, error);
    return root;
}

void testVersionOrder() {
    using rblx::update::compareVersions;
    check(compareVersions("1.0.0", "1.0.0") == 0, "the same version compares equal");
    check(compareVersions("1.10.0", "1.9.2") > 0, "1.10.0 is newer than 1.9.2, not older");
    check(compareVersions("2", "1.99.99") > 0, "a major bump outranks any minor one");
    check(compareVersions("1.2", "1.2.0") == 0, "a missing component counts as zero");
    check(compareVersions("1.3.0-beta", "1.2.9") > 0, "a channel suffix does not change the order");
    check(compareVersions("", "0.0.1") < 0, "an empty version is the oldest");
    check(compareVersions("v1.1.0", "1.0.0") > 0, "a leading v is not part of the number");
}

void testPathSafety() {
    using rblx::update::isSafeRelativePath;
    check(isSafeRelativePath("shaders/basic.vert", nullptr), "a normal relative path is accepted");
    check(isSafeRelativePath("Client.exe", nullptr), "a top level file is accepted");

    std::string reason;
    check(!isSafeRelativePath("../outside.exe", &reason), "a parent directory is refused");
    check(!isSafeRelativePath("games/../../outside.exe", nullptr), "a nested escape is refused");
    check(!isSafeRelativePath("/etc/passwd", nullptr), "an absolute path is refused");
    check(!isSafeRelativePath("C:/Windows/System32/evil.dll", nullptr), "a drive letter is refused");
    check(!isSafeRelativePath("shaders\\basic.vert", nullptr), "a backslash path is refused");
    check(!isSafeRelativePath("games//one.world", nullptr), "an empty segment is refused");
    check(!isSafeRelativePath("./Client.exe", nullptr), "a dot segment is refused");
    check(!isSafeRelativePath("", nullptr), "an empty path is refused");

    const fs::path root = makeScratch("resolve");
    std::string resolved;
    check(rblx::update::resolveUnder(root.string(), "web/public/index.html", &resolved),
          "a nested path resolves inside the install folder");
    check(resolved.find("web/public/index.html") != std::string::npos,
          "the resolved path is the one asked for");
    check(!rblx::update::resolveUnder(root.string(), "../escape.exe", &resolved),
          "a path that leaves the folder does not resolve");
}

void testUrlJoining() {
    using rblx::update::joinUrl;
    check(joinUrl("http://127.0.0.1:3000/rblx", "shaders/basic.vert") ==
              "http://127.0.0.1:3000/rblx/shaders/basic.vert",
          "a relative path is joined to the base URL");
    check(joinUrl("http://127.0.0.1:3000/rblx/", "Client.exe") ==
              "http://127.0.0.1:3000/rblx/Client.exe",
          "a trailing slash on the base is not doubled");
    check(joinUrl("http://host/updates", "games/Sky Platformer.world") ==
              "http://host/updates/games/Sky%20Platformer.world",
          "a space in a file name is percent-encoded");
    check(joinUrl("http://host/updates", "http://evil.example/x").empty(),
          "a manifest path cannot name its own host");
}

void testManifestDocument() {
    rblx::update::Manifest manifest;
    manifest.version = "1.4.2";
    manifest.channel = "stable";
    rblx::update::FileEntry entry;
    entry.path = "Client.exe";
    entry.sha256 = std::string(64, 'a');
    entry.size = 42;
    manifest.files.push_back(entry);

    const std::string text = rblx::update::buildManifestJson(manifest);
    rblx::update::Manifest parsed;
    std::string error;
    check(rblx::update::parseManifest(text, &parsed, &error), "a manifest survives a round trip");
    check(parsed.version == "1.4.2", "the version survives");
    check(parsed.files.size() == 1 && parsed.files[0].path == "Client.exe",
          "the file list survives");
    check(parsed.files[0].size == 42, "the size survives");

    rblx::update::Manifest ignored;
    check(!rblx::update::parseManifest("{", &ignored, &error), "malformed JSON is rejected");
    check(!rblx::update::parseManifest("{\"files\":[]}", &ignored, &error), "a manifest needs a version");
    check(!rblx::update::parseManifest(
              "{\"version\":\"1\",\"files\":[{\"path\":\"../x\",\"sha256\":\"" + std::string(64, 'a') +
                  "\"}]}",
              &ignored, &error),
          "a manifest cannot escape the install folder");
    check(!rblx::update::parseManifest(
              "{\"version\":\"1\",\"files\":[{\"path\":\"a\",\"sha256\":\"short\"}]}", &ignored, &error),
          "an entry without a real SHA-256 is rejected");
    check(!rblx::update::parseManifest(
              "{\"version\":\"1\",\"files\":[{\"path\":\"a\",\"sha256\":\"" + std::string(64, 'a') +
                  "\"},{\"path\":\"a\",\"sha256\":\"" + std::string(64, 'b') + "\"}]}",
              &ignored, &error),
          "the same path twice is rejected");

    // The updater and the machine's own settings are never part of a release.
    check(!rblx::update::isPackagedFile("manifest.json"), "the manifest is not packaged");
    check(!rblx::update::isPackagedFile("update.json"), "the update settings are not packaged");
    check(!rblx::update::isPackagedFile("RBLXUpdater.exe"), "the running updater is not packaged");
    check(rblx::update::isPackagedFile("shaders/basic.vert"), "shaders are packaged");
}

void testPlanning() {
    const fs::path root = makeScratch("plan");
    writeFile(root / "kept.txt", "same");
    writeFile(root / "stale.txt", "old");
    // gone.txt is in the installed manifest but missing from disk already.

    rblx::update::Manifest target;
    target.version = "2.0.0";

    rblx::update::FileEntry kept;
    kept.path = "kept.txt";
    kept.sha256 = rblx::update::sha256Text("same");
    kept.size = 4;

    rblx::update::FileEntry stale;
    stale.path = "stale.txt";
    stale.sha256 = rblx::update::sha256Text("new");
    stale.size = 3;

    rblx::update::FileEntry missing;
    missing.path = "games/one.world";
    missing.sha256 = rblx::update::sha256Text("world");
    missing.size = 5;

    target.files = {kept, stale, missing};

    rblx::update::Manifest installed;
    installed.version = "1.0.0";
    rblx::update::FileEntry gone;
    gone.path = "old/removed.dll";
    gone.sha256 = std::string(64, 'c');
    installed.files = {kept, stale, gone};

    const rblx::update::Plan plan = rblx::update::planUpdate(target, installed, root.string());
    check(plan.current.size() == 1 && plan.current[0] == "kept.txt",
          "a file that already matches is not downloaded");
    check(plan.download.size() == 2, "a changed file and a missing one are downloaded");
    check(plan.downloadBytes == 8, "the download size is the size of what is missing");
    check(plan.remove.size() == 1 && plan.remove[0] == "old/removed.dll",
          "a file the new release dropped is removed");
    check(!plan.upToDate(), "a plan with downloads is not up to date");

    const rblx::update::Plan settled = rblx::update::planUpdate(target, target, root.string());
    check(settled.download.size() == 2,
          "the removed file is not downloaded again, and the stale one still is");

    // Nothing on disk matches an empty folder, including the manifest's own key.
    const rblx::update::Plan fresh = rblx::update::planUpdate(target, rblx::update::Manifest{}, root.string());
    check(fresh.download.size() == 2, "an install fetches what is not on disk");
}

void testApplying() {
    const fs::path root = makeScratch("apply");
    const fs::path target = root / "nested" / "Client.exe";
    writeFile(target, "old build");

    const fs::path staged = root / ".update-staging" / "Client.exe";
    writeFile(staged, "new build");

    std::string error;
    check(rblx::update::applyStagedFile(staged.string(), target.string(), &error),
          "a staged file replaces the installed one");
    check(readFile(target) == "new build", "the new contents are in place");
    check(!fs::exists(fs::path(target.string() + ".old")), "the backup is cleaned up");
    check(!fs::exists(staged), "the staged copy is moved, not copied");

    // A file that did not exist yet is created, parents and all.
    const fs::path fresh = root / "web" / "public" / "index.html";
    const fs::path freshStaged = root / ".update-staging" / "web" / "public" / "index.html";
    writeFile(freshStaged, "page");
    check(rblx::update::applyStagedFile(freshStaged.string(), fresh.string(), &error),
          "a new file is installed");
    check(readFile(fresh) == "page", "the new file has its contents");

    // The manifest round trip on disk, which is what the next run diffs against.
    rblx::update::Manifest manifest;
    manifest.version = "3.1.0";
    const fs::path manifestPath = root / "manifest.json";
    check(rblx::update::writeManifestFile(manifestPath.string(), manifest, &error),
          "a manifest is written");
    rblx::update::Manifest read;
    check(rblx::update::readManifestFile(manifestPath.string(), &read, &error),
          "the manifest is read back");
    check(read.version == "3.1.0", "the version came back");

    // Configuration belongs to the machine and survives an update.
    rblx::update::Config config;
    config.manifestUrl = "http://127.0.0.1:3000/updates/manifest.json";
    config.launch = "Client.exe";
    check(rblx::update::writeConfigFile((root / "update.json").string(), config, &error),
          "update.json is written");
    rblx::update::Config loaded;
    check(rblx::update::readConfigFile((root / "update.json").string(), &loaded, &error),
          "update.json is read back");
    check(loaded.manifestUrl == config.manifestUrl, "the manifest URL survives");
    check(!loaded.read("{\"launch\":\"Client.exe\"}", &error), "a config without a URL is refused");
    check(!loaded.read("{\"manifestUrl\":\"https://example.com/m.json\"}", &error),
          "an https URL is refused with a reason instead of failing at connect");
}

void testHashing() {
    check(rblx::update::sha256Text("") ==
              "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855",
          "the empty SHA-256 is the published one");
    check(rblx::update::sha256Text("abc") ==
              "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad",
          "abc hashes to the published value");

    const fs::path root = makeScratch("hash");
    writeFile(root / "file.bin", "abc");
    std::string error;
    check(rblx::update::sha256File((root / "file.bin").string(), &error) ==
              rblx::update::sha256Text("abc"),
          "a file hashes the same as its contents");
    check(rblx::update::sha256File((root / "missing.bin").string(), &error).empty(),
          "a missing file has no hash and says why");
}

}  // namespace

int main() {
    testVersionOrder();
    testPathSafety();
    testUrlJoining();
    testManifestDocument();
    testPlanning();
    testApplying();
    testHashing();

    if (failures == 0) {
        std::printf("UpdaterTests: all checks passed\n");
        return 0;
    }
    std::fprintf(stderr, "UpdaterTests: %d check(s) failed\n", failures);
    return 1;
}
