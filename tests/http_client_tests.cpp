// Tests for the portable HTTP client that the client and game server use to
// reach the website backend. A tiny listener on an ephemeral port stands in for
// the backend, so the tests check the real socket path rather than a mock.

#include "HttpClient.h"

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
    // Winsock refuses socket(), bind() and listen() until WSAStartup has run.
    // The client does that lazily inside rblx::http::request, but the stub
    // servers below create their listening socket before any request is made,
    // so the test has to initialise Winsock itself. A file-scope object is the
    // earliest point at which that can happen.
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

    // A listener that answers each connection with a canned response and
    // records the request it received.
    class StubServer {
    public:
        StubServer(std::string response, int connections)
            : response_(std::move(response)), remaining_(connections) {}

        ~StubServer() { stop(); }

        bool start() {
            listening_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            if (listening_ == kInvalid) return false;

            int reuse = 1;
            setsockopt(listening_, SOL_SOCKET, SO_REUSEADDR,
                       reinterpret_cast<const char*>(&reuse), sizeof(reuse));

            sockaddr_in address{};
            address.sin_family = AF_INET;
            address.sin_port = 0;  // let the OS pick a free port
            address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

            if (bind(listening_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
                return false;
            }

            sockaddr_in bound{};
#ifdef _WIN32
            int length = sizeof(bound);
#else
            socklen_t length = sizeof(bound);
#endif
            if (getsockname(listening_, reinterpret_cast<sockaddr*>(&bound), &length) != 0) {
                return false;
            }
            port_ = ntohs(bound.sin_port);

            if (listen(listening_, 4) != 0) {
                return false;
            }

            worker_ = std::thread([this] { serve(); });
            return true;
        }

        void stop() {
            if (!started_.exchange(true)) {
                if (worker_.joinable()) {
                    worker_.join();
                }
            }
        }

        uint16_t port() const { return port_; }
        const std::string& lastRequest() const { return lastRequest_; }

    private:
        void serve() {
            while (remaining_-- > 0) {
                const SocketHandle client = ::accept(listening_, nullptr, nullptr);
                if (client == kInvalid) {
                    break;
                }

                std::string request;
                char buffer[2048];
                while (request.find("\r\n\r\n") == std::string::npos) {
                    const auto received = ::recv(client, buffer, sizeof(buffer), 0);
                    if (received <= 0) break;
                    request.append(buffer, static_cast<size_t>(received));
                }

                // Read any request body the client announced.
                size_t bodyStart = request.find("\r\n\r\n");
                if (bodyStart != std::string::npos) {
                    bodyStart += 4;
                    size_t declared = 0;
                    const size_t lengthHeader = request.find("Content-Length:");
                    if (lengthHeader != std::string::npos) {
                        declared = static_cast<size_t>(std::atoi(request.c_str() + lengthHeader + 15));
                    }
                    while (request.size() - bodyStart < declared) {
                        const auto received = ::recv(client, buffer, sizeof(buffer), 0);
                        if (received <= 0) break;
                        request.append(buffer, static_cast<size_t>(received));
                    }
                }

                lastRequest_ = request;
                ::send(client, response_.data(), static_cast<int>(response_.size()), 0);
                TEST_CLOSE(client);
            }
        }

        std::string response_;
        int remaining_ = 1;
        SocketHandle listening_ = kInvalid;
        uint16_t port_ = 0;
        std::thread worker_;
        std::atomic<bool> started_{false};
        std::string lastRequest_;
    };

    std::string baseUrl(const StubServer& server) {
        return "http://127.0.0.1:" + std::to_string(server.port());
    }
}

int main() {
    std::printf("HTTP client tests\n");

    {
        // The declared length matches the body exactly: the client trims to it,
        // so a mismatch here would truncate the assertion below.
        StubServer server("HTTP/1.1 200 OK\r\nContent-Type: application/json\r\n"
                          "Content-Length: 22\r\nConnection: close\r\n\r\n{\"success\":true,\"n\":1}",
                          1);
        check(server.start(), "stub server listens on an ephemeral port");

        const rblx::http::Response response = rblx::http::request("GET", baseUrl(server) + "/api/games");
        check(response.ok(), "GET returns a 2xx status");
        check(response.status == 200, "status code is parsed from the status line");
        check(response.body == "{\"success\":true,\"n\":1}", "body is read in full");
        check(!response.networkError(), "a completed request is not a network error");

        const std::string request = server.lastRequest();
        check(request.compare(0, 4, "GET ") == 0, "request line uses the given method");
        check(request.find("GET /api/games HTTP/1.1") != std::string::npos, "path is sent verbatim");
        check(request.find("Host: 127.0.0.1") != std::string::npos, "Host header carries the port");
        check(request.find("Connection: close") != std::string::npos, "client asks the server to close");

        server.stop();
    }

    {
        StubServer server("HTTP/1.1 201 Created\r\nContent-Length: 2\r\n\r\n{}", 1);
        check(server.start(), "second stub server listens");

        const rblx::http::Response response =
            rblx::http::request("POST", baseUrl(server) + "/api/login", "{\"username\":\"a\"}", "token-123");
        check(response.status == 201, "non-200 success codes are reported as they arrived");
        check(response.ok(), "201 counts as success");

        const std::string request = server.lastRequest();
        check(request.compare(0, 5, "POST ") == 0, "POST method is sent");
        check(request.find("Authorization: Bearer token-123") != std::string::npos, "bearer token is sent");
        check(request.find("Content-Type: application/json") != std::string::npos, "JSON content type is set");
        check(request.find("Content-Length: 16") != std::string::npos, "body length is declared");
        check(request.size() >= 18 && request.substr(request.size() - 16) == "{\"username\":\"a\"}",
              "body follows the headers");

        server.stop();
    }

    {
        StubServer server("HTTP/1.1 404 Not Found\r\nContent-Length: 2\r\n\r\n{}", 1);
        check(server.start(), "third stub server listens");

        const rblx::http::Response response = rblx::http::request("GET", baseUrl(server) + "/missing");
        check(response.status == 404, "error status is reported");
        check(!response.ok(), "404 is not a success");

        server.stop();
    }

    {
        const rblx::http::Response response = rblx::http::request("GET", "https://example.com/");
        check(response.networkError(), "https URLs fail instead of pretending to work");
        check(response.error.find("TLS") != std::string::npos, "https failure explains the missing TLS support");
    }

    {
        // Port 1 on loopback: nothing listens there, so the connect must fail
        // rather than hang.
        const auto started = std::chrono::steady_clock::now();
        const rblx::http::Response response =
            rblx::http::request("GET", "http://127.0.0.1:1/", "", "", 1000);
        const auto elapsed = std::chrono::steady_clock::now() - started;

        check(response.networkError(), "an unreachable server is a network error");
        check(!response.error.empty(), "an unreachable server explains itself");
        check(elapsed < std::chrono::seconds(5), "a failed connect returns promptly");
    }

    if (failures == 0) {
        std::printf("All HTTP client checks passed.\n");
        return 0;
    }
    std::printf("%d HTTP client check(s) failed.\n", failures);
    return 1;
}
