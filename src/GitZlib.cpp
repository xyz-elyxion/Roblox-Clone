#include "GitZlib.h"

#include <algorithm>
#include <cstring>
#include <vector>

namespace rblx {
namespace git {
namespace {

// A decompression bomb is a small file that expands without bound, so the
// reader stops at a size no source file in this project reaches.
const std::size_t kMaxOutput = 256u * 1024u * 1024u;

struct Bits {
    const unsigned char* data = nullptr;
    std::size_t size = 0;
    std::size_t pos = 0;
    unsigned buffer = 0;
    int count = 0;
    bool failed = false;
};

int take(Bits& in, int need) {
    while (in.count < need) {
        if (in.pos >= in.size) {
            in.failed = true;
            return 0;
        }
        in.buffer |= static_cast<unsigned>(in.data[in.pos++]) << in.count;
        in.count += 8;
    }
    const int value = static_cast<int>(in.buffer & ((1u << need) - 1u));
    in.buffer >>= need;
    in.count -= need;
    return value;
}

// Enough for the largest alphabet deflate defines: 288 literal/length symbols.
struct Huffman {
    short count[16] = {};
    short symbol[288] = {};
};

bool build(Huffman& table, const short* lengths, int symbols) {
    for (int i = 0; i < 16; ++i) {
        table.count[i] = 0;
    }
    for (int i = 0; i < symbols; ++i) {
        table.count[lengths[i]]++;
    }
    if (table.count[0] == symbols) {
        return true;  // a tree with no codes; decoding it will fail if used
    }

    int left = 1;
    for (int length = 1; length < 16; ++length) {
        left <<= 1;
        left -= table.count[length];
        if (left < 0) {
            return false;  // over-subscribed: no such tree exists
        }
    }

    short offsets[16] = {};
    for (int length = 1; length < 15; ++length) {
        offsets[length + 1] = static_cast<short>(offsets[length] + table.count[length]);
    }
    for (int i = 0; i < symbols; ++i) {
        if (lengths[i] != 0) {
            table.symbol[offsets[lengths[i]]++] = static_cast<short>(i);
        }
    }
    return true;
}

int decode(Bits& in, const Huffman& table) {
    int code = 0;
    int first = 0;
    int index = 0;
    for (int length = 1; length < 16; ++length) {
        code |= take(in, 1);
        if (in.failed) {
            return -1;
        }
        const int count = table.count[length];
        if (code - first < count) {
            return table.symbol[index + (code - first)];
        }
        index += count;
        first = (first + count) << 1;
        code <<= 1;
    }
    return -1;
}

const short kLengthBase[29] = {3,  4,  5,  6,  7,  8,  9,  10, 11,  13,  15,  17,  19,  23, 27,
                               31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const short kLengthExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const short kDistanceBase[30] = {1,    2,    3,    4,    5,    7,     9,     13,    17,   25,
                                 33,   49,   65,   97,   129,  193,   257,   385,   513,  769,
                                 1025, 1537, 2049, 3073, 4097, 6145,  8193,  12289, 16385, 24577};
const short kDistanceExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,
                                  6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

bool storedBlock(Bits& in, std::string* output, std::string* error) {
    // Stored blocks start on a byte boundary, so the bits already pulled into the
    // buffer are padding and must be dropped rather than counted.
    in.buffer = 0;
    in.count = 0;
    if (in.pos + 4 > in.size) {
        *error = "the compressed stream ends inside a stored block header";
        return false;
    }
    const std::size_t length =
        static_cast<std::size_t>(in.data[in.pos]) | (static_cast<std::size_t>(in.data[in.pos + 1]) << 8);
    const std::size_t complement = static_cast<std::size_t>(in.data[in.pos + 2]) |
                                   (static_cast<std::size_t>(in.data[in.pos + 3]) << 8);
    in.pos += 4;
    if ((length ^ 0xffffu) != complement) {
        *error = "a stored block's length does not match its complement";
        return false;
    }
    if (in.pos + length > in.size) {
        *error = "the compressed stream ends inside a stored block";
        return false;
    }
    if (output->size() + length > kMaxOutput) {
        *error = "the inflated object is larger than this program will hold";
        return false;
    }
    output->append(reinterpret_cast<const char*>(in.data + in.pos), length);
    in.pos += length;
    return true;
}

bool readLengths(Bits& in, Huffman& lengthsTable, short* lengths, int count) {
    int index = 0;
    while (index < count) {
        const int symbol = decode(in, lengthsTable);
        if (symbol < 0) {
            return false;
        }
        if (symbol < 16) {
            lengths[index++] = static_cast<short>(symbol);
        } else if (symbol == 16) {
            if (index == 0) {
                return false;  // nothing to repeat
            }
            const int repeat = 3 + take(in, 2);
            const short previous = lengths[index - 1];
            for (int i = 0; i < repeat && index < count; ++i) {
                lengths[index++] = previous;
            }
        } else if (symbol == 17) {
            const int repeat = 3 + take(in, 3);
            for (int i = 0; i < repeat && index < count; ++i) {
                lengths[index++] = 0;
            }
        } else if (symbol == 18) {
            const int repeat = 11 + take(in, 7);
            for (int i = 0; i < repeat && index < count; ++i) {
                lengths[index++] = 0;
            }
        } else {
            return false;
        }
        if (in.failed) {
            return false;
        }
    }
    return true;
}

bool fixedTables(Huffman* literals, Huffman* distances) {
    short literalLengths[288];
    for (int i = 0; i < 144; ++i) literalLengths[i] = 8;
    for (int i = 144; i < 256; ++i) literalLengths[i] = 9;
    for (int i = 256; i < 280; ++i) literalLengths[i] = 7;
    for (int i = 280; i < 288; ++i) literalLengths[i] = 8;
    short distanceLengths[30];
    for (int i = 0; i < 30; ++i) distanceLengths[i] = 5;
    return build(*literals, literalLengths, 288) && build(*distances, distanceLengths, 30);
}

bool dynamicTables(Bits& in, Huffman* literals, Huffman* distances) {
    const int literalCount = take(in, 5) + 257;
    const int distanceCount = take(in, 5) + 1;
    const int codeLengthCount = take(in, 4) + 4;
    if (in.failed || literalCount > 286 || distanceCount > 30) {
        return false;
    }

    static const short kOrder[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    short codeLengths[19] = {};
    for (int i = 0; i < codeLengthCount; ++i) {
        codeLengths[kOrder[i]] = static_cast<short>(take(in, 3));
    }
    if (in.failed) {
        return false;
    }

    Huffman codeLengthTable;
    if (!build(codeLengthTable, codeLengths, 19)) {
        return false;
    }

    short lengths[320] = {};
    if (!readLengths(in, codeLengthTable, lengths, literalCount + distanceCount)) {
        return false;
    }
    return build(*literals, lengths, literalCount) && build(*distances, lengths + literalCount, distanceCount);
}

bool inflateRaw(const unsigned char* data, std::size_t size, std::string* output, std::string* error) {
    Bits in;
    in.data = data;
    in.size = size;

    bool final = false;
    while (!final) {
        final = take(in, 1) != 0;
        const int type = take(in, 2);
        if (in.failed) {
            *error = "the compressed stream ends before its last block";
            return false;
        }

        if (type == 0) {
            if (!storedBlock(in, output, error)) {
                return false;
            }
            continue;
        }
        if (type == 3) {
            *error = "the compressed stream uses a reserved block type";
            return false;
        }

        Huffman literals;
        Huffman distances;
        const bool built = type == 1 ? fixedTables(&literals, &distances) : dynamicTables(in, &literals, &distances);
        if (!built || in.failed) {
            *error = "the compressed stream has a malformed Huffman table";
            return false;
        }

        while (true) {
            const int symbol = decode(in, literals);
            if (symbol < 0) {
                *error = "the compressed stream ends inside a block";
                return false;
            }
            if (symbol < 256) {
                if (output->size() >= kMaxOutput) {
                    *error = "the inflated object is larger than this program will hold";
                    return false;
                }
                output->push_back(static_cast<char>(symbol));
                continue;
            }
            if (symbol == 256) {
                break;  // end of block
            }

            const int lengthIndex = symbol - 257;
            if (lengthIndex >= 29) {
                *error = "the compressed stream uses an undefined length code";
                return false;
            }
            const int length = kLengthBase[lengthIndex] + take(in, kLengthExtra[lengthIndex]);

            const int distanceSymbol = decode(in, distances);
            if (distanceSymbol < 0 || distanceSymbol >= 30) {
                *error = "the compressed stream uses an undefined distance code";
                return false;
            }
            const std::size_t distance = static_cast<std::size_t>(kDistanceBase[distanceSymbol]) +
                                         static_cast<std::size_t>(take(in, kDistanceExtra[distanceSymbol]));
            if (in.failed) {
                *error = "the compressed stream ends inside a match";
                return false;
            }
            if (distance > output->size()) {
                *error = "the compressed stream refers to data before the start";
                return false;
            }
            if (output->size() + static_cast<std::size_t>(length) > kMaxOutput) {
                *error = "the inflated object is larger than this program will hold";
                return false;
            }
            // The copy may overlap the bytes it is producing, which is how deflate
            // encodes runs; copying forwards one byte at a time is what makes that
            // work, so it must not become a memcpy.
            for (int i = 0; i < length; ++i) {
                output->push_back((*output)[output->size() - distance]);
            }
        }
    }
    return true;
}

}  // namespace

bool inflateZlib(const std::string& input, std::string* output, std::string* error) {
    output->clear();
    if (input.size() < 6) {
        *error = "the object is too short to be a zlib stream";
        return false;
    }
    const unsigned char* bytes = reinterpret_cast<const unsigned char*>(input.data());
    const unsigned cmf = bytes[0];
    const unsigned flg = bytes[1];
    if ((cmf & 0x0f) != 8 || ((cmf >> 4) & 0x0f) > 7) {
        *error = "the object is not deflate data";
        return false;
    }
    if ((cmf * 256 + flg) % 31 != 0) {
        *error = "the zlib header checksum does not match";
        return false;
    }
    if ((flg & 0x20) != 0) {
        *error = "the object expects a preset dictionary, which git never writes";
        return false;
    }

    const std::size_t bodySize = input.size() - 6;
    if (!inflateRaw(bytes + 2, bodySize, output, error)) {
        return false;
    }

    const std::size_t tail = input.size() - 4;
    const std::uint32_t declared = (static_cast<std::uint32_t>(bytes[tail]) << 24) |
                                   (static_cast<std::uint32_t>(bytes[tail + 1]) << 16) |
                                   (static_cast<std::uint32_t>(bytes[tail + 2]) << 8) |
                                   static_cast<std::uint32_t>(bytes[tail + 3]);
    if (declared != adler32(output->data(), output->size())) {
        *error = "the object's Adler-32 checksum does not match its contents";
        output->clear();
        return false;
    }
    return true;
}

bool deflateZlibStored(const std::string& input, std::string* output, std::string* error) {
    (void)error;
    std::string out;
    out.reserve(input.size() + input.size() / 65535 * 5 + 11);
    out.push_back(static_cast<char>(0x78));
    out.push_back(static_cast<char>(0x01));

    std::size_t offset = 0;
    do {
        const std::size_t chunk = std::min<std::size_t>(65535, input.size() - offset);
        const bool last = offset + chunk >= input.size();
        out.push_back(static_cast<char>(last ? 1 : 0));
        out.push_back(static_cast<char>(chunk & 0xff));
        out.push_back(static_cast<char>((chunk >> 8) & 0xff));
        out.push_back(static_cast<char>((~chunk) & 0xff));
        out.push_back(static_cast<char>(((~chunk) >> 8) & 0xff));
        out.append(input, offset, chunk);
        offset += chunk;
    } while (offset < input.size());

    const std::uint32_t sum = adler32(input.data(), input.size());
    out.push_back(static_cast<char>((sum >> 24) & 0xff));
    out.push_back(static_cast<char>((sum >> 16) & 0xff));
    out.push_back(static_cast<char>((sum >> 8) & 0xff));
    out.push_back(static_cast<char>(sum & 0xff));
    *output = std::move(out);
    return true;
}

std::uint32_t adler32(const void* data, std::size_t size) {
    const unsigned char* bytes = static_cast<const unsigned char*>(data);
    std::uint32_t a = 1;
    std::uint32_t b = 0;
    for (std::size_t i = 0; i < size; ++i) {
        a = (a + bytes[i]) % 65521;
        b = (b + a) % 65521;
    }
    return (b << 16) | a;
}

}  // namespace git
}  // namespace rblx
