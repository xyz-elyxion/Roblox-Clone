// The proxy's rules, checked against a real socket rather than a mock: what it
// puts on the request line, what it refuses to pass on in either direction, and
// what it hands back when the backend answers, fails, or answers with something
// it cannot relay. A tiny listener on an ephemeral port stands in for WebApp.

#include "ReverseProxy.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <thread>
#include <utility>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

namespace {

using SocketHandle = SOCKET;
const SocketHandle kInvalid = INVALID_SOCKET;

int failures = 0;

void check(bool condition, const std::string& what) {
    std::printf("%s %s\n", condition ? "ok  " : "FAIL", what.c_str());
    if (!condition) {
        ++failures;
    }
}

// Winsock refuses socket(), bind() and listen() until WSAStartup has run, and
// the stubs below create their listening socket before anything else happens.
struct WinsockInit {
    WinsockInit() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
} g_winsockInit;

// A backend that takes one connection, records everything it was sent and
// answers with a fixed block of bytes.
class StubBackend {
public:
    explicit StubBackend(std::string response) : response_(std::move(response)) {}
    ~StubBackend() { finish(); }

    bool start() {
        listening_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listening_ == kInvalid) {
            return false;
        }

        int reuse = 1;
        ::setsockopt(listening_, SOL_SOCKET, SO_REUSEADDR,
                     reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = 0;  // let the OS pick a free port
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::bind(listening_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
            return false;
        }

        sockaddr_in bound{};
        int length = sizeof(bound);
        if (::getsockname(listening_, reinterpret_cast<sockaddr*>(&bound), &length) != 0) {
            return false;
        }
        port_ = ntohs(bound.sin_port);
        if (::listen(listening_, 4) != 0) {
            return false;
        }

        worker_ = std::thread([this] { serve(); });
        return true;
    }

    void finish() {
        if (worker_.joinable()) {
            worker_.join();
        }
        if (listening_ != kInvalid) {
            ::closesocket(listening_);
            listening_ = kInvalid;
        }
    }

    unsigned short port() const { return port_; }
    const std::string& request() const { return request_; }

private:
    void serve() {
        const SocketHandle client = ::accept(listening_, nullptr, nullptr);
        if (client == kInvalid) {
            return;
        }

        char buffer[4096];
        while (request_.find("\r\n\r\n") == std::string::npos) {
            const int received = ::recv(client, buffer, static_cast<int>(sizeof(buffer)), 0);
            if (received <= 0) {
                break;
            }
            request_.append(buffer, static_cast<std::size_t>(received));
        }

        // Read the body the proxy announced. It writes lower-case header names,
        // like every other server in this tree.
        const std::size_t bodyStart = request_.find("\r\n\r\n");
        if (bodyStart != std::string::npos) {
            std::size_t declared = 0;
            const std::size_t header = request_.find("content-length:");
            if (header != std::string::npos) {
                declared = static_cast<std::size_t>(std::atoi(request_.c_str() + header + 15));
            }
            while (request_.size() - (bodyStart + 4) < declared) {
                const int received = ::recv(client, buffer, static_cast<int>(sizeof(buffer)), 0);
                if (received <= 0) {
                    break;
                }
                request_.append(buffer, static_cast<std::size_t>(received));
            }
        }

        std::size_t sent = 0;
        while (sent < response_.size()) {
            const int chunk = ::send(client, response_.data() + sent,
                                     static_cast<int>(response_.size() - sent), 0);
            if (chunk <= 0) {
                break;
            }
            sent += static_cast<std::size_t>(chunk);
        }
        ::closesocket(client);
    }

    std::string response_;
    std::string request_;
    SocketHandle listening_ = kInvalid;
    unsigned short port_ = 0;
    std::thread worker_;
};

std::string backendAddress(const StubBackend& backend) {
    return "127.0.0.1:" + std::to_string(backend.port());
}

rblx::proxy::Upstream upstreamFor(const StubBackend& backend) {
    rblx::proxy::Upstream upstream;
    upstream.host = "127.0.0.1";
    upstream.port = backend.port();
    upstream.timeoutMs = 2000;
    return upstream;
}

rblx::web::Request browserRequest() {
    rblx::web::Request request;
    request.method = "GET";
    request.path = "/index.html";
    request.httpVersion = "HTTP/1.1";
    request.remoteAddress = "10.1.2.3";
    request.headers["host"] = "site.example";
    request.headers["user-agent"] = "test";
    request.headers["connection"] = "keep-alive";
    request.headers["te"] = "trailers";
    request.headers["x-custom"] = "kept";
    return request;
}

void testHeaderRules() {
    using rblx::proxy::isHopByHop;
    check(isHopByHop("connection"), "connection belongs to one connection");
    check(isHopByHop("Keep-Alive"), "a keep-alive header is hop-by-hop whatever its case");
    check(isHopByHop("transfer-encoding"), "transfer-encoding is hop-by-hop");
    check(isHopByHop("upgrade"), "upgrade is hop-by-hop");
    check(isHopByHop("proxy-authorization"), "proxy credentials are not forwarded");
    check(!isHopByHop("content-type"), "content-type belongs to the message");
    check(!isHopByHop("x-custom"), "an ordinary header is passed on");
}

void testForwardedFor() {
    using rblx::proxy::forwardedFor;
    check(forwardedFor("", "10.1.2.3") == "10.1.2.3", "the first address stands alone");
    check(forwardedFor("1.2.3.4", "10.1.2.3") == "1.2.3.4, 10.1.2.3",
          "an earlier proxy's address is kept in front");
    check(forwardedFor("1.2.3.4", "") == "1.2.3.4",
          "an unknown peer does not erase what earlier proxies said");
    check(forwardedFor("", "").empty(), "nothing is invented when nothing is known");
}

void testTargetEncoding() {
    using rblx::proxy::encodeTarget;
    check(encodeTarget("/index.html", "") == "/index.html", "a plain path is unchanged");
    check(encodeTarget("/games/My Game.html", "") == "/games/My%20Game.html",
          "a space in a file name is escaped again");
    check(encodeTarget("/100%", "") == "/100%25", "a percent sign is escaped, not left dangling");
    check(encodeTarget("", "") == "/", "an empty path becomes the root");
    check(encodeTarget("/search", "q=a%20b") == "/search?q=a%20b",
          "the query is passed on as it arrived, not escaped twice");
    check(encodeTarget("/", "") == "/", "the root stays the root");
}

void testAddressParsing() {
    using rblx::proxy::parseHostPort;

    std::string host;
    int port = 0;
    std::string error;

    check(parseHostPort("127.0.0.1:3000", &host, &port, &error), "an address and port parse");
    check(host == "127.0.0.1" && port == 3000, "the address and port come back as written");

    check(parseHostPort("localhost:8080", &host, &port, &error), "a name is accepted");
    check(host == "localhost" && port == 8080, "the name survives");

    check(!parseHostPort("127.0.0.1", &host, &port, &error), "a port is required");
    check(!parseHostPort(":3000", &host, &port, &error), "a host is required");
    check(!parseHostPort("127.0.0.1:", &host, &port, &error), "a half-written port is refused");
    check(!parseHostPort("127.0.0.1:http", &host, &port, &error), "a port is a number");
    check(!parseHostPort("127.0.0.1:0", &host, &port, &error), "port 0 is not a destination");
    check(!parseHostPort("127.0.0.1:70000", &host, &port, &error), "a port past 65535 is refused");
    check(!parseHostPort("http://127.0.0.1:3000", &host, &port, &error),
          "a URL is refused instead of being half understood");
}

void testForwarding() {
    using namespace rblx::proxy;

    StubBackend backend(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: 5\r\n"
        "X-Backend: yes\r\n"
        "\r\n"
        "hello");
    check(backend.start(), "the stub backend listens");

    UpstreamResponse answer;
    std::string error;
    const bool forwarded = forward(upstreamFor(backend), browserRequest(), &answer, &error);
    backend.finish();

    check(forwarded, "a reachable backend answers: " + error);
    check(answer.status == 200, "the status comes back");
    check(answer.body == "hello", "the body comes back");
    check(answer.contentType == "text/html; charset=utf-8", "the content type comes back");
    check(answer.headers.count("x-backend") == 1, "an ordinary header comes back");
    check(answer.headers.count("content-type") == 0, "content-type is not repeated in the headers");

    const std::string& sent = backend.request();
    check(sent.rfind("GET /index.html HTTP/1.1\r\n", 0) == 0,
          "the request line keeps the method and path");
    check(sent.find("Host: 127.0.0.1:" + std::to_string(backend.port())) != std::string::npos,
          "the backend is addressed by its own host");
    check(sent.find("x-forwarded-for: 10.1.2.3") != std::string::npos,
          "the browser's address is passed on");
    check(sent.find("x-forwarded-host: site.example") != std::string::npos,
          "the address the browser typed is passed on");
    check(sent.find("x-forwarded-proto: http") != std::string::npos, "the scheme is passed on");
    check(sent.find("x-custom: kept") != std::string::npos, "an ordinary header is passed on");
    check(sent.find("te: trailers") == std::string::npos, "a hop-by-hop header is not passed on");
    check(sent.find("keep-alive") == std::string::npos,
          "the browser's keep-alive request is not passed on");
    check(sent.find("connection: close") != std::string::npos,
          "one answer per backend connection is asked for");
}

void testBodyIsForwarded() {
    using namespace rblx::proxy;

    StubBackend backend(
        "HTTP/1.1 201 Created\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: 2\r\n"
        "\r\n"
        "{}");
    check(backend.start(), "the stub backend listens for a POST");

    rblx::web::Request request = browserRequest();
    request.method = "POST";
    request.path = "/api/login";
    request.body = "{\"name\":\"p\"}";

    UpstreamResponse answer;
    std::string error;
    const bool forwarded = forward(upstreamFor(backend), request, &answer, &error);
    backend.finish();

    check(forwarded, "the POST reaches the backend: " + error);
    const std::string& sent = backend.request();
    check(sent.rfind("POST /api/login HTTP/1.1\r\n", 0) == 0, "the method and path survive");
    check(sent.find("content-length: 12") != std::string::npos, "the body length is declared");
    check(sent.size() >= 12 && sent.compare(sent.size() - 12, 12, "{\"name\":\"p\"}") == 0,
          "the body arrives");
    check(answer.status == 201, "a status the site uses comes back unchanged");
}

void testCookiesSurvive() {
    using namespace rblx::proxy;

    StubBackend backend(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 2\r\n"
        "Set-Cookie: session=abc; Expires=Wed, 21 Oct 2026 07:28:00 GMT; Path=/\r\n"
        "Set-Cookie: theme=dark; Path=/\r\n"
        "\r\n"
        "ok");
    check(backend.start(), "the stub backend listens for two cookies");

    UpstreamResponse answer;
    std::string error;
    const bool forwarded = forward(upstreamFor(backend), browserRequest(), &answer, &error);
    backend.finish();

    check(forwarded, "the answer with two cookies is read: " + error);
    const auto cookie = answer.headers.find("set-cookie");
    check(cookie != answer.headers.end(), "the cookies arrive");
    if (cookie != answer.headers.end()) {
        check(cookie->second.find("session=abc") != std::string::npos, "the first cookie is kept");
        check(cookie->second.find("theme=dark") != std::string::npos, "the second cookie is kept");
        check(cookie->second.find("\r\nSet-Cookie: theme=dark") != std::string::npos,
              "the second cookie is written as its own header, not folded into the first");
    }
}

void testChunkedIsRefused() {
    using namespace rblx::proxy;

    StubBackend backend(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Transfer-Encoding: chunked\r\n"
        "\r\n"
        "5\r\nhello\r\n0\r\n\r\n");
    check(backend.start(), "the stub backend listens for a chunked answer");

    UpstreamResponse answer;
    std::string error;
    const bool forwarded = forward(upstreamFor(backend), browserRequest(), &answer, &error);
    backend.finish();

    check(!forwarded, "a chunked answer is not relayed");
    check(error.find("chunked") != std::string::npos, "the reason names chunked encoding");
}

void testUnreachableBackend() {
    using namespace rblx::proxy;

    Upstream upstream;
    upstream.host = "127.0.0.1";
    upstream.port = 1;  // nothing listens on port 1
    upstream.timeoutMs = 1000;

    UpstreamResponse answer;
    std::string error;
    check(!forward(upstream, browserRequest(), &answer, &error), "a dead backend fails the request");
    check(!error.empty(), "the failure carries a reason");

    ReverseProxy front(upstream, false);
    rblx::web::Request request = browserRequest();
    rblx::web::Response response;
    check(front.handle(request, response), "the proxy still answers the browser");
    check(response.status == 502, "the browser is told the site is not answering");
    check(response.body.find("10.1.2.3") == std::string::npos,
          "the failure does not name this machine's backend");
}

void testHandlerAnswer() {
    using namespace rblx::proxy;

    StubBackend backend(
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=utf-8\r\n"
        "Content-Length: 4\r\n"
        "Cache-Control: no-store\r\n"
        "X-Frame-Options: DENY\r\n"
        "Connection: close\r\n"
        "\r\n"
        "page");
    check(backend.start(), "the stub backend listens for the handler");

    ReverseProxy front(upstreamFor(backend), false);
    rblx::web::Request request = browserRequest();
    rblx::web::Response response;
    check(front.handle(request, response), "the handler answers");
    backend.finish();

    check(response.status == 200, "the status reaches the browser");
    check(response.body == "page", "the body reaches the browser");
    check(response.contentType == "text/html; charset=utf-8", "the content type reaches the browser");
    check(response.headers.count("cache-control") == 1, "a cache header reaches the browser");
    check(response.headers.count("content-type") == 0,
          "the content type is not also a header, which would send it twice");
    check(response.headers.count("content-length") == 0,
          "the body length is not passed on twice");
    check(response.headers.count("connection") == 0, "the backend's connection header is dropped");
    check(response.headers.count("x-frame-options") == 0,
          "the server adds its own copy of the security headers");
}

}  // namespace

int main() {
    std::printf("Reverse proxy tests\n");
    testHeaderRules();
    testForwardedFor();
    testTargetEncoding();
    testAddressParsing();
    testForwarding();
    testBodyIsForwarded();
    testCookiesSurvive();
    testChunkedIsRefused();
    testUnreachableBackend();
    testHandlerAnswer();

    if (failures != 0) {
        std::printf("ReverseProxyTests: %d check(s) failed\n", failures);
        return 1;
    }
    std::printf("ReverseProxyTests: all checks passed\n");
    return 0;
}
