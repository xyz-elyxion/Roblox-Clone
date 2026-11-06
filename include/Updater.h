#pragma once

#include <string>
#include <vector>

// The updater's brains, kept apart from the command line and the network so the
// decisions can be checked without a server: what a release contains, whether a
// file in an install folder already matches it, and what has to be downloaded,
// replaced or removed.
//
// A release is a folder plus a manifest.json listing every file it ships with a
// SHA-256. The updater fetches that document, compares it against the copy it
// saved beside the installed files, and downloads only what differs. Nothing is
// written until a download has been checked against its hash, and every
// replacement goes through a rename so an interrupted update cannot leave a
// half-written executable that the game would then try to run.
namespace rblx {
namespace update {

struct FileEntry {
    // Relative to the install folder and written with forward slashes.
    std::string path;
    // Lowercase hex, 64 characters.
    std::string sha256;
    long long size = 0;
};

struct Manifest {
    std::string version;
    std::string channel = "stable";
    std::vector<FileEntry> files;
};

// Returns false with a reason the caller can print when the document is not a
// manifest: bad JSON, no version, an entry without a usable path or hash.
bool parseManifest(const std::string& text, Manifest* out, std::string* error);
std::string buildManifestJson(const Manifest& manifest);

// "1.10.0" is newer than "1.9.2"; missing components count as zero and a
// trailing "-beta" is ignored, because a channel is not an ordering.
int compareVersions(const std::string& left, const std::string& right);

// A manifest names files, so it is also a way to write outside the install
// folder. Only a plain relative path that stays under the folder is accepted.
bool isSafeRelativePath(const std::string& path, std::string* reason);
bool resolveUnder(const std::string& root, const std::string& relative, std::string* resolved);

std::string sha256Text(const std::string& text);
std::string sha256File(const std::string& path, std::string* error);

// Base URL joined with a manifest path, with the path percent-encoded so a file
// name with a space or an accent still makes a valid request line. Returns an
// empty string for a path that is itself an absolute URL.
std::string joinUrl(const std::string& baseUrl, const std::string& relative);

struct Plan {
    std::vector<FileEntry> download;
    std::vector<std::string> current;
    // Shipped by the installed release but gone from the new one.
    std::vector<std::string> remove;
    long long downloadBytes = 0;

    bool upToDate() const { return download.empty() && remove.empty(); }
};

Plan planUpdate(const Manifest& target, const Manifest& installed, const std::string& directory);

bool readManifestFile(const std::string& path, Manifest* out, std::string* error);
bool writeManifestFile(const std::string& path, const Manifest& manifest, std::string* error);
bool readTextFile(const std::string& path, std::string* out);
bool writeFileAtomically(const std::string& path, const std::string& data, std::string* error);

// Moves a verified download into place. The file it replaces is moved aside
// first and only deleted once the new one is in position, so a failure leaves
// the old file where it was.
bool applyStagedFile(const std::string& staged, const std::string& target, std::string* error);

// update.json: where this copy looks for releases and what to start afterwards.
// It belongs to the machine rather than to the release, so an update never
// overwrites it.
struct Config {
    std::string manifestUrl;
    std::string launch = "Client.exe";

    bool read(const std::string& text, std::string* error);
    std::string toJson() const;
};

bool readConfigFile(const std::string& path, Config* out, std::string* error);
bool writeConfigFile(const std::string& path, const Config& config, std::string* error);

// False for the files a release never covers: the updater replaces itself
// through the installer rather than mid-run, and the two configuration files
// belong to the machine. Everything else in an install folder is published.
bool isPackagedFile(const std::string& relativePath);

}  // namespace update
}  // namespace rblx
