#pragma once

#include <string>
#include <vector>

// The service tree a world has, in the order it is listed.
//
// The names live in one table because two places draw them: Studio's Explorer and
// the script's game:GetService. A service that exists in one and not the other is
// worse than one that does not exist at all, the way the guard's process table is
// shared for the same reason.
namespace linear {
namespace worldscript {

struct Service {
    const char* name;
    // What this build keeps in it, in one line: the Explorer shows it under the
    // name, and a script that asks for a service this build does not fill hears
    // the same sentence instead of a nil.
    const char* note;
};

const std::vector<Service>& services();

// The service with this name, or nullptr. Case-sensitive, like everything else a
// script names.
const Service* findService(const std::string& name);

}  // namespace worldscript
}  // namespace linear
