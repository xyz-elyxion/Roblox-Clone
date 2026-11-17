#include "CertificateTrust.h"

#include <windows.h>
#include <wincrypt.h>

#include <cstdio>
#include <fstream>
#include <iterator>

namespace linear::certificate {

namespace {

const StoreTarget kTargets[] = {
    {"ROOT"},
    {"TrustedPublisher"},
};

DWORD locationFor(bool machine) {
    return machine ? CERT_SYSTEM_STORE_LOCAL_MACHINE : CERT_SYSTEM_STORE_CURRENT_USER;
}

HCERTSTORE openStore(const StoreTarget& target, bool machine, std::string* error) {
    // The location is a flag on the open call; the store name is the parameter. Opening
    // per-user needs no elevation, which is why install defaults to it.
    std::wstring name;
    for (const char* p = target.name; *p != '\0'; ++p) {
        name.push_back(static_cast<wchar_t>(*p));
    }
    HCERTSTORE store = CertOpenStore(CERT_STORE_PROV_SYSTEM_W, 0, 0, locationFor(machine), name.c_str());
    if (store == nullptr) {
        if (error != nullptr) {
            *error = std::string("Could not open the ") + target.name + " store: " +
                     std::to_string(static_cast<unsigned long>(GetLastError()));
        }
    }
    return store;
}

PCCERT_CONTEXT contextFrom(const std::vector<unsigned char>& der) {
    return CertCreateCertificateContext(X509_ASN_ENCODING | PKCS_7_ASN_ENCODING, der.data(),
                                        static_cast<DWORD>(der.size()));
}

bool presentIn(HCERTSTORE store, PCCERT_CONTEXT wanted) {
    // Only the certificate body is compared, so a re-issued file with the same subject but a
    // different key is not mistaken for the one already there.
    PCCERT_CONTEXT found = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                      0, CERT_FIND_EXACT, wanted, nullptr);
    if (found == nullptr) {
        return false;
    }
    CertFreeCertificateContext(found);
    return true;
}

}  // namespace

const StoreTarget* targets(int* count) {
    if (count != nullptr) {
        *count = static_cast<int>(sizeof(kTargets) / sizeof(kTargets[0]));
    }
    return kTargets;
}

bool readFile(const std::string& path, std::vector<unsigned char>* der, std::string* error) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        if (error != nullptr) {
            *error = "Could not open " + path;
        }
        return false;
    }
    const std::string raw((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    if (raw.empty()) {
        if (error != nullptr) {
            *error = path + " is empty";
        }
        return false;
    }

    if (raw.find("-----BEGIN CERTIFICATE-----") != std::string::npos) {
        // A PEM file is base64 between the markers; CryptStringToBinary does the decoding so
        // there is no second base64 implementation to get wrong.
        DWORD size = 0;
        if (!CryptStringToBinaryA(raw.c_str(), static_cast<DWORD>(raw.size()), CRYPT_STRING_BASE64HEADER,
                                  nullptr, &size, nullptr, nullptr)) {
            if (error != nullptr) {
                *error = "Could not decode the PEM certificate in " + path;
            }
            return false;
        }
        der->assign(size, 0);
        if (!CryptStringToBinaryA(raw.c_str(), static_cast<DWORD>(raw.size()), CRYPT_STRING_BASE64HEADER,
                                  der->data(), &size, nullptr, nullptr)) {
            if (error != nullptr) {
                *error = "Could not decode the PEM certificate in " + path;
            }
            return false;
        }
        der->resize(size);
        return true;
    }

    der->assign(raw.begin(), raw.end());
    return true;
}

bool install(const std::string& derPath, bool machine, int* alreadyThere, std::string* error) {
    std::vector<unsigned char> der;
    if (!readFile(derPath, &der, error)) {
        return false;
    }

    bool okAll = true;
    int existing = 0;
    int count = 0;
    const StoreTarget* list = targets(&count);
    for (int i = 0; i < count; ++i) {
        HCERTSTORE store = openStore(list[i], machine, error);
        if (store == nullptr) {
            okAll = false;
            continue;
        }

        PCCERT_CONTEXT context = contextFrom(der);
        if (context == nullptr) {
            if (error != nullptr) {
                *error = std::string("Could not read the certificate in ") + derPath +
                         " (" + std::to_string(static_cast<unsigned long>(GetLastError())) + ")";
            }
            CertCloseStore(store, 0);
            okAll = false;
            continue;
        }

        if (presentIn(store, context)) {
            ++existing;
            CertFreeCertificateContext(context);
            CertCloseStore(store, 0);
            continue;
        }

        if (!CertAddCertificateContextToStore(store, context, CERT_STORE_ADD_REPLACE_EXISTING, nullptr)) {
            if (error != nullptr) {
                *error = std::string("Could not add the certificate to the ") + list[i].name +
                         " store: " + std::to_string(static_cast<unsigned long>(GetLastError()));
            }
            okAll = false;
        }
        CertFreeCertificateContext(context);
        CertCloseStore(store, 0);
    }

    if (alreadyThere != nullptr) {
        *alreadyThere = existing;
    }
    return okAll;
}

bool remove(const std::string& derPath, bool machine, int* removedCount, std::string* error) {
    std::vector<unsigned char> der;
    if (!readFile(derPath, &der, error)) {
        return false;
    }

    bool okAll = true;
    int removed = 0;
    int count = 0;
    const StoreTarget* list = targets(&count);
    for (int i = 0; i < count; ++i) {
        HCERTSTORE store = openStore(list[i], machine, error);
        if (store == nullptr) {
            okAll = false;
            continue;
        }

        PCCERT_CONTEXT context = contextFrom(der);
        if (context == nullptr) {
            okAll = false;
            CertCloseStore(store, 0);
            continue;
        }

        PCCERT_CONTEXT found = CertFindCertificateInStore(store, X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                                          0, CERT_FIND_EXACT, context, nullptr);
        CertFreeCertificateContext(context);
        if (found == nullptr) {
            CertCloseStore(store, 0);
            continue;
        }

        if (CertDeleteCertificateFromStore(found)) {
            ++removed;
        } else {
            if (error != nullptr) {
                *error = std::string("Could not remove the certificate from the ") + list[i].name +
                         " store: " + std::to_string(static_cast<unsigned long>(GetLastError()));
            }
            CertFreeCertificateContext(found);
            okAll = false;
        }
        CertCloseStore(store, 0);
    }

    if (removedCount != nullptr) {
        *removedCount = removed;
    }
    return okAll;
}

bool isTrusted(const std::string& derPath, bool machine, bool* trusted, std::string* error) {
    std::vector<unsigned char> der;
    if (!readFile(derPath, &der, error)) {
        return false;
    }

    bool everyStoreHasIt = true;
    int count = 0;
    const StoreTarget* list = targets(&count);
    for (int i = 0; i < count; ++i) {
        HCERTSTORE store = openStore(list[i], machine, error);
        if (store == nullptr) {
            return false;
        }
        PCCERT_CONTEXT context = contextFrom(der);
        if (context == nullptr) {
            CertCloseStore(store, 0);
            return false;
        }
        if (!presentIn(store, context)) {
            everyStoreHasIt = false;
        }
        CertFreeCertificateContext(context);
        CertCloseStore(store, 0);
    }

    if (trusted != nullptr) {
        *trusted = everyStoreHasIt;
    }
    return true;
}

std::string defaultPath(const std::string& executablePath) {
    std::string directory = executablePath;
    const std::size_t slash = directory.find_last_of("\\/");
    directory = slash == std::string::npos ? std::string(".") : directory.substr(0, slash);

    // A packaged folder has certs\ beside the programs; a build tree has it one level up.
    const std::string candidates[] = {
        directory + "\\certs\\linear-codesign.cer",
        directory + "\\linear-codesign.cer",
        directory + "\\..\\certs\\linear-codesign.cer",
    };
    for (const std::string& candidate : candidates) {
        std::ifstream file(candidate, std::ios::binary);
        if (file) {
            return candidate;
        }
    }
    return candidates[0];
}

}  // namespace linear::certificate
