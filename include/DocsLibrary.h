#pragma once

// Finding and reading the Markdown LinearDocs shows.
//
// The documents are read from disk rather than compiled into the program: the
// packager copies them into docs/ beside the executable, and a run from a source
// checkout finds the same files one folder up. That keeps the window showing the
// text that shipped with the build, and keeps the build free of a generated blob
// that would have to be regenerated every time a sentence changed.

#include <string>
#include <vector>

namespace linear {
namespace docs {

struct Source {
    std::string name;   // "README.md"
    std::string title;  // its first heading, or the file name without .md
    std::string path;   // where it was read from
    std::string text;   // the file as it was written
    int lines = 0;
};

// Full path of the running executable, or empty when Windows will not say.
std::string executablePath();

// The folder the documents live in: `docs` beside the program, then the program's
// own folder, then the folder above it - which is what a run from the build tree
// or a source checkout needs. Empty when no folder holds a .md file.
std::string folderFor(const std::string& executablePath);

// Every .md file in `folder`, README first and the rest alphabetically, each with
// its text read. Returns false and sets `error` when the folder cannot be read.
bool loadAll(const std::string& folder, std::vector<Source>* sources, std::string* error);

}  // namespace docs
}  // namespace linear
