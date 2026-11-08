#include "ChatRoom.h"

#include <algorithm>

namespace rblx {
namespace chat {
namespace {

// Every game gets the same four channels. They are named after the work this
// project actually does, which is why a room starts out with somewhere to put a
// build report instead of only a #general.
const std::vector<Channel> kChannels = {
    {"general", "general", "Whatever the current build is doing."},
    {"world-building", "world-building", "Parts, spawns and the things that eat players."},
    {"bug-reports", "bug-reports",
     "One report per message: what you did, what happened, what you expected."},
    {"sound-design", "sound-design", "Synthesised sounds, one mixer, no audio files."},
};

}  // namespace

const std::vector<Channel>& channels() {
    return kChannels;
}

const Channel* findChannel(const std::string& id) {
    const auto entry = std::find_if(kChannels.begin(), kChannels.end(),
                                    [&](const Channel& channel) { return id == channel.id; });
    return entry == kChannels.end() ? nullptr : &*entry;
}

bool normalizeBody(const std::string& input, std::string* out, std::string* error) {
    std::string text;
    text.reserve(input.size());

    // A message is text. A control character is either a paste accident or an
    // attempt to redraw the page, so it never reaches the store. Newlines and
    // tabs survive because a bug report is written as steps.
    for (const char c : input) {
        if (c == '\r') {
            continue;
        }
        if (c == '\n') {
            text.push_back('\n');
            continue;
        }
        if (c == '\t') {
            text.push_back(' ');
            continue;
        }
        const unsigned char byte = static_cast<unsigned char>(c);
        if (byte < 0x20 || byte == 0x7f) {
            continue;
        }
        text.push_back(c);
    }

    // One blank line between paragraphs, never a run of them: a stack of empty
    // rows is not something anybody typed on purpose.
    std::string collapsed;
    collapsed.reserve(text.size());
    int newlines = 0;
    for (const char c : text) {
        if (c == '\n') {
            ++newlines;
            if (newlines > 2) {
                continue;
            }
        } else {
            newlines = 0;
        }
        collapsed.push_back(c);
    }

    // Trailing spaces each line are invisible in the page and would make two
    // messages that read the same compare as different text.
    std::string trimmed;
    trimmed.reserve(collapsed.size());
    std::size_t start = 0;
    while (start <= collapsed.size()) {
        std::size_t end = collapsed.find('\n', start);
        if (end == std::string::npos) {
            end = collapsed.size();
        }
        std::size_t lineStart = start;
        while (lineStart < end && (collapsed[lineStart] == ' ' || collapsed[lineStart] == '\t')) {
            ++lineStart;
        }
        std::size_t lineEnd = end;
        while (lineEnd > lineStart && (collapsed[lineEnd - 1] == ' ' ||
                                       collapsed[lineEnd - 1] == '\t')) {
            --lineEnd;
        }
        trimmed.append(collapsed, lineStart, lineEnd - lineStart);
        if (end == collapsed.size()) {
            break;
        }
        trimmed.push_back('\n');
        start = end + 1;
    }

    const std::size_t first = trimmed.find_first_not_of('\n');
    if (first == std::string::npos) {
        trimmed.clear();
    } else {
        const std::size_t last = trimmed.find_last_not_of('\n');
        trimmed = trimmed.substr(first, last - first + 1);
    }

    if (trimmed.empty()) {
        if (error) {
            *error = "A message needs some text.";
        }
        return false;
    }
    if (trimmed.size() > kMaxMessageLength) {
        if (error) {
            *error = "A message may be at most " + std::to_string(kMaxMessageLength) +
                     " characters.";
        }
        return false;
    }

    if (out) {
        *out = trimmed;
    }
    return true;
}

}  // namespace chat
}  // namespace rblx
