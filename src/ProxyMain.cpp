// RBLXProxy: a front door for the website.
//
// WebApp answers on 127.0.0.1:3000, which is not an address anyone outside this
// machine can type. This program listens where they can - port 80, a LAN
// address, whatever a router forwards - and sends each request on to WebApp,
// handing the answer back unchanged. The forwarding rules live in
// ReverseProxy.cpp and are covered by ReverseProxyTests; this file is the
// console around them.
//
//   RBLXProxy                                  127.0.0.1:8080 -> 127.0.0.1:3000
//   RBLXProxy --listen 0.0.0.0:80 --upstream 127.0.0.1:3000
//   RBLXProxy --help

#include "ReverseProxy.h"
#include "WebHttp.h"

#include <atomic>
#include <cstdio>
#include <string>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace {

rblx::web::HttpServer* g_server = nullptr;

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        // Closing the listening socket is what makes the accept loop return, so
        // the worker threads finish and the process exits cleanly.
        if (g_server != nullptr) {
            g_server->stop();
        }
        return TRUE;
    }
    return FALSE;
}

void printUsage(const char* program) {
    std::printf(
        "RBLXProxy - puts the website on an address other machines can reach\n"
        "\n"
        "  %s [options]\n"
        "\n"
        "Options:\n"
        "  --listen <host:port>    where browsers connect (default 127.0.0.1:8080)\n"
        "  --upstream <host:port>  where WebApp listens (default 127.0.0.1:3000)\n"
        "  --timeout <ms>          how long one backend request may take (default 5000)\n"
        "  --quiet                 print only failures\n"
        "  --help                  this text\n"
        "\n"
        "Each request is sent to the backend with the same method, path, query and\n"
        "body, plus X-Forwarded-For, X-Forwarded-Host and X-Forwarded-Proto, and the\n"
        "answer comes back with its status, headers and body.\n"
        "\n"
        "The listener is loopback by default: pass --listen 0.0.0.0:80 to serve other\n"
        "machines, and remember that the site then answers whoever can reach that port.\n"
        "WebApp sees every request as coming from this proxy, so its per-address limits\n"
        "and logs count the proxy rather than the browser.\n",
        program);
}

}  // namespace

int main(int argc, char** argv) {
    std::string listenSpec = "127.0.0.1:8080";
    std::string upstreamSpec = "127.0.0.1:3000";
    int timeoutMs = 5000;
    bool quiet = false;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--help" || argument == "-h" || argument == "/?") {
            printUsage(argv[0]);
            return 0;
        }
        const bool hasValue = i + 1 < argc;
        if (argument == "--listen" && hasValue) {
            listenSpec = argv[++i];
        } else if (argument == "--upstream" && hasValue) {
            upstreamSpec = argv[++i];
        } else if (argument == "--timeout" && hasValue) {
            try {
                timeoutMs = std::stoi(argv[++i]);
            } catch (...) {
                std::fprintf(stderr, "RBLXProxy: --timeout needs a number of milliseconds\n");
                return 2;
            }
            if (timeoutMs < 100 || timeoutMs > 600000) {
                std::fprintf(stderr, "RBLXProxy: --timeout must be between 100 and 600000\n");
                return 2;
            }
        } else if (argument == "--quiet") {
            quiet = true;
        } else {
            std::fprintf(stderr, "RBLXProxy: unknown option %s (try --help)\n", argument.c_str());
            return 2;
        }
    }

    rblx::proxy::Upstream upstream;
    upstream.timeoutMs = timeoutMs;
    std::string error;
    if (!rblx::proxy::parseHostPort(upstreamSpec, &upstream.host, &upstream.port, &error)) {
        std::fprintf(stderr, "RBLXProxy: --upstream: %s\n", error.c_str());
        return 2;
    }

    std::string listenHost;
    int listenPort = 0;
    if (!rblx::proxy::parseHostPort(listenSpec, &listenHost, &listenPort, &error)) {
        std::fprintf(stderr, "RBLXProxy: --listen: %s\n", error.c_str());
        return 2;
    }

    rblx::web::HttpServer server;
    rblx::proxy::ReverseProxy front(upstream, !quiet);
    server.setFallback(
        [&front](rblx::web::Request& request, rblx::web::Response& response) {
            return front.handle(request, response);
        });

    if (!server.listen(listenHost, listenPort, &error)) {
        std::fprintf(stderr, "RBLXProxy: %s\n", error.c_str());
        return 1;
    }

    g_server = &server;
    ::SetConsoleCtrlHandler(consoleHandler, TRUE);

    std::printf("RBLXProxy: http://%s:%d -> http://%s:%d\n", listenHost.c_str(), server.port(),
                upstream.host.c_str(), upstream.port);
    std::printf("RBLXProxy: start WebApp.exe on the backend port; Ctrl+C stops the proxy.\n");
    std::fflush(stdout);

    server.serveForever();

    g_server = nullptr;
    std::printf("RBLXProxy: stopped.\n");
    return 0;
}
