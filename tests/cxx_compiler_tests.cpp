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
    // A class is a struct here: access is not enforced, so the two differ only in
    // the default, which nothing reads.
    expectCompiles("classes", "class Thing { public: int value; }; "
                              "int main() { class Thing t; t.value = 2; return t.value; }");
    expectCompiles("class forward declaration",
                   "class Thing; int main() { class Thing* none = 0; return none == 0; }");
    expectError("templates", "template<int N> int f() { return N; } int main() { return 0; }",
                "template is not supported");
    expectError("inheritance", "struct A { int x; }; struct B : A { int y; }; int main() { return 0; }",
                "inheritance is not supported");
    expectError("constructors", "struct P { int x; P() { x = 1; } }; int main() { return 0; }",
                "constructors are not supported");
    expectError("virtual functions", "struct P { int x; virtual int get() { return x; } }; "
                                     "int main() { return 0; }",
                "virtual functions are not supported");
    expectError("static initialiser", "int f(); int main() { static int x = f(); return x; }",
                "its initialiser must be a constant");
    expectCompiles("scoped enum", "enum class Color { RED, GREEN }; "
                                  "int main() { Color c = Color::GREEN; return (int)c; }");
    expectError("unknown scoped enumerator",
                "enum class Color { RED }; int main() { return (int)Color::BLUE; }",
                "has no enumerator named");
    expectCompiles("enum with an underlying type",
                   "enum Small : unsigned char { SA, SB }; int main() { return (int)SB; }");
    expectError("variadic macro", "#define LOG(...) printf(__VA_ARGS__)\nint main() { return 0; }",
                "variadic macros are not supported");
    expectError("union named as a struct", "union U { int i; }; struct U u; int main() { return 0; }",
                "is a union, not a struct");
    expectError("case outside a switch", "int main() { case 1: return 0; }",
                "'case' outside a switch");
    expectError("goto without a label", "int main() { goto nowhere; }",
                "no label named 'nowhere'");
    expectError("failed static_assert",
                "static_assert(1 == 2, \"one is not two\"); int main() { return 0; }",
                "static_assert failed: one is not two");
    expectError("switch on a float", "int main() { switch (1.5) { case 1: return 0; } return 1; }",
                "a switch needs an integer");
    expectError("duplicate case",
                "int main() { switch (1) { case 1: return 0; case 1: return 1; } return 2; }",
                "has two case labels");
    expectCompiles("range-for", "int main() { int a[2] = {1, 2}; int t = 0; "
                                "for (auto v : a) { t = t + v; } return t; }");
    expectError("range-for over a pointer",
                "int main() { int* p = 0; for (auto v : p) { } return 0; }", "needs an array");
    expectCompiles("returning a reference",
                   "int g = 4; int& pick() { return g; } int main() { pick() = 7; return g; }");
    expectError("returning a reference to a value", "int& pick() { return 3; } int main() { return 0; }",
                "must be returned from a variable");
    expectError("operator overloading",
                "struct P { int x; int operator+(int o) { return x + o; } }; "
                "int main() { return 0; }",
                "operator overloading is not supported");
    expectError("destructors", "struct P { int x; ~P() { } }; int main() { return 0; }",
                "destructors are not supported");
    expectError("exceptions", "int main() { try { return 0; } catch (...) { return 1; } }",
                "exceptions are not supported");
    expectError("qualified name", "namespace n { int v; } int main() { return n::v; }",
                "qualified names are not supported");
    expectError("incomplete struct by value",
                "struct Node; int main() { struct Node node; return 0; }",
                "has incomplete type struct Node");
    expectError("reference to a value", "int f(int& r); int main() { return f(1); }",
                "needs a variable");
    expectError("assign to const", "int main() { const int x = 1; x = 2; return x; }",
                "a const value cannot be assigned to");
    expectError("increment const", "int main() { const int x = 1; ++x; return x; }",
                "a const value cannot be changed");
    expectCompiles("const read", "int main() { const int x = 3; return x; }");
    expectError("missing include", "#include \"nope.h\"\nint main() { return 0; }",
                "cannot find header 'nope.h'");

    // A quoted include is found beside the source or on the search path, and
    // #pragma once keeps a header from being lexed twice.
    {
        std::error_code ignored;
        const std::filesystem::path directory =
            std::filesystem::temp_directory_path() / "rblxcc-include-case";
        std::filesystem::remove_all(directory, ignored);
        std::filesystem::create_directories(directory, ignored);
        {
            std::ofstream header(directory / "shapes.h");
            header << "#pragma once\nstruct Box { int side; };\nint box_area(struct Box* box);\n";
        }
        {
            std::ofstream header(directory / "legacy.h");
            header << "#ifndef LEGACY_H\n#define LEGACY_H\n#include \"shapes.h\"\n#endif\n";
        }
        rblx::cxx::Options options;
        options.sourceName = (directory / "main.cpp").string();
        options.includePaths.push_back(directory.string());
        const std::string source =
            "#include \"shapes.h\"\n"
            "#include \"shapes.h\"\n"
            "#include \"legacy.h\"\n"
            "int box_area(struct Box* box) { return box->side * box->side; }\n"
            "int main() { struct Box b; b.side = 3; printf(\"%d\\n\", box_area(&b)); return 0; }\n";
        const rblx::cxx::Result compiled = rblx::cxx::compile(source, options);
        ++checks;
        if (!compiled.ok) {
            ++failures;
            std::printf("FAIL include: %s\n", firstMessage(compiled).c_str());
        }
    }
    expectCompiles("unsigned integers",
                   "int main() { unsigned int x = 1; unsigned char c = 2; unsigned long l = 3; "
                   "return x + c + (int)l; }");
    expectError("unsigned floating point", "int main() { unsigned double d = 1.0; return 0; }",
                "never signed or unsigned");
    expectError("signed and unsigned", "int main() { signed unsigned int x = 1; return x; }",
                "both signed and unsigned");
    expectCompiles("floating point", "int main() { double x = 1.5; return x > 1.0; }\n");
    expectError("remainder of floats",
                "int main() { double a = 1.0; double b = 2.0; return a % b; }", "needs integers");
    expectError("bits of floats", "int main() { double a = 1.0; return a << 1; }",
                "needs integers");
    expectError("long double", "int main() { long double x = 1.0; return 0; }",
                "long double is not supported");
    expectError("pointer from double",
                "int main() { double d = 1.0; int* p = (int*)d; return 0; }",
                "cannot become a pointer");
    expectError("struct by value parameter",
                "struct P { int x; }; int take(struct P p) { return p.x; } int main() { return 0; }",
                "pass a struct by pointer");
    expectError("array assignment", "int main() { int a[3]; int b[3]; a = b; return 0; }",
                "an array cannot be assigned");
    expectError("break outside a loop", "int main() { break; return 0; }", "'break' outside a loop");
    // A function name is its own address now, so it can be a pointer but not a
    // number without a cast.
    expectError("function pointer into an int",
                "int helper() { return 1; } int main() { int x = helper; return x; }",
                "cannot put a pointer into int without a cast");
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

    // Floats are a separate register class, so this checks the parts that only
    // show up once a real program runs: printf reading a double from %xmm0, the
    // %al vector count, conversions, comparisons and ++ on a double.
    {"floating point",
     R"CPP(
double g = 2.5;
double scale(double x, int n) { return x * n; }
int main() {
    double a = 1.5;
    double b = 2.0;
    double c = a * b;
    printf("%.2f\n", c);
    printf("%d %.1f\n", 2, scale(1.5, 2));
    float f = 1.5f;
    f = f + 0.5f;
    printf("%.1f\n", (double)f);
    int i = (int)3.9;
    double d = i;
    printf("%d %.1f\n", i, d);
    d++;
    printf("%.1f\n", d);
    printf("%d%d%d\n", 1.5 < 2.0, 2.5 == 2.5, 2.5 != 2.5);
    printf("%d\n", !0.0);
    if (g > 2.0) printf("global\n");
    return c == 3.0 ? 0 : 1;
}
)CPP",
     "3.00\n2 3.0\n2.0\n3 3.0\n4.0\n110\n1\nglobal\n"},

    // Signedness is not cosmetic: it decides the division instruction, the shift
    // and the comparison condition, so every line below would print a different
    // answer if the type were read as signed.
    {"unsigned and short",
     R"CPP(
unsigned int g = 3000000000u;
unsigned int wrap(unsigned int value) { return value * 2u; }
short half(short value) { return value / 2; }
int main() {
    printf("%u\n", g);
    unsigned int a = 3000000000u;
    printf("%u\n", a);
    printf("%u\n", wrap(a));
    unsigned char c = 250;
    c = c + 10;
    printf("%d\n", (int)c);
    unsigned char wide = 200;
    printf("%d\n", wide + 100);
    short s = -5;
    unsigned short us = 65535;
    printf("%d %d\n", (int)s, (int)us);
    us = us + 1;
    printf("%d\n", (int)us);
    unsigned int neg = 0;
    neg = neg - 8;
    printf("%u %u %u\n", neg, neg / 2u, neg % 10u);
    printf("%u\n", neg >> 28);
    printf("%d\n", neg > 2);
    printf("%d %d\n", -1 < 1u, -1 > 1u);
    int negative = -16;
    printf("%d %d\n", negative >> 2, negative / 4);
    unsigned long ul = 0;
    ul = ul - 1;
    printf("%lu\n", ul);
    long l = -1;
    printf("%d\n", l < 1ul);
    printf("%d\n", half(-7) + (int)sizeof(unsigned short));
    return 0;
}
)CPP",
     "3000000000\n3000000000\n1705032704\n4\n300\n-5 65535\n0\n"
     "4294967288 2147483644 8\n15\n1\n0 1\n-4 -4\n18446744073709551615\n0\n-1\n"},

    // Member functions: the object travels as a hidden first argument, and a
    // field or another member named inside a body is reached through it.
    {"classes with methods",
     R"CPP(
struct Rect {
    int width;
    int height;
    public:
    int area();
    int perimeter() { return 2 * (width + height); }
    void grow(int by) { width = width + by; height = height + by; }
};
int Rect::area() { return width * height; }
int main() {
    struct Rect r;
    r.width = 3;
    r.height = 4;
    printf("%d %d\n", r.area(), r.perimeter());
    r.grow(1);
    printf("%d %d\n", r.width, r.area());
    struct Rect* p = &r;
    printf("%d\n", p->area());
    p->grow(2);
    printf("%d\n", p->area());
    return 0;
}
)CPP",
     "12 14\n4 20\n20\n42\n"},

    // A static local lives as long as the program but only its function can name
    // it, so two functions may each have a `calls` and neither can see the other.
    {"static locals",
     R"CPP(
int bump() {
    static int calls = 0;
    calls = calls + 1;
    return calls;
}
int other() {
    static int calls = 10;
    calls = calls + 1;
    return calls;
}
int main() {
    bump();
    bump();
    other();
    printf("%d %d\n", bump(), other());
    static double scale = 2.5;
    scale = scale * 2.0;
    printf("%.1f\n", scale);
    static char* label = "held";
    printf("%s\n", label);
    return 0;
}
)CPP",
     "3 12\n5.0\nheld\n"},

    // The tier-one language additions read together: a macro that takes
    // arguments, a #if that is evaluated, a typedef of an anonymous struct, an
    // enum, a class and a union.
    {"typedef, enum, union, class and function macros",
     R"CPP(
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define SQUARE(x) ((x) * (x))
#define LIMIT 3
#if LIMIT > 2 && defined(LIMIT)
int gate() { return 1; }
#elif LIMIT == 2
int gate() { return 2; }
#else
int gate() { return 3; }
#endif
typedef unsigned long Count;
typedef struct { int x; int y; } Point;
enum Color { RED, GREEN = 5, BLUE };
class Counter {
public:
    int value;
    void bump() { value = value + 1; }
};
union Bits { int whole; unsigned char bytes[4]; };
int main() {
    Count total = 0;
    Point p;
    p.x = 2;
    p.y = 3;
    total = p.x + p.y;
    printf("%lu\n", total);
    printf("%d %d %d\n", RED, GREEN, BLUE);
    printf("%d\n", gate());
    struct Counter c;
    c.value = 0;
    c.bump();
    c.bump();
    printf("%d\n", c.value);
    union Bits bits;
    bits.whole = 0;
    bits.bytes[0] = 1;
    bits.bytes[1] = 2;
    printf("%d\n", bits.whole);
    printf("%d %d\n", MAX(3, 7), SQUARE(LIMIT + 1));
    enum Color color = GREEN;
    printf("%d\n", color);
    return 0;
}
)CPP",
     "5\n0 5 6\n1\n2\n513\n7 16\n5\n"},

    // switch keeps its cases where they are, so one case falls into the next; goto
    // jumps by name, auto takes the initialiser's type, and static_assert is
    // decided while compiling.
    {"switch, goto, auto and static_assert",
     R"CPP(
int classify(int n) {
    switch (n) {
        case 1:
        case 2:
            return 10;
        case 3:
            return 30;
        default:
            return 0;
    }
}
int main() {
    printf("%d %d %d %d\n", classify(1), classify(2), classify(3), classify(9));
    int total = 0;
    int i = 0;
again:
    total = total + i;
    i = i + 1;
    if (i < 4) goto again;
    printf("%d\n", total);
    switch (2) {
        case 1: printf("one\n"); break;
        case 2: printf("two\n");
        case 3: printf("three\n"); break;
        default: printf("other\n");
    }
    auto value = 3;
    static_assert(sizeof(int) == 4, "int is four bytes");
    printf("%d\n", value);
    return 0;
}
)CPP",
     "10 10 30 0\n6\ntwo\nthree\n3\n"},

    // References are pointers the compiler dereferences, a forward declaration is
    // filled in where the struct is defined, and braces initialise an array or a
    // struct element by element.
    {"references, forward declarations, decltype, namespaces and braces",
     R"CPP(
struct Node;
struct Node {
    int value;
    struct Node* next;
};
struct Pair { int first; int second; };
namespace shapes {
    int side = 4;
    int area() { return side * side; }
}
int bump(int& cell) {
    cell = cell + 1;
    return cell;
}
int swapSum(int& a, int& b) {
    int t = a;
    a = b;
    b = t;
    return a * 10 + b;
}
int main() {
    int x = 1;
    int& alias = x;
    alias = 7;
    printf("%d\n", alias);
    printf("%d\n", bump(alias));
    int p = 3;
    int q = 4;
    int sum = swapSum(p, q);
    printf("%d %d %d\n", sum, p, q);
    int values[3] = {1, 2, 3};
    printf("%d\n", values[0] + values[1] + values[2]);
    struct Node a;
    a.value = 5;
    a.next = 0;
    struct Node b;
    b.value = 6;
    b.next = &a;
    printf("%d\n", b.next->value);
    printf("%d\n", area());
    struct Pair pair = {3, 4};
    printf("%d\n", pair.first + pair.second);
    decltype(x) copy = 9;
    printf("%d\n", copy);
    int zeroed[2] = {};
    printf("%d %d\n", zeroed[0], zeroed[1]);
    return 0;
}
)CPP",
     "7\n8\n43 4 3\n6\n5\n16\n7\n9\n0 0\n"},

    // A function pointer is a variable holding an address, and calling it is an
    // indirect call through that address.
    {"function pointers",
     R"CPP(
int step(int value) { return value + 1; }
int twice(int value) { return value * 2; }
int apply(int (*fn)(int), int value) { return fn(value); }
int main() {
    int (*op)(int) = &step;
    printf("%d\n", op(4));
    op = &twice;
    printf("%d\n", op(4));
    printf("%d\n", apply(step, 7));
    printf("%d\n", apply(twice, 7));
    int (*table)(int) = op;
    printf("%d\n", table(10));
    return 0;
}
)CPP",
     "5\n8\n8\n14\n20\n"},
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

std::string quoted(const std::string& path) { return "\"" + path + "\""; }

std::string runAndCapture(const std::string& command) {
    std::string output;
    FILE* pipe = RBLX_POPEN(command.c_str(), "r");
    if (!pipe) return output;
    char buffer[512];
    std::size_t read = 0;
    while ((read = std::fread(buffer, 1, sizeof(buffer), pipe)) > 0) {
        output.append(buffer, read);
    }
    RBLX_PCLOSE(pipe);
    return output;
}

bool writeText(const std::filesystem::path& path, const std::string& text) {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) return false;
    file << text;
    return true;
}

// A program is normally more than one file, so the driver has to compile each
// translation unit on its own and link the objects. These tests drive the real
// binary, which is also the only way to check that two files may each define
// their own `static` helper without the linker complaining.
void driverTests(const std::string& driver) {
    if (driver.empty()) {
        std::printf(
            "cxx_compiler_tests: no RBLXCC path given, so the multi-file driver tests were not "
            "run\n");
        return;
    }
    if (hostCCompiler().empty()) {
        std::printf("cxx_compiler_tests: no cc/gcc/clang on PATH, so the driver tests were not run\n");
        return;
    }

    std::filesystem::path directory =
        std::filesystem::temp_directory_path() / "rblxcc-driver-tests";
    std::error_code ignored;
    std::filesystem::remove_all(directory, ignored);
    std::filesystem::create_directories(directory, ignored);

    const std::filesystem::path first = directory / "first.cpp";
    const std::filesystem::path second = directory / "second.cpp";
    writeText(first,
              "int printf(char* format, ...);\n"
              "int second(int value);\n"
              "static int helper(int value) { return value + 1; }\n"
              "int main() {\n"
              "    printf(\"first=%d second=%d\\n\", helper(1), second(2));\n"
              "    return 0;\n"
              "}\n");
    writeText(second,
              "static int helper(int value) { return value * 10; }\n"
              "int second(int value) { return helper(value); }\n");

#if defined(_WIN32)
    const std::filesystem::path program = directory / "linked.exe";
#else
    const std::filesystem::path program = directory / "linked";
#endif

    const std::string link = quoted(driver) + " " + quoted(first.string()) + " " +
                             quoted(second.string()) + " -o " + quoted(program.string());
    CHECK(std::system(link.c_str()) == 0);
    CHECK(runAndCapture(quoted(program.string())) == "first=2 second=20\n");

    const std::filesystem::path assembly = directory / "second.s";
    const std::string assemblyOnly =
        quoted(driver) + " " + quoted(second.string()) + " -S -o " + quoted(assembly.string());
    CHECK(std::system(assemblyOnly.c_str()) == 0);
    CHECK(std::filesystem::exists(assembly));
    {
        std::string text;
        std::ifstream file(assembly, std::ios::binary);
        std::string line;
        while (std::getline(file, line)) text += line + "\n";
        CHECK(contains(text, "second:"));
        // A translator has no main, so nothing here may claim to be one.
        CHECK(!contains(text, ".globl main"));
    }

    const std::filesystem::path object = directory / "second.o";
    const std::string objectOnly =
        quoted(driver) + " " + quoted(second.string()) + " -c -o " + quoted(object.string());
    CHECK(std::system(objectOnly.c_str()) == 0);
    CHECK(std::filesystem::exists(object));
    CHECK(std::filesystem::exists(object) && std::filesystem::file_size(object) > 0);

    std::filesystem::remove_all(directory, ignored);
}

}  // namespace

int main(int argc, char** argv) {
    diagnosticsTests();
    assemblyTests();
    endToEndTests();
    driverTests(argc > 1 ? std::string(argv[1]) : std::string());

    std::printf("cxx_compiler_tests: %d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
