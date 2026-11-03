#include "CxxInternal.h"

#include <cctype>
#include <cstdlib>
#include <utility>

namespace rblx {
namespace cxx {
namespace detail {

namespace {

// Declarations only: the examples print with printf, and a compiler that cannot
// call the platform's own libc is not much use for programs you can run. These
// are ordinary declarations, so a program that writes its own copy is fine.
const char* kPrelude =
    "int printf(char* format, ...);\n"
    "int puts(char* text);\n"
    "int putchar(int value);\n"
    "void exit(int code);\n"
    "void* malloc(long size);\n"
    "void free(void* memory);\n"
    "long strlen(char* text);\n"
    "int strcmp(char* left, char* right);\n";

bool isIdentifierStart(char c) {
    return std::isalpha(static_cast<unsigned char>(c)) || c == '_';
}

bool isIdentifierPart(char c) {
    return std::isalnum(static_cast<unsigned char>(c)) || c == '_';
}

int digitValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
    if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
    return -1;
}

struct Conditional {
    bool parentActive = true;
    bool active = true;
    bool elseSeen = false;
    int line = 0;
};

class Lexer {
public:
    Lexer(const std::string& text, int firstLine, std::map<std::string, std::string>* macros,
          std::vector<Token>* tokens, std::vector<Diagnostic>* diagnostics)
        : source_(text), line_(firstLine), macros_(macros), tokens_(tokens), diagnostics_(diagnostics) {}

    void run(int depth) {
        if (depth > 16) {
            error("macro expansion nests too deeply", line_, column_);
            return;
        }

        bool atLineStart = true;
        while (position_ < source_.size()) {
            char c = source_[position_];

            if (c == '\n') {
                ++position_;
                ++line_;
                column_ = 1;
                atLineStart = true;
                continue;
            }
            if (c == ' ' || c == '\t' || c == '\r' || c == '\f' || c == '\v') {
                ++position_;
                ++column_;
                continue;
            }
            if (c == '/' && peek(1) == '/') {
                while (position_ < source_.size() && source_[position_] != '\n') {
                    ++position_;
                    ++column_;
                }
                continue;
            }
            if (c == '/' && peek(1) == '*') {
                skipBlockComment();
                continue;
            }
            // A '#' only starts a directive where a directive can start.
            if (atLineStart && c == '#') {
                directive();
                continue;
            }

            if (!active()) {
                // Inside a disabled #ifdef branch nothing is turned into tokens,
                // but comments and line ends still have to be understood above.
                ++position_;
                ++column_;
                continue;
            }

            atLineStart = false;
            int tokenLine = line_;
            int tokenColumn = column_;

            if (isIdentifierStart(c)) {
                identifier(tokenLine, tokenColumn, depth);
                continue;
            }
            if (std::isdigit(static_cast<unsigned char>(c))) {
                number(tokenLine, tokenColumn);
                continue;
            }
            if (c == '"') {
                quoted(tokenLine, tokenColumn, true);
                continue;
            }
            if (c == '\'') {
                quoted(tokenLine, tokenColumn, false);
                continue;
            }
            punctuation(tokenLine, tokenColumn);
        }

        if (!conditionals_.empty()) {
            error("#if or #ifdef without a matching #endif", conditionals_.back().line, 1);
        }
    }

private:
    char peek(std::size_t ahead) const {
        std::size_t index = position_ + ahead;
        return index < source_.size() ? source_[index] : '\0';
    }

    bool active() const { return conditionals_.empty() || conditionals_.back().active; }

    void error(const std::string& message, int line, int column) {
        Diagnostic diagnostic;
        diagnostic.line = line;
        diagnostic.column = column;
        diagnostic.message = message;
        diagnostics_->push_back(diagnostic);
    }

    void add(TokenKind kind, int line, int column) {
        Token token;
        token.kind = kind;
        token.line = line;
        token.column = column;
        tokens_->push_back(std::move(token));
        last_ = tokens_->size() - 1;
    }

    void skipBlockComment() {
        int startLine = line_;
        int startColumn = column_;
        position_ += 2;
        column_ += 2;
        while (position_ < source_.size()) {
            if (source_[position_] == '*' && peek(1) == '/') {
                position_ += 2;
                column_ += 2;
                return;
            }
            if (source_[position_] == '\n') {
                ++position_;
                ++line_;
                column_ = 1;
                continue;
            }
            ++position_;
            ++column_;
        }
        error("unterminated /* comment", startLine, startColumn);
    }

    // Reads the directive word and its argument, then applies it.
    void directive() {
        int directiveLine = line_;
        std::size_t start = position_;
        while (position_ < source_.size() && source_[position_] != '\n') {
            ++position_;
            ++column_;
        }
        std::string text = source_.substr(start, position_ - start);

        std::size_t cursor = 1;  // skip '#'
        while (cursor < text.size() && (text[cursor] == ' ' || text[cursor] == '\t')) ++cursor;
        std::size_t wordStart = cursor;
        while (cursor < text.size() && isIdentifierPart(text[cursor])) ++cursor;
        std::string word = text.substr(wordStart, cursor - wordStart);
        std::string rest = text.substr(cursor);
        while (!rest.empty() && (rest.front() == ' ' || rest.front() == '\t')) rest.erase(rest.begin());
        while (!rest.empty() && (rest.back() == ' ' || rest.back() == '\t' || rest.back() == '\r')) {
            rest.pop_back();
        }

        if (word == "define") {
            if (!active()) return;
            std::size_t nameEnd = 0;
            while (nameEnd < rest.size() && isIdentifierPart(rest[nameEnd])) ++nameEnd;
            std::string name = rest.substr(0, nameEnd);
            if (name.empty()) {
                error("#define needs a name", directiveLine, 1);
                return;
            }
            if (nameEnd < rest.size() && rest[nameEnd] == '(') {
                error("function-like macros are not supported; write the steps out instead",
                      directiveLine, 1);
                return;
            }
            std::string value = rest.substr(nameEnd);
            while (!value.empty() && (value.front() == ' ' || value.front() == '\t')) {
                value.erase(value.begin());
            }
            (*macros_)[name] = value;
            return;
        }
        if (word == "undef") {
            if (!active()) return;
            macros_->erase(rest);
            return;
        }
        if (word == "ifdef" || word == "ifndef") {
            Conditional frame;
            frame.parentActive = active();
            bool defined = macros_->find(rest) != macros_->end();
            bool wanted = (word == "ifdef") ? defined : !defined;
            frame.active = frame.parentActive && wanted;
            frame.line = directiveLine;
            conditionals_.push_back(frame);
            return;
        }
        if (word == "else") {
            if (conditionals_.empty()) {
                error("#else without #if", directiveLine, 1);
                return;
            }
            Conditional& frame = conditionals_.back();
            if (frame.elseSeen) error("a second #else in one #if", directiveLine, 1);
            frame.elseSeen = true;
            frame.active = frame.parentActive && !frame.active;
            return;
        }
        if (word == "endif") {
            if (conditionals_.empty()) {
                error("#endif without #if", directiveLine, 1);
                return;
            }
            conditionals_.pop_back();
            return;
        }
        if (word == "if") {
            error("#if is not supported; use #ifdef or #ifndef", directiveLine, 1);
            return;
        }
        // #include, #pragma and the rest: there is no header search path here, so
        // they are ignored rather than refused. The prelude already declares the
        // C library functions a small program needs.
    }

    void identifier(int tokenLine, int tokenColumn, int depth) {
        std::size_t start = position_;
        while (position_ < source_.size() && isIdentifierPart(source_[position_])) {
            ++position_;
            ++column_;
        }
        std::string name = source_.substr(start, position_ - start);

        auto macro = macros_->find(name);
        bool expanding = false;
        for (const std::string& pending : expanding_) {
            if (pending == name) expanding = true;
        }
        if (macro != macros_->end() && !expanding) {
            expanding_.push_back(name);
            Lexer nested(macro->second, tokenLine, macros_, tokens_, diagnostics_);
            nested.column_ = tokenColumn;
            nested.run(depth + 1);
            expanding_.pop_back();
            return;
        }

        add(TokenKind::Identifier, tokenLine, tokenColumn);
        tokens_->back().text = name;
    }

    // A decimal literal with a fraction or an exponent is a floating point
    // literal. Returns true and leaves `cursor` after the suffix when it is one.
    bool scanFloat(std::size_t start, std::size_t& cursor, bool& floatSuffix) const {
        if (start + 1 < source_.size() && source_[start] == '0' &&
            (source_[start + 1] == 'x' || source_[start + 1] == 'X' || source_[start + 1] == 'b' ||
             source_[start + 1] == 'B')) {
            return false;  // a base-prefixed number is always an integer
        }
        std::size_t position = start;
        bool digits = false;
        while (position < source_.size() &&
               std::isdigit(static_cast<unsigned char>(source_[position]))) {
            digits = true;
            ++position;
        }
        bool fraction = false;
        if (position < source_.size() && source_[position] == '.' &&
            source_.compare(position, 2, "..") != 0) {
            fraction = true;
            ++position;
            while (position < source_.size() &&
                   std::isdigit(static_cast<unsigned char>(source_[position]))) {
                ++position;
            }
        }
        bool exponent = false;
        if (position < source_.size() && (source_[position] == 'e' || source_[position] == 'E')) {
            std::size_t after = position + 1;
            if (after < source_.size() && (source_[after] == '+' || source_[after] == '-')) ++after;
            if (after < source_.size() &&
                std::isdigit(static_cast<unsigned char>(source_[after]))) {
                exponent = true;
                position = after;
                while (position < source_.size() &&
                       std::isdigit(static_cast<unsigned char>(source_[position]))) {
                    ++position;
                }
            }
        }
        if (!digits || (!fraction && !exponent)) return false;

        floatSuffix = false;
        if (position < source_.size() && (source_[position] == 'f' || source_[position] == 'F')) {
            floatSuffix = true;
            ++position;
        }
        cursor = position;
        return true;
    }

    void number(int tokenLine, int tokenColumn) {
        std::size_t start = position_;
        std::size_t floatEnd = 0;
        bool floatSuffix = false;
        if (scanFloat(start, floatEnd, floatSuffix)) {
            if (floatEnd < source_.size() && (source_[floatEnd] == 'l' || source_[floatEnd] == 'L')) {
                error("long double is not supported; use double", tokenLine, tokenColumn);
                ++floatEnd;
            }
            const std::string spelling = source_.substr(start, floatEnd - start);
            const double value = std::strtod(spelling.c_str(), nullptr);
            column_ += static_cast<int>(floatEnd - position_);
            position_ = floatEnd;
            add(TokenKind::Number, tokenLine, tokenColumn);
            tokens_->back().real = value;
            tokens_->back().floating = true;
            tokens_->back().floatSuffix = floatSuffix;
            tokens_->back().text = spelling;
            return;
        }

        int base = 10;
        std::size_t cursor = position_;

        if (source_[cursor] == '0' && (peek(1) == 'x' || peek(1) == 'X')) {
            base = 16;
            cursor += 2;
        } else if (source_[cursor] == '0' && (peek(1) == 'b' || peek(1) == 'B')) {
            base = 2;
            cursor += 2;
        } else if (source_[cursor] == '0' && std::isdigit(static_cast<unsigned char>(peek(1)))) {
            base = 8;
            cursor += 1;
        }

        long long value = 0;
        bool digits = false;
        std::size_t digitStart = cursor;
        while (cursor < source_.size()) {
            int digit = digitValue(source_[cursor]);
            if (digit < 0 || digit >= base) break;
            value = value * base + digit;
            digits = true;
            ++cursor;
        }
        if (!digits && base != 10) {
            error("a number with a base prefix needs digits", tokenLine, tokenColumn);
        }
        if (!digits && base == 10) {
            // A lone '0' is base 10 zero.
            value = 0;
            digits = true;
            if (cursor == digitStart) ++cursor;
        }
        // Integer suffixes say nothing this compiler needs: every integer is
        // 64-bit in a register, and the declared type decides what is stored.
        while (cursor < source_.size()) {
            char c = source_[cursor];
            if (c == 'u' || c == 'U' || c == 'l' || c == 'L') {
                ++cursor;
                continue;
            }
            break;
        }

        column_ += static_cast<int>(cursor - position_);
        position_ = cursor;
        add(TokenKind::Number, tokenLine, tokenColumn);
        tokens_->back().number = value;
        tokens_->back().text = source_.substr(start, cursor - start);
    }

    void quoted(int tokenLine, int tokenColumn, bool isString) {
        char quote = source_[position_];
        (void)isString;
        ++position_;
        ++column_;
        std::string value;
        bool closed = false;
        while (position_ < source_.size()) {
            char c = source_[position_];
            if (c == quote) {
                ++position_;
                ++column_;
                closed = true;
                break;
            }
            if (c == '\n') break;  // a literal cannot span lines
            if (c == '\\') {
                value.push_back(escape());
                continue;
            }
            value.push_back(c);
            ++position_;
            ++column_;
        }
        if (!closed) {
            error(isString ? "unterminated string literal" : "unterminated character literal",
                  tokenLine, tokenColumn);
        }
        if (isString) {
            add(TokenKind::String, tokenLine, tokenColumn);
            tokens_->back().string = value;
        } else {
            add(TokenKind::Character, tokenLine, tokenColumn);
            tokens_->back().number = value.empty() ? 0 : static_cast<unsigned char>(value[0]);
            if (value.size() > 1) {
                error("a character literal holds one character", tokenLine, tokenColumn);
            }
        }
    }

    // Decodes one backslash escape, leaving position_ after it.
    char escape() {
        ++position_;
        ++column_;
        if (position_ >= source_.size()) return '\\';
        char c = source_[position_];
        ++position_;
        ++column_;
        switch (c) {
            case 'n': return '\n';
            case 't': return '\t';
            case 'r': return '\r';
            case '0': return '\0';
            case 'a': return '\a';
            case 'b': return '\b';
            case 'f': return '\f';
            case 'v': return '\v';
            case '\\': return '\\';
            case '\'': return '\'';
            case '"': return '"';
            case '?': return '?';
            case 'x': {
                int value = 0;
                int digits = 0;
                while (digits < 2 && position_ < source_.size() &&
                       digitValue(source_[position_]) >= 0) {
                    value = value * 16 + digitValue(source_[position_]);
                    ++position_;
                    ++column_;
                    ++digits;
                }
                return static_cast<char>(value);
            }
            default: return c;
        }
    }

    void punctuation(int tokenLine, int tokenColumn) {
        static const char* kThree[] = {"<<=", ">>=", "..."};
        static const char* kTwo[] = {"==", "!=", "<=", ">=", "&&", "||", "++", "--", "+=",
                                     "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<", ">>",
                                     "->"};
        for (const char* candidate : kThree) {
            if (source_.compare(position_, 3, candidate) == 0) {
                position_ += 3;
                column_ += 3;
                add(TokenKind::Punctuation, tokenLine, tokenColumn);
                tokens_->back().text = candidate;
                return;
            }
        }
        for (const char* candidate : kTwo) {
            if (source_.compare(position_, 2, candidate) == 0) {
                position_ += 2;
                column_ += 2;
                add(TokenKind::Punctuation, tokenLine, tokenColumn);
                tokens_->back().text = candidate;
                return;
            }
        }
        static const std::string kSingle = "+-*/%<>=!&|^~?:;,.(){}[]";
        char c = source_[position_];
        if (kSingle.find(c) == std::string::npos) {
            error(std::string("unexpected character '") + c + "' in the source", tokenLine,
                  tokenColumn);
        }
        ++position_;
        ++column_;
        add(TokenKind::Punctuation, tokenLine, tokenColumn);
        tokens_->back().text = std::string(1, c);
    }

    const std::string& source_;
    std::size_t position_ = 0;
    int line_ = 1;
    int column_ = 1;
    std::map<std::string, std::string>* macros_;
    std::vector<Token>* tokens_;
    std::vector<Diagnostic>* diagnostics_;
    std::vector<Conditional> conditionals_;
    std::vector<std::string> expanding_;
    std::size_t last_ = 0;
};

}  // namespace

std::string defaultPreludeText() { return std::string(kPrelude); }

LexResult lex(const std::string& source, const std::string& prelude) {
    LexResult result;
    std::map<std::string, std::string> macros;

    if (!prelude.empty()) {
        Lexer preludeLexer(prelude, 0, &macros, &result.tokens, &result.diagnostics);
        preludeLexer.run(0);
    }
    Lexer lexer(source, 1, &macros, &result.tokens, &result.diagnostics);
    lexer.run(0);

    Token end;
    end.kind = TokenKind::End;
    end.line = 0;
    end.column = 0;
    result.tokens.push_back(end);
    return result;
}

// ----------------------------------------------------------------- types ----

TypePtr primitiveType(TypeKind kind) {
    auto type = std::make_shared<Type>();
    type->kind = kind;
    switch (kind) {
        case TypeKind::Void:
            type->size = 0;
            type->align = 1;
            break;
        case TypeKind::Bool:
        case TypeKind::Char:
            type->size = 1;
            type->align = 1;
            break;
        case TypeKind::Int:
            type->size = 4;
            type->align = 4;
            break;
        case TypeKind::Long:
            type->size = 8;
            type->align = 8;
            break;
        case TypeKind::Float:
            type->size = 4;
            type->align = 4;
            break;
        case TypeKind::Double:
            type->size = 8;
            type->align = 8;
            break;
        default:
            type->size = 8;
            type->align = 8;
            break;
    }
    return type;
}

TypePtr pointerType(const TypePtr& element) {
    auto type = std::make_shared<Type>();
    type->kind = TypeKind::Pointer;
    type->element = element ? element : primitiveType(TypeKind::Void);
    type->size = 8;
    type->align = 8;
    return type;
}

TypePtr arrayType(const TypePtr& element, long long elements) {
    auto type = std::make_shared<Type>();
    type->kind = TypeKind::Array;
    type->element = element ? element : primitiveType(TypeKind::Int);
    type->elements = elements;
    int elementSize = type->element->size;
    type->size = static_cast<int>(elementSize * (elements > 0 ? elements : 0));
    type->align = type->element->align;
    return type;
}

std::string typeName(const TypePtr& type) {
    if (!type) return "<unknown>";
    switch (type->kind) {
        case TypeKind::Void: return "void";
        case TypeKind::Bool: return "bool";
        case TypeKind::Char: return "char";
        case TypeKind::Int: return "int";
        case TypeKind::Long: return "long";
        case TypeKind::Float: return "float";
        case TypeKind::Double: return "double";
        case TypeKind::Pointer: return typeName(type->element) + "*";
        case TypeKind::Array:
            return typeName(type->element) + "[" + std::to_string(type->elements) + "]";
        case TypeKind::Struct: return "struct " + type->name;
    }
    return "<unknown>";
}

}  // namespace detail
}  // namespace cxx
}  // namespace rblx
