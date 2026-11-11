#include "ReverseProxy.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <sstream>
#include <utility>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>

namespace rblx {
namespace proxy {
namespace {

using SocketHandle = SOCKET;
const SocketHandle kInvalidSocket = INVALID_SOCKET;

// A head that runs past this is not something a browser or the website sends;
// it is a stuck or hostile peer.
const std::size_t kMaxHeadBytes = 64 * 1024;
// The site serves pages and images, not downloads, so an answer larger than
// this is refused instead of being held in memory on the backend's behalf.
const std::size_t kMaxBodyBytes = 64 * 1024 * 1024;

void ensureWinsock() {
    static const bool started = []() {
        WSADATA data;
        return WSAStartup(MAKEWORD(2, 2), &data) == 0;
    }();
    (void)started;
}

std::string lowerCopy(const std::string& value) {
    std::string out = value;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return out;
}

std::string trimCopy(const std::string& value) {
    std::size_t begin = 0;
    std::size_t end = value.size();
    while (begin < end && (value[begin] == ' ' || value[begin] == '\t')) {
        ++begin;
    }
    while (end > begin && (value[end - 1] == ' ' || value[end - 1] == '\t' ||
                           value[end - 1] == '\r')) {
        --end;
    }
    return value.substr(begin, end - begin);
}

bool sendAll(SocketHandle handle, const std::string& data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        const int chunk =
            ::send(handle, data.data() + sent, static_cast<int>(data.size() - sent), 0);
        if (chunk <= 0) {
            return false;
        }
        sent += static_cast<std::size_t>(chunk);
    }
    return true;
}

// Opens one connection to the backend. The connect itself is bounded by the
// same budget the reads get, because a backend that is down must fail the
// browser's request in a second rather than hold its connection open.
SocketHandle connectTo(const Upstream& upstream, std::string* error) {
    ensureWinsock();

    addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    addrinfo* found = nullptr;
    const std::string port = std::to_string(upstream.port);
    if (::getaddrinfo(upstream.host.c_str(), port.c_str(), &hints, &found) != 0 ||
        found == nullptr) {
        *error = "cannot resolve " + upstream.host;
        return kInvalidSocket;
    }

    const SocketHandle handle = ::socket(found->ai_family, found->ai_socktype, found->ai_protocol);
    if (handle == kInvalidSocket) {
        ::freeaddrinfo(found);
        *error = "cannot create a socket to " + upstream.host;
        return kInvalidSocket;
    }

    u_long nonBlocking = 1;
    ::ioctlsocket(handle, FIONBIO, &nonBlocking);
    const int connected = ::connect(handle, found->ai_addr, static_cast<int>(found->ai_addrlen));
    ::freeaddrinfo(found);

    if (connected != 0) {
        const int code = ::WSAGetLastError();
        if (code != WSAEWOULDBLOCK) {
            ::closesocket(handle);
            *error = "cannot reach the backend: error " + std::to_string(code);
            return kInvalidSocket;
        }

        fd_set writable;
        FD_ZERO(&writable);
        FD_SET(handle, &writable);
        timeval wait{};
        wait.tv_sec = upstream.timeoutMs / 1000;
        wait.tv_usec = static_cast<long>((upstream.timeoutMs % 1000) * 1000);
        if (::select(0, nullptr, &writable, nullptr, &wait) <= 0) {
            ::closesocket(handle);
            *error = "the backend did not answer within " + std::to_string(upstream.timeoutMs) +
                     " ms";
            return kInvalidSocket;
        }

        int pending = 0;
        int length = sizeof(pending);
        if (::getsockopt(handle, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&pending),
                         &length) != 0 ||
            pending != 0) {
            ::closesocket(handle);
            *error = "cannot reach the backend: error " + std::to_string(pending);
            return kInvalidSocket;
        }
    }

    nonBlocking = 0;
    ::ioctlsocket(handle, FIONBIO, &nonBlocking);

    // Every later read and write is bounded by this, so one stuck backend
    // thread cannot pin a proxy thread and the connection it holds.
    const DWORD budget = static_cast<DWORD>(upstream.timeoutMs);
    ::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&budget),
                 sizeof(budget));
    ::setsockopt(handle, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&budget),
                 sizeof(budget));
    return handle;
}

bool readResponse(SocketHandle handle, UpstreamResponse* response, std::string* error) {
    std::string raw;
    char buffer[8192];
    std::size_t headerEnd = std::string::npos;

    while ((headerEnd = raw.find("\r\n\r\n")) == std::string::npos) {
        const int received = ::recv(handle, buffer, static_cast<int>(sizeof(buffer)), 0);
        if (received <= 0) {
            *error = "the backend closed the connection before answering";
            return false;
        }
        raw.append(buffer, static_cast<std::size_t>(received));
        if (raw.size() > kMaxHeadBytes) {
            *error = "the backend's answer does not start with a usable head";
            return false;
        }
    }

    std::istringstream lines(raw.substr(0, headerEnd));
    std::string statusLine;
    if (!std::getline(lines, statusLine)) {
        *error = "the backend's answer is empty";
        return false;
    }
    if (!statusLine.empty() && statusLine.back() == '\r') {
        statusLine.pop_back();
    }

    std::istringstream statusStream(statusLine);
    std::string version;
    int status = 0;
    if (!(statusStream >> version >> status) || version.rfind("HTTP/", 0) != 0 || status < 100 ||
        status > 599) {
        *error = "the backend's answer does not start with an HTTP status line";
        return false;
    }
    response->status = status;

    std::string line;
    while (std::getline(lines, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        if (line.empty()) {
            continue;
        }
        const std::size_t colon = line.find(':');
        if (colon == std::string::npos) {
            continue;
        }
        const std::string name = lowerCopy(trimCopy(line.substr(0, colon)));
        const std::string value = trimCopy(line.substr(colon + 1));
        const auto existing = response->headers.find(name);
        if (existing != response->headers.end()) {
            // A map holds one value per name, but an answer may legitimately
            // carry several Set-Cookie lines and folding them with a comma would
            // corrupt a cookie that has an Expires date in it. The server writes
            // a value out verbatim after "name: ", so the extra lines ride along
            // instead of being lost.
            if (name == "set-cookie") {
                existing->second += "\r\nSet-Cookie: " + value;
            }
            continue;
        }
        response->headers[name] = value;
    }

    const auto transferEncoding = response->headers.find("transfer-encoding");
    if (transferEncoding != response->headers.end() &&
        lowerCopy(transferEncoding->second).find("chunked") != std::string::npos) {
        *error = "the backend answered with chunked encoding, which is not relayed";
        return false;
    }

    std::string body = raw.substr(headerEnd + 4);
    const auto declared = response->headers.find("content-length");
    if (declared != response->headers.end()) {
        std::size_t wanted = 0;
        try {
            wanted = static_cast<std::size_t>(std::stoull(declared->second));
        } catch (...) {
            *error = "the backend declared an unusable Content-Length";
            return false;
        }
        if (wanted > kMaxBodyBytes) {
            *error = "the backend's answer is too large to relay";
            return false;
        }
        while (body.size() < wanted) {
            const int received = ::recv(handle, buffer, static_cast<int>(sizeof(buffer)), 0);
            if (received <= 0) {
                *error = "the backend stopped part way through its answer";
                return false;
            }
            body.append(buffer, static_cast<std::size_t>(received));
        }
        // A recv may have brought the first bytes of the next answer with it;
        // this proxy asks for one answer per connection, so the excess is
        // dropped rather than handed to a request that never asked for it.
        body.resize(wanted);
    } else {
        while (true) {
            const int received = ::recv(handle, buffer, static_cast<int>(sizeof(buffer)), 0);
            if (received == 0) {
                break;
            }
            if (received < 0) {
                *error = "the backend's answer did not finish";
                return false;
            }
            body.append(buffer, static_cast<std::size_t>(received));
            if (body.size() > kMaxBodyBytes) {
                *error = "the backend's answer is too large to relay";
                return false;
            }
        }
    }

    response->body = std::move(body);
    const auto contentType = response->headers.find("content-type");
    response->contentType = contentType == response->headers.end()
                                ? std::string("application/octet-stream")
                                : contentType->second;
    response->headers.erase("content-type");
    return true;
}

}  // namespace

bool parseHostPort(const std::string& spec, std::string* host, int* port, std::string* error) {
    const std::size_t colon = spec.rfind(':');
    if (colon == std::string::npos) {
        *error = spec + ": expected host:port, such as 127.0.0.1:3000";
        return false;
    }

    const std::string name = trimCopy(spec.substr(0, colon));
    const std::string number = trimCopy(spec.substr(colon + 1));
    if (name.empty()) {
        *error = spec + ": no host before the colon";
        return false;
    }
    if (name.find("://") != std::string::npos) {
        *error = spec + ": write host:port, without http://";
        return false;
    }
    if (number.empty()) {
        *error = spec + ": no port after the colon";
        return false;
    }
    if (number.size() > 5 || number.find_first_not_of("0123456789") != std::string::npos) {
        *error = spec + ": " + number + " is not a port number";
        return false;
    }

    const int value = std::stoi(number);
    if (value < 1 || value > 65535) {
        *error = spec + ": " + number + " is outside the port range 1-65535";
        return false;
    }

    *host = name;
    *port = value;
    return true;
}

bool isHopByHop(const std::string& name) {
    const std::string header = lowerCopy(name);
    return header == "connection" || header == "keep-alive" || header == "proxy-authenticate" ||
           header == "proxy-authorization" || header == "proxy-connection" || header == "te" ||
           header == "trailer" || header == "trailers" || header == "transfer-encoding" ||
           header == "upgrade";
}

std::string forwardedFor(const std::string& existing, const std::string& address) {
    if (address.empty()) {
        // Nothing was learned about the peer (the socket had no address to
        // read), so a header that only claims what earlier proxies said is
        // still more honest than inventing an address.
        return existing;
    }
    if (existing.empty()) {
        return address;
    }
    return existing + ", " + address;
}

std::string encodeTarget(const std::string& path, const std::string& query) {
    static const char* kHex = "0123456789ABCDEF";

    std::string out;
    out.reserve(path.size() + query.size() + 1);
    for (unsigned char c : path) {
        // The server hands the path over decoded, so anything that was escaped -
        // a space in a file name above all - has to be escaped again, or the
        // request line would break in two where the space is.
        const bool plain = c > 0x20 && c < 0x7f && c != '%' && c != '"' && c != '<' && c != '>' &&
                           c != '\\' && c != '^' && c != '`' && c != '{' && c != '}' && c != '|';
        if (plain) {
            out.push_back(static_cast<char>(c));
            continue;
        }
        out.push_back('%');
        out.push_back(kHex[c >> 4]);
        out.push_back(kHex[c & 0x0f]);
    }
    if (out.empty()) {
        out = "/";
    }
    // The query is not decoded by the server, so re-escaping it here would turn
    // an existing %20 into %2520.
    if (!query.empty()) {
        out.push_back('?');
        out += query;
    }
    return out;
}

bool forward(const Upstream& upstream, const web::Request& request, UpstreamResponse* response,
             std::string* error) {
    *response = UpstreamResponse();

    std::string head =
        request.method + " " + encodeTarget(request.path, request.query) + " HTTP/1.1\r\n";
    // The backend's Host is the backend's own address: it routes on the path, and
    // the browser's address is passed on separately for logs and links.
    head += "Host: " + upstream.host + ":" + std::to_string(upstream.port) + "\r\n";
    for (const auto& header : request.headers) {
        // Framing and hop-by-hop headers describe the browser's connection to
        // this proxy. The backend gets a fresh connection with its own framing.
        if (isHopByHop(header.first) || header.first == "host" ||
            header.first == "content-length") {
            continue;
        }
        head += header.first + ": " + header.second + "\r\n";
    }
    // The names this proxy adds itself are written the way the request's own
    // headers are: lower case, because that is how the request map holds them and
    // a head that mixes the two is a second spelling for the same header.
    const std::string forwarded =
        forwardedFor(request.header("x-forwarded-for"), request.remoteAddress);
    if (!forwarded.empty()) {
        head += "x-forwarded-for: " + forwarded + "\r\n";
    }
    const std::string originalHost = request.header("host");
    if (!originalHost.empty()) {
        head += "x-forwarded-host: " + originalHost + "\r\n";
    }
    head += "x-forwarded-proto: http\r\n";
    head += "content-length: " + std::to_string(request.body.size()) + "\r\n";
    // One request per backend connection: the answer is read to its end and the
    // connection closed, so nothing here depends on the backend's keep-alive.
    head += "connection: close\r\n\r\n";

    const SocketHandle handle = connectTo(upstream, error);
    if (handle == kInvalidSocket) {
        return false;
    }

    bool ok = sendAll(handle, head + request.body);
    if (!ok) {
        *error = "the backend stopped accepting the request";
    } else {
        ok = readResponse(handle, response, error);
    }
    ::closesocket(handle);
    return ok;
}

ReverseProxy::ReverseProxy(Upstream upstream, bool logRequests)
    : upstream_(std::move(upstream)), logRequests_(logRequests) {}

bool ReverseProxy::handle(web::Request& request, web::Response& response) {
    const auto started = std::chrono::steady_clock::now();
    const std::string target = request.path + (request.query.empty() ? "" : "?" + request.query);

    UpstreamResponse answer;
    std::string error;
    if (!forward(upstream_, request, &answer, &error)) {
        // The browser is told that the site is down and nothing else: why the
        // backend failed is a detail of this machine, not of the page.
        response.json(502, "{\"error\":\"The website is not answering.\"}");
        if (logRequests_) {
            std::fprintf(stderr, "RBLXProxy: %s %s -> 502 (%s)\n", request.method.c_str(),
                         target.c_str(), error.c_str());
            std::fflush(stderr);
        }
        return true;
    }

    response.status = answer.status;
    response.body = std::move(answer.body);
    response.contentType = answer.contentType;
    for (const auto& header : answer.headers) {
        // These belong to the answer this proxy is writing: the server sets
        // the framing itself, and its own copies of the security headers are
        // added below the map. A second copy of a Content-Length would break
        // framing, and a second policy header would narrow it twice.
        if (isHopByHop(header.first) || header.first == "content-length" ||
            header.first == "x-content-type-options" || header.first == "x-frame-options" ||
            header.first == "referrer-policy" || header.first == "content-security-policy") {
            continue;
        }
        response.headers[header.first] = header.second;
    }

    if (logRequests_) {
        const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 std::chrono::steady_clock::now() - started)
                                 .count();
        std::printf("RBLXProxy: %s %s -> %d, %zu bytes, %lld ms\n", request.method.c_str(),
                    target.c_str(), answer.status, response.body.size(),
                    static_cast<long long>(elapsed));
        std::fflush(stdout);
    }
    return true;
}

}  // namespace proxy
}  // namespace rblx
