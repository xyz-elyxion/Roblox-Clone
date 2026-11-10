#pragma once

#include <cstddef>
#include <cstdint>
#include <string>

namespace rblx {
namespace git {

// Loose objects are zlib streams, so a repository cannot be read - or written in
// a form git accepts - without both halves. These are this project's own
// implementations, because the vendored trees carry no compression library.

// Reads a zlib stream (stored, fixed or dynamic Huffman blocks) into `output`.
// The trailing Adler-32 is checked, so a truncated or corrupted object fails
// instead of yielding plausible garbage.
bool inflateZlib(const std::string& input, std::string* output, std::string* error);

// Writes a valid zlib stream using uncompressed blocks only. Real git does the
// same for incompressible data, so this is a legal encoding rather than a
// shortcut, and it keeps the writer small; compression is a size question, not a
// compatibility one.
bool deflateZlibStored(const std::string& input, std::string* output, std::string* error);

std::uint32_t adler32(const void* data, std::size_t size);

}  // namespace git
}  // namespace rblx
