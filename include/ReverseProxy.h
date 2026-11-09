#pragma once

#include <map>
#include <string>

#include "WebHttp.h"

namespace rblx {
namespace proxy {

// The backend every request is sent to, and how long one may take.
struct Upstream {
    std::string host = "127.0.0.1";
    int port = 3000;
    int timeoutMs = 5000;
};

// Reads "127.0.0.1:3000". A name is accepted as well as an address; a spec
// without a port is refused, because a default port here would quietly send
// traffic somewhere nobody asked for.
bool parseHostPort(const std::string& spec, std::string* host, int* port, std::string* error);

// A header that describes one connection rather than the message, and so is
// never passed on in either direction.
bool isHopByHop(const std::string& name);

// X-Forwarded-For: what a proxy in front of this one recorded, then the address
// this client actually connected from.
std::string forwardedFor(const std::string& existing, const std::string& address);

// The path and query as they go back on a request line.
std::string encodeTarget(const std::string& path, const std::string& query);

// One answer from the backend, after it has been read.
struct UpstreamResponse {
    int status = 0;
    std::string contentType;
    std::string body;
    // Lower-case names. A repeated Set-Cookie is folded into one value carrying
    // the extra lines, because that is the shape the server writes a map out in.
    std::map<std::string, std::string> headers;
};

// Sends one request to the backend and reads its whole answer. False, with a
// reason, when the backend cannot be reached or answers with something this
// proxy cannot relay.
bool forward(const Upstream& upstream, const web::Request& request,
             UpstreamResponse* response, std::string* error);

// The HttpServer handler: forward the request, then answer with what came back.
class ReverseProxy {
public:
    explicit ReverseProxy(Upstream upstream, bool logRequests = true);

    bool handle(web::Request& request, web::Response& response);

    const Upstream& upstream() const { return upstream_; }

private:
    Upstream upstream_;
    bool logRequests_ = true;
};

}  // namespace proxy
}  // namespace rblx
