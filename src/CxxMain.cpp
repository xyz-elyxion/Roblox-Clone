// RBLXCC - the command line front end.
//
// The compiler itself is a library: this file only reads the source, prints the
// diagnostics in the shape a terminal wants, and hands the assembly to the
// system assembler and linker (through `cc`, or $RBLX_CC) to make a program.

#include "CxxCompiler.h"

#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* kUsage =
    "RBLXCC - the C++ compiler that ships with RBLX Clone\n"
    "\n"
    "Usage: RBLXCC <file.cpp> [options]\n"
    "\n"
    "Options:\n"
    "  -o <path>     write the program (or the assembly with -S) to <path>\n"
    "  -S            stop after code generation and write assembly\n"
    "  -c            stop after assembling and write an object file\n"
    "  --keep-asm    keep the generated .s file beside the output\n"
    "  --no-prelude  do not declare printf and the other C library functions\n"
    "  --prefix <s>  prefix every symbol with <s> (\"_\" on macOS; guessed here)\n"
    "  -h, --help    this text\n"
    "\n"
    "The assembly is assembled and linked with the C compiler named by RBLX_CC\n"
    "(cc by default), which supplies the C runtime that calls main.\n";

bool readFile(const std::string& path, std::string& contents) {
    std::ifstream file(path, std::ios::binary);
    if (!file) return false;
    std::ostringstream buffer;
    buffer << file.rdbuf();
    contents = buffer.str();
    return true;
}

bool writeFile(const std::string& path, const std::string& contents) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) return false;
    file << contents;
    return static_cast<bool>(file);
}

std::string stemOf(const std::string& path) {
    std::string name = path;
    std::size_t slash = name.find_last_of("/\\");
    if (slash != std::string::npos) name = name.substr(slash + 1);
    std::size_t dot = name.find_last_of('.');
    if (dot != std::string::npos && dot > 0) name = name.substr(0, dot);
    return name;
}

std::string quote(const std::string& path) { return "\"" + path + "\""; }

int run(const std::string& command) {
    int status = std::system(command.c_str());
    return status == 0 ? 0 : 1;
}

std::string hostCompiler() {
    const char* configured = std::getenv("RBLX_CC");
    if (configured && *configured) return configured;
    return "cc";
}

std::string hostSymbolPrefix() {
    const char* configured = std::getenv("RBLXCC_SYMBOL_PREFIX");
    if (configured) return configured;
#if defined(__APPLE__)
    return "_";
#else
    return "";
#endif
}

}  // namespace

int main(int argc, char** argv) {
    std::string input;
    std::string output;
    bool assemblyOnly = false;
    bool objectOnly = false;
    bool keepAssembly = false;
    bool includePrelude = true;
    std::string prefix = hostSymbolPrefix();

    for (int i = 1; i < argc; ++i) {
        std::string argument = argv[i];
        if (argument == "-h" || argument == "--help") {
            std::cout << kUsage << "\n" << rblx::cxx::supportedSummary() << "\n";
            return 0;
        }
        if (argument == "-o" && i + 1 < argc) {
            output = argv[++i];
            continue;
        }
        if (argument == "-S") {
            assemblyOnly = true;
            continue;
        }
        if (argument == "-c") {
            objectOnly = true;
            continue;
        }
        if (argument == "--keep-asm") {
            keepAssembly = true;
            continue;
        }
        if (argument == "--no-prelude") {
            includePrelude = false;
            continue;
        }
        if (argument == "--prefix" && i + 1 < argc) {
            prefix = argv[++i];
            continue;
        }
        if (!argument.empty() && argument[0] == '-' && argument != "-") {
            std::cerr << "RBLXCC: unknown option " << argument << "\n";
            return 2;
        }
        if (!input.empty()) {
            std::cerr << "RBLXCC: more than one input file\n";
            return 2;
        }
        input = argument;
    }

    if (input.empty()) {
        std::cerr << kUsage;
        return 2;
    }

    std::string source;
    if (input == "-") {
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        source = buffer.str();
        input = "stdin.cpp";
    } else if (!readFile(input, source)) {
        std::cerr << "RBLXCC: cannot read " << input << "\n";
        return 2;
    }

    rblx::cxx::Options options;
    options.sourceName = input;
    options.includePrelude = includePrelude;
    options.symbolPrefix = prefix;
#if defined(__APPLE__)
    options.usePlt = false;  // Mach-O has no PLT; symbols are called directly
#endif

    rblx::cxx::Result result = rblx::cxx::compile(source, options);
    if (!result.ok) {
        for (const rblx::cxx::Diagnostic& diagnostic : result.diagnostics) {
            std::cerr << input << ":" << diagnostic.line << ":" << diagnostic.column
                      << ": error: " << diagnostic.message << "\n";
        }
        return 1;
    }

    if (assemblyOnly || (!output.empty() && output.size() > 2 &&
                         output.compare(output.size() - 2, 2, ".s") == 0)) {
        std::string destination = output.empty() ? stemOf(input) + ".s" : output;
        if (destination == "-") {
            std::cout << result.assembly;
            return 0;
        }
        if (!writeFile(destination, result.assembly)) {
            std::cerr << "RBLXCC: cannot write " << destination << "\n";
            return 1;
        }
        return 0;
    }

    std::string program = stemOf(input);
    if (objectOnly) program += ".o";
    if (!output.empty()) program = output;
    std::string assemblyPath = program + ".s";

    if (!writeFile(assemblyPath, result.assembly)) {
        std::cerr << "RBLXCC: cannot write " << assemblyPath << "\n";
        return 1;
    }

    std::string command = quote(hostCompiler()) + (objectOnly ? " -c" : "") + " -x assembler " +
                          quote(assemblyPath) + " -o " + quote(program);
    int status = run(command);
    if (status != 0) {
        std::cerr << "RBLXCC: " << hostCompiler() << " could not " << (objectOnly ? "assemble " : "link ")
                  << program << ". The assembly is still at " << assemblyPath << ".\n";
        return 1;
    }

    if (!keepAssembly) std::remove(assemblyPath.c_str());
    return 0;
}
