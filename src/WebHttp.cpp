#include "WebHttp.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <sstream>
#include <thread>
#include <utility>

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <cerrno>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#endif

namespace rblx {
namespace web {
namespace {

#ifdef _WIN32
using SocketHandle = SOCKET;
const SocketHandle kInvalidSocket = INVALID_SOCKET;
#else
using SocketHandle = int;
const SocketHandle kInvalidSocket = -1;
#endif

void closeSocket(SocketHandle handle) {
#ifdef _WIN32
    closesocket(handle);
#else
    ::close(handle);
#endif
}

int lastSocketError() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
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

int hexDigitValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }
    return -1;
}

std::string urlDecode(const std::string& value, bool plusIsSpace) {
    std::string out;
    out.reserve(value.size());
    for (std::size_t i = 0; i < value.size(); ++i) {
        const char c = value[i];
        if (c == '%' && i + 2 < value.size()) {
            const int high = hexDigitValue(value[i + 1]);
            const int low = hexDigitValue(value[i + 2]);
            if (high >= 0 && low >= 0) {
                out.push_back(static_cast<char>((high << 4) | low));
                i += 2;
                continue;
            }
        }
        if (plusIsSpace && c == '+') {
            out.push_back(' ');
            continue;
        }
        out.push_back(c);
    }
    return out;
}

void parseQueryString(const std::string& query, std::map<std::string, std::string>* out) {
    std::size_t start = 0;
    while (start <= query.size()) {
        const std::size_t amp = query.find('&', start);
        const std::string pair =
            query.substr(start, amp == std::string::npos ? std::string::npos : amp - start);
        if (!pair.empty()) {
            const std::size_t equals = pair.find('=');
            const std::string key =
                urlDecode(pair.substr(0, equals), true);
            const std::string value = equals == std::string::npos
                                          ? std::string()
                                          : urlDecode(pair.substr(equals + 1), true);
            (*out)[key] = value;
        }
        if (amp == std::string::npos) {
            break;
        }
        start = amp + 1;
    }
}

const char* statusText(int status) {
    switch (status) {
        case 200: return "OK";
        case 201: return "Created";
        case 204: return "No Content";
        case 301: return "Moved Permanently";
        case 302: return "Found";
        case 400: return "Bad Request";
        case 403: return "Forbidden";
        case 404: return "Not Found";
        case 405: return "Method Not Allowed";
        case 409: return "Conflict";
        case 413: return "Payload Too Large";
        case 415: return "Unsupported Media Type";
        case 429: return "Too Many Requests";
        case 500: return "Internal Server Error";
        default: return "OK";
    }
}

bool sendAll(SocketHandle handle, const std::string& data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
#ifdef _WIN32
        const int written = ::send(handle, data.data() + sent,
                                   static_cast<int>(data.size() - sent), 0);
#else
        const ssize_t written = ::send(handle, data.data() + sent,
                                       data.size() - sent, 0);
#endif
        if (written <= 0) {
            return false;
        }
        sent += static_cast<std::size_t>(written);
    }
    return true;
}

bool readFile(const std::string& path, std::string* out) {
    std::ifstream stream(path.c_str(), std::ios::binary);
    if (!stream) {
        return false;
    }
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    *out = buffer.str();
    return true;
}

}  // namespace

std::string Request::header(const std::string& name) const {
    const auto found = headers.find(lowerCopy(name));
    return found == headers.end() ? std::string() : found->second;
}

std::string Request::queryParam(const std::string& name) const {
    const auto found = queryParams.find(name);
    return found == queryParams.end() ? std::string() : found->second;
}

std::string Request::param(const std::string& name) const {
    const auto found = params.find(name);
    return found == params.end() ? std::string() : found->second;
}

bool Request::contentTypeStartsWith(const std::string& prefix) const {
    return lowerCopy(header("content-type")).compare(0, prefix.size(), lowerCopy(prefix)) == 0;
}

void Response::json(int statusCode, const std::string& payload) {
    status = statusCode;
    contentType = "application/json; charset=utf-8";
    body = payload;
    headers.erase("Content-Type");
}

void Response::text(int statusCode, const std::string& payload, const std::string& type) {
    status = statusCode;
    contentType = type;
    body = payload;
    headers.erase("Content-Type");
}

void Response::file(int statusCode, const std::string& absolutePath, const std::string& type) {
    status = statusCode;
    contentType = type;
    body.clear();
    headers["X-Served-File"] = absolutePath;
}

HttpServer::HttpServer() {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

HttpServer::~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void HttpServer::route(const std::string& method, const std::string& pattern,
                       Handler handler) {
    RouteEntry entry;
    entry.method = method;
    entry.pattern = pattern;
    entry.handler = std::move(handler);
    routes_.push_back(std::move(entry));
}

void HttpServer::setFallback(Handler handler) {
    fallback_ = std::move(handler);
}

bool HttpServer::matchesPattern(const std::string& pattern, const std::string& path,
                                std::map<std::string, std::string>* params) {
    std::vector<std::string> patternParts;
    std::vector<std::string> pathParts;

    auto split = [](const std::string& value, std::vector<std::string>* out) {
        std::size_t start = 1;
        if (value.empty() || value[0] != '/') {
            return;
        }
        while (start <= value.size()) {
            const std::size_t slash = value.find('/', start);
            out->push_back(value.substr(
                start, slash == std::string::npos ? std::string::npos : slash - start));
            if (slash == std::string::npos) {
                break;
            }
            start = slash + 1;
        }
    };

    split(pattern, &patternParts);
    split(path, &pathParts);
    if (patternParts.size() != pathParts.size()) {
        return false;
    }

    for (std::size_t i = 0; i < patternParts.size(); ++i) {
        const std::string& expected = patternParts[i];
        if (!expected.empty() && expected[0] == ':') {
            (*params)[expected.substr(1)] = pathParts[i];
            continue;
        }
        if (expected != pathParts[i]) {
            return false;
        }
    }
    return true;
}

bool HttpServer::dispatch(Request& request, Response& response) const {
    for (const RouteEntry& entry : routes_) {
        if (entry.method != request.method && entry.method != "*") {
            continue;
        }
        std::map<std::string, std::string> params;
        if (!matchesPattern(entry.pattern, request.path, &params)) {
            continue;
        }
        request.params = params;
        return entry.handler(request, response);
    }
    if (fallback_) {
        return fallback_(request, response);
    }
    return false;
}

bool HttpServer::listen(const std::string& host, int port, std::string* error) {
    const SocketHandle handle = ::socket(AF_INET, SOCK_STREAM, 0);
    if (handle == kInvalidSocket) {
        if (error) {
            *error = "Could not create a listening socket (error " +
                     std::to_string(lastSocketError()) + ").";
        }
        return false;
    }

    int reuse = 1;
    ::setsockopt(handle, SOL_SOCKET, SO_REUSEADDR,
                 reinterpret_cast<const char*>(&reuse), sizeof(reuse));

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<unsigned short>(port));
    if (::inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    }

    if (::bind(handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) != 0) {
        if (error) {
            *error = "Could not bind " + host + ":" + std::to_string(port) + " (error " +
                     std::to_string(lastSocketError()) + ").";
        }
        closeSocket(handle);
        return false;
    }

    if (::listen(handle, 64) != 0) {
        if (error) {
            *error = "Could not listen on " + host + ":" + std::to_string(port) + ".";
        }
        closeSocket(handle);
        return false;
    }

    if (port == 0) {
        sockaddr_in bound{};
#ifdef _WIN32
        int length = sizeof(bound);
#else
        socklen_t length = sizeof(bound);
#endif
        if (::getsockname(handle, reinterpret_cast<sockaddr*>(&bound), &length) == 0) {
            port_ = ntohs(bound.sin_port);
        }
    } else {
        port_ = port;
    }

    listenSocket_ = static_cast<std::intptr_t>(handle);
    running_ = true;
    return true;
}

void HttpServer::stop() {
    running_ = false;
    if (listenSocket_ >= 0) {
        closeSocket(static_cast<SocketHandle>(listenSocket_));
        listenSocket_ = -1;
    }
}

bool HttpServer::readRequest(std::intptr_t socketHandle, Request* request, bool* timedOut) {
    const SocketHandle handle = static_cast<SocketHandle>(socketHandle);
    *timedOut = false;

    std::string buffer;
    char chunk[4096];
    std::size_t headerEnd = std::string::npos;

    while ((headerEnd = buffer.find("\r\n\r\n")) == std::string::npos) {
#ifdef _WIN32
        const int received = ::recv(handle, chunk, static_cast<int>(sizeof(chunk)), 0);
#else
        const ssize_t received = ::recv(handle, chunk, sizeof(chunk), 0);
#endif
        if (received <= 0) {
            *timedOut = received < 0;
            return false;
        }
        buffer.append(chunk, static_cast<std::size_t>(received));
        if (buffer.size() > 64 * 1024) {
            return false;
        }
    }

    const std::string headerBlock = buffer.substr(0, headerEnd);
    std::istringstream headerStream(headerBlock);
    std::string requestLine;
    if (!std::getline(headerStream, requestLine)) {
        return false;
    }
    if (!requestLine.empty() && requestLine.back() == '\r') {
        requestLine.pop_back();
    }

    std::istringstream requestLineStream(requestLine);
    std::string target;
    if (!(requestLineStream >> request->method >> target >> request->httpVersion)) {
        return false;
    }

    const std::size_t queryStart = target.find('?');
    request->path = urlDecode(target.substr(0, queryStart), false);
    if (queryStart != std::string::npos) {
        request->query = target.substr(queryStart + 1);
        parseQueryString(request->query, &request->queryParams);
    }

    std::string line;
    while (std::getline(headerStream, line)) {
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
        request->headers[lowerCopy(trimCopy(line.substr(0, colon)))] =
            trimCopy(line.substr(colon + 1));
    }

    std::size_t contentLength = 0;
    const std::string declaredLength = request->header("content-length");
    if (!declaredLength.empty()) {
        try {
            contentLength = static_cast<std::size_t>(std::stoull(declaredLength));
        } catch (...) {
            return false;
        }
    }
    if (contentLength > bodyLimitBytes_) {
        request->body.clear();
        request->headers["x-body-too-large"] = "1";
        return true;
    }

    std::string body = buffer.substr(headerEnd + 4);
    while (body.size() < contentLength) {
#ifdef _WIN32
        const int received = ::recv(handle, chunk, static_cast<int>(sizeof(chunk)), 0);
#else
        const ssize_t received = ::recv(handle, chunk, sizeof(chunk), 0);
#endif
        if (received <= 0) {
            return false;
        }
        body.append(chunk, static_cast<std::size_t>(received));
    }
    body.resize(contentLength);
    request->body = body;

    sockaddr_in peer{};
#ifdef _WIN32
    int peerLength = sizeof(peer);
#else
    socklen_t peerLength = sizeof(peer);
#endif
    if (::getpeername(handle, reinterpret_cast<sockaddr*>(&peer), &peerLength) == 0) {
        char text[64];
        if (::inet_ntop(AF_INET, &peer.sin_addr, text, sizeof(text)) != nullptr) {
            request->remoteAddress = text;
        }
    }
    return true;
}

bool HttpServer::writesResponse(std::intptr_t socketHandle, const Request& request,
                               Response& response, bool* keepAlive) {
    const SocketHandle handle = static_cast<SocketHandle>(socketHandle);

    std::string body = response.body;
    const auto fileHeader = response.headers.find("X-Served-File");
    if (fileHeader != response.headers.end()) {
        const std::string path = fileHeader->second;
        response.headers.erase(fileHeader);
        if (!readFile(path, &body)) {
            response.status = 404;
            response.contentType = "application/json; charset=utf-8";
            body = "{\"error\":\"Not found.\"}";
        }
    }

    const std::string connectionHeader = lowerCopy(request.header("connection"));
    *keepAlive = request.httpVersion != "HTTP/1.0" || connectionHeader == "keep-alive";
    if (connectionHeader == "close") {
        *keepAlive = false;
    }

    std::string out;
    out.reserve(body.size() + 256);
    out += "HTTP/1.1 " + std::to_string(response.status) + " " +
           std::string(statusText(response.status)) + "\r\n";
    out += "Content-Type: " + response.contentType + "\r\n";
    out += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    out += "Connection: ";
    out += *keepAlive ? "keep-alive" : "close";
    out += "\r\n";
    // The service only ever serves its own pages and JSON to its own origin, so
    // framing, MIME sniffing and referrer leakage are all safely refused.
    out += "X-Content-Type-Options: nosniff\r\n";
    out += "X-Frame-Options: DENY\r\n";
    out += "Referrer-Policy: same-origin\r\n";
    // Every page loads only its own scripts, styles, images and the vendored
    // three.js copy, and no page has an inline handler or inline script, so the
    // policy can stay this narrow. style-src needs 'unsafe-inline' because the
    // avatar preview sets colours through the style attribute.
    out += "Content-Security-Policy: default-src 'self'; script-src 'self'; "
           "style-src 'self' 'unsafe-inline'; img-src 'self' data: blob:; "
           "font-src 'self'; connect-src 'self'; worker-src 'self' blob:; "
           "object-src 'none'; base-uri 'none'; form-action 'self'; "
           "frame-ancestors 'none'\r\n";
    for (const auto& header : response.headers) {
        out += header.first + ": " + header.second + "\r\n";
    }
    out += "\r\n";
    out += body;

    return sendAll(handle, out);
}

void HttpServer::handleConnection(std::intptr_t socketHandle) {
    const SocketHandle handle = static_cast<SocketHandle>(socketHandle);

    // Browsers hold keep-alive sockets open, so cap the idle wait instead of
    // blocking the accept loop on a connection that never speaks again.
#ifdef _WIN32
    DWORD timeout = 1500;
    ::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout),
                 sizeof(timeout));
#else
    timeval timeout{};
    timeout.tv_sec = 1;
    timeout.tv_usec = 500000;
    ::setsockopt(handle, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif

    while (running_) {
        Request request;
        bool timedOut = false;
        if (!readRequest(socketHandle, &request, &timedOut)) {
            break;
        }

        Response response;
        if (request.headers.find("x-body-too-large") != request.headers.end()) {
            response.json(413, "{\"error\":\"Request body is too large.\"}");
        } else if (!dispatch(request, response)) {
            response.json(404, "{\"error\":\"Not found.\"}");
        }

        bool keepAlive = false;
        if (!writesResponse(socketHandle, request, response, &keepAlive)) {
            break;
        }
        if (!keepAlive) {
            break;
        }
    }

    closeSocket(handle);
}

bool HttpServer::serveOneConnection() {
    if (listenSocket_ < 0) {
        return false;
    }
    const SocketHandle handle =
        ::accept(static_cast<SocketHandle>(listenSocket_), nullptr, nullptr);
    if (handle == kInvalidSocket) {
        return false;
    }
    handleConnection(static_cast<std::intptr_t>(handle));
    return true;
}

void HttpServer::serveForever() {
    while (running_ && listenSocket_ >= 0) {
        const SocketHandle handle =
            ::accept(static_cast<SocketHandle>(listenSocket_), nullptr, nullptr);
        if (handle == kInvalidSocket) {
            if (!running_) {
                break;
            }
            continue;
        }

        std::thread worker(&HttpServer::handleConnection, this,
                           static_cast<std::intptr_t>(handle));
        worker.detach();
    }
}

std::string HttpServer::mimeTypeForPath(const std::string& path) {
    const std::size_t dot = path.find_last_of('.');
    std::string extension = dot == std::string::npos ? std::string() : path.substr(dot);
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (extension == ".html" || extension == ".htm") {
        return "text/html; charset=utf-8";
    }
    if (extension == ".css") {
        return "text/css; charset=utf-8";
    }
    if (extension == ".js" || extension == ".mjs") {
        return "text/javascript; charset=utf-8";
    }
    if (extension == ".json") {
        return "application/json; charset=utf-8";
    }
    if (extension == ".png") {
        return "image/png";
    }
    if (extension == ".jpg" || extension == ".jpeg") {
        return "image/jpeg";
    }
    if (extension == ".gif") {
        return "image/gif";
    }
    if (extension == ".svg") {
        return "image/svg+xml";
    }
    if (extension == ".ico") {
        return "image/x-icon";
    }
    if (extension == ".wasm") {
        return "application/wasm";
    }
    if (extension == ".glsl" || extension == ".txt" || extension == ".world") {
        return "text/plain; charset=utf-8";
    }
    return "application/octet-stream";
}

}  // namespace web
}  // namespace rblx
