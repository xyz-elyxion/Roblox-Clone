#include "CoolifyClient.h"

#include "HttpClient.h"
#include "WebJson.h"

#include <algorithm>
#include <cctype>
#include <cstdio>

namespace rblx {
namespace coolify {
namespace {

// A listing is cheap, but a start queues a deployment and Coolify answers only
// once it has written the record, so the two get different budgets.
constexpr int kListTimeoutMs = 8000;
constexpr int kActionTimeoutMs = 20000;

std::string trim(const std::string& value) {
    const char* whitespace = " \t\r\n";
    const std::size_t first = value.find_first_not_of(whitespace);
    if (first == std::string::npos) return std::string();
    const std::size_t last = value.find_last_not_of(whitespace);
    return value.substr(first, last - first + 1);
}

std::string lowered(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string loweredPrefix(const std::string& value, std::size_t count) {
    return lowered(value.substr(0, std::min(count, value.size())));
}

// Coolify describes a refusal in the body: {"message": "..."}. That sentence is
// better than anything this file can invent, so it is used whenever it exists.
std::string messageFromBody(const std::string& body) {
    std::string parseError;
    const web::Json parsed = web::Json::parse(body, &parseError);
    if (!parseError.empty()) return std::string();
    if (parsed.isObject()) return trim(parsed.member("message").asString());
    return std::string();
}

// Every call needs these two, and the panel should say which one is missing
// rather than sending a request that comes back 401.
bool validate(const Config& config, std::string* error) {
    if (config.baseUrl.empty()) {
        *error = "enter the Coolify address first, for example http://192.168.1.10:8000";
        return false;
    }
    if (config.token.empty()) {
        *error = "enter an API token: Coolify's Keys & Tokens -> API tokens";
        return false;
    }
    return true;
}

std::string statusText(int status) {
    return status > 0 ? "HTTP " + std::to_string(status) : std::string("no answer");
}

}  // namespace

bool Application::running() const {
    const std::string loweredStatus = lowered(status);
    return loweredStatus.compare(0, 7, "running") == 0;
}

bool normalizeBaseUrl(const std::string& input, std::string* out, std::string* error) {
    const std::string trimmed = trim(input);
    if (trimmed.empty()) {
        *error = "enter the Coolify address first, for example http://192.168.1.10:8000";
        return false;
    }

    // Compared case-insensitively: somebody who typed HTTPS:// meant https.
    if (loweredPrefix(trimmed, 8) == "https://") {
        *error = "this client has no TLS: use Coolify's http port, or put a local proxy in front of it";
        return false;
    }

    std::string base = trimmed;
    if (loweredPrefix(base, 7) != "http://") base = "http://" + base;

    // "http://" alone is not an address.
    if (base.size() <= 7) {
        *error = "that is not an address: it needs a host, for example http://192.168.1.10:8000";
        return false;
    }

    while (!base.empty() && base.back() == '/') base.pop_back();
    *out = base;
    return true;
}

std::string apiUrl(const std::string& baseUrl, const std::string& path) {
    std::string base = baseUrl;
    while (!base.empty() && base.back() == '/') base.pop_back();
    if (path.empty()) return base + "/api/v1";
    if (path.front() == '/') return base + "/api/v1" + path;
    return base + "/api/v1/" + path;
}

const char* actionSlug(Action action) {
    switch (action) {
        case Action::Start: return "start";
        case Action::Stop: return "stop";
        case Action::Restart: return "restart";
    }
    return "start";
}

const char* actionLabel(Action action) {
    switch (action) {
        case Action::Start: return "Start";
        case Action::Stop: return "Stop";
        case Action::Restart: return "Restart";
    }
    return "Start";
}

std::string explainFailure(int status, const std::string& body, const std::string& transportError) {
    if (status == 0) {
        if (transportError.find("https") != std::string::npos) {
            return "this client has no TLS: use Coolify's http port, or a proxy that speaks http";
        }
        if (transportError.empty()) return "the server did not answer";
        return "could not reach the server: " + transportError;
    }

    const std::string detail = messageFromBody(body);
    switch (status) {
        case 400:
            // "Application is not running." is a 400 from the logs endpoint and
            // is the whole explanation, so Coolify's own sentence wins.
            return detail.empty() ? "Coolify refused the request (HTTP 400)" : detail;
        case 401:
            return "the token was rejected: create one under Keys & Tokens -> API tokens";
        case 403:
            return detail.empty()
                       ? std::string("the token is valid but lacks the ability this call needs")
                       : "the token cannot do that: " + detail;
        case 404:
            return "nothing at that address: check the URL (the API is under /api/v1) and the application id";
        case 422:
            return detail.empty() ? "Coolify rejected the request (HTTP 422)" : detail;
        case 429:
            return "Coolify is rate limiting this token: wait a moment and try again";
        default:
            if (!detail.empty()) return detail + " (" + statusText(status) + ")";
            return "Coolify answered " + statusText(status);
    }
}

bool listApplications(const Config& config, std::vector<Application>* out, std::string* error) {
    if (out == nullptr) return false;
    out->clear();
    if (!validate(config, error)) return false;

    const http::Response response =
        http::request("GET", apiUrl(config.baseUrl, "/applications"), std::string(), config.token,
                      kListTimeoutMs);
    if (!response.ok()) {
        *error = explainFailure(response.status, response.body, response.error);
        return false;
    }

    std::string parseError;
    const web::Json parsed = web::Json::parse(response.body, &parseError);
    if (!parseError.empty()) {
        *error = "Coolify answered with something that is not JSON: " + parseError;
        return false;
    }
    // An account with no applications gets [], which is an answer, not a fault.
    if (!parsed.isArray()) {
        *error = "Coolify answered with something that is not a list of applications";
        return false;
    }

    for (const web::Json& entry : parsed.elements()) {
        if (!entry.isObject()) continue;
        Application application;
        application.id = static_cast<int>(entry.member("id").asInteger(0));
        application.uuid = entry.member("uuid").asString();
        application.name = entry.member("name").asString();
        application.status = entry.member("status").asString();
        application.fqdn = entry.member("fqdn").asString();
        if (application.name.empty()) application.name = application.uuid;
        if (application.uuid.empty()) continue;
        out->push_back(std::move(application));
    }
    return true;
}

bool performAction(const Config& config, const std::string& uuid, Action action,
                   std::string* message, std::string* error) {
    if (message != nullptr) message->clear();
    if (!validate(config, error)) return false;
    if (uuid.empty()) {
        *error = "choose an application first";
        return false;
    }

    const std::string path = "/applications/" + uuid + "/" + actionSlug(action);
    // POST with no body: the parameters these endpoints take (force,
    // instant_deploy) all default to false, which is what the panel means.
    const http::Response response =
        http::request("POST", apiUrl(config.baseUrl, path), std::string(), config.token,
                      kActionTimeoutMs);
    if (!response.ok()) {
        *error = explainFailure(response.status, response.body, response.error);
        return false;
    }

    if (message != nullptr) {
        std::string text = messageFromBody(response.body);
        if (text.empty()) text = std::string(actionLabel(action)) + " accepted.";
        *message = text;
    }
    return true;
}

bool fetchLogs(const Config& config, const std::string& uuid, int lines, std::string* logs,
               std::string* error) {
    if (logs == nullptr) return false;
    logs->clear();
    if (!validate(config, error)) return false;
    if (uuid.empty()) {
        *error = "choose an application first";
        return false;
    }
    if (lines <= 0) lines = 100;

    const std::string path =
        "/applications/" + uuid + "/logs?lines=" + std::to_string(lines);
    const http::Response response =
        http::request("GET", apiUrl(config.baseUrl, path), std::string(), config.token,
                      kListTimeoutMs);
    if (!response.ok()) {
        *error = explainFailure(response.status, response.body, response.error);
        return false;
    }

    std::string parseError;
    const web::Json parsed = web::Json::parse(response.body, &parseError);
    if (!parseError.empty()) {
        *error = "Coolify answered with something that is not JSON: " + parseError;
        return false;
    }
    // {"logs": "..."} - the field is a string, and empty when the container has
    // not printed anything yet.
    const std::string text = parsed.isObject() ? parsed.member("logs").asString()
                                               : (parsed.isString() ? parsed.asString()
                                                                    : std::string());
    if (text.empty()) {
        *logs = "(the container has not printed anything yet)";
        return true;
    }
    *logs = text;
    return true;
}

}  // namespace coolify
}  // namespace rblx
