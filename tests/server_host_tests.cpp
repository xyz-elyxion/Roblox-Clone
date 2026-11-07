// The supervisor's decisions, checked with a fake clock and a fake launcher, plus
// one real process on POSIX to prove the system launcher actually starts and
// stops something.

#include "ServerHost.h"

#include <cstdio>
#include <fstream>
#include <map>
#include <string>
#include <vector>

#ifndef _WIN32
    #include <sys/stat.h>
    #include <unistd.h>
#endif

namespace {

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

// Stands in for the operating system: records launches, answers whether a pid
// is alive from its own table, and records stops.
class FakeLauncher : public rblx::hosting::Launcher {
public:
    bool launch(const rblx::hosting::Config&, int port, const std::string& logPath, long* pid,
                std::string* error) override {
        ++launchCount;
        launchedPorts.push_back(port);
        launchedLogs.push_back(logPath);
        if (failLaunch) {
            *error = failReason.empty() ? std::string("no such file") : failReason;
            return false;
        }
        *pid = ++nextPid;
        alive_[*pid] = true;
        return true;
    }

    bool alive(long pid) override { return alive_[pid]; }

    void stop(long pid) override {
        stopped.push_back(pid);
        alive_[pid] = false;
    }

    // Kills a pid without the supervisor asking, which is what a crashed server
    // looks like from here.
    void kill(long pid) { alive_[pid] = false; }

    int launchCount = 0;
    std::vector<int> launchedPorts;
    std::vector<std::string> launchedLogs;
    std::vector<long> stopped;
    bool failLaunch = false;
    std::string failReason;

private:
    std::map<long, bool> alive_;
    long nextPid = 100;
};

rblx::hosting::Config smallConfig(int instances = 2) {
    rblx::hosting::Config config;
    config.instances = instances;
    config.firstPort = 7777;
    config.restartDelayMs = 1000;
    config.restartWindowMs = 60000;
    config.maxRestarts = 3;
    return config;
}

void testConfig() {
    rblx::hosting::Config config;
    std::string error;

    check(rblx::hosting::parseConfig("", &config, &error), "an empty file is the defaults");
    check(config.instances == 2 && config.firstPort == 7777 && config.server == "Server.exe",
          "the defaults are two servers from 7777");

    const std::string text =
        "# a comment\n"
        "; another\n"
        "server = bin/Server\n"
        "port = 9000\n"
        "instances = 4\n"
        "world = Arena.world\n"
        "max-restarts = 9\n"
        "unknown-key = whatever\n";
    check(rblx::hosting::parseConfig(text, &config, &error), "a full file is read");
    check(config.server == "bin/Server", "server is read");
    check(config.firstPort == 9000 && config.instances == 4, "port and instances are read");
    check(config.world == "Arena.world", "world is read");
    check(config.maxRestarts == 9, "max-restarts is read");
    check(config.restartWindowMs == 60000, "a missing key keeps its default");

    error.clear();
    check(!rblx::hosting::parseConfig("instances = two\n", &config, &error) &&
              contains(error, "instances"),
          "a word where a number belongs names the key");
    error.clear();
    check(!rblx::hosting::parseConfig("instances = 0\n", &config, &error), "zero instances is refused");
    error.clear();
    check(!rblx::hosting::parseConfig("instances = 999\n", &config, &error),
          "an absurd instance count is refused");
    error.clear();
    check(!rblx::hosting::parseConfig("port = 70000\n", &config, &error), "an out-of-range port is refused");
    error.clear();
    check(!rblx::hosting::parseConfig("just a line\n", &config, &error) &&
              contains(error, "line 1"),
          "a line that is not a pair is reported by line number");
    error.clear();
    check(!rblx::hosting::parseConfig("server = \n", &config, &error), "an empty server is refused");

    const std::string defaults = rblx::hosting::defaultConfigText();
    rblx::hosting::Config fromDefaults;
    error.clear();
    check(rblx::hosting::parseConfig(defaults, &fromDefaults, &error),
          "the file init writes parses back");
    check(fromDefaults.instances == 2 && fromDefaults.firstPort == 7777,
          "and it round-trips its values");
}

void testFirstStart() {
    FakeLauncher launcher;
    rblx::hosting::Supervisor supervisor(smallConfig(3), &launcher);

    supervisor.tick(0);
    check(launcher.launchCount == 3, "the first tick starts every slot");
    check(supervisor.runningCount() == 3, "all three report running");
    check(launcher.launchedPorts.size() == 3 && launcher.launchedPorts[0] == 7777 &&
              launcher.launchedPorts[1] == 7778 && launcher.launchedPorts[2] == 7779,
          "the ports count up from the first one");
    check(launcher.launchedLogs.size() == 3 &&
              launcher.launchedLogs[0] == "logs/host-7777.log",
          "each slot gets its own log path");
    // FakeLauncher hands out 101, 102, 103 for the three slots.
    check(supervisor.instances()[0].pid == 101 && supervisor.instances()[1].pid == 102,
          "the pid the launcher reported is kept, slot by slot");

    supervisor.tick(1000);
    check(launcher.launchCount == 3, "a running slot is not started twice");
}

void testRestart() {
    FakeLauncher launcher;
    rblx::hosting::Supervisor supervisor(smallConfig(1), &launcher);
    supervisor.tick(0);
    const long pid = supervisor.instances()[0].pid;

    launcher.kill(pid);
    supervisor.tick(1000);
    check(supervisor.instances()[0].state == rblx::hosting::Instance::State::Backoff,
          "a death puts the slot in backoff, not straight back up");
    check(launcher.launchCount == 1, "and it is not restarted the same instant");
    check(supervisor.instances()[0].restarts == 1, "the death counts as a restart");

    supervisor.tick(1999);
    check(launcher.launchCount == 1, "still waiting inside the delay");

    supervisor.tick(2000);
    check(launcher.launchCount == 2, "at the delay it is started again");
    check(supervisor.instances()[0].state == rblx::hosting::Instance::State::Running,
          "and reports running again");
}

void testBackoffGrows() {
    FakeLauncher launcher;
    rblx::hosting::Config config = smallConfig(1);
    config.maxRestarts = 10;
    rblx::hosting::Supervisor supervisor(config, &launcher);

    supervisor.tick(0);                 // first start
    launcher.kill(supervisor.instances()[0].pid);
    supervisor.tick(1000);              // failure 1: wait 1000
    check(supervisor.instances()[0].state == rblx::hosting::Instance::State::Backoff,
          "first failure backs off");
    supervisor.tick(2000);              // restarted
    check(launcher.launchCount == 2, "first restart happens after 1s");

    launcher.kill(supervisor.instances()[0].pid);
    supervisor.tick(3000);              // failure 2: wait 2000
    supervisor.tick(4500);
    check(launcher.launchCount == 2, "the second wait is longer than the first");
    supervisor.tick(5000);
    check(launcher.launchCount == 3, "and it does come back");
}

void testGivingUp() {
    FakeLauncher launcher;
    rblx::hosting::Config config = smallConfig(1);
    config.maxRestarts = 2;
    config.restartWindowMs = 60000;
    config.restartDelayMs = 100;
    rblx::hosting::Supervisor supervisor(config, &launcher);

    long long now = 0;
    supervisor.tick(now);
    for (int attempt = 0; attempt < 10 && supervisor.instances()[0].state !=
                                             rblx::hosting::Instance::State::Failed;
         ++attempt) {
        const long pid = supervisor.instances()[0].pid;
        if (pid != 0) launcher.kill(pid);
        now += 1000;
        supervisor.tick(now);
    }

    check(supervisor.instances()[0].state == rblx::hosting::Instance::State::Failed,
          "a slot that keeps dying is given up on");
    check(supervisor.allFailed(), "and the supervisor says every slot failed");
    const int launches = launcher.launchCount;
    now += 10000;
    supervisor.tick(now);
    check(launcher.launchCount == launches, "a failed slot is never started again");
    check(contains(supervisor.table(now), "failed"), "the table shows it as failed");
}

void testLaunchFailure() {
    FakeLauncher launcher;
    launcher.failLaunch = true;
    launcher.failReason = "no such file";
    rblx::hosting::Config config = smallConfig(1);
    config.maxRestarts = 1;
    config.restartDelayMs = 0;
    rblx::hosting::Supervisor supervisor(config, &launcher);

    supervisor.tick(0);
    check(supervisor.instances()[0].state == rblx::hosting::Instance::State::Backoff,
          "a server that will not start backs off instead of spinning");
    check(contains(supervisor.table(0), "no such file"), "the launch error is shown");
    supervisor.tick(10);
    check(supervisor.instances()[0].state == rblx::hosting::Instance::State::Failed,
          "and it is given up on once the budget is spent");
}

void testStopAll() {
    FakeLauncher launcher;
    rblx::hosting::Supervisor supervisor(smallConfig(2), &launcher);
    supervisor.tick(0);
    check(launcher.launchCount == 2, "two servers up");

    supervisor.stopAll();
    check(launcher.stopped.size() == 2, "stopping the supervisor stops both pids");
    check(supervisor.runningCount() == 0, "nothing is left running");
    supervisor.tick(5000);
    check(launcher.launchCount == 2, "a stopped supervisor starts nothing afterwards");
}

void testTable() {
    FakeLauncher launcher;
    rblx::hosting::Supervisor supervisor(smallConfig(2), &launcher);
    supervisor.tick(0);
    const std::string table = supervisor.table(65000);
    check(contains(table, "instance"), "the table has a header");
    check(contains(table, "7777"), "the table shows the ports");
    check(contains(table, "00:01:05"), "the table shows uptime against the clock it is given");
    check(contains(table, "2 of 2 running"), "the table summarises the running count");
}

#ifndef _WIN32
void testSystemLauncher() {
    // A stand-in server: a shell script that ignores its arguments and sleeps.
    // It proves the POSIX launcher starts, sees and stops a real child process.
    const std::string script = "/tmp/rblx_hosts_fake_server.sh";
    {
        std::ofstream file(script, std::ios::binary);
        file << "#!/bin/sh\nsleep 30\n";
    }
    ::chmod(script.c_str(), 0755);

    rblx::hosting::Config config;
    config.server = script;
    config.directory = ".";

    std::unique_ptr<rblx::hosting::Launcher> launcher = rblx::hosting::makeSystemLauncher();
    long pid = 0;
    std::string error;
    const bool started = launcher->launch(config, 7788, "/tmp/rblx_hosts_fake.log", &pid, &error);
    check(started && pid > 0, "the system launcher starts a real process");
    check(launcher->alive(pid), "and sees it running");

    launcher->stop(pid);
    check(!launcher->alive(pid), "and sees it gone after stop");
    std::remove(script.c_str());
    std::remove("/tmp/rblx_hosts_fake.log");
}
#endif

}  // namespace

int main() {
    std::printf("ServerHostTests\n");
    testConfig();
    testFirstStart();
    testRestart();
    testBackoffGrows();
    testGivingUp();
    testLaunchFailure();
    testStopAll();
    testTable();
#ifndef _WIN32
    testSystemLauncher();
#endif

    if (failures == 0) {
        std::printf("All server host checks passed.\n");
        return 0;
    }
    std::printf("%d check(s) failed.\n", failures);
    return 1;
}
