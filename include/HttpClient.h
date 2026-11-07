#pragma once

#include <functional>
#include <string>

// A small blocking HTTP/1.1 client used by the client and game server to talk
// to the website backend (login, token verification, avatars, playtime and
// instance heartbeats).
//
// It replaces the WinHTTP code both binaries carried, so the same code runs on
// Windows and on POSIX. Only plain HTTP is supported: the backend is a local
// development server (http://localhost:3000, or 127.0.0.1 inside a game
// server), there is no TLS stack in this project, and a request to an https://
// URL fails with an explanatory error instead of silently returning nothing.
namespace rblx {
namespace http {

struct Response {
    int status = 0;          // HTTP status, or 0 when the request never completed
    std::string body;        // response body, empty on failure
    std::string error;       // why the request failed, empty on success

    // A 2xx response that actually arrived.
    bool ok() const { return status >= 200 && status < 300; }
    bool networkError() const { return status == 0; }
};

// Called as a response body arrives: the bytes read so far, and the length the
// server declared (negative when it declared none). A caller that draws a
// progress bar passes one; everything else ignores it.
using ProgressCallback = std::function<void(long long received, long long total)>;

// Asked while a request is in flight, so a Cancel button is answered in a
// fraction of a second instead of when the read timeout expires.
using CancelCheck = std::function<bool()>;

// Sends one request and waits for the response. `timeoutMs` bounds the
// connect, each write and each read, so a hung server cannot block a caller
// forever. A `cancelled` check makes the wait interruptible: the request returns
// at once with the error "cancelled" rather than reading out its timeout.
Response request(const std::string& method,
                 const std::string& url,
                 const std::string& body = std::string(),
                 const std::string& bearerToken = std::string(),
                 int timeoutMs = 5000,
                 const ProgressCallback& onProgress = ProgressCallback(),
                 const CancelCheck& cancelled = CancelCheck());

}  // namespace http
}  // namespace rblx
