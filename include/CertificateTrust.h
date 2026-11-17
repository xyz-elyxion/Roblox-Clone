#pragma once

#include <string>
#include <vector>

// Windows keeps the certificates it trusts in stores: "ROOT" for certificate authorities
// whose signatures are accepted, "TrustedPublisher" for the publishers whose Authenticode
// signature Windows accepts without asking. Both are per-user or per-machine; the per-user
// ones need no elevation, which is why install defaults to them.

namespace linear::certificate {

// The stores an unsigned installer has to touch for a signed executable to stop warning.
struct StoreTarget {
    const char* name;  // "ROOT" or "TrustedPublisher"
};

const StoreTarget* targets(int* count);

// Reads a DER (or PEM) certificate file. Returns false and sets error when it cannot.
bool readFile(const std::string& path, std::vector<unsigned char>* der, std::string* error);

// Adds the certificate in `derPath` to every store, per-user or per-machine. `alreadyThere`
// counts the stores that already had it, so a second run can say so instead of claiming work.
bool install(const std::string& derPath, bool machine, int* alreadyThere, std::string* error);

// Removes it again, leaving the other certificates in those stores alone.
bool remove(const std::string& derPath, bool machine, int* removedCount, std::string* error);

// True when every store already trusts the certificate.
bool isTrusted(const std::string& derPath, bool machine, bool* trusted, std::string* error);

// Where an install looks for the certificate when the command line does not name one:
// beside the executable, in certs/, or one folder up from a program in a subfolder.
std::string defaultPath(const std::string& executablePath);

}  // namespace linear::certificate
