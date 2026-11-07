// Tests for the Coolify API client: the address handling, the bearer header,
// what each failure is called, and the shapes it parses. A stub server stands in
// for Coolify, so these run through the real socket path without an instance.

#include "CoolifyClient.h"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketHandle = SOCKET;
    constexpr SocketHandle kInvalid = INVALID_SOCKET;
    #define TEST_CLOSE(s) closesocket(s)
#else
    #include <arpa/inet.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
    using SocketHandle = int;
    constexpr SocketHandle kInvalid = -1;
    #define TEST_CLOSE(s) ::close(s)
#endif

namespace {

#ifdef _WIN32
// The stub servers create their listening socket before any request is made, so
// Winsock has to be up before that; the client's own lazy start is too late.
struct WinsockInit {
    WinsockInit() {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    }
} g_winsockInit;
#endif

int failures = 0;

void check(bool condition, const char* what) {
    if (condition) {
        std::printf("  ok   %s\n", what);
    } else {
        std::printf("  FAIL %s\n", what);
        ++failures;
    }
}

bool contains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}

std::string lowered(std::string value) {
    for (char& c : value) {
        if (c >= 'A' && c <= 'Z') c = static_cast<char>(c - 'A' + 'a');
    }
    return value;
}

// One connection, one canned response, and everything the client sent kept for
// the assertions.
class StubServer {
public:
    explicit StubServer(std::string response) : response_(std::move(response)) {}
    ~StubServer() { stop(); }

    bool start() {
        listening_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
        if (listening_ == kInvalid) return false;

        int reuse = 1;
        setsockopt(listening_, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&reuse), sizeof(reuse));

        sockaddr_in address{};
        address.sin_family = AF_INET;
        address.sin_port = 0;
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        if (::bind(listening_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
            return false;
        }

        sockaddr_in bound{};
#ifdef _WIN32
        int length = sizeof(bound);
#else
        socklen_t length = sizeof(bound);
#endif
        if (::getsockname(listening_, reinterpret_cast<sockaddr*>(&bound), &length) != 0) {
            return false;
        }
        port_ = ntohs(bound.sin_port);

        if (::listen(listening_, 4) != 0) return false;
        worker_ = std::thread([this] { serve(); });
        return true;
    }

    std::string baseUrl() const { return "http://127.0.0.1:" + std::to_string(port_); }

    // Ends the server and hands back the request it received.
    std::string request() {
        stop();
        return request_;
    }

    void stop() {
        if (worker_.joinable()) worker_.join();
        if (listening_ != kInvalid) {
            TEST_CLOSE(listening_);
            listening_ = kInvalid;
        }
    }

private:
    void serve() {
        const SocketHandle client = ::accept(listening_, nullptr, nullptr);
        if (client == kInvalid) return;

        char buffer[2048];
        for (;;) {
            const int received = static_cast<int>(::recv(client, buffer, sizeof(buffer), 0));
            if (received <= 0) break;
            request_.append(buffer, static_cast<std::size_t>(received));
            if (request_.find("\r\n\r\n") != std::string::npos) break;
        }

        const char* cursor = response_.c_str();
        std::size_t remaining = response_.size();
        while (remaining > 0) {
            const int sent = static_cast<int>(::send(client, cursor, remaining, 0));
            if (sent <= 0) break;
            cursor += sent;
            remaining -= static_cast<std::size_t>(sent);
        }
        TEST_CLOSE(client);
    }

    std::string response_;
    std::string request_;
    SocketHandle listening_ = kInvalid;
    std::thread worker_;
    unsigned short port_ = 0;
};

std::string responseFor(int status, const char* reason, const std::string& body) {
    return "HTTP/1.1 " + std::to_string(status) + " " + reason +
           "\r\nContent-Type: application/json\r\nContent-Length: " + std::to_string(body.size()) +
           "\r\nConnection: close\r\n\r\n" + body;
}

// ---------------------------------------------------------------------------

void testAddresses() {
    std::string out;
    std::string error;

    check(rblx::coolify::normalizeBaseUrl("192.168.1.10:8000", &out, &error) &&
              out == "http://192.168.1.10:8000",
          "a bare host and port gets http://");
    check(rblx::coolify::normalizeBaseUrl("  http://cool.example/  ", &out, &error) &&
              out == "http://cool.example",
          "spaces and trailing slashes are dropped");
    check(rblx::coolify::normalizeBaseUrl("HTTP://cool.example/", &out, &error) &&
              out == "HTTP://cool.example",
          "an explicit http scheme is kept as typed");

    error.clear();
    check(!rblx::coolify::normalizeBaseUrl("https://cool.example", &out, &error) &&
              contains(error, "TLS"),
          "https is refused with a TLS explanation");
    error.clear();
    check(!rblx::coolify::normalizeBaseUrl("   ", &out, &error) && contains(error, "address"),
          "an empty address is refused");
    error.clear();
    check(!rblx::coolify::normalizeBaseUrl("http://", &out, &error),
          "a scheme with no host is refused");

    check(rblx::coolify::apiUrl("http://host:8000/", "/applications") ==
              "http://host:8000/api/v1/applications",
          "apiUrl joins the v1 prefix");
    check(rblx::coolify::apiUrl("http://host:8000", "applications") ==
              "http://host:8000/api/v1/applications",
          "apiUrl accepts a path without a leading slash");
}

void testList() {
    const std::string body =
        "[{\"id\":1,\"uuid\":\"app-one\",\"name\":\"game-server\",\"status\":\"running:healthy\","
        "\"fqdn\":\"http://game.example\"},"
        "{\"id\":2,\"uuid\":\"app-two\",\"name\":\"\",\"status\":\"exited\",\"fqdn\":null}]";
    StubServer server(responseFor(200, "OK", body));
    check(server.start(), "stub server listens");

    rblx::coolify::Config config;
    config.baseUrl = server.baseUrl();
    config.token = "test-token";

    std::vector<rblx::coolify::Application> applications;
    std::string error;
    const bool listed = rblx::coolify::listApplications(config, &applications, &error);
    check(listed, "the list is read");
    check(error.empty(), "a successful list has no error");
    check(applications.size() == 2, "both applications are parsed");
    check(applications.size() == 2 && applications[0].name == "game-server" &&
              applications[0].status == "running:healthy" && applications[0].id == 1,
          "name, status and id come through");
    check(applications.size() == 2 && applications[0].running(), "running:healthy counts as up");
    check(applications.size() == 2 && !applications[1].running(), "exited does not count as up");
    check(applications.size() == 2 && applications[1].name == "app-two",
          "an application with no name falls back to its uuid");

    const std::string request = server.request();
    check(contains(request, "GET /api/v1/applications "), "it asks for the applications endpoint");
    check(contains(lowered(request), "authorization: bearer test-token"),
          "it sends the token as a bearer header");
}

void testEmptyList() {
    StubServer server(responseFor(200, "OK", "[]"));
    check(server.start(), "stub server listens");

    rblx::coolify::Config config;
    config.baseUrl = server.baseUrl();
    config.token = "t";

    std::vector<rblx::coolify::Application> applications;
    std::string error;
    check(rblx::coolify::listApplications(config, &applications, &error),
          "an account with no applications is an answer, not a fault");
    check(applications.empty(), "the empty list stays empty");
    server.request();
}

void testRefusals() {
    {
        StubServer server(responseFor(401, "Unauthorized", "{\"message\":\"Unauthenticated.\"}"));
        check(server.start(), "stub server listens");
        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "wrong";
        std::vector<rblx::coolify::Application> applications;
        std::string error;
        check(!rblx::coolify::listApplications(config, &applications, &error),
              "a 401 is a failure");
        check(contains(error, "API tokens"), "a 401 points at the API tokens screen");
        server.request();
    }
    {
        StubServer server(responseFor(404, "Not Found", "{\"message\":\"Not found.\"}"));
        check(server.start(), "stub server listens");
        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "t";
        std::vector<rblx::coolify::Application> applications;
        std::string error;
        check(!rblx::coolify::listApplications(config, &applications, &error), "a 404 is a failure");
        check(contains(error, "/api/v1"), "a 404 mentions where the API lives");
        server.request();
    }
    {
        StubServer server(responseFor(200, "OK", "not json at all"));
        check(server.start(), "stub server listens");
        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "t";
        std::vector<rblx::coolify::Application> applications;
        std::string error;
        check(!rblx::coolify::listApplications(config, &applications, &error),
              "a body that is not JSON is a failure, not an empty list");
        server.request();
    }
    {
        // Nothing listens there: the transport error must reach the caller.
        rblx::coolify::Config config;
        config.baseUrl = "http://127.0.0.1:1";
        config.token = "t";
        std::vector<rblx::coolify::Application> applications;
        std::string error;
        check(!rblx::coolify::listApplications(config, &applications, &error),
              "an unreachable server is a failure");
        check(contains(error, "could not reach"), "an unreachable server says so");
    }
    {
        rblx::coolify::Config config;
        std::vector<rblx::coolify::Application> applications;
        std::string error;
        check(!rblx::coolify::listApplications(config, &applications, &error) &&
                  contains(error, "Coolify address"),
              "no address is reported before a request is sent");
    }
}

void testActions() {
    const rblx::coolify::Action actions[] = {rblx::coolify::Action::Start,
                                             rblx::coolify::Action::Stop,
                                             rblx::coolify::Action::Restart};
    const char* slugs[] = {"start", "stop", "restart"};
    const char* labels[] = {"Start", "Stop", "Restart"};

    for (int index = 0; index < 3; ++index) {
        StubServer server(responseFor(200, "OK",
                                      "{\"message\":\"Deployment request queued.\","
                                      "\"deployment_uuid\":\"doogksw\"}"));
        check(server.start(), "stub server listens");

        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "test-token";

        std::string message;
        std::string error;
        const bool acted =
            rblx::coolify::performAction(config, "abc", actions[index], &message, &error);
        check(acted, "the action is accepted");
        check(contains(message, "queued"), "Coolify's own confirmation is passed on");
        check(std::string(rblx::coolify::actionSlug(actions[index])) == slugs[index],
              "the action path segment matches the endpoint");
        check(std::string(rblx::coolify::actionLabel(actions[index])) == labels[index],
              "the action label matches the button");

        const std::string request = server.request();
        const std::string expected = "POST /api/v1/applications/abc/" + std::string(slugs[index]);
        check(contains(request, expected), "it posts to the action endpoint");
    }
}

void testLogs() {
    {
        StubServer server(responseFor(200, "OK",
                                      "{\"logs\":\"line one\\nline two\\n\"}"));
        check(server.start(), "stub server listens");

        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "test-token";

        std::string logs;
        std::string error;
        check(rblx::coolify::fetchLogs(config, "abc", 50, &logs, &error), "logs are read");
        check(logs == "line one\nline two\n", "the log text is unescaped as sent");

        const std::string request = server.request();
        check(contains(request, "GET /api/v1/applications/abc/logs?lines=50"),
              "it asks for the requested number of lines");
    }
    {
        // A stopped container answers 400 with the reason; that sentence is the
        // whole story and must not be replaced with a generic one.
        StubServer server(responseFor(400, "Bad Request",
                                      "{\"message\":\"Application is not running.\"}"));
        check(server.start(), "stub server listens");

        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "test-token";

        std::string logs;
        std::string error;
        check(!rblx::coolify::fetchLogs(config, "abc", 50, &logs, &error), "a 400 is a failure");
        check(contains(error, "not running"), "Coolify's reason for the 400 is shown");
        server.request();
    }
    {
        StubServer server(responseFor(200, "OK", "{\"logs\":\"\"}"));
        check(server.start(), "stub server listens");

        rblx::coolify::Config config;
        config.baseUrl = server.baseUrl();
        config.token = "t";

        std::string logs;
        std::string error;
        check(rblx::coolify::fetchLogs(config, "abc", 50, &logs, &error), "empty logs are fine");
        check(contains(logs, "has not printed"), "empty logs say so instead of showing nothing");
        server.request();
    }
}

}  // namespace

int main() {
    std::printf("CoolifyClientTests\n");
    testAddresses();
    testList();
    testEmptyList();
    testRefusals();
    testActions();
    testLogs();

    if (failures == 0) {
        std::printf("All Coolify client checks passed.\n");
        return 0;
    }
    std::printf("%d check(s) failed.\n", failures);
    return 1;
}
