// RBLXCC - the command line front end.
//
// The compiler itself is a library: this file reads the sources, prints the
// diagnostics in the shape a terminal wants, and hands the assembly to the
// system assembler and linker (through `cc`, or $RBLX_CC) to make a program.
//
// A program is usually more than one file, so this behaves like a real driver:
// every source is compiled on its own, the objects are linked together, and
// object files, archives and linker flags handed to it are passed straight to
// the link step.

#include "CxxCompiler.h"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace {

namespace fs = std::filesystem;

const char* kUsage =
    "RBLXCC - the C++ compiler that ships with RBLX Clone\n"
    "\n"
    "Usage: RBLXCC [options] <file.cpp>... [objects and libraries]\n"
    "\n"
    "Options:\n"
    "  -o <path>     write the program (or the assembly with -S) to <path>\n"
    "  -S            stop after code generation and write assembly\n"
    "  -c            stop after assembling and write object files\n"
    "  --keep-asm    keep the generated .s and .o files beside the output\n"
    "  --no-prelude  do not declare printf and the other C library functions\n"
    "  --prefix <s>  prefix every symbol with <s> (\"_\" on macOS; guessed here)\n"
    "  -I <dir>      add <dir> to the search path for #include\n"
    "  -h, --help    this text\n"
    "\n"
    "Every source file is compiled on its own and the results are linked into\n"
    "one program, the way a C compiler driver does it. Anything else on the\n"
    "command line (-l, -L, -pthread, .o, .a) is passed to the linker unchanged.\n"
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
    std::string name = fs::path(path).filename().string();
    std::size_t dot = name.find_last_of('.');
    if (dot != std::string::npos && dot > 0) name = name.substr(0, dot);
    return name.empty() ? "out" : name;
}

std::string lowercaseExtension(const std::string& path) {
    std::string extension = fs::path(path).extension().string();
    for (char& character : extension) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return extension;
}

bool isSourceFile(const std::string& path) {
    const std::string extension = lowercaseExtension(path);
    return extension == ".cpp" || extension == ".cc" || extension == ".cxx" ||
           extension == ".c++";
}

bool isLinkInput(const std::string& path) {
    const std::string extension = lowercaseExtension(path);
    return extension == ".o" || extension == ".obj" || extension == ".a" ||
           extension == ".lib" || extension == ".so" || extension == ".dylib";
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

std::string defaultProgramName() {
#if defined(_WIN32)
    return "a.exe";
#else
    return "a.out";
#endif
}

// Compiles one source to assembly text, reporting its diagnostics as it goes.
bool compileSource(const std::string& path, const rblx::cxx::Options& options,
                   std::string& assembly) {
    std::string name = path;
    std::string source;
    if (path == "-") {
        std::ostringstream buffer;
        buffer << std::cin.rdbuf();
        source = buffer.str();
        name = "stdin.cpp";
    } else if (!readFile(path, source)) {
        std::cerr << "RBLXCC: cannot read " << path << "\n";
        return false;
    }

    rblx::cxx::Options unitOptions = options;
    unitOptions.sourceName = name;
    // One file of several: whether the program has a main is decided once every
    // unit has been compiled, by the link step.
    unitOptions.requireEntryPoint = false;

    const rblx::cxx::Result result = rblx::cxx::compile(source, unitOptions);
    if (!result.ok) {
        for (const rblx::cxx::Diagnostic& diagnostic : result.diagnostics) {
            // A diagnostic from a header names the header, not the file it was
            // included from.
            const std::string where = diagnostic.file.empty() ? name : diagnostic.file;
            std::cerr << where << ":" << diagnostic.line << ":" << diagnostic.column
                      << ": error: " << diagnostic.message << "\n";
        }
        return false;
    }
    assembly = result.assembly;
    return true;
}

// Assembles one generated .s into an object file through the host toolchain.
bool assembleToObject(const std::string& assemblyPath, const std::string& objectPath) {
    const std::string command = quote(hostCompiler()) + " -c -x assembler " + quote(assemblyPath) +
                                " -o " + quote(objectPath);
    if (run(command) != 0) {
        std::cerr << "RBLXCC: " << hostCompiler() << " could not assemble " << assemblyPath
                  << ". The assembly is still there.\n";
        return false;
    }
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    std::vector<std::string> sources;
    std::vector<std::string> linkInputs;
    std::vector<std::string> linkerFlags;
    std::string output;
    bool assemblyOnly = false;
    bool objectOnly = false;
    bool keepIntermediate = false;
    bool includePrelude = true;
    std::vector<std::string> includePaths;
    std::string prefix = hostSymbolPrefix();

    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];
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
            keepIntermediate = true;
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
        if (argument == "-I" && i + 1 < argc) {
            includePaths.push_back(argv[++i]);
            continue;
        }
        if (argument.size() > 2 && argument.compare(0, 2, "-I") == 0) {
            includePaths.push_back(argument.substr(2));
            continue;
        }
        if (argument == "-") {
            sources.push_back(argument);
            continue;
        }
        if (!argument.empty() && argument[0] == '-') {
            // -l, -L, -pthread and friends belong to the link step.
            linkerFlags.push_back(argument);
            continue;
        }
        if (isSourceFile(argument)) {
            sources.push_back(argument);
            continue;
        }
        if (isLinkInput(argument)) {
            linkInputs.push_back(argument);
            continue;
        }
        std::cerr << "RBLXCC: " << argument << " is not a C++ source file or an object file\n";
        return 2;
    }

    const bool linking = !assemblyOnly && !objectOnly;
    if (sources.empty() && linkInputs.empty()) {
        std::cerr << kUsage;
        return 2;
    }
    if (sources.empty() && !linking) {
        std::cerr << "RBLXCC: -S and -c need a C++ source file\n";
        return 2;
    }
    if (!assemblyOnly && !objectOnly && sources.size() + linkInputs.size() > 1 && !output.empty()) {
        // Fine: -o names the linked program. Only the per-file modes are picky.
    }
    if ((assemblyOnly || objectOnly) && output.empty() == false &&
        sources.size() > 1 && linkInputs.empty()) {
        std::cerr << "RBLXCC: -o names one file, but there are " << sources.size()
                  << " inputs; drop -o and each file gets its own output\n";
        return 2;
    }
    if (sources.size() > 1) {
        for (const std::string& source : sources) {
            if (source == "-") {
                std::cerr << "RBLXCC: standard input must be the only source file\n";
                return 2;
            }
        }
    }

    rblx::cxx::Options options;
    options.includePrelude = includePrelude;
    options.includePaths = includePaths;
    options.symbolPrefix = prefix;
#if defined(__APPLE__)
    options.usePlt = false;  // Mach-O has no PLT; symbols are called directly
#endif

    // ------------------------------------------------------- one file at a time

    if (assemblyOnly || objectOnly) {
        for (const std::string& source : sources) {
            std::string assembly;
            if (!compileSource(source, options, assembly)) return 1;

            if (!output.empty() && output.size() > 2 &&
                output.compare(output.size() - 2, 2, ".s") == 0) {
                if (writeFile(output, assembly)) continue;
                std::cerr << "RBLXCC: cannot write " << output << "\n";
                return 1;
            }

            if (assemblyOnly) {
                const std::string destination = output.empty() ? stemOf(source) + ".s" : output;
                if (destination == "-") {
                    std::cout << assembly;
                    continue;
                }
                if (!writeFile(destination, assembly)) {
                    std::cerr << "RBLXCC: cannot write " << destination << "\n";
                    return 1;
                }
                continue;
            }

            const std::string object = output.empty() ? stemOf(source) + ".o" : output;
            const std::string assemblyPath = object + ".s";
            if (!writeFile(assemblyPath, assembly)) {
                std::cerr << "RBLXCC: cannot write " << assemblyPath << "\n";
                return 1;
            }
            if (!assembleToObject(assemblyPath, object)) return 1;
            if (!keepIntermediate) std::remove(assemblyPath.c_str());
        }
        return 0;
    }

    // --------------------------------------------- every source, then one link

    const std::string program = output.empty() ? defaultProgramName() : output;
    fs::path objectDirectory;
    if (!sources.empty()) {
        // Objects live next to the program so a failure can be inspected, and so
        // two sources with the same name in different folders do not collide.
        objectDirectory = fs::path(program + ".objs");
        std::error_code ignored;
        fs::remove_all(objectDirectory, ignored);
        fs::create_directories(objectDirectory, ignored);
        if (!fs::is_directory(objectDirectory)) {
            std::cerr << "RBLXCC: cannot create " << objectDirectory.string() << "\n";
            return 1;
        }
    }

    std::vector<std::string> objects;
    bool definesMain = false;
    for (const std::string& source : sources) {
        std::string assembly;
        if (!compileSource(source, options, assembly)) return 1;
        if (assembly.find("\n" + prefix + "main:\n") != std::string::npos) definesMain = true;

        std::string stem = stemOf(source);
        std::string object = (objectDirectory / (stem + ".o")).string();
        for (int suffix = 2; fs::exists(object); ++suffix) {
            object = (objectDirectory / (stem + "-" + std::to_string(suffix) + ".o")).string();
        }
        const std::string assemblyPath = object + ".s";
        if (!writeFile(assemblyPath, assembly)) {
            std::cerr << "RBLXCC: cannot write " << assemblyPath << "\n";
            return 1;
        }
        if (!assembleToObject(assemblyPath, object)) return 1;
        objects.push_back(object);
    }

    if (!sources.empty() && !definesMain) {
        std::cerr << "RBLXCC: none of the source files defines int main(), so there is nothing "
                     "to run\n";
        return 1;
    }

    std::string command = quote(hostCompiler()) + " -o " + quote(program);
    for (const std::string& object : objects) command += " " + quote(object);
    for (const std::string& input : linkInputs) command += " " + quote(input);
    for (const std::string& flag : linkerFlags) command += " " + flag;

    if (run(command) != 0) {
        std::cerr << "RBLXCC: " << hostCompiler() << " could not link " << program
                  << ". The objects are still in " << objectDirectory.string() << ".\n";
        return 1;
    }

    if (!keepIntermediate) {
        std::error_code ignored;
        fs::remove_all(objectDirectory, ignored);
    } else if (!sources.empty()) {
        std::cout << "RBLXCC: kept the generated assembly and objects in "
                  << objectDirectory.string() << "\n";
    }
    return 0;
}
