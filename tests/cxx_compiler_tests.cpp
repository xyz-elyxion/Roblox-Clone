// Tests for RBLXCC, the compiler in src/Cxx*.cpp.
//
// The compiler is a library, so most of this works on strings: source in,
// diagnostics and assembly out. The second half goes further - each program is
// compiled, assembled, linked with the host C compiler and run, and its stdout
// is compared byte for byte - because a compiler that emits plausible assembly
// but the wrong answer is worse than one that refuses the program.

#include "CxxCompiler.h"

#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace {

int checks = 0;
int failures = 0;

void check(bool condition, const char* text, int line) {
    ++checks;
    if (condition) return;
    ++failures;
    std::printf("FAIL line %d: %s\n", line, text);
}

#define CHECK(condition) check((condition), #condition, __LINE__)

rblx::cxx::Result compileSource(const std::string& source, bool includePrelude = true) {
    rblx::cxx::Options options;
    options.sourceName = "case.cpp";
    options.includePrelude = includePrelude;
    return rblx::cxx::compile(source, options);
}

std::string firstMessage(const rblx::cxx::Result& result) {
    return result.diagnostics.empty() ? std::string("<no diagnostic>")
                                      : result.diagnostics.front().message;
}

bool contains(const std::string& text, const std::string& needle) {
    return text.find(needle) != std::string::npos;
}

void expectCompiles(const std::string& name, const std::string& source) {
    ++checks;
    rblx::cxx::Result result = compileSource(source);
    if (result.ok && !result.assembly.empty()) return;
    ++failures;
    std::printf("FAIL %s: should compile but said \"%s\"\n", name.c_str(),
                firstMessage(result).c_str());
}

void expectError(const std::string& name, const std::string& source, const char* fragment) {
    ++checks;
    rblx::cxx::Result result = compileSource(source);
    if (result.ok) {
        ++failures;
        std::printf("FAIL %s: compiled, but should have been refused\n", name.c_str());
        return;
    }
    if (!contains(firstMessage(result), fragment)) {
        ++failures;
        std::printf("FAIL %s: said \"%s\", wanted \"%s\"\n", name.c_str(),
                    firstMessage(result).c_str(), fragment);
    }
}

// ------------------------------------------------------------- diagnostics ----

void diagnosticsTests() {
    expectCompiles("empty main", "int main() { return 0; }");

    expectError("undeclared variable", "int main() { return missing; }", "is not declared");
    expectError("undeclared call", "int main() { return helper(1); }", "is not declared");
    expectError("missing main", "int helper() { return 1; }", "no main function");
    expectError("classes", "class Thing { }; int main() { return 0; }",
                "class is not supported");
    expectError("templates", "template<int N> int f() { return N; } int main() { return 0; }",
                "template is not supported");
    expectError("unsigned", "int main() { unsigned int x = 1; return x; }",
                "unsigned types are not supported");
    expectError("floating point", "int main() { float x = 1.5; return 0; }\n",
                "floating point");
    expectError("struct by value parameter",
                "struct P { int x; }; int take(struct P p) { return p.x; } int main() { return 0; }",
                "pass a struct by pointer");
    expectError("array assignment", "int main() { int a[3]; int b[3]; a = b; return 0; }",
                "an array cannot be assigned");
    expectError("break outside a loop", "int main() { break; return 0; }", "'break' outside a loop");
    expectError("function as a value", "int helper() { return 1; } int main() { int x = helper; return x; }",
                "call it with ()");
    expectError("wrong argument count",
                "int add(int a, int b) { return a + b; } int main() { return add(1); }",
                "takes 2 arguments");
    expectError("pointer plus pointer", "int main() { int* a = 0; int* b = 0; return a + b; }",
                "two pointers cannot be added");
    expectError("value returned from void",
                "void nothing() { return 1; } int main() { nothing(); return 0; }",
                "this function is void");
    expectError("missing semicolon", "int main() { int x = 1 return 0; }",
                "expected ';'");
    expectError("unterminated string", "int main() { printf(\"oops); return 0; }",
                "unterminated string literal");
    expectError("array length not constant",
                "int main() { int n = 3; int a[n]; return 0; }", "must be a constant");
    expectError("duplicate definition",
                "int twice() { return 1; } int twice() { return 2; } int main() { return 0; }",
                "is defined twice");

    // The prelude is what declares printf; without it the same program is refused.
    rblx::cxx::Result withoutPrelude =
        compileSource("int main() { printf(\"hi\"); return 0; }", false);
    CHECK(!withoutPrelude.ok);
    CHECK(contains(firstMessage(withoutPrelude), "'printf' is not declared"));

    // A diagnostic carries the line it came from, not just a message.
    rblx::cxx::Result located = compileSource("int main() {\n    return missing;\n}\n");
    CHECK(!located.ok);
    CHECK(!located.diagnostics.empty() && located.diagnostics.front().line == 2);
    CHECK(contains(located.report(), "2:"));
}

// ---------------------------------------------------------------- assembly ----

void assemblyTests() {
    rblx::cxx::Result minimal = compileSource("int main() { return 3; }");
    CHECK(minimal.ok);
    CHECK(contains(minimal.assembly, ".globl main"));
    CHECK(contains(minimal.assembly, "main:"));
    CHECK(contains(minimal.assembly, "pushq %rbp"));
    CHECK(contains(minimal.assembly, "popq %rbp"));
    CHECK(contains(minimal.assembly, "ret"));
    CHECK(contains(minimal.assembly, "$3"));

    rblx::cxx::Result call = compileSource("int main() { printf(\"%d\\n\", 7); return 0; }");
    CHECK(call.ok);
    CHECK(contains(call.assembly, "call printf@PLT"));
    CHECK(contains(call.assembly, "movb $0, %al"));

    rblx::cxx::Result staticFunction =
        compileSource("static int hidden() { return 1; } int main() { return hidden(); }");
    CHECK(staticFunction.ok);
    CHECK(contains(staticFunction.assembly, "call hidden\n"));
    CHECK(!contains(staticFunction.assembly, ".globl hidden"));

    rblx::cxx::Result data = compileSource("int counter = 7; int main() { return counter; }");
    CHECK(data.ok);
    CHECK(contains(data.assembly, ".data"));
    CHECK(contains(data.assembly, ".globl counter"));
    CHECK(contains(data.assembly, ".long 7"));

    // Strings go in .rodata and are reached through the label, not copied in.
    rblx::cxx::Result strings = compileSource("int main() { puts(\"text\"); return 0; }");
    CHECK(strings.ok);
    CHECK(contains(strings.assembly, ".section .rodata"));
    CHECK(contains(strings.assembly, ".Lstring0:"));
    CHECK(contains(strings.assembly, ".string \"text\""));
}

// --------------------------------------------------------------- end to end ----

struct ProgramCase {
    const char* name;
    const char* source;
    const char* expected;
};

const ProgramCase kPrograms[] = {
    {"hello", R"CPP(
int printf(char* format, ...);

int main() {
    printf("hello, world\n");
    return 0;
}
)CPP",
     "hello, world\n"},

    {"arithmetic", R"CPP(
int printf(char* format, ...);

int main() {
    int a = 17;
    int b = 5;
    printf("%d %d %d %d\n", a + b, a - b, a * b, a / b);
    printf("%d %d %d\n", a % b, -a / b, -a % b);

    int c = 3;
    c += 4;
    c *= 2;
    c -= 1;
    c /= 3;
    printf("c=%d\n", c);

    int d = 5;
    d++;
    ++d;
    d--;
    printf("d=%d\n", d);

    printf("bits=%d %d %d\n", 1 << 10, 255 & 15, 12 | 3);
    printf("xor=%d not=%d\n", 12 ^ 10, ~0);
    long big = 1000000;
    printf("big=%ld\n", big * big);
    return 0;
}
)CPP",
     "22 12 85 3\n2 -3 -2\nc=4\nd=6\nbits=1024 15 15\nxor=6 not=-1\nbig=1000000000000\n"},

    {"control flow", R"CPP(
int printf(char* format, ...);

int score(int value) {
    if (value > 10) return 3;
    else if (value > 5) return 2;
    else if (value > 0) return 1;
    return 0;
}

int main() {
    int seen = 0;
    for (int i = 0; i < 20; i++) {
        if (i % 3 == 0) continue;
        if (i > 12) break;
        seen += 1;
    }
    printf("seen=%d\n", seen);

    int n = 0;
    while (n < 5) n += 2;
    printf("while=%d\n", n);

    int m = 10;
    do {
        m -= 3;
    } while (m > 0);
    printf("do=%d\n", m);

    int calls = 0;
    int flag = 0;
    int both = (flag != 0) && (++calls > 0);
    printf("short=%d calls=%d\n", both, calls);

    printf("ternary=%d %d\n", n > m ? n : m, flag ? 1 : 2);
    printf("scores=%d %d %d %d\n", score(20), score(7), score(2), score(-1));

    int empty = 0;
    for (;;) {
        empty += 1;
        if (empty == 3) break;
    }
    printf("loop=%d\n", empty);
    return 0;
}
)CPP",
     "seen=8\nwhile=6\ndo=-2\nshort=0 calls=0\nternary=6 2\nscores=3 2 1 0\nloop=3\n"},

    {"functions", R"CPP(
int printf(char* format, ...);

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int fib(int n) {
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

void swap(int* left, int* right) {
    int temp = *left;
    *left = *right;
    *right = temp;
}

int add8(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a + b + c + d + e + f + g + h;
}

void show(int value) {
    printf("show=%d\n", value);
    return;
}

int main() {
    printf("fact=%d fib=%d\n", factorial(6), fib(10));

    int x = 1;
    int y = 2;
    swap(&x, &y);
    printf("swap=%d %d\n", x, y);

    printf("add8=%d\n", add8(1, 2, 3, 4, 5, 6, 7, 8));
    show(add8(10, 10, 10, 10, 10, 10, 10, 10));
    return 0;
}
)CPP",
     "fact=720 fib=55\nswap=2 1\nadd8=36\nshow=80\n"},

    {"arrays and pointers", R"CPP(
int printf(char* format, ...);

int main() {
    int values[5];
    for (int i = 0; i < 5; i++) values[i] = i * 3;

    int total = 0;
    for (int i = 0; i < 5; i++) total += values[i];
    printf("total=%d first=%d\n", total, values[0]);

    int* cursor = values;
    cursor++;
    printf("second=%d\n", *cursor);
    *cursor = 99;
    printf("changed=%d\n", values[1]);

    int grid[2][3];
    for (int row = 0; row < 2; row++) {
        for (int col = 0; col < 3; col++) grid[row][col] = row * 10 + col;
    }
    printf("grid=%d %d %d\n", grid[0][0], grid[1][2], grid[0][2]);
    printf("sizes=%ld %ld %ld\n", sizeof(values), sizeof(grid), sizeof(grid[0]));

    int* picked = 0;
    printf("null=%d\n", picked == 0);
    return 0;
}
)CPP",
     "total=30 first=0\nsecond=3\nchanged=99\ngrid=0 12 2\nsizes=20 24 12\nnull=1\n"},

    {"structs", R"CPP(
int printf(char* format, ...);

struct Pair {
    int first;
    int second;
};

struct Node {
    int value;
    struct Node* next;
};

int main() {
    struct Pair a;
    a.first = 4;
    a.second = 7;

    struct Pair b = a;
    b.first = 9;
    printf("pair=%d %d %d\n", a.first, b.first, b.second);

    struct Pair* p = &b;
    p->second = 11;
    printf("through pointer=%d\n", b.second);

    struct Node tail;
    tail.value = 2;
    tail.next = 0;

    struct Node head;
    head.value = 1;
    head.next = &tail;
    printf("list=%d %d\n", head.value, head.next->value);
    printf("sizes=%ld %ld\n", sizeof(struct Pair), sizeof(struct Node*));
    return 0;
}
)CPP",
     "pair=4 9 7\nthrough pointer=11\nlist=1 2\nsizes=8 8\n"},

    {"globals", R"CPP(
int printf(char* format, ...);

int counter = 7;
char* greeting = "hello from a global";
int table[4];

static int bump() {
    counter += 1;
    return counter;
}

int main() {
    table[0] = 5;
    table[1] = table[0] * 2;
    printf("%s\n", greeting);
    printf("counter=%d\n", counter);
    printf("bump=%d after=%d\n", bump(), counter);
    printf("table=%d %d %d\n", table[0], table[1], table[3]);
    return 0;
}
)CPP",
     "hello from a global\ncounter=7\nbump=8 after=8\ntable=5 10 0\n"},

    {"strings and library calls", R"CPP(
int printf(char* format, ...);
int puts(char* text);
long strlen(char* text);
int strcmp(char* left, char* right);

int main() {
    char* name = "RBLX";
    printf("%s has %ld letters\n", name, strlen(name));
    printf("cmp=%d %d\n", strcmp("abc", "abd"), strcmp("same", "same"));
    puts("puts works");
    printf("escapes=%d %d\n", '\n', '\t');
    printf("concat=%s\n", "left" "right");
    return 0;
}
)CPP",
     "RBLX has 4 letters\ncmp=-1 0\nputs works\nescapes=10 9\nconcat=leftright\n"},

    {"preprocessor", R"CPP(
int printf(char* format, ...);

#define LIMIT 4
#define STEP 2
#define NAME "preprocessor"

#include <stdio.h>

int main() {
#ifdef NEVER_DEFINED
    printf("wrong\n");
#else
    int total = 0;
    for (int i = 0; i < LIMIT; i += STEP) total += i;
    printf("%s total=%d\n", NAME, total);
#endif
#ifndef ALSO_MISSING
    printf("second branch\n");
#endif
    return 0;
}
)CPP",
     "preprocessor total=2\nsecond branch\n"},

    {"chars and bools", R"CPP(
int printf(char* format, ...);

int main() {
    char letter = 'A';
    letter = letter + 2;
    printf("letter=%c code=%d\n", letter, letter);

    bool ready = 5 > 3;
    bool never = 0;
    printf("bool=%d %d\n", ready, never);

    int* nothing = 0;
    if (nothing) printf("wrong\n"); else printf("null is false\n");

    int value = 2;
    printf("logic=%d %d %d\n", !value, value && 1, value || 0);
    return 0;
}
)CPP",
     "letter=C code=67\nbool=1 0\nnull is false\nlogic=0 1 1\n"},

    {"nested calls and aggregates", R"CPP(
int printf(char* format, ...);

struct Pair {
    int first;
    int second;
};

struct Pair pairs[2];

int add(int a, int b) {
    return a + b;
}

int scale(int value, int factor) {
    return value * factor;
}

int add7(int a, int b, int c, int d, int e, int f, int g) {
    return a + b + c + d + e + f + g;
}

int combine(int a, int b, int c, int d, int e, int f, int g, int h) {
    return a + b * 2 + c * 3 + d * 4 + e * 5 + f * 6 + g * 7 + h * 8;
}

int main() {
    pairs[1].first = 5;
    pairs[1].second = add(pairs[1].first, scale(2, 3));
    printf("pair=%d %d\n", pairs[1].first, pairs[1].second);

    printf("nested=%d\n", add(scale(2, 3), combine(1, 1, 1, 1, 1, 1, 1, 1)));
    printf("seven=%d\n", add7(1, 2, 3, 4, 5, 6, 7));

    int* table[3];
    int a = 1;
    int b = 2;
    int c = 3;
    table[0] = &a;
    table[1] = &b;
    table[2] = &c;

    int total = 0;
    for (int i = 0; i < 3; i++) total += *table[i];
    printf("table=%d\n", total);
    return 0;
}
)CPP",
     "pair=5 11\nnested=42\nseven=28\ntable=6\n"},

    {"allocation and casts", R"CPP(
int printf(char* format, ...);
void* malloc(long size);
void free(void* memory);

int main() {
    int* cells = (int*)malloc(4 * sizeof(int));
    if (!cells) {
        printf("no memory\n");
        return 1;
    }
    for (int i = 0; i < 4; i++) cells[i] = (i + 1) * 5;
    printf("cells=%d %d\n", cells[0], cells[3]);
    free(cells);
    printf("freed\n");
    return 0;
}
)CPP",
     "cells=5 20\nfreed\n"},

    {"pointer returns", R"CPP(
int printf(char* format, ...);

int* pick(int* a, int* b, int which) {
    if (which) return a;
    return b;
}

int main() {
    int x = 10;
    int y = 20;
    int* chosen = pick(&x, &y, 0);
    printf("chosen=%d\n", *chosen);
    *pick(&x, &y, 1) = 15;
    printf("x=%d\n", x);
    return 0;
}
)CPP",
     "chosen=20\nx=15\n"},
};

#if defined(_WIN32)
#define RBLX_POPEN _popen
#define RBLX_PCLOSE _pclose
#else
#define RBLX_POPEN popen
#define RBLX_PCLOSE pclose
#endif

std::string hostCCompiler() {
    const char* configured = std::getenv("RBLX_CC");
    if (configured && *configured) return configured;
#if defined(_WIN32)
    const char* quiet = " > NUL 2>&1";
#else
    const char* quiet = " > /dev/null 2>&1";
#endif
    for (const char* candidate : {"cc", "gcc", "clang"}) {
        std::string probe = std::string(candidate) + " --version" + quiet;
        if (std::system(probe.c_str()) == 0) return candidate;
    }
    return std::string();
}

bool buildAndRun(const ProgramCase& test, const std::filesystem::path& directory,
                 const std::string& compilerName, std::string& output, std::string& problem) {
    rblx::cxx::Result result = compileSource(test.source);
    if (!result.ok) {
        problem = "did not compile: " + result.report();
        return false;
    }

    const std::filesystem::path assembly = directory / (std::string(test.name) + ".s");
#if defined(_WIN32)
    const std::filesystem::path binary = directory / (std::string(test.name) + ".exe");
#else
    const std::filesystem::path binary = directory / test.name;
#endif
    {
        std::ofstream file(assembly, std::ios::binary | std::ios::trunc);
        if (!file) {
            problem = "could not write the assembly";
            return false;
        }
        file << result.assembly;
    }

#if defined(_WIN32)
    const std::string quiet = " > NUL 2>&1";
#else
    const std::string quiet = " > /dev/null 2>&1";
#endif
    const std::string link = compilerName + " -x assembler \"" + assembly.string() + "\" -o \"" +
                             binary.string() + "\"" + quiet;
    if (std::system(link.c_str()) != 0) {
        problem = "the assembler or linker refused the output";
        return false;
    }

    FILE* pipe = RBLX_POPEN(("\"" + binary.string() + "\"").c_str(), "r");
    if (!pipe) {
        problem = "could not start the program";
        return false;
    }
    char buffer[512];
    std::size_t read = 0;
    while ((read = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        output.append(buffer, read);
    }
    const int status = RBLX_PCLOSE(pipe);
    if (status != 0) {
        problem = "the program exited with " + std::to_string(status);
        return false;
    }
    return true;
}

void endToEndTests() {
    const std::string compilerName = hostCCompiler();
    if (compilerName.empty()) {
        std::printf(
            "cxx_compiler_tests: no cc/gcc/clang on PATH, so the compiled programs were not "
            "built or run (set RBLX_CC to point at one)\n");
        return;
    }

    std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "rblxcc-compiler-tests";
    std::error_code ignored;
    std::filesystem::remove_all(directory, ignored);
    std::filesystem::create_directories(directory, ignored);

    for (const ProgramCase& test : kPrograms) {
        ++checks;
        std::string output;
        std::string problem;
        if (!buildAndRun(test, directory, compilerName, output, problem)) {
            ++failures;
            std::printf("FAIL %s: %s\n", test.name, problem.c_str());
            continue;
        }
        if (output != test.expected) {
            ++failures;
            std::printf("FAIL %s: got \"%s\", wanted \"%s\"\n", test.name, output.c_str(),
                        test.expected);
        }
    }

    std::filesystem::remove_all(directory, ignored);
}

}  // namespace

int main() {
    diagnosticsTests();
    assemblyTests();
    endToEndTests();

    std::printf("cxx_compiler_tests: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
