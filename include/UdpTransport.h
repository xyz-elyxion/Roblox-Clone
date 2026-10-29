#pragma once

// A small datagram transport for the game protocol.
//
// The game streams position updates that only matter until the next one, so
// they are sent as plain datagrams: a lost packet costs nothing and nothing
// waits for a retransmit. Everything that must arrive (login, the world
// snapshot, chat, joins and leaves) rides a reliable channel with sequence
// numbers, acknowledgements, retransmission and fragmentation.
//
// This file does no socket I/O on purpose: the caller supplies a send function
// and feeds received datagrams in, which keeps the protocol testable without a
// network.

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <deque>
#include <functional>
#include <map>
#include <memory>
#include <random>
#include <vector>

namespace rblx {
namespace net {

// Stay below the smallest MTU we expect (1500 - 20 IP - 8 UDP). Relying on IP
// fragmentation loses the entire message when any one piece goes missing.
constexpr size_t kMaxDatagramSize = 1400;
constexpr size_t kMaxMessageSize = 1u << 20;
constexpr uint16_t kMaxFragments = 2048;

constexpr uint8_t kMagic0 = 'R';
constexpr uint8_t kMagic1 = 'B';
constexpr uint8_t kVersion = 1;

constexpr uint64_t kAckIntervalMs = 10;
constexpr uint64_t kKeepAliveIntervalMs = 500;
constexpr uint64_t kInitialRtoMs = 120;
constexpr uint64_t kMaxRtoMs = 600;
constexpr int kMaxSendAttempts = 24;
constexpr size_t kFragmentsPerPump = 48;
constexpr size_t kMaxPendingMessages = 128;
constexpr size_t kMaxPendingBytes = 8u << 20;
constexpr size_t kMaxIncompleteMessages = 64;
constexpr uint64_t kIncompleteTimeoutMs = 5000;

enum UdpFlags : uint8_t {
    kFlagAck = 1 << 0,
    kFlagReliable = 1 << 1,
    kFlagFragment = 1 << 2
};

#pragma pack(push, 1)
struct UdpHeader {
    uint8_t magic0;
    uint8_t magic1;
    uint8_t version;
    uint8_t flags;
    uint32_t connectionId;
    uint32_t sequence;
    uint32_t ack;
    uint32_t ackBits;
    uint16_t fragmentIndex;
    uint16_t fragmentCount;
};
#pragma pack(pop)

constexpr size_t kMaxFragmentPayload = kMaxDatagramSize - sizeof(UdpHeader);

// Sequence numbers wrap, so compare them as signed distances.
inline bool sequenceNewer(uint32_t candidate, uint32_t reference) {
    return static_cast<int32_t>(candidate - reference) > 0;
}

inline bool isAcknowledged(uint32_t ack, uint32_t ackBits, uint32_t sequence) {
    if (sequence == ack) {
        return true;
    }
    if (!sequenceNewer(ack, sequence)) {
        return false;
    }
    const uint32_t distance = ack - sequence;
    if (distance > 32) {
        return false;
    }
    return (ackBits & (1u << (distance - 1))) != 0;
}

class UdpSession {
public:
    using SendFunction = std::function<bool(const char* data, size_t size)>;

    explicit UdpSession(uint32_t connectionId = 0) { reset(connectionId); }

    void reset(uint32_t connectionId) {
        connectionId_ = connectionId != 0 ? connectionId : makeConnectionId();
        outgoing_.clear();
        pendingBytes_ = 0;
        nextReliableSequence_ = 1;
        nextUnreliableSequence_ = 1;
        hasReliable_ = false;
        highestReliable_ = 0;
        reliableBits_ = 0;
        hasUnreliable_ = false;
        highestUnreliable_ = 0;
        ackPending_ = false;
        lastAckSentAtMs_ = 0;
        lastSendAtMs_ = 0;
        lastReceiveAtMs_ = 0;
        incomplete_.clear();
    }

    static uint32_t makeConnectionId() {
        std::random_device device;
        uint32_t value = static_cast<uint32_t>(device()) ^
                         (static_cast<uint32_t>(device()) << 16);
        return value != 0 ? value : 1;
    }

    uint32_t connectionId() const { return connectionId_; }

    // Queues a message. Payloads larger than one datagram are always sent on
    // the reliable channel, because a half-received message is unusable.
    bool sendMessage(const void* data, size_t size, bool reliable) {
        if (!data && size > 0) {
            return false;
        }
        if (size > kMaxMessageSize) {
            return false;
        }
        if (outgoing_.size() >= kMaxPendingMessages) {
            return false;
        }
        if (pendingBytes_ + size > kMaxPendingBytes) {
            return false;
        }

        OutMessage message;
        const char* bytes = static_cast<const char*>(data);
        message.payload = std::make_shared<std::vector<char>>(bytes, bytes + size);
        message.fragmented = size > kMaxFragmentPayload;
        message.reliable = reliable || message.fragmented;
        message.fragmentCount = message.fragmented
                                    ? static_cast<uint16_t>((size + kMaxFragmentPayload - 1) / kMaxFragmentPayload)
                                    : 1;
        if (message.fragmentCount > kMaxFragments) {
            return false;
        }

        if (message.reliable) {
            message.sequence = nextReliableSequence_++;
            if (nextReliableSequence_ == 0) {
                nextReliableSequence_ = 1;
            }
        } else {
            message.sequence = nextUnreliableSequence_++;
            if (nextUnreliableSequence_ == 0) {
                nextUnreliableSequence_ = 1;
            }
        }

        pendingBytes_ += size;
        outgoing_.push_back(std::move(message));
        return true;
    }

    // Sends whatever is due: queued messages, a pending acknowledgement, and a
    // keep-alive when the session has been quiet.
    void pump(uint64_t nowMs, const SendFunction& send) {
        if (!send) {
            return;
        }
        if (lastSendAtMs_ == 0) {
            lastSendAtMs_ = nowMs;
        }
        if (lastReceiveAtMs_ == 0) {
            lastReceiveAtMs_ = nowMs;
        }

        // Acks go first: they stop the peer's retransmit timers.
        if (ackPending_ && nowMs - lastAckSentAtMs_ >= kAckIntervalMs) {
            if (transmitAck(send)) {
                ackPending_ = false;
                lastAckSentAtMs_ = nowMs;
            }
        }

        bool sentAnything = false;

        for (size_t index = 0; index < outgoing_.size(); ++index) {
            OutMessage& message = outgoing_[index];
            if (message.dead) {
                continue;
            }

            if (!message.reliable) {
                if (message.attempts >= kMaxSendAttempts) {
                    message.dead = true;
                    continue;
                }
                if (!transmit(message, 0, send)) {
                    ++message.attempts;
                    continue;
                }
                message.dead = true;
                sentAnything = true;
                continue;
            }

            if (message.fullySent && nowMs - message.lastSendAtMs < message.rto) {
                continue;
            }

            if (message.fullySent) {
                // The previous pass went unanswered: start again from the top.
                if (message.attempts >= kMaxSendAttempts) {
                    message.dead = true;
                    continue;
                }
                message.nextFragment = 0;
                message.fullySent = false;
                message.rto = std::min<uint64_t>(message.rto * 2, kMaxRtoMs);
                ++message.attempts;
            }

            size_t budget = kFragmentsPerPump;
            while (message.nextFragment < message.fragmentCount && budget > 0) {
                if (!transmit(message, message.nextFragment, send)) {
                    break;
                }
                ++message.nextFragment;
                --budget;
                sentAnything = true;
            }

            if (message.nextFragment >= message.fragmentCount) {
                message.fullySent = true;
                message.lastSendAtMs = nowMs;
            }
        }

        pruneIncomplete(nowMs);
        removeDead();

        if (sentAnything) {
            lastSendAtMs_ = nowMs;
        }

        if (nowMs - lastSendAtMs_ >= kKeepAliveIntervalMs) {
            if (transmitKeepAlive(send)) {
                lastSendAtMs_ = nowMs;
            }
        }
    }

    // Consumes one received datagram. Any frames it completed are appended to
    // `completed` in arrival order.
    bool receiveDatagram(const char* data, size_t size, uint64_t nowMs,
                         std::vector<std::vector<char>>& completed) {
        if (!data || size < sizeof(UdpHeader) || size > kMaxDatagramSize) {
            return false;
        }

        UdpHeader header;
        std::memcpy(&header, data, sizeof(header));
        if (header.magic0 != kMagic0 || header.magic1 != kMagic1 || header.version != kVersion) {
            return false;
        }
        if (header.fragmentCount == 0 || header.fragmentIndex >= header.fragmentCount) {
            return false;
        }

        lastReceiveAtMs_ = nowMs;

        if ((header.flags & kFlagAck) != 0) {
            applyAck(header.ack, header.ackBits);
            return true;
        }

        const char* payload = data + sizeof(UdpHeader);
        const size_t payloadSize = size - sizeof(UdpHeader);

        if ((header.flags & kFlagReliable) != 0) {
            noteReliableReceived(header.sequence);
            if ((header.flags & kFlagFragment) == 0) {
                completed.emplace_back(payload, payload + payloadSize);
            } else {
                addFragment(header, payload, payloadSize, nowMs, completed);
            }
            return true;
        }

        // Unreliable: an empty datagram is a keep-alive, and state that arrives
        // after newer state is already obsolete.
        if (payloadSize == 0) {
            return true;
        }
        if (!hasUnreliable_ || sequenceNewer(header.sequence, highestUnreliable_)) {
            highestUnreliable_ = header.sequence;
            hasUnreliable_ = true;
            completed.emplace_back(payload, payload + payloadSize);
        }
        return true;
    }

    bool timedOut(uint64_t nowMs, uint64_t timeoutMs) const {
        return lastReceiveAtMs_ != 0 && nowMs - lastReceiveAtMs_ > timeoutMs;
    }

    uint64_t lastReceiveAtMs() const { return lastReceiveAtMs_; }
    uint64_t lastSendAtMs() const { return lastSendAtMs_; }

    size_t pendingMessageCount() const {
        size_t count = 0;
        for (const OutMessage& message : outgoing_) {
            if (!message.dead) {
                ++count;
            }
        }
        return count;
    }

    size_t incompleteMessageCount() const { return incomplete_.size(); }

    uint64_t pendingBytes() const { return pendingBytes_; }

private:
    struct OutMessage {
        std::shared_ptr<const std::vector<char>> payload;
        uint32_t sequence = 0;
        bool reliable = false;
        bool fragmented = false;
        uint16_t fragmentCount = 1;
        uint16_t nextFragment = 0;
        bool fullySent = false;
        bool dead = false;
        uint64_t lastSendAtMs = 0;
        uint64_t rto = kInitialRtoMs;
        int attempts = 0;
    };

    struct Incomplete {
        std::vector<std::vector<char>> parts;
        std::vector<bool> have;
        uint32_t receivedParts = 0;
        uint64_t startedAtMs = 0;
    };

    bool transmit(const OutMessage& message, uint16_t fragmentIndex, const SendFunction& send) const {
        const size_t offset = static_cast<size_t>(fragmentIndex) * kMaxFragmentPayload;
        const size_t remaining = message.payload->size() - offset;
        const size_t chunk = std::min(kMaxFragmentPayload, remaining);

        uint8_t flags = message.reliable ? kFlagReliable : 0;
        if (message.fragmented) {
            flags |= kFlagFragment;
        }

        UdpHeader header;
        std::memset(&header, 0, sizeof(header));
        header.magic0 = kMagic0;
        header.magic1 = kMagic1;
        header.version = kVersion;
        header.flags = flags;
        header.connectionId = connectionId_;
        header.sequence = message.sequence;
        header.ack = hasReliable_ ? highestReliable_ : 0;
        header.ackBits = reliableBits_;
        header.fragmentIndex = fragmentIndex;
        header.fragmentCount = message.fragmentCount;

        std::vector<char> datagram(sizeof(UdpHeader) + chunk);
        std::memcpy(datagram.data(), &header, sizeof(header));
        if (chunk > 0) {
            std::memcpy(datagram.data() + sizeof(UdpHeader),
                        message.payload->data() + offset, chunk);
        }
        return send(datagram.data(), datagram.size());
    }

    bool transmitAck(const SendFunction& send) const {
        UdpHeader header;
        std::memset(&header, 0, sizeof(header));
        header.magic0 = kMagic0;
        header.magic1 = kMagic1;
        header.version = kVersion;
        header.flags = kFlagAck;
        header.connectionId = connectionId_;
        header.ack = hasReliable_ ? highestReliable_ : 0;
        header.ackBits = reliableBits_;
        header.fragmentCount = 1;

        char datagram[sizeof(UdpHeader)];
        std::memcpy(datagram, &header, sizeof(header));
        return send(datagram, sizeof(datagram));
    }

    bool transmitKeepAlive(const SendFunction& send) const {
        UdpHeader header;
        std::memset(&header, 0, sizeof(header));
        header.magic0 = kMagic0;
        header.magic1 = kMagic1;
        header.version = kVersion;
        header.flags = 0;
        header.connectionId = connectionId_;
        header.ack = hasReliable_ ? highestReliable_ : 0;
        header.ackBits = reliableBits_;
        header.fragmentCount = 1;

        char datagram[sizeof(UdpHeader)];
        std::memcpy(datagram, &header, sizeof(header));
        return send(datagram, sizeof(datagram));
    }

    void noteReliableReceived(uint32_t sequence) {
        if (!hasReliable_) {
            hasReliable_ = true;
            highestReliable_ = sequence;
            reliableBits_ = 0;
        } else if (sequenceNewer(sequence, highestReliable_)) {
            const uint32_t shift = sequence - highestReliable_;
            reliableBits_ = shift < 32 ? (reliableBits_ << shift) : 0;
            if (shift >= 1 && shift <= 32) {
                reliableBits_ |= (1u << (shift - 1));
            }
            highestReliable_ = sequence;
        } else if (sequence != highestReliable_) {
            const uint32_t distance = highestReliable_ - sequence;
            if (distance >= 1 && distance <= 32) {
                reliableBits_ |= (1u << (distance - 1));
            }
        }
        ackPending_ = true;
    }

    void applyAck(uint32_t ack, uint32_t ackBits) {
        if (ack == 0 && ackBits == 0) {
            return;
        }
        for (OutMessage& message : outgoing_) {
            if (message.dead || !message.reliable) {
                continue;
            }
            if (isAcknowledged(ack, ackBits, message.sequence)) {
                message.dead = true;
            }
        }
    }

    void addFragment(const UdpHeader& header, const char* payload, size_t payloadSize,
                     uint64_t nowMs, std::vector<std::vector<char>>& completed) {
        auto found = incomplete_.find(header.sequence);
        if (found == incomplete_.end()) {
            if (incomplete_.size() >= kMaxIncompleteMessages) {
                return;
            }
            Incomplete fresh;
            fresh.parts.resize(header.fragmentCount);
            fresh.have.assign(header.fragmentCount, false);
            fresh.startedAtMs = nowMs;
            found = incomplete_.emplace(header.sequence, std::move(fresh)).first;
        }

        Incomplete& entry = found->second;
        if (entry.parts.size() != header.fragmentCount) {
            incomplete_.erase(found);
            return;
        }

        if (!entry.have[header.fragmentIndex]) {
            entry.have[header.fragmentIndex] = true;
            entry.parts[header.fragmentIndex].assign(payload, payload + payloadSize);
            ++entry.receivedParts;
        }

        if (entry.receivedParts < header.fragmentCount) {
            return;
        }

        size_t total = 0;
        for (const std::vector<char>& part : entry.parts) {
            total += part.size();
        }
        if (total > kMaxMessageSize) {
            incomplete_.erase(found);
            return;
        }

        std::vector<char> assembled;
        assembled.reserve(total);
        for (const std::vector<char>& part : entry.parts) {
            assembled.insert(assembled.end(), part.begin(), part.end());
        }
        incomplete_.erase(found);
        completed.push_back(std::move(assembled));
    }

    void pruneIncomplete(uint64_t nowMs) {
        for (auto it = incomplete_.begin(); it != incomplete_.end();) {
            if (nowMs - it->second.startedAtMs > kIncompleteTimeoutMs) {
                it = incomplete_.erase(it);
            } else {
                ++it;
            }
        }
    }

    void removeDead() {
        for (auto it = outgoing_.begin(); it != outgoing_.end();) {
            if (!it->dead) {
                ++it;
                continue;
            }
            const size_t size = it->payload ? it->payload->size() : 0;
            pendingBytes_ -= std::min(pendingBytes_, size);
            it = outgoing_.erase(it);
        }
    }

    uint32_t connectionId_ = 0;
    std::deque<OutMessage> outgoing_;
    size_t pendingBytes_ = 0;
    uint32_t nextReliableSequence_ = 1;
    uint32_t nextUnreliableSequence_ = 1;
    bool hasReliable_ = false;
    uint32_t highestReliable_ = 0;
    uint32_t reliableBits_ = 0;
    bool hasUnreliable_ = false;
    uint32_t highestUnreliable_ = 0;
    bool ackPending_ = false;
    uint64_t lastAckSentAtMs_ = 0;
    uint64_t lastSendAtMs_ = 0;
    uint64_t lastReceiveAtMs_ = 0;
    std::map<uint32_t, Incomplete> incomplete_;
};

}  // namespace net
}  // namespace rblx
