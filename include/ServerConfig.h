#pragma once

// The settings a server install keeps in server.conf.
//
// Every key in that file is an environment variable the website already reads, so
// the file changes nothing about how WebApp is configured: it saves an operator
// from setting seven variables by hand before starting the server. The
// environment still outranks the file, which is what keeps the file safe to edit
// and keeps a service or a build script able to override one value for one run
// without touching what ServerSetup wrote.
//
// This unit is deliberately free of Windows and of the environment: it reads,
// writes and validates the document, and the host that owns the process decides
// what to do with the values. That is what lets ServerConfigTests run anywhere.

#include <string>
#include <utility>
#include <vector>

namespace rblx {
namespace server {

// The file lives beside the web folder, which is the folder the programs run
// from, because that is the one folder every path in this project is relative to.
constexpr const char* kSettingsFileName = "server.conf";

struct Settings {
    // "production" is what ServerSetup writes. It is what turns on the checks a
    // published install needs: a real signing secret and a real owner password.
    // Empty means the file says nothing about it and the site's own default
    // applies, so a hand-written file that only sets a port does not silently
    // become a production install.
    std::string nodeEnv;
    std::string port;
    // What the game client is told to talk to. Empty means the site works the
    // address out from the request's Host header, which is right for one machine
    // and wrong behind a proxy - which is why the setup asks for it.
    std::string publicBaseUrl;
    // Comma-separated extra accounts allowed into /admin. The account that owns
    // the install is always allowed on top of these, so this may stay empty.
    std::string adminUsernames;
    // The password of the seeded owner account, and the two secrets the site signs
    // sessions and seals the data file with.
    std::string starterPassword;
    std::string jwtSecret;
    std::string dataKey;
};

// `key = value` per line, `#` or `;` to the end of a line is a comment, blank
// lines are ignored, and unknown keys are ignored so a file written for a later
// version still loads. A missing file, or a line that is not a pair, is an error
// that says which line it was.
bool loadSettings(const std::string& path, Settings* settings, std::string* error);

// Writes the file, replacing whatever a previous run put there. It is written to
// a temporary name first and then renamed, so a crash or a full disk leaves the
// previous file rather than half of a new one - the file holds the secrets the
// data is sealed with, and losing it makes the data unreadable.
bool saveSettings(const std::string& path, const Settings& settings, std::string* error);

// What has to hold before the site will start with these settings: a stated
// environment, a port in range, a signing secret long enough for the production
// check, an owner password at least 12 characters long, and a public URL that is a
// plain http(s) address. Empty means success.
bool validateSettings(const Settings& settings, std::string* error);

// Whether this folder can be set up as a server at all: it has to contain the
// website (web/public/index.html) and the WebApp.exe that serves it. Checked before
// anything is written, because a settings file in a folder with no server in it
// reads as success and does nothing.
bool checkServerFolder(const std::string& folder, std::string* error);

// The settings as environment assignments, in a fixed order, skipping the ones
// left empty. The names are the ones WebApp reads, which is the whole point of
// the file; a value that is empty is left out rather than set to nothing, so the
// site's own default still applies.
std::vector<std::pair<std::string, std::string>> settingsAsEnvironment(const Settings& settings);

std::string settingsPathFor(const std::string& directory);

}  // namespace server
}  // namespace rblx
