#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace rblx {
namespace chat {

// A message longer than this is a paste, not a message. The website enforces it
// on the way in and the composer stops at it, so a refusal names the same limit
// the page shows.
const std::size_t kMaxMessageLength = 500;

// How much of a channel the data file keeps. History is bounded on purpose: the
// data file is the install's whole database, and a chat nobody ever trims would
// eventually be the largest thing in it.
const std::size_t kMaxMessagesPerChannel = 200;

// The same bound across every game and channel, so many quiet rooms still cannot
// grow the file without one busy room being cut back.
const std::size_t kMaxMessagesTotal = 4000;

// How many messages one reply carries, and how many servers the rail shows.
const std::size_t kMessagePageSize = 100;
const std::size_t kMaxServers = 12;

// How long somebody stays in a room's member list after their last poll or
// message, and how recently they have to have been seen to get the online dot.
const long long kMemberWindowMs = 5 * 60 * 1000;
const long long kOnlineWindowMs = 60 * 1000;

// How many members one room lists.
const std::size_t kMaxMembers = 50;

// One text channel. The list is fixed and lives on the server: a channel the
// page invented would have nowhere to store its messages, so the page is told
// what exists and renders that.
struct Channel {
    const char* id;
    const char* title;
    const char* topic;
};

const std::vector<Channel>& channels();

// Nullptr for a name that is not one of the channels above.
const Channel* findChannel(const std::string& id);

// Cleans one message and refuses what cannot be posted: control characters out,
// CRLF folded to one newline, runs of blank lines collapsed, every line trimmed,
// and the length capped. Returns false with a reason the page can show.
bool normalizeBody(const std::string& input, std::string* out, std::string* error);

}  // namespace chat
}  // namespace rblx
