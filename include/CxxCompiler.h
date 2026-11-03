#pragma once

// RBLXCC - the C++ compiler that ships with this repository.
//
// The project is written in C++ and has always needed an outside toolchain to
// build it. This is our own compiler for the subset of C++ the project's own
// small tools are written in: it lexes, preprocesses, parses, type-checks and
// emits x86-64 assembly that an assembler and the system linker turn into a
// runnable program. It is deliberately a subset - no templates, classes,
// exceptions or floating point - and the README says exactly what it takes.
//
// The API is a pure function so the tests can drive the whole pipeline without
// touching the disk: hand it source, get assembly and diagnostics back.

#include <string>
#include <vector>

namespace rblx {
namespace cxx {

// One thing the compiler could not accept, in the shape editors expect.
struct Diagnostic {
    int line = 0;
    int column = 0;
    std::string message;
};

struct Options {
    // Only used to prefix diagnostics; the compiler never reads the file itself.
    std::string sourceName = "<input>";

    // Declare the handful of C library functions the examples print with
    // (printf, puts, putchar, exit, malloc, free, strlen, strcmp). A program
    // that declares them itself is unaffected, and turning this off makes the
    // compiler accept nothing it was not told about.
    bool includePrelude = true;

    // "_" on macOS, where the assembler prefixes every C symbol. Empty on
    // Linux and other ELF targets, where symbols are used as written.
    std::string symbolPrefix;

    // Put the source line above the instructions it produced. Off by default so
    // the assembly stays readable; the tests use it to pin instructions to code.
    bool annotate = false;

    // Call functions outside this file through the procedure linkage table, the
    // ELF convention. Mach-O has no PLT, so the driver turns this off on macOS.
    bool usePlt = true;
};

struct Result {
    bool ok = false;
    std::string assembly;

    // Empty when ok. Every entry has a line and a column, and the pipeline stops
    // at the first stage that rejects the program rather than emitting code for
    // something it could not understand.
    std::vector<Diagnostic> diagnostics;

    // "file:line:col: message", one per line - the format a terminal wants.
    std::string report() const;
};

// Compile one translation unit. Never throws: a broken program comes back as
// ok == false with diagnostics.
Result compile(const std::string& source, const Options& options = Options());

// The names the compiler reserves, for --help and for the README.
std::string supportedSummary();

}  // namespace cxx
}  // namespace rblx
