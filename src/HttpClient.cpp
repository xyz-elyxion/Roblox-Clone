#include "HttpClient.h"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <vector>

#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif
#define SECURITY_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <wincrypt.h>
#include <security.h>
#include <schannel.h>
#pragma comment(lib, "ws2_32.lib")
// SChannel is Windows' own TLS, so https:// costs this project a link line
// rather than a vendored crypto library (see AGENTS.md rule 2).
#pragma comment(lib, "secur32.lib")
// CertVerifyCertificateChainPolicy, which checks the name on the certificate
// against the host that was typed.
#pragma comment(lib, "crypt32.lib")

using SocketHandle = SOCKET;
constexpr SocketHandle kInvalidSocket = INVALID_SOCKET;
#define LINEAR_CLOSE_SOCKET(s) closesocket(s)
#define LINEAR_LAST_ERROR() WSAGetLastError()

namespace linear {
namespace http {
namespace {

constexpr int kReadChunk = 16384;

// A TLS record carries at most 2^14 bytes of plaintext, plus its header and the
// cipher's trailer. A record longer than this is not something to try to read.
constexpr std::size_t kMaxTlsRecord = 5 + 16384 + 2048;

// A handshake that has not finished after this many bytes is not a handshake
// this client is going to finish, whatever the other end is doing.
constexpr std::size_t kMaxHandshakeBytes = 256 * 1024;

struct ParsedUrl {
    std::string host;
    std::string path;   // path plus query string
    uint16_t port = 80;
    bool secure = false;
};

void ensureSockets() {
    static std::once_flag once;
    std::call_once(once, [] {
        WSADATA data;
        WSAStartup(MAKEWORD(2, 2), &data);
    });
}

std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return value;
}

std::string hexCode(unsigned long code) {
    char text[16] = {};
    std::snprintf(text, sizeof(text), "0x%08lX", code);
    return text;
}

// The first bytes of an answer, in words. When a server answers a TLS
// ClientHello with a plain HTTP error - or an alert instead of a flight - the
// only useful thing to report is what actually arrived, not "something went
// wrong".
std::string describeHeard(const std::string& bytes) {
    if (bytes.empty()) return "nothing at all";
    const unsigned char first = static_cast<unsigned char>(bytes[0]);
    if (first == 0x15) return "a TLS alert instead of a handshake reply";
    if (first == 0x14) return "a TLS session change instead of a handshake reply";
    if (first == 0x16) return "a TLS handshake record";
    if (first == 0x17) return "encrypted data instead of a handshake reply";
    if (bytes.compare(0, 5, "HTTP/") == 0) return "a plain HTTP response";
    char text[32] = {};
    std::snprintf(text, sizeof(text), "the byte 0x%02X", first);
    return text;
}

std::wstring widen(const std::string& text) {
    const int length = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
    if (length <= 1) return std::wstring();
    std::wstring wide(static_cast<std::size_t>(length), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wide[0], length);
    wide.resize(static_cast<std::size_t>(length - 1));
    return wide;
}

// Accepts http://host[:port][/path] and https://host[:port][/path]. The port is
// the scheme's own when neither says otherwise.
bool parseUrl(const std::string& url, ParsedUrl& out, std::string* error) {
    const bool secure = url.compare(0, 8, "https://") == 0;
    if (!secure && url.compare(0, 7, "http://") != 0) {
        if (error) *error = "URL must start with http:// or https://.";
        return false;
    }

    const std::size_t hostBegin = secure ? 8 : 7;
    const std::size_t pathBegin = url.find('/', hostBegin);
    const std::string authority = url.substr(hostBegin,
                                             pathBegin == std::string::npos ? std::string::npos
                                                                            : pathBegin - hostBegin);
    out.secure = secure;
    out.port = secure ? 443 : 80;
    out.path = pathBegin == std::string::npos ? "/" : url.substr(pathBegin);
    if (authority.empty()) {
        if (error) *error = "URL has no host.";
        return false;
    }

    const std::size_t colon = authority.rfind(':');
    if (colon != std::string::npos && authority.find(']') == std::string::npos) {
        out.host = authority.substr(0, colon);
        const int port = std::atoi(authority.c_str() + colon + 1);
        if (port > 0 && port <= 65535) {
            out.port = static_cast<uint16_t>(port);
        }
    } else {
        out.host = authority;
    }

    if (out.host.empty()) {
        if (error) *error = "URL has no host.";
        return false;
    }
    return true;
}

SocketHandle connectToHost(const std::string& host, uint16_t port, int timeoutMs, std::string* error) {
    addrinfo hints{};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* results = nullptr;
    const std::string service = std::to_string(port);
    if (getaddrinfo(host.c_str(), service.c_str(), &hints, &results) != 0 || !results) {
        if (error) *error = "Could not resolve " + host + ".";
        return kInvalidSocket;
    }

    SocketHandle connected = kInvalidSocket;
    for (addrinfo* entry = results; entry != nullptr; entry = entry->ai_next) {
        SocketHandle candidate = ::socket(entry->ai_family, entry->ai_socktype, entry->ai_protocol);
        if (candidate == kInvalidSocket) {
            continue;
        }

        // Connect without blocking so the timeout is ours, not the stack's.
        unsigned long nonBlocking = 1;
        ioctlsocket(candidate, FIONBIO, &nonBlocking);

        const int rc = ::connect(candidate, entry->ai_addr, static_cast<int>(entry->ai_addrlen));
        bool ready = rc == 0;
        if (!ready) {
            const int lastError = LINEAR_LAST_ERROR();
            const bool inProgress =
                lastError == WSAEWOULDBLOCK;
            if (inProgress) {
                fd_set writable;
                FD_ZERO(&writable);
                FD_SET(candidate, &writable);
                timeval timeout{};
                timeout.tv_sec = timeoutMs / 1000;
                timeout.tv_usec = (timeoutMs % 1000) * 1000;
                ready = ::select(static_cast<int>(candidate) + 1, nullptr, &writable, nullptr, &timeout) > 0;
            }
        }

        if (!ready) {
            LINEAR_CLOSE_SOCKET(candidate);
            continue;
        }

        // Back to blocking, with read/write timeouts so a stalled peer returns
        // instead of hanging the caller.
        unsigned long blocking = 0;
        ioctlsocket(candidate, FIONBIO, &blocking);
        DWORD timeout = static_cast<DWORD>(timeoutMs);
        setsockopt(candidate, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
        setsockopt(candidate, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));

        connected = candidate;
        break;
    }

    freeaddrinfo(results);

    if (connected == kInvalidSocket && error) {
        *error = "Could not connect to " + host + ":" + service + ".";
    }
    return connected;
}

bool sendAll(SocketHandle socket, const std::string& data) {
    std::size_t sent = 0;
    while (sent < data.size()) {
        const int written = ::send(socket, data.data() + sent,
                                   static_cast<int>(data.size() - sent), 0);
        if (written <= 0) {
            return false;
        }
        sent += static_cast<std::size_t>(written);
    }
    return true;
}

// Content-Length from a block of headers, or -1 when the server did not say.
// Read before the body so a progress bar knows what it is moving towards (and
// so a chunked answer, which declares no length, reports -1).
long long declaredLength(const std::string& headers) {
    const std::string lowered = lower(headers);
    const std::size_t position = lowered.find("content-length:");
    if (position == std::string::npos) {
        return -1;
    }
    return std::atoll(headers.c_str() + position + std::strlen("content-length:"));
}

bool isChunked(const std::string& headers) {
    const std::string lowered = lower(headers);
    const std::size_t position = lowered.find("transfer-encoding:");
    if (position == std::string::npos) {
        return false;
    }
    std::size_t end = lowered.find("\r\n", position);
    if (end == std::string::npos) end = lowered.size();
    return lowered.substr(position, end - position).find("chunked") != std::string::npos;
}

// Removes the lengths from a chunked body. Real servers answer this way as soon
// as the body is generated rather than measured, so without this a page would
// carry its chunk sizes as text.
std::string decodeChunked(const std::string& body) {
    std::string out;
    std::size_t cursor = 0;
    while (cursor < body.size()) {
        const std::size_t lineEnd = body.find("\r\n", cursor);
        if (lineEnd == std::string::npos) break;
        std::string digits = body.substr(cursor, lineEnd - cursor);
        const std::size_t extension = digits.find(';');
        if (extension != std::string::npos) digits.resize(extension);
        const unsigned long long size = std::strtoull(digits.c_str(), nullptr, 16);
        if (size == 0) break;  // the last chunk, followed by optional trailers
        cursor = lineEnd + 2;
        if (cursor + static_cast<std::size_t>(size) > body.size()) {
            out.append(body, cursor, body.size() - cursor);
            break;
        }
        out.append(body, cursor, static_cast<std::size_t>(size));
        cursor += static_cast<std::size_t>(size);
        if (body.compare(cursor, 2, "\r\n") == 0) cursor += 2;
    }
    return out;
}

// How the bytes of one response travel. A plain socket is one implementation
// and TLS another, so the request below has a single read loop rather than two
// copies of one.
class Transport {
public:
    virtual ~Transport() = default;
    virtual bool send(const std::string& data, std::string* error) = 0;
    // >0 bytes read, 0 when the server closed, -1 on failure.
    virtual int receive(char* buffer, int capacity, std::string* error) = 0;
    // True while decrypted bytes are waiting, so a caller that polls a Cancel
    // button does not wait on the socket for data it already has.
    virtual bool hasBuffered() const { return false; }
    // 1 readable, 0 timed out, -1 failed.
    virtual int waitReadable(int timeoutMs) = 0;
};

class PlainTransport : public Transport {
public:
    explicit PlainTransport(SocketHandle socket) : socket_(socket) {}

    bool send(const std::string& data, std::string* error) override {
        if (!sendAll(socket_, data)) {
            if (error) *error = "Could not send the request.";
            return false;
        }
        return true;
    }

    int receive(char* buffer, int capacity, std::string* error) override {
        const int received = ::recv(socket_, buffer, capacity, 0);
        if (received < 0) {
            if (error) *error = "Could not read the response.";
            return -1;
        }
        return received;
    }

    int waitReadable(int timeoutMs) override {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(socket_, &readable);
        timeval wait{};
        wait.tv_sec = timeoutMs / 1000;
        wait.tv_usec = (timeoutMs % 1000) * 1000;
        const int ready =
            ::select(static_cast<int>(socket_) + 1, &readable, nullptr, nullptr, &wait);
        if (ready < 0) return -1;
        return ready == 0 ? 0 : 1;
    }

private:
    SocketHandle socket_;
};

// A TLS session over a connected socket. SChannel does the handshake and the
// record layer; the certificate it presents is checked against the machine's
// trust store and the host that was typed before a single byte of the response
// is read.
class TlsTransport : public Transport {
public:
    explicit TlsTransport(SocketHandle socket) : socket_(socket) {}
    ~TlsTransport() override {
        if (haveContext_) DeleteSecurityContext(&context_);
        if (haveCredentials_) FreeCredentialsHandle(&credentials_);
    }

    TlsTransport(const TlsTransport&) = delete;
    TlsTransport& operator=(const TlsTransport&) = delete;

    bool handshake(const std::string& host, std::string* error) {
        // SCHANNEL_CRED is the long-lived way to say "TLS 1.2, strong ciphers,
        // Windows validates the server": it needs no newer SDK than the CRT
        // already does, and every site worth reaching still speaks 1.2.
        SCHANNEL_CRED settings{};
        settings.dwVersion = SCHANNEL_CRED_VERSION;
        settings.grbitEnabledProtocols = SP_PROT_TLS1_2_CLIENT;
        settings.dwFlags = SCH_USE_STRONG_CRYPTO | SCH_CRED_AUTO_CRED_VALIDATION |
                           SCH_CRED_NO_DEFAULT_CREDS;
        settings.dwMinimumCipherStrength = 128;

        TimeStamp expiry{};
        SECURITY_STATUS status = AcquireCredentialsHandleW(
            nullptr, const_cast<SEC_WCHAR*>(UNISP_NAME_W), SECPKG_CRED_OUTBOUND, nullptr,
            &settings, nullptr, nullptr, &credentials_, &expiry);
        if (status != SEC_E_OK) {
            if (error) {
                *error = "Could not start TLS for " + host + " (" +
                         hexCode(static_cast<unsigned long>(status)) + ").";
            }
            return false;
        }
        haveCredentials_ = true;

        const std::wstring target = widen(host);
        // Extended errors are left out on purpose: without that flag a failure
        // comes back as the SEC_E_ value that says what went wrong (an
        // untrusted root, a name that does not match) instead of a request to
        // send one more alert and read again.

        // ISC_REQ_ALLOCATE_MEMORY is not optional here: the output buffer below
        // starts empty, so without it SChannel has nowhere to write the
        // ClientHello and answers SEC_E_INSUFFICIENT_MEMORY (0x80090300), which
        // reads like an out-of-memory failure and is not one. Every sample that
        // leaves pvBuffer NULL sets this flag, and the token it hands back is
        // released with FreeContextBuffer.
        const DWORD requested = ISC_REQ_STREAM | ISC_REQ_SEQUENCE_DETECT |
                                ISC_REQ_REPLAY_DETECT | ISC_REQ_CONFIDENTIALITY |
                                ISC_REQ_ALLOCATE_MEMORY;

        // The bytes the server has sent that SChannel has not consumed yet. A
        // server's half of a handshake is several records, and one read can stop
        // in the middle of them: SChannel then consumes what it can and hands the
        // rest back as a SECBUFFER_EXTRA buffer. Those bytes are the head of the
        // next token, so they are kept and the next read appends *after* them.
        // Dropping them - or replacing the buffer with a fresh read - feeds
        // SChannel a token that starts inside a record, which is what
        // SEC_E_INVALID_TOKEN (0x80090308, "the token supplied to the function is
        // invalid") reports.
        std::string pending;
        bool firstCall = true;
        while (true) {
            SecBuffer inBuffers[2] = {};
            inBuffers[0].BufferType = SECBUFFER_TOKEN;
            inBuffers[0].cbBuffer = static_cast<unsigned long>(pending.size());
            inBuffers[0].pvBuffer = pending.empty() ? nullptr : &pending[0];
            inBuffers[1].BufferType = SECBUFFER_EMPTY;
            SecBufferDesc inDescriptor{SECBUFFER_VERSION, 2, inBuffers};

            SecBuffer outBuffer{};
            outBuffer.BufferType = SECBUFFER_TOKEN;
            SecBufferDesc outDescriptor{SECBUFFER_VERSION, 1, &outBuffer};

            DWORD outFlags = 0;
            status = InitializeSecurityContextW(
                &credentials_, firstCall ? nullptr : &context_,
                const_cast<SEC_WCHAR*>(target.c_str()), requested, 0, 0,
                firstCall ? nullptr : &inDescriptor, 0,
                firstCall ? &context_ : nullptr, &outDescriptor, &outFlags, &expiry);
            // A failure before the first token leaves no context to delete, so
            // the handle only becomes the destructor's business once SChannel
            // has produced one.
            if (firstCall && (status == SEC_E_OK || status == SEC_I_CONTINUE_NEEDED ||
                              status == SEC_E_INCOMPLETE_MESSAGE)) {
                haveContext_ = true;
            }
            firstCall = false;

            // Schannel never asks for this, but a token that says it is only
            // half built is finished here rather than sent that way.
            if (status == SEC_I_COMPLETE_NEEDED || status == SEC_I_COMPLETE_AND_CONTINUE) {
                if (CompleteAuthToken(&context_, &outDescriptor) != SEC_E_OK) {
                    if (error) *error = describeHandshakeFailure(status, host, heard_);
                    return false;
                }
                status = status == SEC_I_COMPLETE_NEEDED ? SEC_E_OK : SEC_I_CONTINUE_NEEDED;
            }

            // Whatever was consumed is dropped; whatever was not is the head of
            // the next token.
            if (inBuffers[1].BufferType == SECBUFFER_EXTRA && inBuffers[1].cbBuffer > 0 &&
                inBuffers[1].cbBuffer <= pending.size()) {
                const std::size_t extra = inBuffers[1].cbBuffer;
                std::memmove(&pending[0], pending.data() + (pending.size() - extra), extra);
                pending.resize(extra);
            } else {
                pending.clear();
            }

            if (outBuffer.pvBuffer != nullptr) {
                bool written = true;
                if (outBuffer.cbBuffer > 0) {
                    written = rawSend(static_cast<const char*>(outBuffer.pvBuffer),
                                      outBuffer.cbBuffer);
                }
                FreeContextBuffer(outBuffer.pvBuffer);
                if (!written) {
                    if (error) *error = "Could not send the TLS handshake to " + host + ".";
                    return false;
                }
            }

            if (status == SEC_E_OK) break;

            if (status != SEC_I_CONTINUE_NEEDED && status != SEC_E_INCOMPLETE_MESSAGE) {
                if (error) *error = describeHandshakeFailure(status, host, heard_);
                return false;
            }

            if (pending.size() >= kMaxHandshakeBytes) {
                if (error) {
                    *error = host + " kept sending without agreeing to a secure connection (";
                    *error += describeHeard(heard_) + ").";
                }
                return false;
            }

            char raw[8192];
            const int received = rawReceive(raw, sizeof(raw));
            if (received <= 0) {
                if (error) *error = host + " did not answer the TLS handshake.";
                return false;
            }
            if (heard_.empty()) {
                heard_.assign(raw, static_cast<std::size_t>(std::min<int>(received, 16)));
            }
            pending.append(raw, static_cast<std::size_t>(received));
        }

        // What SChannel left over is the start of the server's answer.
        incoming_ = pending;

        status = QueryContextAttributesW(&context_, SECPKG_ATTR_STREAM_SIZES, &sizes_);
        if (status != SEC_E_OK) {
            if (error) {
                *error = "The secure connection to " + host +
                         " did not report its record sizes (" +
                         hexCode(static_cast<unsigned long>(status)) + ").";
            }
            return false;
        }
        return verifyCertificate(host, error);
    }

    bool send(const std::string& data, std::string* error) override {
        // One TLS record carries a bounded amount of plaintext, so a large body
        // (a POST, say) is split across records.
        const std::size_t maximum =
            std::max<std::size_t>(1024, static_cast<std::size_t>(sizes_.cbMaximumMessage));
        std::size_t offset = 0;
        while (offset < data.size()) {
            const std::size_t length = std::min(maximum, data.size() - offset);
            if (!sendRecord(data.data() + offset, length)) {
                if (error) *error = "Could not send the request.";
                return false;
            }
            offset += length;
        }
        return true;
    }

    int receive(char* buffer, int capacity, std::string* error) override {
        while (plain_.empty()) {
            if (closed_) return 0;
            if (!hasCompleteRecord()) {
                char raw[kReadChunk];
                const int received = rawReceive(raw, sizeof(raw));
                if (received < 0) {
                    if (error) *error = "Could not read the response.";
                    return -1;
                }
                if (received == 0) {
                    closed_ = true;
                    return 0;
                }
                incoming_.append(raw, static_cast<std::size_t>(received));
                continue;
            }
            if (!decryptRecord(error)) return -1;
        }

        const std::size_t take =
            std::min<std::size_t>(plain_.size(), static_cast<std::size_t>(capacity));
        std::memcpy(buffer, plain_.data(), take);
        plain_.erase(0, take);
        return static_cast<int>(take);
    }

    bool hasBuffered() const override { return !plain_.empty(); }

    int waitReadable(int timeoutMs) override {
        if (!plain_.empty()) return 1;
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(socket_, &readable);
        timeval wait{};
        wait.tv_sec = timeoutMs / 1000;
        wait.tv_usec = (timeoutMs % 1000) * 1000;
        const int ready =
            ::select(static_cast<int>(socket_) + 1, &readable, nullptr, nullptr, &wait);
        if (ready < 0) return -1;
        return ready == 0 ? 0 : 1;
    }

private:
    static std::string describeHandshakeFailure(SECURITY_STATUS status,
                                                const std::string& host,
                                                const std::string& heard) {
        const unsigned long code = static_cast<unsigned long>(status);
        switch (status) {
            case SEC_E_UNTRUSTED_ROOT:
                return "The certificate " + host + " presented is not trusted by this computer.";
            case SEC_E_WRONG_PRINCIPAL:
                return "The certificate " + host + " presented is for a different name.";
            case SEC_E_CERT_EXPIRED:
                return "The certificate " + host + " presented has expired.";
            case SEC_E_ILLEGAL_MESSAGE:
                return "The secure connection to " + host +
                       " was closed with an error (a TLS alert).";
            case SEC_E_ALGORITHM_MISMATCH:
                return "This computer and " + host + " share no cipher for a secure connection.";
            case SEC_E_INVALID_TOKEN:
                return host + " answered the TLS handshake with " + describeHeard(heard) +
                       ", which is not something a secure connection can be built on.";
            case SEC_E_TARGET_UNKNOWN:
                return "Could not reach a TLS service at " + host + ".";
            case SEC_I_INCOMPLETE_CREDENTIALS:
                return host +
                       " asked for a client certificate, which this browser does not send.";
            case SEC_E_NO_CREDENTIALS:
                return host +
                       " asked for a client certificate, which this browser does not send.";
            default:
                return "Could not establish a secure connection to " + host + " (" +
                       hexCode(code) + ").";
        }
    }

    bool verifyCertificate(const std::string& host, std::string* error) {
        // SChannel validated the chain as it completed (the credential does not
        // ask for manual validation), and this asks Windows the same question
        // about the name, with its own rules for wildcards and alternative
        // names, so a certificate for example.com cannot speak for
        // www.example.com.
        PCCERT_CONTEXT remote = nullptr;
        if (QueryContextAttributesW(&context_, SECPKG_ATTR_REMOTE_CERT_CONTEXT, &remote) !=
                SEC_E_OK ||
            remote == nullptr) {
            if (error) *error = "The secure connection to " + host + " presented no certificate.";
            return false;
        }

        const std::wstring wideHost = widen(host);
        HTTPSPolicyCallbackData https{};
        https.cbSize = sizeof(https);
        https.dwAuthType = AUTHTYPE_SERVER;
        https.pwszServerName = const_cast<wchar_t*>(wideHost.c_str());

        CERT_CHAIN_POLICY_PARA parameters{};
        parameters.cbSize = sizeof(parameters);
        parameters.pvExtraPolicyPara = &https;

        CERT_CHAIN_POLICY_STATUS status{};
        status.cbSize = sizeof(status);

        // The policy answers about a chain, not one certificate, so the chain
        // the machine would build for it is built first.
        CERT_CHAIN_PARA chainParameters{};
        chainParameters.cbSize = sizeof(chainParameters);
        PCCERT_CHAIN_CONTEXT chain = nullptr;
        const BOOL built = CertGetCertificateChain(nullptr, remote, nullptr, remote->hCertStore,
                                                   &chainParameters, 0, nullptr, &chain);
        bool verified = false;
        unsigned long problem = 0;
        if (built && chain != nullptr) {
            verified = CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_SSL, chain,
                                                        &parameters, &status) != FALSE;
            problem = static_cast<unsigned long>(status.dwError);
            CertFreeCertificateChain(chain);
        }
        CertFreeCertificateContext(remote);

        if (verified && problem == 0) return true;

        if (error) {
            const std::string code = hexCode(problem);
            if (!built || chain == nullptr) {
                *error = "Could not check the certificate " + host + " presented.";
            } else if (problem == static_cast<unsigned long>(CERT_E_CN_NO_MATCH)) {
                *error = "The certificate " + host + " presented is for a different name.";
            } else if (problem == static_cast<unsigned long>(CERT_E_EXPIRED)) {
                *error = "The certificate " + host + " presented has expired.";
            } else if (problem == static_cast<unsigned long>(CERT_E_UNTRUSTEDROOT)) {
                *error = "The certificate " + host + " presented is not trusted by this computer.";
            } else if (problem == 0) {
                *error = "The certificate " + host + " presented does not vouch for it.";
            } else {
                *error = "The certificate " + host + " presented does not vouch for it (" +
                         code + ").";
            }
        }
        return false;
    }

    bool rawSend(const char* data, std::size_t size) {
        std::size_t sent = 0;
        while (sent < size) {
            const int written = ::send(socket_, data + sent, static_cast<int>(size - sent), 0);
            if (written <= 0) return false;
            sent += static_cast<std::size_t>(written);
        }
        return true;
    }

    int rawReceive(char* buffer, int capacity) {
        const int received = ::recv(socket_, buffer, capacity, 0);
        return received < 0 ? -1 : received;
    }

    bool sendRecord(const char* data, std::size_t size) {
        const std::size_t header = sizes_.cbHeader;
        const std::size_t trailer = sizes_.cbTrailer;
        std::vector<char> record(header + size + trailer, 0);
        SecBuffer buffers[3] = {};
        buffers[0].BufferType = SECBUFFER_STREAM_HEADER;
        buffers[0].cbBuffer = static_cast<unsigned long>(header);
        buffers[0].pvBuffer = record.data();
        buffers[1].BufferType = SECBUFFER_DATA;
        buffers[1].cbBuffer = static_cast<unsigned long>(size);
        buffers[1].pvBuffer = record.data() + header;
        buffers[2].BufferType = SECBUFFER_STREAM_TRAILER;
        buffers[2].cbBuffer = static_cast<unsigned long>(trailer);
        buffers[2].pvBuffer = record.data() + header + size;
        std::memcpy(buffers[1].pvBuffer, data, size);

        SecBufferDesc descriptor{SECBUFFER_VERSION, 3, buffers};
        if (EncryptMessage(&context_, 0, &descriptor, 0) != SEC_E_OK) return false;
        return rawSend(record.data(), record.size());
    }

    // Whether the first record in incoming_ arrived whole. Reading record by
    // record keeps the decryption deterministic: SChannel is never handed a
    // partial record it could only answer with "incomplete message".
    bool hasCompleteRecord() const {
        if (incoming_.size() < 5) return false;
        const std::size_t length = (static_cast<unsigned char>(incoming_[3]) << 8) |
                                   static_cast<unsigned char>(incoming_[4]);
        return length > 0 && length < kMaxTlsRecord && incoming_.size() >= 5 + length;
    }

    bool decryptRecord(std::string* error) {
        const std::size_t length = (static_cast<unsigned char>(incoming_[3]) << 8) |
                                   static_cast<unsigned char>(incoming_[4]);
        std::vector<char> record(incoming_.begin(), incoming_.begin() + 5 + length);
        incoming_.erase(0, 5 + length);

        SecBuffer buffers[4] = {};
        buffers[0].BufferType = SECBUFFER_DATA;
        buffers[0].cbBuffer = static_cast<unsigned long>(record.size());
        buffers[0].pvBuffer = record.data();
        for (int index = 1; index < 4; ++index) buffers[index].BufferType = SECBUFFER_EMPTY;
        SecBufferDesc descriptor{SECBUFFER_VERSION, 4, buffers};

        const SECURITY_STATUS status = DecryptMessage(&context_, &descriptor, 0, nullptr);
        if (status == SEC_E_OK) {
            for (int index = 0; index < 4; ++index) {
                if (buffers[index].BufferType == SECBUFFER_DATA && buffers[index].cbBuffer > 0) {
                    plain_.append(static_cast<const char*>(buffers[index].pvBuffer),
                                  buffers[index].cbBuffer);
                } else if (buffers[index].BufferType == SECBUFFER_ALERT) {
                    closed_ = true;
                }
            }
            return true;
        }
        if (status == SEC_I_CONTEXT_EXPIRED) {
            // A close_notify: the server has said everything it is going to say.
            closed_ = true;
            return true;
        }
        if (status == SEC_I_RENEGOTIATE) {
            if (error) *error = "The server asked to renegotiate the secure connection.";
            return false;
        }
        if (error) {
            *error = "Could not read the secure response (" +
                     hexCode(static_cast<unsigned long>(status)) + ").";
        }
        return false;
    }

    SocketHandle socket_;
    CredHandle credentials_{};
    CtxtHandle context_{};
    bool haveCredentials_ = false;
    bool haveContext_ = false;
    SecPkgContext_StreamSizes sizes_{};
    std::string heard_;     // the first bytes the server sent, for error messages
    std::string incoming_;  // encrypted bytes not yet turned into a record
    std::string plain_;     // decrypted bytes not yet handed to the caller
    bool closed_ = false;
};

}  // namespace

Response request(const std::string& method,
                 const std::string& url,
                 const std::string& body,
                 const std::string& bearerToken,
                 int timeoutMs,
                 const ProgressCallback& onProgress,
                 const CancelCheck& cancelled,
                 const std::string& accept) {
    Response response;

    ParsedUrl parsed;
    if (!parseUrl(url, parsed, &response.error)) {
        return response;
    }

    ensureSockets();

    if (cancelled && cancelled()) {
        response.error = "cancelled";
        return response;
    }

    const SocketHandle socket = connectToHost(parsed.host, parsed.port, timeoutMs, &response.error);
    if (socket == kInvalidSocket) {
        return response;
    }

    std::unique_ptr<Transport> transport;
    if (parsed.secure) {
        std::unique_ptr<TlsTransport> tls(new TlsTransport(socket));
        if (!tls->handshake(parsed.host, &response.error)) {
            LINEAR_CLOSE_SOCKET(socket);
            return response;
        }
        transport = std::move(tls);
    } else {
        transport.reset(new PlainTransport(socket));
    }

    std::string head = method + " " + parsed.path + " HTTP/1.1\r\n";
    head += "Host: " + parsed.host;
    const uint16_t defaultPort = parsed.secure ? 443 : 80;
    if (parsed.port != defaultPort) {
        head += ":" + std::to_string(parsed.port);
    }
    head += "\r\n";
    head += "User-Agent: Linear Client/1.0\r\n";
    head += "Accept: " + (accept.empty() ? std::string("application/json") : accept) + "\r\n";
    // Nothing here inflates a compressed body, so the server is told not to
    // send one. Chunked bodies, on the other hand, are decoded below.
    head += "Accept-Encoding: identity\r\n";
    head += "Connection: close\r\n";
    if (!body.empty() || method == "POST" || method == "PUT") {
        head += "Content-Type: application/json\r\n";
        head += "Content-Length: " + std::to_string(body.size()) + "\r\n";
    }
    if (!bearerToken.empty()) {
        head += "Authorization: Bearer " + bearerToken + "\r\n";
    }
    head += "\r\n";

    if (!transport->send(head + body, &response.error)) {
        if (response.error.empty()) response.error = "Could not send the request to " + parsed.host + ".";
        transport.reset();
        LINEAR_CLOSE_SOCKET(socket);
        return response;
    }

    // The request asked for Connection: close, so reading to EOF is enough to
    // know the response is complete (a chunked body ends with a zero chunk).
    std::string raw;
    char buffer[4096];
    bool headersSeen = false;
    bool aborted = false;
    std::size_t bodyStart = 0;
    long long declared = -1;
    while (true) {
        // A caller with a Cancel button gets a socket that is waited on briefly
        // rather than blocked on, so the button is answered between chunks
        // instead of after the read timeout.
        if (cancelled) {
            if (cancelled()) {
                aborted = true;
                break;
            }
            if (!transport->hasBuffered()) {
                const int ready = transport->waitReadable(200);
                if (ready == 0) continue;
                if (ready < 0) break;
            }
        }

        const int received = transport->receive(buffer, static_cast<int>(sizeof(buffer)),
                                                &response.error);
        if (received < 0) break;
        if (received == 0) break;
        raw.append(buffer, static_cast<std::size_t>(received));

        // The length is worth reading as soon as the headers are complete, even
        // though the body is assembled below: a caller watching a download wants
        // a total to move towards.
        if (!headersSeen) {
            const std::size_t headerEnd = raw.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                headersSeen = true;
                bodyStart = headerEnd + 4;
                declared = declaredLength(raw.substr(0, headerEnd));
            }
        }
        if (onProgress) {
            const long long inBody =
                headersSeen ? static_cast<long long>(raw.size() - bodyStart) : 0;
            onProgress(inBody, declared);
        }

        if (raw.size() > 8 * 1024 * 1024) {
            break;  // a runaway response must not grow without bound
        }
    }
    // The TLS context has to go before the socket it sits on.
    transport.reset();
    LINEAR_CLOSE_SOCKET(socket);

    if (aborted) {
        // An abandoned download is not a server fault, so it says so plainly.
        // The caller decides whether a partial body is worth keeping; the
        // updater throws it away and leaves the installed copy alone.
        response.error = "cancelled";
        return response;
    }

    if (raw.empty()) {
        if (response.error.empty()) response.error = "No response from " + parsed.host + ".";
        return response;
    }

    const std::size_t headerEnd = raw.find("\r\n\r\n");
    if (headerEnd == std::string::npos) {
        response.error = "Malformed HTTP response.";
        return response;
    }

    const std::string headers = raw.substr(0, headerEnd);
    std::string payload = raw.substr(headerEnd + 4);

    // Status line: HTTP/1.1 200 OK
    const std::size_t firstSpace = headers.find(' ');
    if (firstSpace == std::string::npos) {
        response.error = "Malformed HTTP status line.";
        return response;
    }
    response.status = std::atoi(headers.c_str() + firstSpace + 1);

    if (isChunked(headers)) {
        payload = decodeChunked(payload);
    } else {
        // Honour Content-Length when present: with keep-alive headers a server
        // may leave the connection open, and the body would otherwise be
        // ambiguous.
        const std::string lowered = lower(headers);
        const std::size_t lengthHeader = lowered.find("content-length:");
        if (lengthHeader != std::string::npos) {
            const std::size_t valueBegin = lengthHeader + std::strlen("content-length:");
            const long long announced = std::atoll(headers.c_str() + valueBegin);
            if (announced >= 0 && static_cast<std::size_t>(announced) <= payload.size()) {
                payload.resize(static_cast<std::size_t>(announced));
            }
        }
    }

    response.body = std::move(payload);
    response.headers = headers;
    return response;
}

}  // namespace http
}  // namespace linear
