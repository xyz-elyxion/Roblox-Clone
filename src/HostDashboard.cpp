#include "HostDashboard.h"

#include "WebHttp.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <bcrypt.h>

namespace rblx {
namespace hosting {
namespace {

using rblx::web::Request;
using rblx::web::Response;

constexpr const char* kCookieName = "rblxhosts";
// The page only ever shows the end of a log, so the server never reads a whole
// file that has been growing since the machine booted.
constexpr std::size_t kMaxLogBytes = 256 * 1024;
constexpr int kDefaultTailLines = 200;
constexpr int kMaxTailLines = 2000;

std::string joinPath(const std::string& directory, const std::string& name) {
    if (directory.empty()) return name;
    if (directory.back() == '/' || directory.back() == '\\') return directory + name;
    return directory + "/" + name;
}

std::string trimCopy(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && (value[begin] == ' ' || value[begin] == '\t')) ++begin;
    while (end > begin &&
           (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r')) {
        --end;
    }
    return value.substr(begin, end - begin);
}

std::string jsonText(const std::string& value) {
    std::string out = "\"";
    for (const unsigned char c : value) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char escaped[8];
                    std::snprintf(escaped, sizeof(escaped), "\\u%04x", c);
                    out += escaped;
                } else {
                    out += static_cast<char>(c);
                }
                break;
        }
    }
    out += '"';
    return out;
}

// The system RNG, so the token is not something another process on this machine
// can guess from the clock. If it is unavailable the token is still made from
// what the process knows, which is weaker; the loopback bind is the real
// boundary either way, and SECURITY.md says so.
std::string randomToken() {
    unsigned char bytes[16] = {};
    const NTSTATUS status = ::BCryptGenRandom(nullptr, bytes, static_cast<ULONG>(sizeof(bytes)),
                                              BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status != 0) {
        LARGE_INTEGER ticks{};
        ::QueryPerformanceCounter(&ticks);
        unsigned long long seed = static_cast<unsigned long long>(ticks.QuadPart) ^
                                  (static_cast<unsigned long long>(::GetCurrentProcessId()) << 32) ^
                                  static_cast<unsigned long long>(::GetTickCount64());
        for (std::size_t i = 0; i < sizeof(bytes); ++i) {
            seed = seed * 6364136223846793005ull + 1442695040888963407ull;
            bytes[i] = static_cast<unsigned char>((seed >> 33) & 0xFF);
        }
    }

    static const char* digits = "0123456789abcdef";
    std::string out;
    out.reserve(sizeof(bytes) * 2);
    for (const unsigned char byte : bytes) {
        out += digits[byte >> 4];
        out += digits[byte & 0x0F];
    }
    return out;
}

bool tokenMatches(const std::string& given, const std::string& expected) {
    if (given.empty() || given.size() != expected.size()) return false;
    unsigned char difference = 0;
    for (std::size_t i = 0; i < given.size(); ++i) {
        difference |= static_cast<unsigned char>(given[i] ^ expected[i]);
    }
    return difference == 0;
}

std::string cookieValue(const Request& request) {
    const std::string header = request.header("cookie");
    if (header.empty()) return std::string();

    const std::string prefix = std::string(kCookieName) + "=";
    std::size_t start = 0;
    while (start < header.size()) {
        const std::size_t semicolon = header.find(';', start);
        const std::string part =
            trimCopy(header.substr(start, semicolon == std::string::npos
                                              ? std::string::npos
                                              : semicolon - start));
        if (part.compare(0, prefix.size(), prefix) == 0) return part.substr(prefix.size());
        if (semicolon == std::string::npos) break;
        start = semicolon + 1;
    }
    return std::string();
}

bool readWholeFile(const std::string& path, std::string* out) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream contents;
    contents << file.rdbuf();
    *out = contents.str();
    return true;
}

std::string executableDirectory() {
    std::vector<char> buffer(MAX_PATH * 8, '\0');
    const DWORD length = ::GetModuleFileNameA(nullptr, buffer.data(),
                                              static_cast<DWORD>(buffer.size()));
    if (length == 0) return std::string(".");
    const std::string path(buffer.data(), length);
    const std::size_t slash = path.find_last_of("/\\");
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
}

// web/host/index.html, found the way WebApp finds web/public: walk up a few
// levels from the executable, then from the working directory. The package keeps
// the executables and web/ in one folder, so the first step usually finds it.
std::string findPageDirectory() {
    std::vector<std::string> bases;
    bases.push_back(executableDirectory());
    bases.push_back(std::string("."));

    for (const std::string& base : bases) {
        std::string candidate = base;
        for (int depth = 0; depth < 5 && !candidate.empty(); ++depth) {
            const std::string roots[] = {joinPath(candidate, "web"), candidate};
            for (const std::string& root : roots) {
                const std::string directory = joinPath(root, "host");
                std::ifstream probe(joinPath(directory, "index.html"));
                if (probe) return directory;
            }
            const std::size_t slash = candidate.find_last_of("/\\");
            if (slash == std::string::npos) break;
            candidate = candidate.substr(0, slash);
        }
    }
    return std::string();
}

std::string tailOfFile(const std::string& path, int maxLines) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return "(no log yet: " + path + ")";

    file.seekg(0, std::ios::end);
    const std::streamoff size = file.tellg();
    const std::streamoff window =
        std::min<std::streamoff>(size < 0 ? 0 : size, static_cast<std::streamoff>(kMaxLogBytes));
    file.seekg(size - window, std::ios::beg);

    std::string text(static_cast<std::size_t>(window), '\0');
    file.read(text.empty() ? nullptr : &text[0], window);
    text.resize(static_cast<std::size_t>(file.gcount()));

    // The window usually starts mid-line; the first line would be a fragment, so
    // drop it.
    if (window < size) {
        const std::size_t newline = text.find('\n');
        if (newline != std::string::npos) text.erase(0, newline + 1);
    }

    int lines = 0;
    for (std::size_t i = text.size(); i > 0; --i) {
        if (text[i - 1] == '\n' && ++lines > maxLines) {
            text.erase(0, i);
            break;
        }
    }
    return text;
}

std::string buildStatusJson(const Config& config, const Supervisor& supervisor, long long nowMs) {
    std::ostringstream out;
    out << "{\"running\":" << supervisor.runningCount()
        << ",\"total\":" << supervisor.instances().size() << ",\"instances\":[";

    bool first = true;
    for (const Instance& instance : supervisor.instances()) {
        if (!first) out << ',';
        first = false;

        long long uptimeSeconds = 0;
        if (instance.state == Instance::State::Running && instance.startedAtMs >= 0 &&
            nowMs >= instance.startedAtMs) {
            uptimeSeconds = (nowMs - instance.startedAtMs) / 1000;
        }

        out << "{\"index\":" << instance.index << ",\"port\":" << instance.port
            << ",\"pid\":" << instance.pid << ",\"state\":" << jsonText(stateName(instance.state))
            << ",\"restarts\":" << instance.restarts << ",\"uptimeSeconds\":" << uptimeSeconds
            << ",\"log\":" << jsonText(instance.logPath)
            << ",\"lastError\":" << jsonText(instance.lastError) << '}';
    }

    out << "],\"config\":{\"server\":" << jsonText(config.server)
        << ",\"directory\":" << jsonText(config.directory) << ",\"world\":" << jsonText(config.world)
        << ",\"web\":" << jsonText(config.webUrl)
        << ",\"logDirectory\":" << jsonText(config.logDirectory)
        << ",\"firstPort\":" << config.firstPort << ",\"instances\":" << config.instances
        << ",\"maxRestarts\":" << config.maxRestarts
        << ",\"restartWindowMs\":" << config.restartWindowMs
        << ",\"restartDelayMs\":" << config.restartDelayMs
        << ",\"dashboardHost\":" << jsonText(config.dashboardHost)
        << ",\"dashboardPort\":" << config.dashboardPort << "}}";
    return out.str();
}

const char* kFallbackPage =
    "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\">"
    "<title>RBLXHosts</title><style>body{font-family:Inter,system-ui,sans-serif;"
    "max-width:40rem;margin:3rem auto;padding:0 1rem;line-height:1.5}"
    "code{font-family:Consolas,monospace}</style></head><body>"
    "<h1>RBLXHosts</h1>"
    "<p>The control panel page is missing. It ships with the release as "
    "<code>web/host/index.html</code>; keep that folder next to "
    "<code>RBLXHosts.exe</code>, or run from the repository root.</p>"
    "<p>The pool is still supervised, and <code>GET /api/status</code> works.</p>"
    "</body></html>";

bool parseInstanceIndex(const Request& request, int* index) {
    const std::string value = request.param("index");
    if (value.empty()) return false;
    char* end = nullptr;
    const long parsed = std::strtol(value.c_str(), &end, 10);
    if (end == nullptr || *end != '\0' || parsed < 0 || parsed > 63) return false;
    *index = static_cast<int>(parsed);
    return true;
}

}  // namespace

struct HostDashboard::Impl {
    rblx::web::HttpServer server;
    std::thread thread;
    std::mutex mutex;
    std::string statusJson = "{\"running\":0,\"total\":0,\"instances\":[],\"config\":{}}";
    std::vector<std::string> logPaths;
    std::vector<Command> commands;
    std::string pageDirectory;
    std::string token;
    std::string host;
    std::string origin;
    int port = 0;
};

namespace {

bool authorized(const HostDashboard::Impl& impl, const Request& request) {
    if (tokenMatches(cookieValue(request), impl.token)) return true;
    // curl and scripts can skip the cookie exchange; the page cannot, which is
    // what the redirect is for.
    return tokenMatches(request.header("x-rblx-token"), impl.token);
}

// A POST from another site would arrive with its own Origin. Same-origin posts
// and non-browser clients (no Origin header at all) are the two that belong here.
bool sameOrigin(const HostDashboard::Impl& impl, const Request& request) {
    const std::string origin = request.header("origin");
    return origin.empty() || origin == impl.origin;
}

bool requireAccess(const HostDashboard::Impl& impl, const Request& request, Response& response,
                   bool mutating) {
    if (!authorized(impl, request)) {
        response.json(401, "{\"error\":\"Open the URL with ?token= that RBLXHosts printed.\"}");
        return false;
    }
    if (mutating && !sameOrigin(impl, request)) {
        response.json(403, "{\"error\":\"A request from another origin is refused.\"}");
        return false;
    }
    return true;
}

void queue(HostDashboard::Impl& impl, Command::Kind kind, int index) {
    Command command;
    command.kind = kind;
    command.index = index;
    std::lock_guard<std::mutex> lock(impl.mutex);
    impl.commands.push_back(command);
}

bool servePageFile(HostDashboard::Impl& impl, const Request& request, Response& response,
                   const std::string& name, const std::string& type) {
    if (!requireAccess(impl, request, response, false)) return true;
    std::string contents;
    if (impl.pageDirectory.empty() ||
        !readWholeFile(joinPath(impl.pageDirectory, name), &contents)) {
        response.text(404, "The control panel page is missing: " + name + "\n");
        return true;
    }
    response.text(200, contents, type);
    return true;
}

}  // namespace

HostDashboard::HostDashboard() = default;

HostDashboard::~HostDashboard() {
    stop();
}

bool HostDashboard::running() const {
    return impl_ != nullptr;
}

std::string HostDashboard::url() const {
    const std::string shown = (host_ == "0.0.0.0" || host_.empty()) ? "127.0.0.1" : host_;
    return "http://" + shown + ":" + std::to_string(port_) + "/?token=" + token_;
}

std::string HostDashboard::token() const {
    return token_;
}

bool HostDashboard::start(const DashboardSettings& settings, std::string* error) {
    if (impl_ != nullptr) {
        if (error != nullptr) *error = "the dashboard is already running";
        return false;
    }

    std::unique_ptr<Impl> impl(new Impl());
    impl->host = settings.host.empty() ? std::string("127.0.0.1") : settings.host;
    impl->token = settings.token.empty() ? randomToken() : settings.token;
    impl->pageDirectory =
        settings.pageDirectory.empty() ? findPageDirectory() : settings.pageDirectory;

    Impl* raw = impl.get();
    raw->server.route("GET", "/", [raw](Request& request, Response& response) {
        const std::string given = request.queryParam("token");
        if (!given.empty()) {
            if (!tokenMatches(given, raw->token)) {
                response.text(401, "That is not the token RBLXHosts printed at startup.\n");
                return true;
            }
            // The token becomes an HttpOnly cookie and leaves the address bar, so
            // it is not in the history, a bookmark or a page's script.
            response.status = 303;
            response.contentType = "text/html; charset=utf-8";
            response.headers["Location"] = "/";
            response.headers["Set-Cookie"] = std::string(kCookieName) + "=" + raw->token +
                                             "; Path=/; HttpOnly; SameSite=Strict";
            response.body = "<a href=\"/\">Continue</a>";
            return true;
        }
        if (!requireAccess(*raw, request, response, false)) return true;

        std::string page;
        if (!raw->pageDirectory.empty() &&
            readWholeFile(joinPath(raw->pageDirectory, "index.html"), &page)) {
            response.text(200, page, "text/html; charset=utf-8");
            return true;
        }
        response.text(200, kFallbackPage, "text/html; charset=utf-8");
        return true;
    });

    raw->server.route("GET", "/style.css", [raw](Request& request, Response& response) {
        return servePageFile(*raw, request, response, "style.css", "text/css; charset=utf-8");
    });

    raw->server.route("GET", "/app.js", [raw](Request& request, Response& response) {
        return servePageFile(*raw, request, response, "app.js",
                             "text/javascript; charset=utf-8");
    });

    raw->server.route("GET", "/api/status", [raw](Request& request, Response& response) {
        if (!requireAccess(*raw, request, response, false)) return true;
        std::string status;
        {
            std::lock_guard<std::mutex> lock(raw->mutex);
            status = raw->statusJson;
        }
        response.json(200, status);
        return true;
    });

    raw->server.route("GET", "/api/logs/:index", [raw](Request& request, Response& response) {
        if (!requireAccess(*raw, request, response, false)) return true;

        int index = 0;
        if (!parseInstanceIndex(request, &index)) {
            response.json(400, "{\"error\":\"The instance has to be a number.\"}");
            return true;
        }

        std::string path;
        {
            std::lock_guard<std::mutex> lock(raw->mutex);
            if (static_cast<std::size_t>(index) >= raw->logPaths.size()) {
                response.json(404, "{\"error\":\"There is no such instance.\"}");
                return true;
            }
            path = raw->logPaths[static_cast<std::size_t>(index)];
        }

        int tail = kDefaultTailLines;
        const std::string requested = request.queryParam("tail");
        if (!requested.empty()) {
            const long parsed = std::strtol(requested.c_str(), nullptr, 10);
            tail = parsed < 1 ? kDefaultTailLines
                              : (parsed > kMaxTailLines ? kMaxTailLines : static_cast<int>(parsed));
        }

        response.text(200, tailOfFile(path, tail));
        return true;
    });

    const auto instanceCommand = [raw](const char* action, Command::Kind kind) {
        return [raw, action, kind](Request& request, Response& response) {
            if (!requireAccess(*raw, request, response, true)) return true;
            int index = 0;
            if (!parseInstanceIndex(request, &index)) {
                response.json(400, "{\"error\":\"The instance has to be a number.\"}");
                return true;
            }
            queue(*raw, kind, index);
            response.json(202, std::string("{\"queued\":\"") + action + "\"}");
            return true;
        };
    };

    raw->server.route("POST", "/api/instances/:index/start",
                      instanceCommand("start", Command::Kind::Start));
    raw->server.route("POST", "/api/instances/:index/stop",
                      instanceCommand("stop", Command::Kind::Stop));
    raw->server.route("POST", "/api/instances/:index/restart",
                      instanceCommand("restart", Command::Kind::Restart));

    raw->server.route("POST", "/api/pool/start", [raw](Request& request, Response& response) {
        if (!requireAccess(*raw, request, response, true)) return true;
        queue(*raw, Command::Kind::StartAll, 0);
        response.json(202, "{\"queued\":\"start-all\"}");
        return true;
    });

    raw->server.route("POST", "/api/pool/stop", [raw](Request& request, Response& response) {
        if (!requireAccess(*raw, request, response, true)) return true;
        queue(*raw, Command::Kind::StopAll, 0);
        response.json(202, "{\"queued\":\"stop-all\"}");
        return true;
    });

    raw->server.setFallback([](Request&, Response& response) {
        response.json(404, "{\"error\":\"Not found.\"}");
        return true;
    });

    std::string listenError;
    if (!impl->server.listen(impl->host, settings.port, &listenError)) {
        if (error != nullptr) {
            *error = listenError.empty()
                         ? "cannot listen on " + impl->host + ":" + std::to_string(settings.port)
                         : listenError;
        }
        return false;
    }

    impl->port = impl->server.port();
    impl->origin = "http://" + impl->host + ":" + std::to_string(impl->port);
    impl->thread = std::thread([raw]() { raw->server.serveForever(); });

    host_ = impl->host;
    port_ = impl->port;
    token_ = impl->token;
    impl_ = std::move(impl);
    return true;
}

void HostDashboard::stop() {
    if (impl_ == nullptr) return;
    impl_->server.stop();
    if (impl_->thread.joinable()) impl_->thread.join();
    impl_.reset();
}

void HostDashboard::publish(const Config& config, const Supervisor& supervisor, long long nowMs) {
    if (impl_ == nullptr) return;

    std::string status = buildStatusJson(config, supervisor, nowMs);
    std::vector<std::string> logPaths;
    logPaths.reserve(supervisor.instances().size());
    for (const Instance& instance : supervisor.instances()) {
        logPaths.push_back(instance.logPath);
    }

    std::lock_guard<std::mutex> lock(impl_->mutex);
    impl_->statusJson.swap(status);
    impl_->logPaths.swap(logPaths);
}

std::vector<Command> HostDashboard::takeCommands() {
    std::vector<Command> commands;
    if (impl_ == nullptr) return commands;
    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        commands.swap(impl_->commands);
    }
    return commands;
}

}  // namespace hosting
}  // namespace rblx
