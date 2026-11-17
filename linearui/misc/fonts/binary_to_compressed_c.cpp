// dear linearui
// (binary_to_compressed_c.cpp)
// Helper tool to turn a file into a C array, if you want to embed font data in your source code.

// The data is first compressed with linearstb_compress() to reduce source code size.
// Then stored in a C array:
// - Base85:   ~5 bytes of source code for 4 bytes of input data. 5 bytes stored in binary (suggested by @mmalex).
// - As int:  ~11 bytes of source code for 4 bytes of input data. 4 bytes stored in binary. Endianness dependent, need swapping on big-endian CPU.
// - As char: ~12 bytes of source code for 4 bytes of input data. 4 bytes stored in binary. Not endianness dependent.
// Load compressed TTF fonts with LinearUI::GetIO().Fonts->AddFontFromMemoryCompressedTTF()

// Build with, e.g:
//   # cl.exe binary_to_compressed_c.cpp
//   # g++ binary_to_compressed_c.cpp
//   # clang++ binary_to_compressed_c.cpp
// You can also find a precompiled Windows binary in the binary/demo package available from https://github.com/ocornut/linearui

// Usage:
//   binary_to_compressed_c.exe [-nocompress] [-nostatic] [-base85] <inputfile> <symbolname>
// Usage example:
//   # binary_to_compressed_c.exe myfont.ttf MyFont > myfont.cpp
//   # binary_to_compressed_c.exe -base85 myfont.ttf MyFont > myfont.cpp
// Note:
//   Base85 encoding will be obsoleted by future version of Dear LinearUI!

#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// linearstb_compress* from linearstb.h - declaration
typedef unsigned int linearstb_uint;
typedef unsigned char linearstb_uchar;
linearstb_uint linearstb_compress(linearstb_uchar* out, linearstb_uchar* in, linearstb_uint len);

enum SourceEncoding
{
    SourceEncoding_U8,      // New default since 2024/11
    SourceEncoding_U32,
    SourceEncoding_Base85,
};

static bool binary_to_compressed_c(const char* filename, const char* symbol, SourceEncoding source_encoding, bool use_compression, bool use_static);

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        printf("Syntax: %s [-u8|-u32|-base85] [-nocompress] [-nostatic] <inputfile> <symbolname>\n", argv[0]);
        printf("Source encoding types:\n");
        printf(" -u8     = ~12 bytes of source per 4 bytes of data. 4 bytes in binary.\n");
        printf(" -u32    = ~11 bytes of source per 4 bytes of data. 4 bytes in binary. Need endianness swapping on big-endian.\n");
        printf(" -base85 =  ~5 bytes of source per 4 bytes of data. 5 bytes in binary. Need decoder.\n");
        return 0;
    }

    int argn = 1;
    bool use_compression = true;
    bool use_static = true;
    SourceEncoding source_encoding = SourceEncoding_U8; // New default
    while (argn < (argc - 2) && argv[argn][0] == '-')
    {
        if (strcmp(argv[argn], "-u8") == 0) { source_encoding = SourceEncoding_U8; argn++; }
        else if (strcmp(argv[argn], "-u32") == 0) { source_encoding = SourceEncoding_U32; argn++; }
        else if (strcmp(argv[argn], "-base85") == 0) { source_encoding = SourceEncoding_Base85; argn++; }
        else if (strcmp(argv[argn], "-nocompress") == 0) { use_compression = false; argn++; }
        else if (strcmp(argv[argn], "-nostatic") == 0) { use_static = false; argn++; }
        else
        {
            fprintf(stderr, "Unknown argument: '%s'\n", argv[argn]);
            return 1;
        }
    }

    bool ret = binary_to_compressed_c(argv[argn], argv[argn + 1], source_encoding, use_compression, use_static);
    if (!ret)
        fprintf(stderr, "Error opening or reading file: '%s'\n", argv[argn]);
    return ret ? 0 : 1;
}

char Encode85Byte(unsigned int x)
{
    x = (x % 85) + 35;
    return (char)((x >= '\\') ? x + 1 : x);
}

bool binary_to_compressed_c(const char* filename, const char* symbol, SourceEncoding source_encoding, bool use_compression, bool use_static)
{
    // Read file
    FILE* f = fopen(filename, "rb");
    if (!f) return false;
    int data_sz;
    if (fseek(f, 0, SEEK_END) || (data_sz = (int)ftell(f)) == -1 || fseek(f, 0, SEEK_SET)) { fclose(f); return false; }
    char* data = new char[data_sz + 4];
    if (fread(data, 1, data_sz, f) != (size_t)data_sz) { fclose(f); delete[] data; return false; }
    memset((void*)(((char*)data) + data_sz), 0, 4);
    fclose(f);

    // Compress
    int maxlen = data_sz + 512 + (data_sz >> 2) + sizeof(int); // total guess
    char* compressed = use_compression ? new char[maxlen] : data;
    int compressed_sz = use_compression ? linearstb_compress((linearstb_uchar*)compressed, (linearstb_uchar*)data, data_sz) : data_sz;
    if (use_compression)
        memset(compressed + compressed_sz, 0, maxlen - compressed_sz);

    // Output as Base85 encoded
    FILE* out = stdout;
    fprintf(out, "// File: '%s' (%d bytes)\n", filename, (int)data_sz);
    const char* static_str = use_static ? "static " : "";
    const char* compressed_str = use_compression ? "compressed_" : "";
    if (source_encoding == SourceEncoding_Base85)
    {
        fprintf(out, "// Exported using binary_to_compressed_c.exe -base85 \"%s\" %s\n", filename, symbol);
        fprintf(out, "%sconst char %s_%sdata_base85[%d+1] =\n    \"", static_str, symbol, compressed_str, (int)((compressed_sz + 3) / 4)*5);
        char prev_c = 0;
        for (int src_i = 0; src_i < compressed_sz; src_i += 4)
        {
            // This is made a little more complicated by the fact that ??X sequences are interpreted as trigraphs by old C/C++ compilers. So we need to escape pairs of ??.
            unsigned int d = *(unsigned int*)(compressed + src_i);
            for (unsigned int n5 = 0; n5 < 5; n5++, d /= 85)
            {
                char c = Encode85Byte(d);
                fprintf(out, (c == '?' && prev_c == '?') ? "\\%c" : "%c", c);
                prev_c = c;
            }
            if ((src_i % 112) == 112 - 4)
                fprintf(out, "\"\n    \"");
        }
        fprintf(out, "\";\n\n");
    }
    else if (source_encoding == SourceEncoding_U8)
    {
        // As individual bytes, not subject to endianness issues.
        fprintf(out, "// Exported using binary_to_compressed_c.exe -u8 \"%s\" %s\n", filename, symbol);
        fprintf(out, "%sconst unsigned int %s_%ssize = %d;\n", static_str, symbol, compressed_str, (int)compressed_sz);
        fprintf(out, "%sconst unsigned char %s_%sdata[%d] =\n{", static_str, symbol, compressed_str, (int)compressed_sz);
        int column = 0;
        for (int i = 0; i < compressed_sz; i++)
        {
            unsigned char d = *(unsigned char*)(compressed + i);
            if (column == 0)
                fprintf(out, "\n    ");
            column += fprintf(out, "%d,", d);
            if (column >= 180)
                column = 0;
        }
        fprintf(out, "\n};\n\n");
    }
    else if (source_encoding == SourceEncoding_U32)
    {
        // As integers
        fprintf(out, "// Exported using binary_to_compressed_c.exe -u32 \"%s\" %s\n", filename, symbol);
        fprintf(out, "%sconst unsigned int %s_%ssize = %d;\n", static_str, symbol, compressed_str, (int)compressed_sz);
        fprintf(out, "%sconst unsigned int %s_%sdata[%d/4] =\n{", static_str, symbol, compressed_str, (int)((compressed_sz + 3) / 4)*4);
        int column = 0;
        for (int i = 0; i < compressed_sz; i += 4)
        {
            unsigned int d = *(unsigned int*)(compressed + i);
            if ((column++ % 14) == 0)
                fprintf(out, "\n    0x%08x, ", d);
            else
                fprintf(out, "0x%08x, ", d);
        }
        fprintf(out, "\n};\n\n");
    }

    // Cleanup
    delete[] data;
    if (use_compression)
        delete[] compressed;
    return true;
}

// linearstb_compress* from linearstb.h - definition

////////////////////           compressor         ///////////////////////

static linearstb_uint linearstb_adler32(linearstb_uint adler32, linearstb_uchar *buffer, linearstb_uint buflen)
{
    const unsigned long ADLER_MOD = 65521;
    unsigned long s1 = adler32 & 0xffff, s2 = adler32 >> 16;
    unsigned long blocklen, i;

    blocklen = buflen % 5552;
    while (buflen) {
        for (i=0; i + 7 < blocklen; i += 8) {
            s1 += buffer[0], s2 += s1;
            s1 += buffer[1], s2 += s1;
            s1 += buffer[2], s2 += s1;
            s1 += buffer[3], s2 += s1;
            s1 += buffer[4], s2 += s1;
            s1 += buffer[5], s2 += s1;
            s1 += buffer[6], s2 += s1;
            s1 += buffer[7], s2 += s1;

            buffer += 8;
        }

        for (; i < blocklen; ++i)
            s1 += *buffer++, s2 += s1;

        s1 %= ADLER_MOD, s2 %= ADLER_MOD;
        buflen -= blocklen;
        blocklen = 5552;
    }
    return (s2 << 16) + s1;
}

static unsigned int linearstb_matchlen(linearstb_uchar *m1, linearstb_uchar *m2, linearstb_uint maxlen)
{
    linearstb_uint i;
    for (i=0; i < maxlen; ++i)
        if (m1[i] != m2[i]) return i;
    return i;
}

// simple implementation that just takes the source data in a big block

static linearstb_uchar *linearstb__out;
static FILE      *linearstb__outfile;
static linearstb_uint   linearstb__outbytes;

static void linearstb__write(unsigned char v)
{
    fputc(v, linearstb__outfile);
    ++linearstb__outbytes;
}

//#define linearstb_out(v)    (linearstb__out ? *linearstb__out++ = (linearstb_uchar) (v) : linearstb__write((linearstb_uchar) (v)))
#define linearstb_out(v)    do { if (linearstb__out) *linearstb__out++ = (linearstb_uchar) (v); else linearstb__write((linearstb_uchar) (v)); } while (0)

static void linearstb_out2(linearstb_uint v) { linearstb_out(v >> 8); linearstb_out(v); }
static void linearstb_out3(linearstb_uint v) { linearstb_out(v >> 16); linearstb_out(v >> 8); linearstb_out(v); }
static void linearstb_out4(linearstb_uint v) { linearstb_out(v >> 24); linearstb_out(v >> 16); linearstb_out(v >> 8 ); linearstb_out(v); }

static void outliterals(linearstb_uchar *in, int numlit)
{
    while (numlit > 65536) {
        outliterals(in,65536);
        in     += 65536;
        numlit -= 65536;
    }

    if      (numlit ==     0)    ;
    else if (numlit <=    32)    linearstb_out (0x000020 + numlit-1);
    else if (numlit <=  2048)    linearstb_out2(0x000800 + numlit-1);
    else /*  numlit <= 65536) */ linearstb_out3(0x070000 + numlit-1);

    if (linearstb__out) {
        memcpy(linearstb__out,in,numlit);
        linearstb__out += numlit;
    } else
        fwrite(in, 1, numlit, linearstb__outfile);
}

static int linearstb__window = 0x40000; // 256K

static int linearstb_not_crap(int best, int dist)
{
    return   ((best > 2  &&  dist <= 0x00100)
        || (best > 5  &&  dist <= 0x04000)
        || (best > 7  &&  dist <= 0x80000));
}

static  linearstb_uint linearstb__hashsize = 32768;

// note that you can play with the hashing functions all you
// want without needing to change the decompressor
#define linearstb__hc(q,h,c)      (((h) << 7) + ((h) >> 25) + q[c])
#define linearstb__hc2(q,h,c,d)   (((h) << 14) + ((h) >> 18) + (q[c] << 7) + q[d])
#define linearstb__hc3(q,c,d,e)   ((q[c] << 14) + (q[d] << 7) + q[e])

static unsigned int linearstb__running_adler;

static int linearstb_compress_chunk(linearstb_uchar *history,
    linearstb_uchar *start,
    linearstb_uchar *end,
    int length,
    int *pending_literals,
    linearstb_uchar **chash,
    linearstb_uint mask)
{
    (void)history;
    int window = linearstb__window;
    linearstb_uint match_max;
    linearstb_uchar *lit_start = start - *pending_literals;
    linearstb_uchar *q = start;

#define LINEARSTB__SCRAMBLE(h)   (((h) + ((h) >> 16)) & mask)

    // stop short of the end so we don't scan off the end doing
    // the hashing; this means we won't compress the last few bytes
    // unless they were part of something longer
    while (q < start+length && q+12 < end) {
        int m;
        linearstb_uint h1,h2,h3,h4, h;
        linearstb_uchar *t;
        int best = 2, dist=0;

        if (q+65536 > end)
            match_max = (linearstb_uint)(end-q);
        else
            match_max = 65536;

#define linearstb__nc(b,d)  ((d) <= window && ((b) > 9 || linearstb_not_crap((int)(b),(int)(d))))

#define LINEARSTB__TRY(t,p)  /* avoid retrying a match we already tried */ \
    if (p ? dist != (int)(q-t) : 1)                             \
    if ((m = linearstb_matchlen(t, q, match_max)) > best)     \
    if (linearstb__nc(m,q-(t)))                                \
    best = m, dist = (int)(q - (t))

        // rather than search for all matches, only try 4 candidate locations,
        // chosen based on 4 different hash functions of different lengths.
        // this strategy is inspired by LZO; hashing is unrolled here using the
        // 'hc' macro
        h = linearstb__hc3(q,0, 1, 2); h1 = LINEARSTB__SCRAMBLE(h);
        t = chash[h1]; if (t) LINEARSTB__TRY(t,0);
        h = linearstb__hc2(q,h, 3, 4); h2 = LINEARSTB__SCRAMBLE(h);
        h = linearstb__hc2(q,h, 5, 6);        t = chash[h2]; if (t) LINEARSTB__TRY(t,1);
        h = linearstb__hc2(q,h, 7, 8); h3 = LINEARSTB__SCRAMBLE(h);
        h = linearstb__hc2(q,h, 9,10);        t = chash[h3]; if (t) LINEARSTB__TRY(t,1);
        h = linearstb__hc2(q,h,11,12); h4 = LINEARSTB__SCRAMBLE(h);
        t = chash[h4]; if (t) LINEARSTB__TRY(t,1);

        // because we use a shared hash table, can only update it
        // _after_ we've probed all of them
        chash[h1] = chash[h2] = chash[h3] = chash[h4] = q;

        if (best > 2)
            assert(dist > 0);

        // see if our best match qualifies
        if (best < 3) { // fast path literals
            ++q;
        } else if (best > 2  &&  best <= 0x80    &&  dist <= 0x100) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            linearstb_out(0x80 + best-1);
            linearstb_out(dist-1);
        } else if (best > 5  &&  best <= 0x100   &&  dist <= 0x4000) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            linearstb_out2(0x4000 + dist-1);
            linearstb_out(best-1);
        } else if (best > 7  &&  best <= 0x100   &&  dist <= 0x80000) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            linearstb_out3(0x180000 + dist-1);
            linearstb_out(best-1);
        } else if (best > 8  &&  best <= 0x10000 &&  dist <= 0x80000) {
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            linearstb_out3(0x100000 + dist-1);
            linearstb_out2(best-1);
        } else if (best > 9                      &&  dist <= 0x1000000) {
            if (best > 65536) best = 65536;
            outliterals(lit_start, (int)(q-lit_start)); lit_start = (q += best);
            if (best <= 0x100) {
                linearstb_out(0x06);
                linearstb_out3(dist-1);
                linearstb_out(best-1);
            } else {
                linearstb_out(0x04);
                linearstb_out3(dist-1);
                linearstb_out2(best-1);
            }
        } else {  // fallback literals if no match was a balanced tradeoff
            ++q;
        }
    }

    // if we didn't get all the way, add the rest to literals
    if (q-start < length)
        q = start+length;

    // the literals are everything from lit_start to q
    *pending_literals = (int)(q - lit_start);

    linearstb__running_adler = linearstb_adler32(linearstb__running_adler, start, (linearstb_uint)(q - start));
    return (int)(q - start);
}

static int linearstb_compress_inner(linearstb_uchar *input, linearstb_uint length)
{
    int literals = 0;
    linearstb_uint len,i;

    linearstb_uchar **chash;
    chash = (linearstb_uchar**) malloc(linearstb__hashsize * sizeof(linearstb_uchar*));
    if (chash == nullptr) return 0; // failure
    for (i=0; i < linearstb__hashsize; ++i)
        chash[i] = nullptr;

    // stream signature
    linearstb_out(0x57); linearstb_out(0xbc);
    linearstb_out2(0);

    linearstb_out4(0);       // 64-bit length requires 32-bit leading 0
    linearstb_out4(length);
    linearstb_out4(linearstb__window);

    linearstb__running_adler = 1;

    len = linearstb_compress_chunk(input, input, input+length, length, &literals, chash, linearstb__hashsize-1);
    assert(len == length);

    outliterals(input+length - literals, literals);

    free(chash);

    linearstb_out2(0x05fa); // end opcode

    linearstb_out4(linearstb__running_adler);

    return 1; // success
}

linearstb_uint linearstb_compress(linearstb_uchar *out, linearstb_uchar *input, linearstb_uint length)
{
    linearstb__out = out;
    linearstb__outfile = nullptr;

    linearstb_compress_inner(input, length);

    return (linearstb_uint)(linearstb__out - out);
}
