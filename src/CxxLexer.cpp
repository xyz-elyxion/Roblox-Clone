#include "CxxInternal.h"

#include <cctype>
#include <cstdlib>
#include <fstream>
#include <set>
#include <sstream>
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
    // Some branch of this #if has already been taken, which is what decides
    // whether an #elif or #else is live.
    bool takenAny = false;
    int line = 0;
};

class Lexer {
public:
    Lexer(const std::string& text, int firstLine, std::map<std::string, std::string>* macros,
          std::vector<Token>* tokens, std::vector<Diagnostic>* diagnostics,
          const Options* options = nullptr, std::string file = std::string(),
          int includeDepth = 0, std::set<std::string>* once = nullptr)
        : source_(text),
          line_(firstLine),
          macros_(macros),
          tokens_(tokens),
          diagnostics_(diagnostics),
          options_(options),
          file_(std::move(file)),
          includeDepth_(includeDepth),
          once_(once) {}

    // A function-like macro's expansion is shared by every file that is lexed, so
    // the driver hands the same table to each header it pulls in.
    void shareFunctionMacros(
        std::map<std::string, std::pair<std::vector<std::string>, std::string>>* table) {
        functionMacros_ = table;
    }

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
        diagnostic.file = file_;
        diagnostics_->push_back(diagnostic);
    }

    void add(TokenKind kind, int line, int column) {
        Token token;
        token.kind = kind;
        token.line = line;
        token.column = column;
        token.file = file_;
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
                std::vector<std::string> parameters;
                std::size_t cursor = nameEnd + 1;
                bool closed = false;
                while (cursor < rest.size()) {
                    const char c = rest[cursor];
                    if (c == ' ' || c == '\t' || c == ',') {
                        ++cursor;
                        continue;
                    }
                    if (c == ')') {
                        closed = true;
                        ++cursor;
                        break;
                    }
                    if (rest.compare(cursor, 3, "...") == 0) {
                        error("variadic macros are not supported yet; write the call out",
                              directiveLine, 1);
                        return;
                    }
                    if (!isIdentifierStart(c)) {
                        error("a macro parameter must be a name", directiveLine, 1);
                        return;
                    }
                    std::size_t paramStart = cursor;
                    while (cursor < rest.size() && isIdentifierPart(rest[cursor])) ++cursor;
                    parameters.push_back(rest.substr(paramStart, cursor - paramStart));
                }
                if (!closed) {
                    error("the parameter list of a macro needs a ')'", directiveLine, 1);
                    return;
                }
                std::string body = rest.substr(cursor);
                while (!body.empty() && (body.front() == ' ' || body.front() == '\t')) {
                    body.erase(body.begin());
                }
                if (functionMacros_) {
                    (*functionMacros_)[name] = std::make_pair(parameters, body);
                }
                return;
            }
            // A name cannot be both: the newer definition wins.
            if (functionMacros_) functionMacros_->erase(name);
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
            if (functionMacros_) functionMacros_->erase(rest);
            return;
        }
        if (word == "ifdef" || word == "ifndef") {
            Conditional frame;
            frame.parentActive = active();
            bool defined = macros_->find(rest) != macros_->end();
            bool wanted = (word == "ifdef") ? defined : !defined;
            frame.active = frame.parentActive && wanted;
            frame.takenAny = wanted;
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
            frame.active = frame.parentActive && !frame.takenAny;
            frame.takenAny = true;
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
        if (word == "if" || word == "elif") {
            if (word == "elif") {
                if (conditionals_.empty()) {
                    error("#elif without #if", directiveLine, 1);
                    return;
                }
                Conditional& open = conditionals_.back();
                if (open.elseSeen) {
                    error("#elif after #else", directiveLine, 1);
                    return;
                }
                bool wanted = false;
                if (open.parentActive && !open.takenAny) {
                    if (!evaluateIfExpression(rest, wanted, directiveLine)) return;
                }
                open.active = open.parentActive && !open.takenAny && wanted;
                open.takenAny = open.takenAny || wanted;
                return;
            }
            Conditional frame;
            frame.parentActive = active();
            frame.line = directiveLine;
            bool wanted = false;
            // A branch that is already dead is not evaluated, the way a real
            // preprocessor skips it.
            if (frame.parentActive) {
                if (!evaluateIfExpression(rest, wanted, directiveLine)) return;
            }
            frame.active = frame.parentActive && wanted;
            frame.takenAny = wanted;
            conditionals_.push_back(frame);
            return;
        }
        if (word == "include") {
            if (!active()) return;
            include(rest, directiveLine);
            return;
        }
        if (word == "pragma") {
            // #pragma once is the only pragma that means anything here.
            if (active() && rest == "once" && !file_.empty() && once_) once_->insert(file_);
            return;
        }
        // Anything else is ignored: there is nothing else to configure.
    }

    // The directory a file lives in, which a quoted include is looked in first.
    static std::string directoryOf(const std::string& path) {
        std::size_t slash = path.find_last_of("/\\");
        return slash == std::string::npos ? std::string() : path.substr(0, slash + 1);
    }

    bool readHeader(const std::string& name, bool angled, std::string& path,
                    std::string& text) const {
        std::vector<std::string> candidates;
        if (!angled) {
            // "..." is looked for beside the file that wrote it, which is the
            // file being lexed, or the one the compiler was handed.
            const std::string from = !file_.empty() ? file_
                                                    : (options_ ? options_->sourceName
                                                                : std::string());
            candidates.push_back(directoryOf(from) + name);
        }
        if (options_) {
            for (const std::string& directory : options_->includePaths) {
                candidates.push_back(directory + "/" + name);
            }
        }
        for (const std::string& candidate : candidates) {
            std::ifstream header(candidate, std::ios::binary);
            if (!header) continue;
            std::ostringstream buffer;
            buffer << header.rdbuf();
            text = buffer.str();
            path = candidate;
            return true;
        }
        return false;
    }

    // A header is lexed where the directive stands, so its tokens land in the
    // place the declarations are needed and include guards work as written.
    void include(const std::string& spec, int directiveLine) {
        const bool angled = !spec.empty() && spec.front() == '<';
        const char closing = angled ? '>' : '"';
        if (spec.size() < 3 || spec.back() != closing) {
            error("#include needs a header name in \"...\" or <...>", directiveLine, 1);
            return;
        }
        const std::string name = spec.substr(1, spec.size() - 2);
        std::string path;
        std::string text;
        if (!readHeader(name, angled, path, text)) {
            // There are no system headers here, so an angle include that is not on
            // the search path is ignored: the prelude already declares libc.
            if (!angled) error("cannot find header '" + name + "'", directiveLine, 1);
            return;
        }
        if (once_ && once_->find(path) != once_->end()) return;
        if (includeDepth_ >= 32) {
            error("#include is more than 32 files deep; a header is probably including itself",
                  directiveLine, 1);
            return;
        }
        Lexer nested(text, 1, macros_, tokens_, diagnostics_, options_, path, includeDepth_ + 1,
                     once_);
        nested.shareFunctionMacros(functionMacros_);
        nested.run(0);
    }

    // ------------------------------------------------------- function macros ----

    static std::string trimSpaces(const std::string& text) {
        std::size_t begin = 0;
        std::size_t end = text.size();
        while (begin < end && (text[begin] == ' ' || text[begin] == '\t')) ++begin;
        while (end > begin && (text[end - 1] == ' ' || text[end - 1] == '\t')) --end;
        return text.substr(begin, end - begin);
    }

    // Collects the arguments of a macro call: everything between the matching
    // parentheses, split at commas that are not inside another pair. Strings and
    // character literals are copied through, and a call cannot span a line.
    bool collectArguments(std::size_t& cursor, const std::string& name,
                          std::vector<std::string>& arguments, int tokenLine, int tokenColumn) {
        std::size_t position = cursor + 1;  // past '('
        int depth = 1;
        std::string current;
        bool any = false;
        while (position < source_.size()) {
            const char c = source_[position];
            if (c == '\n') break;
            if (c == '"' || c == '\'') {
                const char quote = c;
                current += c;
                ++position;
                while (position < source_.size() && source_[position] != '\n') {
                    current += source_[position];
                    if (source_[position] == '\\' && position + 1 < source_.size()) {
                        ++position;
                        current += source_[position];
                        ++position;
                        continue;
                    }
                    if (source_[position] == quote) {
                        ++position;
                        break;
                    }
                    ++position;
                }
                any = true;
                continue;
            }
            if (c == '(') {
                ++depth;
                current += c;
                any = true;
                ++position;
                continue;
            }
            if (c == ')') {
                --depth;
                if (depth == 0) {
                    ++position;
                    if (any || !current.empty()) arguments.push_back(trimSpaces(current));
                    column_ += static_cast<int>(position - position_);
                    position_ = position;
                    return true;
                }
                current += c;
                any = true;
                ++position;
                continue;
            }
            if (c == ',' && depth == 1) {
                arguments.push_back(trimSpaces(current));
                current.clear();
                any = true;  // an empty argument is still an argument
                ++position;
                continue;
            }
            current += c;
            any = true;
            ++position;
        }
        error("the call to '" + name + "' does not close before the end of the line", tokenLine,
              tokenColumn);
        return false;
    }

    // Replaces each parameter in the body with its argument, parenthesised so that
    // `SQUARE(x + 1)` is not read as `x + 1 * x + 1`.
    std::string substituteArguments(const std::vector<std::string>& parameters,
                                    const std::string& body,
                                    const std::vector<std::string>& arguments,
                                    const std::string& name, int tokenLine, int tokenColumn) {
        if (parameters.size() != arguments.size()) {
            error("'" + name + "' takes " + std::to_string(parameters.size()) + " argument" +
                      (parameters.size() == 1 ? "" : "s") + " but was given " +
                      std::to_string(arguments.size()),
                  tokenLine, tokenColumn);
            return std::string();
        }
        std::string out;
        std::size_t i = 0;
        while (i < body.size()) {
            const char c = body[i];
            if (c == '"' || c == '\'') {
                // A literal is copied whole: a parameter name inside it is text.
                const char quote = c;
                out += c;
                ++i;
                while (i < body.size()) {
                    out += body[i];
                    if (body[i] == '\\' && i + 1 < body.size()) {
                        ++i;
                        out += body[i];
                        ++i;
                        continue;
                    }
                    if (body[i] == quote) {
                        ++i;
                        break;
                    }
                    ++i;
                }
                continue;
            }
            if (isIdentifierStart(c)) {
                const std::size_t start = i;
                while (i < body.size() && isIdentifierPart(body[i])) ++i;
                const std::string word = body.substr(start, i - start);
                bool replaced = false;
                for (std::size_t p = 0; p < parameters.size(); ++p) {
                    if (parameters[p] == word) {
                        out += "(" + arguments[p] + ")";
                        replaced = true;
                        break;
                    }
                }
                if (!replaced) out += word;
                continue;
            }
            out += c;
            ++i;
        }
        return out;
    }

    // -------------------------------------------------------------- #if ----

    struct IfExpression {
        std::string text;
        std::size_t cursor = 0;
        int depth = 0;
    };

    void skipIfSpace(IfExpression& expression) const {
        while (expression.cursor < expression.text.size() &&
               (expression.text[expression.cursor] == ' ' ||
                expression.text[expression.cursor] == '\t')) {
            ++expression.cursor;
        }
    }

    bool acceptIfText(IfExpression& expression, const char* spelling) const {
        skipIfSpace(expression);
        const std::string text(spelling);
        if (expression.text.compare(expression.cursor, text.size(), text) != 0) return false;
        expression.cursor += text.size();
        return true;
    }

    bool evaluateIfExpression(const std::string& text, bool& value, int line) {
        IfExpression expression;
        expression.text = text;
        long long result = 0;
        if (!evaluateIfOr(expression, result, line)) return false;
        skipIfSpace(expression);
        if (expression.cursor < expression.text.size()) {
            error("unexpected '" + expression.text.substr(expression.cursor, 1) +
                      "' in the #if expression",
                  line, 1);
            return false;
        }
        value = result != 0;
        return true;
    }

    bool evaluateIfOr(IfExpression& expression, long long& value, int line) {
        long long left = 0;
        if (!evaluateIfAnd(expression, left, line)) return false;
        while (acceptIfText(expression, "||")) {
            long long right = 0;
            if (!evaluateIfAnd(expression, right, line)) return false;
            left = (left != 0 || right != 0) ? 1 : 0;
        }
        value = left;
        return true;
    }

    bool evaluateIfAnd(IfExpression& expression, long long& value, int line) {
        long long left = 0;
        if (!evaluateIfEquality(expression, left, line)) return false;
        while (acceptIfText(expression, "&&")) {
            long long right = 0;
            if (!evaluateIfEquality(expression, right, line)) return false;
            left = (left != 0 && right != 0) ? 1 : 0;
        }
        value = left;
        return true;
    }

    bool evaluateIfEquality(IfExpression& expression, long long& value, int line) {
        long long left = 0;
        if (!evaluateIfRelational(expression, left, line)) return false;
        for (;;) {
            if (acceptIfText(expression, "==")) {
                long long right = 0;
                if (!evaluateIfRelational(expression, right, line)) return false;
                left = left == right ? 1 : 0;
                continue;
            }
            if (acceptIfText(expression, "!=")) {
                long long right = 0;
                if (!evaluateIfRelational(expression, right, line)) return false;
                left = left != right ? 1 : 0;
                continue;
            }
            break;
        }
        value = left;
        return true;
    }

    bool evaluateIfRelational(IfExpression& expression, long long& value, int line) {
        long long left = 0;
        if (!evaluateIfAdditive(expression, left, line)) return false;
        for (;;) {
            if (acceptIfText(expression, "<=")) {
                long long right = 0;
                if (!evaluateIfAdditive(expression, right, line)) return false;
                left = left <= right ? 1 : 0;
                continue;
            }
            if (acceptIfText(expression, ">=")) {
                long long right = 0;
                if (!evaluateIfAdditive(expression, right, line)) return false;
                left = left >= right ? 1 : 0;
                continue;
            }
            if (acceptIfText(expression, "<")) {
                long long right = 0;
                if (!evaluateIfAdditive(expression, right, line)) return false;
                left = left < right ? 1 : 0;
                continue;
            }
            if (acceptIfText(expression, ">")) {
                long long right = 0;
                if (!evaluateIfAdditive(expression, right, line)) return false;
                left = left > right ? 1 : 0;
                continue;
            }
            break;
        }
        value = left;
        return true;
    }

    bool evaluateIfAdditive(IfExpression& expression, long long& value, int line) {
        long long left = 0;
        if (!evaluateIfMultiplicative(expression, left, line)) return false;
        for (;;) {
            if (acceptIfText(expression, "+")) {
                long long right = 0;
                if (!evaluateIfMultiplicative(expression, right, line)) return false;
                left += right;
                continue;
            }
            if (acceptIfText(expression, "-")) {
                long long right = 0;
                if (!evaluateIfMultiplicative(expression, right, line)) return false;
                left -= right;
                continue;
            }
            break;
        }
        value = left;
        return true;
    }

    bool evaluateIfMultiplicative(IfExpression& expression, long long& value, int line) {
        long long left = 0;
        if (!evaluateIfPrimary(expression, left, line)) return false;
        for (;;) {
            if (acceptIfText(expression, "*")) {
                long long right = 0;
                if (!evaluateIfPrimary(expression, right, line)) return false;
                left *= right;
                continue;
            }
            if (acceptIfText(expression, "/")) {
                long long right = 0;
                if (!evaluateIfPrimary(expression, right, line)) return false;
                left = right == 0 ? 0 : left / right;
                continue;
            }
            if (acceptIfText(expression, "%")) {
                long long right = 0;
                if (!evaluateIfPrimary(expression, right, line)) return false;
                left = right == 0 ? 0 : left % right;
                continue;
            }
            break;
        }
        value = left;
        return true;
    }

    bool evaluateIfPrimary(IfExpression& expression, long long& value, int line) {
        skipIfSpace(expression);
        if (acceptIfText(expression, "(")) {
            long long inner = 0;
            if (!evaluateIfOr(expression, inner, line)) return false;
            if (!acceptIfText(expression, ")")) {
                error("expected ')' in the #if expression", line, 1);
                return false;
            }
            value = inner;
            return true;
        }
        if (acceptIfText(expression, "!")) {
            long long inner = 0;
            if (!evaluateIfPrimary(expression, inner, line)) return false;
            value = inner == 0 ? 1 : 0;
            return true;
        }
        if (acceptIfText(expression, "~")) {
            long long inner = 0;
            if (!evaluateIfPrimary(expression, inner, line)) return false;
            value = ~inner;
            return true;
        }
        if (acceptIfText(expression, "-")) {
            long long inner = 0;
            if (!evaluateIfPrimary(expression, inner, line)) return false;
            value = -inner;
            return true;
        }
        if (acceptIfText(expression, "+")) {
            return evaluateIfPrimary(expression, value, line);
        }
        skipIfSpace(expression);
        if (expression.text.compare(expression.cursor, 7, "defined") == 0 &&
            (expression.cursor + 7 >= expression.text.size() ||
             !isIdentifierPart(expression.text[expression.cursor + 7]))) {
            expression.cursor += 7;
            const bool parenthesised = acceptIfText(expression, "(");
            skipIfSpace(expression);
            const std::size_t start = expression.cursor;
            while (expression.cursor < expression.text.size() &&
                   isIdentifierPart(expression.text[expression.cursor])) {
                ++expression.cursor;
            }
            if (start == expression.cursor) {
                error("defined needs a name", line, 1);
                return false;
            }
            const std::string name = expression.text.substr(start, expression.cursor - start);
            if (parenthesised && !acceptIfText(expression, ")")) {
                error("expected ')' after defined", line, 1);
                return false;
            }
            value = macros_->find(name) != macros_->end() ? 1 : 0;
            return true;
        }
        skipIfSpace(expression);
        if (expression.cursor < expression.text.size() &&
            std::isdigit(static_cast<unsigned char>(expression.text[expression.cursor]))) {
            const char* begin = expression.text.c_str() + expression.cursor;
            char* end = nullptr;
            value = std::strtoll(begin, &end, 0);
            expression.cursor += static_cast<std::size_t>(end - begin);
            // 4u and 4L are still 4. The signedness of a #if is not interesting.
            while (expression.cursor < expression.text.size() &&
                   (expression.text[expression.cursor] == 'u' ||
                    expression.text[expression.cursor] == 'U' ||
                    expression.text[expression.cursor] == 'l' ||
                    expression.text[expression.cursor] == 'L')) {
                ++expression.cursor;
            }
            return true;
        }
        // A name: its macro value, or 0 when it is not defined, as in any
        // preprocessor.
        const std::size_t start = expression.cursor;
        while (expression.cursor < expression.text.size() &&
               isIdentifierPart(expression.text[expression.cursor])) {
            ++expression.cursor;
        }
        if (start == expression.cursor) {
            error("#if needs an expression", line, 1);
            return false;
        }
        const std::string word = expression.text.substr(start, expression.cursor - start);
        auto macro = macros_->find(word);
        if (macro == macros_->end() || macro->second.empty()) {
            value = 0;
            return true;
        }
        if (expression.depth >= 8) {
            error("#if macro expansion nests too deeply", line, 1);
            return false;
        }
        IfExpression nested;
        nested.text = macro->second;
        nested.depth = expression.depth + 1;
        long long substituted = 0;
        if (!evaluateIfOr(nested, substituted, line)) return false;
        value = substituted;
        return true;
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
        // A function-like macro expands only where it is called: the name on its
        // own is just an identifier.
        if (functionMacros_ && !expanding) {
            auto function = functionMacros_->find(name);
            if (function != functionMacros_->end()) {
                std::size_t scan = position_;
                while (scan < source_.size() && (source_[scan] == ' ' || source_[scan] == '\t')) {
                    ++scan;
                }
                if (scan < source_.size() && source_[scan] == '(') {
                    std::vector<std::string> arguments;
                    if (!collectArguments(scan, name, arguments, tokenLine, tokenColumn)) return;
                    const std::string text =
                        substituteArguments(function->second.first, function->second.second,
                                            arguments, name, tokenLine, tokenColumn);
                    if (text.empty()) return;
                    expanding_.push_back(name);
                    Lexer nested(text, tokenLine, macros_, tokens_, diagnostics_, options_, file_,
                                 includeDepth_, once_);
                    nested.shareFunctionMacros(functionMacros_);
                    nested.expanding_ = expanding_;
                    nested.column_ = tokenColumn;
                    nested.run(depth + 1);
                    expanding_.pop_back();
                    return;
                }
            }
        }
        if (macro != macros_->end() && !expanding) {
            expanding_.push_back(name);
            Lexer nested(macro->second, tokenLine, macros_, tokens_, diagnostics_, options_, file_,
                         includeDepth_, once_);
            nested.shareFunctionMacros(functionMacros_);
            nested.expanding_ = expanding_;
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
        // A `u` suffix makes the literal unsigned. `l` and `L` change nothing,
        // because every integer is held in a 64-bit register and the declared
        // type decides what is stored.
        bool unsignedLiteral = false;
        bool longLiteral = false;
        while (cursor < source_.size()) {
            char c = source_[cursor];
            if (c == 'u' || c == 'U') {
                unsignedLiteral = true;
                ++cursor;
                continue;
            }
            if (c == 'l' || c == 'L') {
                longLiteral = true;
                ++cursor;
                continue;
            }
            break;
        }

        column_ += static_cast<int>(cursor - position_);
        position_ = cursor;
        add(TokenKind::Number, tokenLine, tokenColumn);
        tokens_->back().number = value;
        tokens_->back().unsignedLiteral = unsignedLiteral;
        tokens_->back().longLiteral = longLiteral;
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
                                     "->", "::"};
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
    const Options* options_ = nullptr;
    std::string file_;  // the header being lexed; empty for the file handed in
    int includeDepth_ = 0;
    std::set<std::string>* once_ = nullptr;  // headers that asked for #pragma once
    std::map<std::string, std::pair<std::vector<std::string>, std::string>>* functionMacros_ =
        nullptr;  // #define NAME(a, b) body
    std::vector<Conditional> conditionals_;
    std::vector<std::string> expanding_;
    std::size_t last_ = 0;
};

}  // namespace

std::string defaultPreludeText() { return std::string(kPrelude); }

LexResult lex(const std::string& source, const std::string& prelude, const Options& options) {
    LexResult result;
    std::map<std::string, std::string> macros;
    std::set<std::string> once;
    std::map<std::string, std::pair<std::vector<std::string>, std::string>> functionMacros;

    if (!prelude.empty()) {
        Lexer preludeLexer(prelude, 0, &macros, &result.tokens, &result.diagnostics, &options);
        preludeLexer.shareFunctionMacros(&functionMacros);
        preludeLexer.run(0);
    }
    Lexer lexer(source, 1, &macros, &result.tokens, &result.diagnostics, &options, std::string(), 0,
                &once);
    lexer.shareFunctionMacros(&functionMacros);
    lexer.run(0);

    Token end;
    end.kind = TokenKind::End;
    end.line = 0;
    end.column = 0;
    // An error at the end of the tokens belongs to the file the last one came
    // from, which is a header when the source ends inside one.
    if (!result.tokens.empty()) end.file = result.tokens.back().file;
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
        case TypeKind::Short:
            type->size = 2;
            type->align = 2;
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

TypePtr integerType(TypeKind kind, bool isUnsigned) {
    TypePtr type = primitiveType(kind);
    type->isUnsigned = isUnsigned;
    return type;
}

// Integral promotion: char, short and bool all fit in an int, so arithmetic on
// them happens at int width and signedness - which is why `unsigned char` 200
// plus 100 is 300 rather than 44.
TypePtr promotedType(const TypePtr& type) {
    if (!type) return type;
    if (type->isSmallInteger()) return integerType(TypeKind::Int, false);
    return type;
}

// The usual arithmetic conversions: the type both sides of a binary operation
// are computed in. Signedness is part of the answer, which is what keeps
// `4000000000u / 2u` from being read as a negative number.
TypePtr arithmeticType(const TypePtr& left, const TypePtr& right) {
    if (!left) return right;
    if (!right) return left;
    if (left->isFloating() || right->isFloating()) {
        if (left->kind == TypeKind::Double || right->kind == TypeKind::Double) {
            return primitiveType(TypeKind::Double);
        }
        return primitiveType(TypeKind::Float);
    }
    TypePtr promotedLeft = promotedType(left);
    TypePtr promotedRight = promotedType(right);
    if (promotedLeft->kind == TypeKind::Long || promotedRight->kind == TypeKind::Long) {
        // A 64-bit signed long holds every unsigned int, so only an unsigned
        // long makes the result unsigned.
        const bool isUnsigned = (promotedLeft->kind == TypeKind::Long && promotedLeft->isUnsigned) ||
                                (promotedRight->kind == TypeKind::Long && promotedRight->isUnsigned);
        return integerType(TypeKind::Long, isUnsigned);
    }
    return integerType(TypeKind::Int, promotedLeft->isUnsigned || promotedRight->isUnsigned);
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
        case TypeKind::Char: return type->isUnsigned ? "unsigned char" : "char";
        case TypeKind::Short: return type->isUnsigned ? "unsigned short" : "short";
        case TypeKind::Int: return type->isUnsigned ? "unsigned int" : "int";
        case TypeKind::Long: return type->isUnsigned ? "unsigned long" : "long";
        case TypeKind::Float: return "float";
        case TypeKind::Double: return "double";
        case TypeKind::Pointer: return typeName(type->element) + "*";
        case TypeKind::Array:
            return typeName(type->element) + "[" + std::to_string(type->elements) + "]";
        case TypeKind::Struct: return (type->isUnion ? "union " : "struct ") + type->name;
    }
    return "<unknown>";
}

}  // namespace detail
}  // namespace cxx
}  // namespace rblx
