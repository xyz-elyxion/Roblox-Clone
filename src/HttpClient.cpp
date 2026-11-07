#include "HttpClient.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <mutex>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    using SocketHandle = SOCKET;
    constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
    #define RBLX_CLOSE_SOCKET(s) closesocket(s)
    #define RBLX_LAST_ERROR() WSAGetLastError()
#else
    #include <arpa/inet.h>
    #include <cerrno>
    #include <csignal>
    #include <fcntl.h>
    #include <netdb.h>
    #include <netinet/in.h>
    #include <sys/select.h>
    #include <sys/socket.h>
    #include <sys/time.h>
    #include <unistd.h>
    using SocketHandle = int;
    constexpr SocketHandle kInvalidSocket = -1;
    #define RBLX_CLOSE_SOCKET(s) ::close(s)
    #define RBLX_LAST_ERROR() errno
#endif

namespace rblx {
namespace http {
namespace {

struct ParsedUrl {
    std::string host;
    std::string path;   // path plus query string
    uint16_t port = 80;
};

void ensureSockets() {
#ifdef _WIN32
    static std::once_flag once;
    std::call_once(once, [] {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    });
#else
    static std::once_flag once;
    std::call_once(once, [] {
#ifdef SIGPIPE
        std::signal(SIGPIPE, SIG_IGN);
#endif
    });
#endif
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

// Accepts http://host[:port][/path]. Anything else is rejected, including
// https://, because this project has no TLS implementation to offer.
bool parseUrl(const std::string& url, ParsedUrl& out, std::string* error) {
    const std::string prefix = "http://";
    if (url.compare(0, prefix.size(), prefix) != 0) {
        if (error) {
            *error = url.compare(0, 8, "https://") == 0
                         ? "Only plain http:// URLs are supported by this client (no TLS)."
                         : "URL must start with http://.";
        }
        return false;
    }

    const size_t hostBegin = prefix.size();
    const size_t pathBegin = url.find('/', hostBegin);
    const std::string authority = url.substr(hostBegin,
                                             pathBegin == std::string::npos ? std::string::npos
                                                                            : pathBegin - hostBegin);
    out.path = pathBegin == std::string::npos ? "/" : url.substr(pathBegin);
    if (authority.empty()) {
        if (error) *error = "URL has no host.";
        return false;
    }

    const size_t colon = authority.rfind(':');
    if (colon != std::string::npos && authority.find(']') == std::string::npos) {
        out.host = authority.substr(0, colon);
        const int port = std::atoi(authority.c_str() + colon + 1);
        if (port > 0 && port <= 65535) {
            out.port = static_cast<uint16_t>(port);
        }
    } else {
        out.host = authority;
    }

    if (out.host.empty()) {
        if (error) *error = "URL has no host.";
        return false;
    }
    return true;
}

SocketHandle connectToHost(const std::string& host, uint16_t port, int timeoutMs, std::string* error) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* results = nullptr;
    const std::string service = std::to_string(port);
    if (getaddrinfo(host.c_str(), service.c_str(), &hints, &results) != 0 || !results) {
        if (error) *error = "Could not resolve " + host + ".";
        return kInvalidSocket;
    }

    SocketHandle connected = kInvalidSocket;
    for (addrinfo* entry = results; entry != nullptr; entry = entry->ai_next) {
        SocketHandle candidate = ::socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (candidate == kInvalidSocket) {
            continue;
        }

        // Connect without blocking so the timeout is ours, not the stack's.
#ifdef _WIN32
        unsigned long nonBlocking = 1;
        ioctlsocket(candidate, FIONBIO, &nonBlocking);
#else
        const int flags = fcntl(candidate, F_GETFL, 0);
        fcntl(candidate, F_SETFL, flags | O_NONBLOCK);
#endif

        const int rc = ::connect(candidate, entry->ai_addr, static_cast<int>(entry->ai_addrlen));
        bool ready = rc == 0;
        if (!ready) {
            const int lastError = RBLX_LAST_ERROR();
            const bool inProgress =
#ifdef _WIN32
                lastError == WSAEWOULDBLOCK;
#else
                lastError == EINPROGRESS || lastError == EWOULDBLOCK;
#endif
            if (inProgress) {
                fd_set writable;
                FD_ZERO(&writable);
                FD_SET(candidate, &writable);
                timeval timeout{};
                timeout.tv_sec = timeoutMs / 1000;
                timeout.tv_usec = (timeoutMs % 1000) * 1000;
                ready = ::select(static_cast<int>(candidate) + 1, nullptr, &writable, nullptr, &timeout) > 0;
            }
        }

        if (!ready) {
            RBLX_CLOSE_SOCKET(candidate);
            continue;
        }

        // Back to blocking, with read/write timeouts so a stalled peer returns
        // instead of hanging the caller.
#ifdef _WIN32
        unsigned long blocking = 0;
        ioctlsocket(candidate, FIONBIO, &blocking);
        DWORD timeout = static_cast<DWORD>(timeoutMs);
        setsockopt(candidate, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        setsockopt(candidate, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
#else
        fcntl(candidate, F_SETFL, flags);
        timeval timeout{};
        timeout.tv_sec = timeoutMs / 1000;
        timeout.tv_usec = (timeoutMs % 1000) * 1000;
        setsockopt(candidate, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
        setsockopt(candidate, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));
#endif

        connected = candidate;
        break;
    }

    freeaddrinfo(results);

    if (connected == kInvalidSocket && error) {
        *error = "Could not connect to " + host + ":" + service + ".";
    }
    return connected;
}

bool sendAll(SocketHandle socket, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
#ifdef _WIN32
        const int written = ::send(socket, data.data() + sent, static_cast<int>(data.size() - sent), 0);
#else
        const ssize_t written = ::send(socket, data.data() + sent, data.size() - sent, 0);
#endif
        if (written <= 0) {
            return false;
        }
        sent += static_cast<size_t>(written);
    }
    return true;
}

// Content-Length from a block of headers, or -1 when the server did not say.
// Read before the body so a progress bar knows what it is moving towards.
long long declaredLength(const std::string& headers) {
    const std::string lowered = lower(headers);
    const size_t position = lowered.find("content-length:");
    if (position == std::string::npos) {
        return -1;
    }
    return std::atoll(headers.c_str() + position + std::strlen("content-length:"));
}

}  // namespace

Response request(const std::string& method,
                 const std::string& url,
                 const std::string& body,
                 const std::string& bearerToken,
                 int timeoutMs,
                 const ProgressCallback& onProgress) {
    Response response;

    ParsedUrl parsed;
    if (!parseUrl(url, parsed, &response.error)) {
        return response;
    }

    ensureSockets();

    const SocketHandle socket = connectToHost(parsed.host, parsed.port, timeoutMs, &response.error);
    if (socket == kInvalidSocket) {
        return response;
    }

    std::string head = method + " " + parsed.path + " HTTP/1.1\r\n";
    head += "Host: " + parsed.host;
    if (parsed.port != 80) {
        head += ":" + std::to_string(parsed.port);
    }
    head += "\r\n";
    head += "User-Agent: RBLX Client/1.0\r\n";
    head += "Accept: application/json\r\n";
    head += "Connection: close\r\n";
    if (!body.empty() || method == "POST" || method == "PUT") {
        head += "Content-Type: application/json\r\n";
        head += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    }
    if (!bearerToken.empty()) {
        head += "Authorization: Bearer " + bearerToken + "\r\n";
    }
    head += "\r\n";

    if (!sendAll(socket, head + body)) {
        RBLX_CLOSE_SOCKET(socket);
        response.error = "Could not send the request to " + parsed.host + ".";
        return response;
    }

    // The backend answers with Connection: close, so reading to EOF is enough
    // to know the response is complete.
    std::string raw;
    char buffer[4096];
    bool headersSeen = false;
    std::size_t bodyStart = 0;
    long long declared = -1;
    while (true) {
#ifdef _WIN32
        const int received = ::recv(socket, buffer, static_cast<int>(sizeof(buffer)), 0);
#else
        const ssize_t received = ::recv(socket, buffer, sizeof(buffer), 0);
#endif
        if (received <= 0) {
            break;
        }
        raw.append(buffer, static_cast<size_t>(received));

        // The length is worth reading as soon as the headers are complete, even
        // though the body is assembled below: a caller watching a download wants
        // a total to move towards.
        if (!headersSeen) {
            const size_t headerEnd = raw.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headersSeen = true;
                bodyStart = headerEnd + 4;
                declared = declaredLength(raw.substr(0, headerEnd));
            }
        }
        if (onProgress) {
            const long long inBody =
                headersSeen ? static_cast<long long>(raw.size() - bodyStart) : 0;
            onProgress(inBody, declared);
        }

        if (raw.size() > 8 * 1024 * 1024) {
            break;  // a runaway response must not grow without bound
        }
    }
    RBLX_CLOSE_SOCKET(socket);

    if (raw.empty()) {
        response.error = "No response from " + parsed.host + ".";
        return response;
    }

    const size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        response.error = "Malformed HTTP response.";
        return response;
    }

    const std::string headers = raw.substr(0, headerEnd);
    std::string payload = raw.substr(headerEnd + 4);

    // Status line: HTTP/1.1 200 OK
    const size_t firstSpace = headers.find(' ');
    if (firstSpace == std::string::npos) {
        response.error = "Malformed HTTP status line.";
        return response;
    }
    response.status = std::atoi(headers.c_str() + firstSpace + 1);

    // Honour Content-Length when present: with keep-alive headers a server may
    // leave the connection open, and the body would otherwise be ambiguous.
    const std::string lowered = lower(headers);
    const size_t lengthHeader = lowered.find("content-length:");
    if (lengthHeader != std::string::npos) {
        const size_t valueBegin = lengthHeader + std::strlen("content-length:");
        const long long declared = std::atoll(headers.c_str() + valueBegin);
        if (declared >= 0 && static_cast<size_t>(declared) <= payload.size()) {
            payload.resize(static_cast<size_t>(declared));
        }
    }

    response.body = std::move(payload);
    return response;
}

}  // namespace http
}  // namespace rblx
