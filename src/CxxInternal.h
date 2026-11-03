#pragma once

// Shared guts of RBLXCC: tokens, the type model, the AST, and the three stages
// that work on them. Everything here is internal to the compiler, so it lives
// beside the sources instead of in include/ with the public headers.

#include "CxxCompiler.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rblx {
namespace cxx {
namespace detail {

// ---------------------------------------------------------------- tokens ----

enum class TokenKind { End, Identifier, Number, String, Character, Punctuation };

struct Token {
    TokenKind kind = TokenKind::End;
    std::string text;    // spelling for identifiers and punctuation
    long long number = 0;  // value of an integer or character literal
    double real = 0;       // value of a floating point literal
    bool floating = false;  // the literal had a fraction or an exponent
    bool floatSuffix = false;  // ... and an f suffix, so its type is float
    bool unsignedLiteral = false;  // ... and a u suffix, so it is unsigned
    bool longLiteral = false;      // ... and an l suffix, so it is long
    std::string string;  // decoded contents of a string literal
    int line = 0;
    int column = 0;

    bool is(const char* value) const { return text == value; }
    bool isKind(TokenKind kindValue) const { return kind == kindValue; }
};

struct LexResult {
    std::vector<Token> tokens;
    std::vector<Diagnostic> diagnostics;
};

// `prelude` is lexed first with line 0, so a diagnostic that points into it is
// recognisable as the compiler's own doing rather than the user's.
LexResult lex(const std::string& source, const std::string& prelude);

// The C library declarations the compiler knows about (printf and friends).
std::string defaultPreludeText();

// ----------------------------------------------------------------- types ----

enum class TypeKind { Void, Bool, Char, Short, Int, Long, Float, Double, Pointer, Array, Struct };

struct Type;
using TypePtr = std::shared_ptr<Type>;

struct Field {
    std::string name;
    TypePtr type;
    int offset = 0;
};

struct Type {
    TypeKind kind = TypeKind::Int;
    TypePtr element;         // pointee or element type
    long long elements = 0;  // array length
    std::string name;        // struct tag
    std::vector<Field> fields;
    int size = 4;            // sizeof, in bytes
    int align = 4;           // alignment, in bytes
    bool isUnsigned = false;  // an unsigned integer: char, short, int or long

    bool isVoid() const { return kind == TypeKind::Void; }
    // Types narrower than int, which arithmetic promotes to int before it does
    // anything else.
    bool isSmallInteger() const {
        return kind == TypeKind::Bool || kind == TypeKind::Char || kind == TypeKind::Short;
    }
    bool isInteger() const {
        return kind == TypeKind::Bool || kind == TypeKind::Char || kind == TypeKind::Short ||
               kind == TypeKind::Int || kind == TypeKind::Long;
    }
    bool isFloating() const { return kind == TypeKind::Float || kind == TypeKind::Double; }
    bool isPointer() const { return kind == TypeKind::Pointer; }
    bool isArray() const { return kind == TypeKind::Array; }
    bool isBool() const { return kind == TypeKind::Bool; }
    // Arrays behave as pointers to their first element in every expression, and
    // pointer arithmetic on them is scaled the same way.
    bool isPointerLike() const { return isPointer() || isArray(); }
    bool isStruct() const { return kind == TypeKind::Struct; }
    // Anything that can be held in a register: integers, floating point values
    // and pointers.
    bool isScalar() const { return isInteger() || isFloating() || isPointer(); }
    // Arrays and structs name storage; the rest are values.
    bool isAggregate() const { return isArray() || isStruct(); }
};

TypePtr primitiveType(TypeKind kind);
TypePtr integerType(TypeKind kind, bool isUnsigned);
// Integral promotion: char, short and bool become int.
TypePtr promotedType(const TypePtr& type);
// The usual arithmetic conversions: the type both operands are computed in.
TypePtr arithmeticType(const TypePtr& left, const TypePtr& right);
TypePtr pointerType(const TypePtr& element);
TypePtr arrayType(const TypePtr& element, long long elements);
std::string typeName(const TypePtr& type);

// ------------------------------------------------------------------- AST ----

enum class ExprKind {
    Integer,
    Floating,  // number is the index into Program::reals
    Character,
    String,
    Name,
    Function,    // a function name; only valid as the target of a call
    Unary,       // text is the operator: - + ! ~ * & ++ --
    Postfix,     // text is ++ or --
    Binary,
    Assign,      // text is = += -= *= /= %= &= |= ^= <<= >>=
    Conditional, // a ? b : c
    Call,        // a is the callee, args the arguments
    Index,       // a[b]
    Member,      // a.text, or a->text when arrow
    Cast,        // (type)a
    SizeOf       // sizeof: type is the operand type, number the result
};

struct Expr;
using ExprPtr = std::unique_ptr<Expr>;

struct Expr {
    ExprKind kind = ExprKind::Integer;
    int line = 0;
    int column = 0;

    long long number = 0;  // integer/character literal value, sizeof result
    double real = 0;       // floating point literal value
    std::string text;      // operator, name, member name
    std::string string;    // string literal contents, pooled in Program::strings

    TypePtr type;          // result type, filled in by the parser
    bool lvalue = false;   // can be assigned to / have its address taken
    bool globalRef = false;  // a Name that resolves to a global
    bool arrow = false;      // Member through a pointer
    int offset = 0;          // stack offset for a local Name
    int memberIndex = -1;    // index into the struct's fields

    ExprPtr a;
    ExprPtr b;
    ExprPtr c;
    std::vector<ExprPtr> args;
};

struct LocalDecl {
    std::string name;
    TypePtr type;
    int offset = 0;
    ExprPtr init;
};

enum class StmtKind {
    Empty,
    Block,
    Declaration,
    Expression,
    If,
    While,
    DoWhile,
    For,
    Return,
    Break,
    Continue
};

struct Stmt;
using StmtPtr = std::unique_ptr<Stmt>;

struct Stmt {
    StmtKind kind = StmtKind::Empty;
    int line = 0;
    int column = 0;

    std::vector<StmtPtr> body;      // Block
    std::vector<LocalDecl> declarations;  // Declaration
    ExprPtr expr;                   // Expression, Return value, condition
    StmtPtr init;                   // For initialiser (declaration or expression)
    ExprPtr step;                   // For increment
    StmtPtr thenBranch;
    StmtPtr elseBranch;
};

// --------------------------------------------------------------- program ----

struct Parameter {
    std::string name;
    TypePtr type;
    int offset = 0;
};

struct FunctionDecl {
    std::string name;
    TypePtr result;
    std::vector<Parameter> parameters;
    bool variadic = false;
    bool defined = false;
    bool internal = false;  // static: not exported
    int line = 0;
};

struct FunctionDef {
    FunctionDecl* declaration = nullptr;
    StmtPtr body;
    int localBytes = 0;  // stack reserved for locals and parameters
    int parameterCount = 0;
};

struct GlobalDef {
    std::string name;
    TypePtr type;
    bool internal = false;
    bool hasInit = false;
    // `extern int g;` only says where to find g, so the compiler refers to the
    // symbol without defining storage for it.
    bool external = false;
    bool stringInit = false;
    long long intInit = 0;
    bool realInit = false;
    double realValue = 0;
    std::string stringLabel;
    int line = 0;
};

// A floating point literal, pooled because there is no instruction that loads an
// immediate double into an %xmm register: it has to come from .rodata.
struct RealLiteral {
    double value = 0;
    bool isFloat = false;  // an f suffix: stored and loaded as a 4-byte float
};

struct Program {
    std::map<std::string, std::unique_ptr<FunctionDecl>> functions;
    std::vector<FunctionDef> definitions;
    std::vector<GlobalDef> globals;
    std::vector<std::string> strings;  // string literal pool, index = label number
    std::vector<RealLiteral> reals;    // floating point literal pool
};

struct ParseResult {
    std::unique_ptr<Program> program;
    std::vector<Diagnostic> diagnostics;
};

ParseResult parse(const std::vector<Token>& tokens, const Options& options);

struct GenerateResult {
    std::string assembly;
    std::vector<Diagnostic> diagnostics;
};

GenerateResult generate(const Program& program, const Options& options);

}  // namespace detail
}  // namespace cxx
}  // namespace rblx
