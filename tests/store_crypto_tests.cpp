// Checks for the sealed data file: AES-256-GCM itself, and the store's promise
// that nothing is decrypted until a request asks for that one account.
//
// The cipher is verified against the published NIST vector rather than against a
// value this code produced, so a mistake in the AES rounds, the GHASH multiply or
// the length block shows up as a mismatch instead of agreeing with itself.

#include "WebCrypto.h"
#include "WebStore.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

namespace {

int failures = 0;

void check(bool condition, const std::string& what) {
    if (!condition) {
        std::printf("FAIL %s\n", what.c_str());
        ++failures;
    }
}

std::string hexToBytes(const std::string& hex) {
    std::string out;
    out.reserve(hex.size() / 2);
    for (std::size_t i = 0; i + 1 < hex.size(); i += 2) {
        out.push_back(static_cast<char>(std::stoi(hex.substr(i, 2), nullptr, 16)));
    }
    return out;
}

std::string readAll(const std::string& path) {
    std::ifstream stream(path.c_str(), std::ios::binary);
    std::ostringstream buffer;
    buffer << stream.rdbuf();
    return buffer.str();
}

void writeAll(const std::string& path, const std::string& contents) {
    std::ofstream stream(path.c_str(), std::ios::binary | std::ios::trunc);
    stream << contents;
}

const char* kTempPath = "store_crypto_tests.json";

// Test case 16 of the GCM specification: AES-256, a 96-bit IV and associated
// data, so it covers the tag path as well as the keystream.
const char* kNistKey =
    "feffe9928665731c6d6a8f9467308308feffe9928665731c6d6a8f9467308308";
const char* kNistIv = "cafebabefacedbaddecaf888";
const char* kNistAad = "feedfacedeadbeeffeedfacedeadbeefabaddad2";
const char* kNistPlaintext =
    "d9313225f88406e5a55909c5aff5269a86a7a9531534f7da2e4c303d8a318a72"
    "1c3c0c95956809532fcf0e2449a6b525b16aedf5aa0de657ba637b39";
const char* kNistCiphertext =
    "522dc1f099567d07f47f37a32a84427d643a8cdcbfe5c0c97598a2bd2555d1aa"
    "8cb08e48590dbb3da7b08b1056828838c5f61e6393ba7a0abcc9f662";
const char* kNistTag = "76fc6ece0f4e1768cddf8853bb2d551b";

void testCipherAgainstNist() {
    using namespace rblx::web;

    const std::string key = hexToBytes(kNistKey);
    const std::string aad = hexToBytes(kNistAad);

    // Build the sealed form the store would have written with this nonce: the
    // published ciphertext and tag, so opening it exercises the whole recompute.
    const std::string payload = hexToBytes(kNistCiphertext) + hexToBytes(kNistTag);
    const std::string sealedValue =
        "aes256gcm." + base64UrlEncode(
                           reinterpret_cast<const std::uint8_t*>(hexToBytes(kNistIv).data()), 12) +
        "." + base64UrlEncode(reinterpret_cast<const std::uint8_t*>(payload.data()),
                              payload.size());

    std::string opened;
    check(aes256GcmOpen(key, sealedValue, aad, &opened), "the NIST vector opens");
    check(opened == hexToBytes(kNistPlaintext), "the NIST vector decrypts to its plaintext");

    check(!aes256GcmOpen(key, sealedValue, "different", &opened),
          "a different associated value fails the tag");
    check(!aes256GcmOpen(hexToBytes(std::string(kNistKey).substr(2) + "00"), sealedValue, aad,
                         &opened),
          "a different key fails the tag");

    std::string tampered = sealedValue;
    tampered[tampered.size() - 5] = (tampered[tampered.size() - 5] == 'A' ? 'B' : 'A');
    check(!aes256GcmOpen(key, tampered, aad, &opened), "a changed ciphertext fails the tag");
}

void testRoundTrip() {
    using namespace rblx::web;

    const std::string key(32, 'k');
    const std::string plaintext = "the quick brown fox jumps over the lazy dog";
    const std::string aad = "rblx-store-user:7";

    const std::string sealed = aes256GcmSeal(key, plaintext, aad);
    check(sealed.compare(0, 10, "aes256gcm.") == 0, "a sealed value names its cipher");

    std::string opened;
    check(aes256GcmOpen(key, sealed, aad, &opened), "a sealed value opens with its own key");
    check(opened == plaintext, "a sealed value round-trips");

    // Two seals of the same text must not match: that is the fresh nonce, and it
    // is what stops the file from revealing that two accounts share a password.
    check(aes256GcmSeal(key, plaintext, aad) != sealed, "each seal uses a fresh nonce");

    std::string empty;
    check(aes256GcmOpen(key, aes256GcmSeal(key, "", aad), aad, &empty) && empty.empty(),
          "an empty value round-trips");
    check(aes256GcmSeal("too short", plaintext, aad).empty(), "a short key is refused");
}

void testDataKey() {
    using namespace rblx::web;

    const std::string hex("0123456789abcdef0123456789abcdef0123456789abcdef0123456789abcdef");
    const std::string key = dataKeyFromText(hex);
    check(key.size() == 32, "64 hex characters become a 32-byte key");
    check(static_cast<unsigned char>(key[0]) == 0x01 && static_cast<unsigned char>(key[1]) == 0x23,
          "hex characters are read as bytes, not as text");

    const std::string passphrase = dataKeyFromText("a passphrase");
    check(passphrase.size() == 32, "text becomes a 32-byte key");
    check(passphrase == dataKeyFromText("a passphrase"), "the same text gives the same key");
    check(passphrase != dataKeyFromText("another passphrase"), "different text gives a different key");
}

void testStoreWithoutKey() {
    using namespace rblx::web;

    std::remove(kTempPath);
    {
        Store store(kTempPath);
        const int id = store.createUser("plainuser", "pbkdf2-sha256$looks-like-a-hash", "2026-01-01");
        check(id > 0, "a store without a key still creates accounts");
        std::string error;
        check(store.save(&error), "a store without a key still saves: " + error);
    }

    const std::string file = readAll(kTempPath);
    check(file.find("pbkdf2-sha256$looks-like-a-hash") != std::string::npos,
          "without a key the file keeps the old readable layout");
    check(file.find("rblx-store/1") != std::string::npos, "without a key the file is marked as v1");

    Store reloaded(kTempPath);
    std::string error;
    check(reloaded.load(&error), "a v1 file loads without a key: " + error);
    const UserRecord* user = reloaded.findUserByName("plainuser");
    check(user != nullptr && user->passwordHash == "pbkdf2-sha256$looks-like-a-hash",
          "a v1 file keeps its password hash readable");
}

void testSealedStore() {
    using namespace rblx::web;

    std::remove(kTempPath);
    const std::string key = dataKeyFromText(
        "00112233445566778899aabbccddeeff00112233445566778899aabbccddeeff");
    const std::string passwordHash = "pbkdf2-sha256$120000$secret-salt$secret-key";
    const std::string recoveryHash = "pbkdf2-sha256$120000$code-salt$code-key";
    const long long pendingAt = 1700000000000LL;

    {
        Store store(kTempPath);
        store.setDataKey(key);
        check(store.hasDataKey(), "the store takes a key");

        const int id = store.createUser("sealeduser", passwordHash, "2026-02-01");
        check(id > 0, "a sealed store creates accounts");
        store.setRecoveryHash(id, recoveryHash);
        store.addPendingDetection(id, "speed: 41 m/s over 20 ticks", "proof-blob", "expected-blob",
                                  "replay-blob", pendingAt);
        store.addInfraction(id, "warning", "speed: 41 m/s over 20 ticks", true, 1, "", "2026-02-02", 1,
                            "proof-blob", "expected-blob", "replay-blob");

        std::string error;
        check(store.save(&error), "a sealed store saves: " + error);
    }

    const std::string file = readAll(kTempPath);
    check(file.find("rblx-store/2") != std::string::npos, "the file names its format");
    check(file.find("aes-256-gcm") != std::string::npos, "the file names its cipher");
    check(file.find("aes256gcm.") != std::string::npos, "the account is sealed");
    check(file.find("sealed-salt") == std::string::npos, "the password hash is not in the file");
    check(file.find("secret-key") == std::string::npos, "nor is the derived key");
    check(file.find("code-key") == std::string::npos, "nor is the recovery hash");
    check(file.find("proof-blob") == std::string::npos, "nor is the detection evidence");
    check(file.find("sealeduser") != std::string::npos, "the account name stays readable, for lookup");

    {
        Store reloaded(kTempPath);
        reloaded.setDataKey(key);
        std::string error;
        check(reloaded.load(&error), "a sealed file loads with its key: " + error);

        const UserRecord* user = reloaded.findUserByName("sealeduser");
        check(user != nullptr, "the account is found by name after loading");
        check(user != nullptr && user->passwordHash == passwordHash,
              "opening the account brings its password hash back");
        check(user != nullptr && user->recoveryHash == recoveryHash,
              "and its recovery hash");
        check(user != nullptr && reloaded.pendingDetectionsFor(user->id).size() == 1,
              "and the detection queue");
        check(user != nullptr && reloaded.infractionsFor(user->id).size() == 1,
              "and the moderation record");

        const std::vector<std::pair<int, long long>> queued = reloaded.queuedDetections();
        check(queued.size() == 1 && queued[0].second == pendingAt,
              "the sweep finds a queued detection without opening every account");
    }

    // A record nothing asked for is written back untouched, so a save does not
    // decrypt the whole file to write it again.
    {
        Store store(kTempPath);
        store.setDataKey(key);
        std::string error;
        check(store.load(&error), "the file loads again: " + error);
        const int second = store.createUser("seconduser", "pbkdf2-sha256$second", "2026-02-03");
        check(second > 0, "a second account is created");
        check(store.save(&error), "the store saves with one account open and one sealed: " + error);
    }
    {
        const std::string saved = readAll(kTempPath);
        check(saved.find("pbkdf2-sha256$second") == std::string::npos,
              "the account created while saving is sealed too");

        Store reloaded(kTempPath);
        reloaded.setDataKey(key);
        std::string error;
        check(reloaded.load(&error), "the file loads after a mixed save: " + error);
        const UserRecord* first = reloaded.findUserByName("sealeduser");
        const UserRecord* second = reloaded.findUserByName("seconduser");
        check(first != nullptr && first->passwordHash == passwordHash,
              "the record that was never opened this run still opens");
        check(second != nullptr && second->passwordHash == "pbkdf2-sha256$second",
              "the newly written record opens");
    }

    // Without the key the file is refused rather than read as empty accounts.
    {
        Store reloaded(kTempPath);
        std::string error;
        check(!reloaded.load(&error), "a sealed file is refused without a key");
        check(error.find("DATA_KEY") != std::string::npos, "the refusal says what to set: " + error);
    }

    // A changed byte in the blob is a record that will not open: the account is
    // still listed, and the hash is simply absent, which denies sign-in.
    {
        std::string contents = readAll(kTempPath);
        const std::size_t start = contents.find("aes256gcm.");
        check(start != std::string::npos, "the sealed value is in the file");
        const std::size_t quote = contents.find('"', start);
        contents[quote - 4] = (contents[quote - 4] == 'A' ? 'B' : 'A');
        writeAll(kTempPath, contents);

        Store reloaded(kTempPath);
        reloaded.setDataKey(key);
        std::string error;
        check(reloaded.load(&error), "a damaged record does not stop the file loading: " + error);
        const UserRecord* user = reloaded.findUserByName("sealeduser");
        check(user != nullptr, "the damaged account is still listed");
        check(user != nullptr && user->passwordHash.empty(),
              "a damaged record opens as empty rather than as something else");
    }

    std::remove(kTempPath);
}

}  // namespace

int main() {
    testCipherAgainstNist();
    testRoundTrip();
    testDataKey();
    testStoreWithoutKey();
    testSealedStore();

    if (failures == 0) {
        std::printf("store crypto: all checks passed\n");
        return 0;
    }
    std::printf("store crypto: %d check(s) failed\n", failures);
    return 1;
}
