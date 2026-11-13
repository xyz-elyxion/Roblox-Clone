// RBLXHub: a repository browser for the repository on this machine.
//
// The pages and the routing are HubSite, which WebApp also serves under /hub; this
// file is only the program around it - the command line, a loopback listener and
// the shutdown handler. There is no template engine, no JavaScript and no
// database: issues live in one JSON file beside the reflog, and the pages are HTML
// that a browser can read with scripting switched off.
//
//   RBLXHub                      the repository this folder is inside, 127.0.0.1:3100
//   RBLXHub --repo .. --port 8080 --host 0.0.0.0
//   RBLXHub --help

#include "HubSite.h"
#include "WebHttp.h"

#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>

namespace hub = rblx::hub;

namespace {

rblx::web::HttpServer* g_server = nullptr;

BOOL WINAPI consoleHandler(DWORD signal) {
    if (signal == CTRL_C_EVENT || signal == CTRL_BREAK_EVENT || signal == CTRL_CLOSE_EVENT) {
        if (g_server != nullptr) {
            g_server->stop();
        }
        return TRUE;
    }
    return FALSE;
}

std::string executableDirectory() {
    std::vector<char> buffer(1024);
    const DWORD length = ::GetModuleFileNameA(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (length == 0) {
        return std::string(".");
    }
    const std::string path(buffer.data(), length);
    const std::size_t slash = path.find_last_of("/\\");
    return slash == std::string::npos ? std::string(".") : path.substr(0, slash);
}

// web, found the way RBLXHosts finds its panel and WebApp finds the site: next to
// the executables first, then by walking up from them and from the folder this
// program was started in. The package keeps web/ beside the executables, so the
// first step usually wins.
std::string findWebDirectory() {
    std::vector<std::string> bases{executableDirectory(), std::string(".")};
    for (const std::string& base : bases) {
        std::string candidate = base;
        for (int depth = 0; depth < 5 && !candidate.empty(); ++depth) {
            for (const std::string& root : {candidate + "/web", candidate}) {
                std::ifstream probe(root + "/hub/hub.css");
                if (probe) {
                    return root;
                }
            }
            const std::size_t slash = candidate.find_last_of("/\\");
            if (slash == std::string::npos) {
                break;
            }
            candidate = candidate.substr(0, slash);
        }
    }
    return std::string();
}

}  // namespace

int main(int argc, char** argv) {
    std::string repoPath = ".";
    std::string host = "127.0.0.1";
    int port = 3100;

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
        if (argument == "--help" || argument == "-h" || argument == "/?") {
            std::printf(
                "RBLXHub - browse the git repository in this folder\n"
                "\n"
                "  RBLXHub [repository] [--host address] [--port number]\n"
                "\n"
                "Start the repository with RBLXGit init and commit something first; the\n"
                "hub shows the commits, the files at HEAD and the issues kept beside the\n"
                "repository. It listens on loopback only unless --host says otherwise.\n"
                "\n"
                "WebApp.exe serves the same pages under /hub for the machine it runs on.\n");
            return 0;
        }
        if (argument == "--host" && i + 1 < argc) {
            host = argv[++i];
        } else if (argument == "--port" && i + 1 < argc) {
            try {
                port = std::stoi(argv[++i]);
            } catch (...) {
                std::fprintf(stderr, "RBLXHub: --port needs a number\n");
                return 2;
            }
        } else if (argument.rfind("--", 0) == 0) {
            std::fprintf(stderr, "RBLXHub: unknown option %s (try --help)\n", argument.c_str());
            return 2;
        } else {
            repoPath = argument;
        }
    }
    if (port < 1 || port > 65535) {
        std::fprintf(stderr, "RBLXHub: --port must be between 1 and 65535\n");
        return 2;
    }

    std::string error;
    hub::HubSite site;
    // The pages are the whole site here, so they are mounted at the root.
    if (!site.open(repoPath, findWebDirectory(), std::string(), &error)) {
        std::fprintf(stderr, "RBLXHub: %s\n", error.c_str());
        std::fprintf(stderr, "RBLXHub: run RBLXGit init and make a commit first.\n");
        return 2;
    }

    rblx::web::HttpServer server;
    // A form post is a title and a few lines of text; anything larger is someone
    // pushing on the socket rather than using the page.
    server.setBodyLimitBytes(64 * 1024);

    server.setFallback([&site](rblx::web::Request& request, rblx::web::Response& response) {
        hub::HubPage page;
        site.route(request.method, request.path, request.body, &page);
        response.text(page.status, page.body, page.contentType);
        if (!page.location.empty()) {
            response.headers["Location"] = page.location;
        }
        // No scripts anywhere: the pages are static HTML, so the browser is told
        // that nothing but this origin's stylesheet may load or run.
        response.headers["Content-Security-Policy"] =
            "default-src 'none'; style-src 'self'; form-action 'self'";
        response.headers["X-Content-Type-Options"] = "nosniff";
        response.headers["Referrer-Policy"] = "no-referrer";
        return true;
    });

    if (!server.listen(host, port, &error)) {
        std::fprintf(stderr, "RBLXHub: %s\n", error.c_str());
        return 1;
    }

    g_server = &server;
    ::SetConsoleCtrlHandler(consoleHandler, TRUE);

    std::printf("RBLXHub: %s on http://%s:%d\n", site.name().c_str(), host.c_str(), server.port());
    std::printf("RBLXHub: Ctrl+C stops the server. Pages are rendered from the repository.\n");
    std::fflush(stdout);

    server.serveForever();

    g_server = nullptr;
    std::printf("RBLXHub: stopped.\n");
    return 0;
}
