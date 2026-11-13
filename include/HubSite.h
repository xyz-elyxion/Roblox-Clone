#pragma once

#include "GitRepo.h"
#include "HubViews.h"

#include <mutex>
#include <string>
#include <vector>

namespace rblx {
namespace hub {

// One rendered answer: what a host has to write back to the socket. The pages are
// strings, so the routing below is shared by the standalone RBLXHub (pages at the
// root of its own server) and by WebApp (the same pages under /hub) instead of
// being written twice.
struct HubPage {
    int status = 200;
    std::string contentType = "text/html; charset=utf-8";
    std::string body;
    // Set by the issue form's redirect; empty for every other answer.
    std::string location;
};

// The repository a hub reads. It is opened once and every page is rebuilt per
// request, because the repository can change under a running server. Calls are
// serialised: the issues file is written here, so two posts must not interleave.
class HubSite {
public:
    // `repoPath` is any folder inside the repository. `webRoot` is the site's web
    // folder, where hub/hub.css is read from, or empty when it is not beside the
    // program. `mountPath` is "" for RBLXHub, and "/hub" for the pages WebApp
    // serves: every absolute link in them is built from it.
    bool open(const std::string& repoPath, const std::string& webRoot,
              const std::string& mountPath, std::string* error);

    bool ready() const { return ready_; }
    const std::string& name() const { return name_; }
    // The folder the repository was opened from, for the console line.
    const std::string& root() const { return root_; }

    // `path` is the hub's own path, without a mount prefix: "/", "/tree/src",
    // "/blob/README.md", "/commits", "/commit/<id>", "/issues", "/issues/new" or
    // "/hub.css".
    void route(const std::string& method, const std::string& path, const std::string& body,
               HubPage* page);

private:
    RepoSummary summary(const std::vector<IssueRow>& issues);

    git::Repo repo_;
    std::string name_;
    std::string root_;
    std::string issuesPath_;
    std::string css_;
    bool ready_ = false;
    std::mutex mutex_;
};

}  // namespace hub
}  // namespace rblx
