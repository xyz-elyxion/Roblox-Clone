// The chat room's rules and the store behind it, checked without a socket or a
// running website: what a channel list must contain, what a message may look
// like once it has been cleaned, and what the data file keeps.
//
// What matters here is what a reader depends on: channels that are stable and
// distinct, text that cannot carry control characters into the page, history
// that stays bounded without losing the newest messages, and a data file that
// comes back the same after a restart.

#include "ChatRoom.h"
#include "WebStore.h"

#include <cstdio>
#include <cstdlib>
#include <set>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::printf("FAIL: %s\n", what.c_str());
        ++failures;
    }
}

bool normalizesTo(const std::string& input, const std::string& expected) {
    std::string output;
    std::string error;
    if (!rblx::chat::normalizeBody(input, &output, &error)) {
        std::printf("FAIL: %s was refused: %s\n", input.c_str(), error.c_str());
        ++failures;
        return false;
    }
    if (output != expected) {
        std::printf("FAIL: %s normalized to \"%s\", expected \"%s\"\n", input.c_str(),
                    output.c_str(), expected.c_str());
        ++failures;
        return false;
    }
    return true;
}

void testChannels() {
    const std::vector<rblx::chat::Channel>& channels = rblx::chat::channels();
    check(!channels.empty(), "there is at least one channel");

    std::set<std::string> seen;
    for (const rblx::chat::Channel& channel : channels) {
        const std::string id = channel.id;
        check(!id.empty(), "a channel has an id");
        check(seen.insert(id).second, id + " is not a duplicate");
        check(std::string(channel.title) == id, id + " shows its own name");
        check(std::string(channel.topic).size() > 8, id + " has a topic to explain it");
        // The id travels in a query string and in a URL the page can link to, so
        // it is lowercase, dash-separated and nothing a browser would escape.
        for (const char c : id) {
            const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
            check(ok, id + " is URL-safe");
        }
        check(rblx::chat::findChannel(id) != nullptr, id + " is found by name");
    }

    check(rblx::chat::findChannel("") == nullptr, "an empty channel is not a channel");
    check(rblx::chat::findChannel("General") == nullptr, "channel names are case sensitive");
    check(rblx::chat::findChannel("nope") == nullptr, "an invented channel is refused");
    check(rblx::chat::channels().front().id == std::string("general"),
          "general is the channel a room opens on");
}

void testNormalize() {
    normalizesTo("hello", "hello");
    normalizesTo("  hello  ", "hello");
    normalizesTo("hello\r\nworld", "hello\nworld");
    normalizesTo("hello\n\n\n\nworld", "hello\n\nworld");
    normalizesTo("hello\tworld", "hello world");
    // A bell in a paste must not survive to be rendered by somebody's terminal
    // or copied into a page.
    normalizesTo("ding\x07" "dong", "dingdong");
    normalizesTo("\n\n  steps:\n  1. open\n  2. close\n\n", "steps:\n1. open\n2. close");
    normalizesTo("line one\nline two", "line one\nline two");
    normalizesTo("keep  inner  spaces", "keep  inner  spaces");

    std::string output;
    std::string error;
    check(!rblx::chat::normalizeBody("", &output, &error), "an empty message is refused");
    check(!error.empty(), "an empty message comes with a reason");
    check(!rblx::chat::normalizeBody(" \t\r\n ", &output, &error),
          "whitespace alone is not a message");

    const std::string limit(rblx::chat::kMaxMessageLength, 'x');
    check(rblx::chat::normalizeBody(limit, &output, &error),
          "a message at the limit is accepted");
    check(!rblx::chat::normalizeBody(limit + "x", &output, &error),
          "a message over the limit is refused");
    check(error.find(std::to_string(rblx::chat::kMaxMessageLength)) != std::string::npos,
          "the refusal names the limit");

    // Trimming must not quietly shorten something that only looked long.
    check(rblx::chat::normalizeBody(" " + limit + " ", &output, &error),
          "padding does not count against the limit");
}

struct Harness {
    rblx::web::Store store{"chat-tests-data.json"};
    int nextId = 1;
    long long nowMs = 1000;

    int post(int gameId, const std::string& channel, int userId, const std::string& text) {
        rblx::web::ChatMessageRecord record;
        record.gameId = gameId;
        record.channel = channel;
        record.userId = userId;
        record.username = "user" + std::to_string(userId);
        record.body = text;
        record.createdAt = "2026-01-01T00:00:00Z";
        record.atMs = nowMs++;
        const int id = store.addChatMessage(record);
        if (id > 0) {
            ++nextId;
        }
        return id;
    }
};

void testStore() {
    Harness harness;

    const int first = harness.post(1, "general", 1, "first");
    const int second = harness.post(1, "general", 2, "second");
    const int other = harness.post(1, "bug-reports", 1, "a report");
    const int elsewhere = harness.post(2, "general", 1, "another game");
    check(first > 0 && second > first, "ids rise with the message order");
    check(other > second && elsewhere > other, "one counter serves every room");
    check(harness.post(1, "invented", 1, "nowhere") == 0, "an unknown channel is refused");
    check(harness.post(0, "general", 1, "no game") == 0, "a message needs a game");
    check(harness.post(1, "general", 0, "no author") == 0, "a message needs an author");
    check(harness.post(1, "general", 1, "") == 0, "a message needs text");

    const auto general = harness.store.chatMessages(1, "general", 0, 100);
    check(general.size() == 2, "a channel returns only its own messages");
    check(general.size() == 2 && general[0]->body == "first" && general[1]->body == "second",
          "messages come back oldest first");

    const auto newer = harness.store.chatMessages(1, "general", first, 100);
    check(newer.size() == 1 && newer[0]->id == second, "afterId returns only what is newer");
    const auto nothing = harness.store.chatMessages(1, "general", second, 100);
    check(nothing.empty(), "afterId at the newest message returns nothing");

    const auto page = harness.store.chatMessages(1, "general", 0, 1);
    check(page.size() == 1 && page[0]->id == second, "a page is the newest messages");

    // Bounding one channel must leave the newest messages, not the first ones.
    for (int i = 0; i < static_cast<int>(rblx::chat::kMaxMessagesPerChannel) + 10; ++i) {
        harness.post(1, "general", 1, "filler " + std::to_string(i));
    }
    const auto trimmed = harness.store.chatMessages(1, "general", 0, 1000);
    check(trimmed.size() == rblx::chat::kMaxMessagesPerChannel,
          "a channel keeps its allowance of messages");
    check(!trimmed.empty() && trimmed.back()->body == "filler " +
                                                        std::to_string(
                                                            rblx::chat::kMaxMessagesPerChannel + 9),
          "the newest message survives the trim");
    check(!trimmed.empty() && trimmed.front()->body != "first", "the oldest messages go first");

    // The other channel and the other game are untouched by that trim.
    const auto report = harness.store.chatMessages(1, "bug-reports", 0, 100);
    check(report.size() == 1 && report[0]->id == other, "trimming one channel spares another");
    const auto otherGame = harness.store.chatMessages(2, "general", 0, 100);
    check(otherGame.size() == 1 && otherGame[0]->id == elsewhere,
          "trimming one game spares another");

    harness.store.removeChatMessagesForUser(2);
    const auto afterUser = harness.store.chatMessages(1, "general", 0, 1000);
    bool anyLeft = false;
    for (const rblx::web::ChatMessageRecord* message : afterUser) {
        anyLeft = anyLeft || message->userId == 2;
    }
    check(!anyLeft, "an account's messages go with the account");

    harness.store.removeChatMessagesForGame(2);
    check(harness.store.chatMessages(2, "general", 0, 100).empty(),
          "a game's rooms go with the game");
    check(harness.store.chatMessages(1, "general", 0, 1000).size() ==
              rblx::chat::kMaxMessagesPerChannel,
          "deleting one game spares the other");
}

void testUnread() {
    Harness harness;
    const int first = harness.post(1, "general", 1, "one");
    harness.post(1, "general", 2, "two");
    harness.post(1, "bug-reports", 2, "three");
    harness.post(2, "general", 1, "other room");

    check(harness.store.chatReadMark(2, 1, "general") == 0,
          "a channel nobody opened has no mark");
    check(harness.store.unreadChatMessages(2, 1, "general") == 2, "both messages are unread");
    check(harness.store.unreadChatMessagesInGame(2, 1) == 3, "a room's channels add up");
    check(harness.store.unreadChatMessages(2, 2, "general") == 1,
          "another room counts on its own");

    check(harness.store.setChatReadMark(2, 1, "general", first), "a mark advances");
    check(harness.store.chatReadMark(2, 1, "general") == first,
          "the mark keeps the newest id read");
    check(harness.store.unreadChatMessages(2, 1, "general") == 1, "what was read stops counting");
    check(!harness.store.setChatReadMark(2, 1, "general", first - 1),
          "a mark never moves backwards");
    check(!harness.store.setChatReadMark(2, 1, "general", first),
          "the same mark is not an advance");
    check(!harness.store.setChatReadMark(2, 1, "invented", first + 1),
          "an unknown channel has no mark");

    check(harness.store.setChatReadMark(2, 1, "bug-reports", first + 10),
          "a mark may pass everything a channel holds");
    check(harness.store.unreadChatMessages(2, 1, "bug-reports") == 0,
          "a mark past the end reads the channel");

    check(harness.store.unreadChatMessages(1, 1, "general") == 2,
          "another account's unread is untouched");

    // Deleting an account takes its messages and its own marks, and leaves the
    // rest of the room exactly as it was for everybody else.
    harness.store.removeChatMessagesForUser(2);
    check(harness.store.chatReadMark(2, 1, "general") == 0,
          "a deleted account takes its read marks with it");
    bool anyFromDeleted = false;
    for (const rblx::web::ChatMessageRecord* message :
         harness.store.chatMessages(1, "general", 0, 100)) {
        anyFromDeleted = anyFromDeleted || message->userId == 2;
    }
    check(!anyFromDeleted, "and its messages");
    check(harness.store.unreadChatMessages(1, 1, "general") == 1,
          "another account still sees the channel's messages as unread");

    harness.store.setChatReadMark(3, 1, "general", first);
    harness.store.removeChatMessagesForGame(1);
    check(harness.store.chatReadMark(3, 1, "general") == 0,
          "deleting a room clears the marks of it");
}

void testPersistence() {
    const std::string path = "chat-tests-data.json";
    std::remove(path.c_str());

    int lastId = 0;
    {
        Harness harness;
        harness.store.setPath(path);
        harness.post(1, "general", 1, "before the restart");
        harness.post(1, "sound-design", 2, "second line");
        lastId =        harness.post(2, "bug-reports", 1, "kept");
        check(harness.store.setChatReadMark(2, 1, "general", 1), "a read mark is set");

        std::string error;
        check(harness.store.save(&error), "the data file is written: " + error);
    }

    {
        rblx::web::Store reloaded{path};
        std::string error;
        check(reloaded.load(&error), "the data file is read back: " + error);
        check(reloaded.chatMessageCount() == 3, "every message comes back");

        const auto general = reloaded.chatMessages(1, "general", 0, 100);
        check(general.size() == 1 && general[0]->body == "before the restart",
              "a message keeps its text");
        check(general.size() == 1 && general[0]->username == "user1",
              "a message keeps its author's name");
        check(general.size() == 1 && general[0]->atMs == 1000,
              "a message keeps its time");
        check(reloaded.chatReadMark(2, 1, "general") == 1, "a read mark survives a reload");
        check(reloaded.unreadChatMessages(2, 1, "general") == 0,
              "and the reloaded room agrees it was read");

        // The id counter continues rather than restarting over live ids.
        rblx::web::ChatMessageRecord record;
        record.gameId = 1;
        record.channel = "general";
        record.userId = 1;
        record.username = "user1";
        record.body = "after the restart";
        record.createdAt = "2026-01-01T00:00:00Z";
        record.atMs = 2000;
        const int id = reloaded.addChatMessage(record);
        check(id == lastId + 1, "the id counter continues after a reload");
    }

    std::remove(path.c_str());
}

}  // namespace

int main() {
    testChannels();
    testNormalize();
    testStore();
    testUnread();
    testPersistence();

    if (failures != 0) {
        std::printf("chat_tests: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("chat_tests: all checks passed\n");
    return 0;
}
