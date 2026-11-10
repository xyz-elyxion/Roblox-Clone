#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <vector>

namespace rblx {
namespace web {

struct Request {
    std::string method;
    std::string path;
    std::string query;
    std::string httpVersion;
    std::string body;
    std::string remoteAddress;
    std::map<std::string, std::string> headers;
    std::map<std::string, std::string> queryParams;
    std::map<std::string, std::string> params;

    std::string header(const std::string& name) const;
    std::string queryParam(const std::string& name) const;
    std::string param(const std::string& name) const;
    bool contentTypeStartsWith(const std::string& prefix) const;
};

struct Response {
    int status = 200;
    std::string body;
    std::string contentType = "application/json";
    std::map<std::string, std::string> headers;

    void json(int statusCode, const std::string& payload);
    void text(int statusCode, const std::string& payload,
              const std::string& type = "text/plain; charset=utf-8");
    void file(int statusCode, const std::string& absolutePath, const std::string& type);
};

// Small blocking HTTP/1.1 server: one thread per connection, keep-alive with a
// short idle timeout. Handlers return true when they produced a response.
class HttpServer {
public:
    using Handler = std::function<bool(Request&, Response&)>;

    HttpServer();
    ~HttpServer();

    void route(const std::string& method, const std::string& pattern, Handler handler);
    void setFallback(Handler handler);
    void setBodyLimitBytes(std::size_t limit) { bodyLimitBytes_ = limit; }
    // Connections served at the same time; a peer past the ceiling is closed
    // rather than given a worker thread.
    void setMaxConnections(int limit) { maxConnections_ = limit > 0 ? limit : 1; }

    bool listen(const std::string& host, int port, std::string* error);
    int port() const { return port_; }
    void serveForever();
    void stop();

    // Accepts and answers exactly one connection. Used by the tests.
    bool serveOneConnection();

    static std::string mimeTypeForPath(const std::string& path);

private:
    struct RouteEntry {
        std::string method;
        std::string pattern;
        Handler handler;
    };

    void handleConnection(std::intptr_t socketHandle);
    bool readRequest(std::intptr_t socketHandle, Request* request, bool* timedOut);
    bool writesResponse(std::intptr_t socketHandle, const Request& request,
                        Response& response, bool* keepAlive);
    bool dispatch(Request& request, Response& response) const;
    static bool matchesPattern(const std::string& pattern, const std::string& path,
                               std::map<std::string, std::string>* params);

    std::vector<RouteEntry> routes_;
    Handler fallback_;
    std::size_t bodyLimitBytes_ = 8 * 1024 * 1024;
    std::atomic<int> activeConnections_{0};
    int maxConnections_ = 64;
    std::intptr_t listenSocket_ = -1;
    int port_ = 0;
    bool running_ = false;
};

}  // namespace web
}  // namespace rblx
