#pragma once

#include <string>
#include <vector>

// The native client for a Coolify instance, used by the hosting panel.
//
// Coolify's API is JSON over HTTP with a bearer token, so this is the part that
// knows the endpoints, the shapes it gets back and what each failure means; the
// window only has to draw. Everything here blocks and touches no UI, which is
// what lets CoolifyClientTests check it against a stub server.
//
// The endpoints come from the Coolify source this repository carries in
// hosting/coolify (routes/api.php, every route under the v1 prefix):
//
//   GET  /api/v1/applications                    list; token ability "read"
//   POST /api/v1/applications/{uuid}/start       "deploy"
//   POST /api/v1/applications/{uuid}/restart     "deploy"
//   POST /api/v1/applications/{uuid}/stop        "deploy"
//   GET  /api/v1/applications/{uuid}/logs?lines= "read:sensitive"
//
// The create endpoints are deliberately absent: making an application is a
// choice with a dozen answers (source, build pack, port, domain) and belongs in
// Coolify's own dashboard, which this tool opens.
namespace rblx {
namespace coolify {

struct Config {
    // http://host:8000, with or without a trailing slash.
    std::string baseUrl;
    // Keys & Tokens -> API tokens. Never logged, never written to disk.
    std::string token;
};

struct Application {
    int id = 0;
    std::string uuid;
    std::string name;
    // Coolify's own word for it: "running:healthy", "running:unknown",
    // "exited", "exited:unhealthy" and so on.
    std::string status;
    std::string fqdn;

    // "running:healthy" and "running:unknown" both count as up; anything else,
    // including an empty status, does not.
    bool running() const;
};

// Turns what somebody typed into a base URL: adds http:// when no scheme was
// given and drops trailing slashes and spaces. https:// is refused with an
// explanation rather than attempted - this project has no TLS stack, and a
// silent failure here would look like a wrong token.
bool normalizeBaseUrl(const std::string& input, std::string* out, std::string* error);

// {base}/api/v1{path}; `path` may or may not start with a slash.
std::string apiUrl(const std::string& baseUrl, const std::string& path);

enum class Action { Start, Stop, Restart };

// The path segment and the word on the button, so the two cannot drift apart.
const char* actionSlug(Action action);
const char* actionLabel(Action action);

// Fills `out` with every application the token can see. Fails with `error` set.
bool listApplications(const Config& config, std::vector<Application>* out, std::string* error);

// Queues one of start/stop/restart. `message` receives Coolify's answer
// ("Deployment request queued." and the like) when the call is accepted.
bool performAction(const Config& config, const std::string& uuid, Action action,
                   std::string* message, std::string* error);

// The last `lines` lines of the application's container output.
bool fetchLogs(const Config& config, const std::string& uuid, int lines,
               std::string* logs, std::string* error);

// One sentence for a failed request, from the HTTP status, Coolify's own
// {"message": ...} when it sent one, and the transport error when the request
// never completed.
std::string explainFailure(int status, const std::string& body, const std::string& transportError);

}  // namespace coolify
}  // namespace rblx
