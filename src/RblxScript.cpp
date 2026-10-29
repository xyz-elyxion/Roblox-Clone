#include "RblxScript.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <utility>

namespace rblx {
namespace script {

namespace {

struct ScriptError {
    std::string message;
    int line = 0;
};

std::string numberToString(double value) {
    if (std::isnan(value)) return "nan";
    if (std::isinf(value)) return value > 0 ? "inf" : "-inf";
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%.14g", value);
    return buffer;
}

struct Block;
struct Environment;

}  // namespace

// Script closures: either a native host callback or a parsed body plus the
// environment it was defined in.
struct Function {
    bool native = false;
    std::function<Value(Interpreter&, const std::vector<Value>&)> nativeCall;
    std::vector<std::string> params;
    std::shared_ptr<Block> body;
    std::shared_ptr<Environment> closure;
};

namespace {

using EnvPtr = std::shared_ptr<Environment>;

struct Environment {
    std::shared_ptr<Environment> parent;
    std::map<std::string, Value> vars;
};

struct Expr {
    enum class Kind {
        Nil, True, False, Number, String, Name, Function, Table, Unary, Binary, Index, Call,
        MethodCall
    };

    Kind kind = Kind::Nil;
    int line = 0;
    double number = 0.0;
    std::string text;
    std::vector<std::unique_ptr<Expr>> list;
    std::vector<std::pair<std::string, std::unique_ptr<Expr>>> named;
    std::vector<std::pair<std::unique_ptr<Expr>, std::unique_ptr<Expr>>> keyed;
    std::unique_ptr<Expr> a;
    std::unique_ptr<Expr> b;
    std::vector<std::string> params;
    std::shared_ptr<Block> body;
};
using ExprPtr = std::unique_ptr<Expr>;

struct Stmt {
    enum class Kind {
        Local, Assign, Expr, If, While, Repeat, ForNum, Do, Return, Break, FunctionDecl
    };

    Kind kind = Kind::Expr;
    int line = 0;
    std::vector<std::string> names;
    std::vector<ExprPtr> targets;
    std::vector<ExprPtr> values;
    std::unique_ptr<Block> body;
    std::vector<std::pair<ExprPtr, std::unique_ptr<Block>>> branches;
    std::unique_ptr<Block> elseBody;
    ExprPtr cond;
    ExprPtr start;
    ExprPtr limit;
    ExprPtr step;
    ExprPtr functionTarget;
    std::shared_ptr<Block> functionBody;
    std::vector<std::string> params;
    bool hasSelf = false;
};
using StmtPtr = std::unique_ptr<Stmt>;

struct Block {
    std::vector<StmtPtr> statements;
};

enum class Tok {
    Eof, Name, Number, String,
    And, Break, Do, Else, Elseif, End, False, For, Function, If, In, Local, Nil, Not, Or,
    Repeat, Return, Then, True, Until, While,
    Plus, Minus, Star, Slash, Percent, Caret,
    Eq, EqEq, NotEq, Less, LessEq, Greater, GreaterEq,
    LParen, RParen, LBrace, RBrace, LBracket, RBracket,
    Semi, Colon, Comma, Dot, DotDot
};

struct Token {
    Tok type = Tok::Eof;
    std::string text;
    double number = 0.0;
    int line = 1;
};

[[noreturn]] void fail(int line, const std::string& message) {
    throw ScriptError{message, line};
}

class Lexer {
public:
    explicit Lexer(const std::string& source) : source_(source) {}

    bool lex(std::vector<Token>& out, std::string& error) {
        try {
            while (true) {
                Token token = next();
                out.push_back(token);
                if (token.type == Tok::Eof) return true;
            }
        } catch (const ScriptError& failure) {
            error = "line " + std::to_string(failure.line) + ": " + failure.message;
            return false;
        }
    }

private:
    char peek(std::size_t offset = 0) const {
        const std::size_t index = position_ + offset;
        return index < source_.size() ? source_[index] : '\0';
    }

    char take() {
        const char c = peek();
        if (c != '\0') {
            ++position_;
            if (c == '\n') ++line_;
        }
        return c;
    }

    bool match(char expected) {
        if (peek() != expected) return false;
        take();
        return true;
    }

    void skipWhitespaceAndComments() {
        while (true) {
            while (std::isspace(static_cast<unsigned char>(peek()))) {
                take();
            }
            if (peek() == '-' && peek(1) == '-') {
                while (peek() != '\0' && peek() != '\n') take();
                continue;
            }
            break;
        }
    }

    Token next() {
        skipWhitespaceAndComments();
        const int line = line_;
        const char c = peek();
        if (c == '\0') return Token{Tok::Eof, "", 0.0, line};

        if (std::isalpha(static_cast<unsigned char>(c)) || c == '_') {
            std::string text;
            while (std::isalnum(static_cast<unsigned char>(peek())) || peek() == '_') {
                text.push_back(take());
            }
            static const std::map<std::string, Tok> keywords = {
                {"and", Tok::And},       {"break", Tok::Break}, {"do", Tok::Do},
                {"else", Tok::Else},     {"elseif", Tok::Elseif}, {"end", Tok::End},
                {"false", Tok::False},   {"for", Tok::For},     {"function", Tok::Function},
                {"if", Tok::If},         {"in", Tok::In},       {"local", Tok::Local},
                {"nil", Tok::Nil},       {"not", Tok::Not},     {"or", Tok::Or},
                {"repeat", Tok::Repeat}, {"return", Tok::Return}, {"then", Tok::Then},
                {"true", Tok::True},     {"until", Tok::Until}, {"while", Tok::While},
            };
            const auto found = keywords.find(text);
            return Token{found == keywords.end() ? Tok::Name : found->second, text, 0.0, line};
        }

        if (std::isdigit(static_cast<unsigned char>(c)) ||
            (c == '.' && std::isdigit(static_cast<unsigned char>(peek(1))))) {
            std::string text;
            while (std::isdigit(static_cast<unsigned char>(peek()))) text.push_back(take());
            if (peek() == '.') {
                text.push_back(take());
                while (std::isdigit(static_cast<unsigned char>(peek()))) text.push_back(take());
            }
            if (peek() == 'e' || peek() == 'E') {
                text.push_back(take());
                if (peek() == '+' || peek() == '-') text.push_back(take());
                while (std::isdigit(static_cast<unsigned char>(peek()))) text.push_back(take());
            }
            return Token{Tok::Number, text, std::strtod(text.c_str(), nullptr), line};
        }

        if (c == '"' || c == '\'') {
            const char quote = take();
            std::string text;
            while (true) {
                const char ch = peek();
                if (ch == '\0') fail(line, "unfinished string");
                if (ch == quote) {
                    take();
                    break;
                }
                if (ch == '\\') {
                    take();
                    const char escape = take();
                    switch (escape) {
                        case 'n': text.push_back('\n'); break;
                        case 't': text.push_back('\t'); break;
                        case 'r': text.push_back('\r'); break;
                        case '0': text.push_back('\0'); break;
                        case '\\': text.push_back('\\'); break;
                        case '"': text.push_back('"'); break;
                        case '\'': text.push_back('\''); break;
                        default: text.push_back(escape); break;
                    }
                    continue;
                }
                text.push_back(take());
            }
            return Token{Tok::String, text, 0.0, line};
        }

        switch (c) {
            case '+': take(); return Token{Tok::Plus, "+", 0.0, line};
            case '-': take(); return Token{Tok::Minus, "-", 0.0, line};
            case '*': take(); return Token{Tok::Star, "*", 0.0, line};
            case '/': take(); return Token{Tok::Slash, "/", 0.0, line};
            case '%': take(); return Token{Tok::Percent, "%", 0.0, line};
            case '^': take(); return Token{Tok::Caret, "^", 0.0, line};
            case '(': take(); return Token{Tok::LParen, "(", 0.0, line};
            case ')': take(); return Token{Tok::RParen, ")", 0.0, line};
            case '{': take(); return Token{Tok::LBrace, "{", 0.0, line};
            case '}': take(); return Token{Tok::RBrace, "}", 0.0, line};
            case '[': take(); return Token{Tok::LBracket, "[", 0.0, line};
            case ']': take(); return Token{Tok::RBracket, "]", 0.0, line};
            case ';': take(); return Token{Tok::Semi, ";", 0.0, line};
            case ':': take(); return Token{Tok::Colon, ":", 0.0, line};
            case ',': take(); return Token{Tok::Comma, ",", 0.0, line};
            case '=':
                take();
                return match('=') ? Token{Tok::EqEq, "==", 0.0, line}
                                  : Token{Tok::Eq, "=", 0.0, line};
            case '~':
                take();
                if (match('=')) return Token{Tok::NotEq, "~=", 0.0, line};
                fail(line, "unexpected '~'");
            case '<':
                take();
                return match('=') ? Token{Tok::LessEq, "<=", 0.0, line}
                                  : Token{Tok::Less, "<", 0.0, line};
            case '>':
                take();
                return match('=') ? Token{Tok::GreaterEq, ">=", 0.0, line}
                                  : Token{Tok::Greater, ">", 0.0, line};
            case '.':
                take();
                return match('.') ? Token{Tok::DotDot, "..", 0.0, line}
                                  : Token{Tok::Dot, ".", 0.0, line};
            default:
                fail(line, std::string("unexpected character '") + c + "'");
        }
    }

    const std::string& source_;
    std::size_t position_ = 0;
    int line_ = 1;
};

int binaryPrecedence(Tok type) {
    switch (type) {
        case Tok::Or: return 1;
        case Tok::And: return 2;
        case Tok::Less:
        case Tok::LessEq:
        case Tok::Greater:
        case Tok::GreaterEq:
        case Tok::EqEq:
        case Tok::NotEq: return 3;
        case Tok::DotDot: return 4;
        case Tok::Plus:
        case Tok::Minus: return 5;
        case Tok::Star:
        case Tok::Slash:
        case Tok::Percent: return 6;
        case Tok::Caret: return 8;
        default: return 0;
    }
}

bool isRightAssociative(Tok type) {
    return type == Tok::Caret || type == Tok::DotDot;
}

class Parser {
public:
    explicit Parser(std::vector<Token> tokens) : tokens_(std::move(tokens)) {}

    std::unique_ptr<Block> parseChunk(std::string& error) {
        try {
            auto block = parseBlock();
            expect(Tok::Eof, "unexpected token after the end of the script");
            return block;
        } catch (const ScriptError& failure) {
            error = "line " + std::to_string(failure.line) + ": " + failure.message;
            return nullptr;
        }
    }

private:
    const Token& current() const { return tokens_[index_]; }
    const Token& peek(std::size_t offset) const {
        const std::size_t index = std::min(index_ + offset, tokens_.size() - 1);
        return tokens_[index];
    }
    void advance() {
        if (index_ + 1 < tokens_.size()) ++index_;
    }
    bool check(Tok type) const { return current().type == type; }
    bool accept(Tok type) {
        if (!check(type)) return false;
        advance();
        return true;
    }
    void expect(Tok type, const std::string& message) {
        if (!check(type)) fail(current().line, message);
        advance();
    }

    bool atBlockEnd() const {
        return check(Tok::Eof) || check(Tok::End) || check(Tok::Else) ||
               check(Tok::Elseif) || check(Tok::Until);
    }

    std::unique_ptr<Block> parseBlock() {
        auto block = std::make_unique<Block>();
        while (!atBlockEnd()) {
            if (accept(Tok::Semi)) continue;
            block->statements.push_back(parseStatement());
        }
        return block;
    }

    StmtPtr parseStatement() {
        const int line = current().line;

        if (accept(Tok::Local)) {
            if (accept(Tok::Function)) {
                auto stmt = std::make_unique<Stmt>();
                stmt->kind = Stmt::Kind::Local;
                stmt->line = line;
                const std::string name = expectName("expected a function name");
                stmt->names.push_back(name);
                auto function = parseFunctionBody(line);
                stmt->values.push_back(std::move(function));
                return stmt;
            }
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::Local;
            stmt->line = line;
            stmt->names.push_back(expectName("expected a local name"));
            while (accept(Tok::Comma)) {
                stmt->names.push_back(expectName("expected a local name"));
            }
            if (accept(Tok::Eq)) {
                stmt->values.push_back(parseExpr());
                while (accept(Tok::Comma)) stmt->values.push_back(parseExpr());
            }
            return stmt;
        }

        if (accept(Tok::Function)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::FunctionDecl;
            stmt->line = line;
            const std::string base = expectName("expected a function name");
            auto target = std::make_unique<Expr>();
            target->kind = Expr::Kind::Name;
            target->line = line;
            target->text = base;
            while (accept(Tok::Dot)) {
                auto index = std::make_unique<Expr>();
                index->kind = Expr::Kind::Index;
                index->line = line;
                index->a = std::move(target);
                auto key = std::make_unique<Expr>();
                key->kind = Expr::Kind::String;
                key->line = line;
                key->text = expectName("expected a field name");
                index->b = std::move(key);
                target = std::move(index);
            }
            if (accept(Tok::Colon)) {
                auto index = std::make_unique<Expr>();
                index->kind = Expr::Kind::Index;
                index->line = line;
                index->a = std::move(target);
                auto key = std::make_unique<Expr>();
                key->kind = Expr::Kind::String;
                key->line = line;
                key->text = expectName("expected a method name");
                index->b = std::move(key);
                target = std::move(index);
                stmt->hasSelf = true;
            }
            auto function = parseFunctionBody(line);
            stmt->params = function->params;
            stmt->functionBody = function->body;
            stmt->functionTarget = std::move(target);
            return stmt;
        }

        if (accept(Tok::If)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::If;
            stmt->line = line;
            auto condition = parseExpr();
            expect(Tok::Then, "expected 'then'");
            auto body = parseBlock();
            stmt->branches.emplace_back(std::move(condition), std::move(body));
            while (accept(Tok::Elseif)) {
                auto elseifCondition = parseExpr();
                expect(Tok::Then, "expected 'then'");
                auto elseifBody = parseBlock();
                stmt->branches.emplace_back(std::move(elseifCondition), std::move(elseifBody));
            }
            if (accept(Tok::Else)) {
                stmt->elseBody = parseBlock();
            }
            expect(Tok::End, "expected 'end' after the if block");
            return stmt;
        }

        if (accept(Tok::While)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::While;
            stmt->line = line;
            stmt->cond = parseExpr();
            expect(Tok::Do, "expected 'do' after the while condition");
            stmt->body = parseBlock();
            expect(Tok::End, "expected 'end' after the while loop");
            return stmt;
        }

        if (accept(Tok::Repeat)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::Repeat;
            stmt->line = line;
            stmt->body = parseBlock();
            expect(Tok::Until, "expected 'until' after the repeat block");
            stmt->cond = parseExpr();
            return stmt;
        }

        if (accept(Tok::For)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::ForNum;
            stmt->line = line;
            stmt->names.push_back(expectName("expected a loop variable"));
            expect(Tok::Eq, "expected '=' in a numeric for loop");
            stmt->start = parseExpr();
            expect(Tok::Comma, "expected ',' after the loop start");
            stmt->limit = parseExpr();
            if (accept(Tok::Comma)) stmt->step = parseExpr();
            expect(Tok::Do, "expected 'do' after the for loop header");
            stmt->body = parseBlock();
            expect(Tok::End, "expected 'end' after the for loop");
            return stmt;
        }

        if (accept(Tok::Do)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::Do;
            stmt->line = line;
            stmt->body = parseBlock();
            expect(Tok::End, "expected 'end' after the do block");
            return stmt;
        }

        if (accept(Tok::Return)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::Return;
            stmt->line = line;
            if (!atBlockEnd() && !check(Tok::Semi)) {
                stmt->values.push_back(parseExpr());
                while (accept(Tok::Comma)) stmt->values.push_back(parseExpr());
            }
            accept(Tok::Semi);
            return stmt;
        }

        if (accept(Tok::Break)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::Break;
            stmt->line = line;
            return stmt;
        }

        auto first = parsePrimary();
        if (check(Tok::Eq) || check(Tok::Comma)) {
            auto stmt = std::make_unique<Stmt>();
            stmt->kind = Stmt::Kind::Assign;
            stmt->line = line;
            stmt->targets.push_back(std::move(first));
            while (accept(Tok::Comma)) stmt->targets.push_back(parsePrimary());
            expect(Tok::Eq, "expected '=' in an assignment");
            stmt->values.push_back(parseExpr());
            while (accept(Tok::Comma)) stmt->values.push_back(parseExpr());
            return stmt;
        }

        if (first->kind != Expr::Kind::Call && first->kind != Expr::Kind::MethodCall) {
            fail(line, "expected an assignment or a function call");
        }

        auto stmt = std::make_unique<Stmt>();
        stmt->kind = Stmt::Kind::Expr;
        stmt->line = line;
        stmt->values.push_back(std::move(first));
        return stmt;
    }

    std::string expectName(const std::string& message) {
        if (!check(Tok::Name)) fail(current().line, message);
        std::string name = current().text;
        advance();
        return name;
    }

    ExprPtr parseExpr() { return parseSubExpr(0); }

    ExprPtr parseSubExpr(int limit) {
        auto left = parseUnary();
        while (true) {
            const Tok type = current().type;
            const int precedence = binaryPrecedence(type);
            if (precedence == 0 || precedence <= limit) break;
            const int line = current().line;
            const std::string op = current().text;
            advance();
            const int nextLimit = isRightAssociative(type) ? precedence - 1 : precedence;
            auto right = parseSubExpr(nextLimit);
            auto binary = std::make_unique<Expr>();
            binary->kind = Expr::Kind::Binary;
            binary->line = line;
            binary->text = op;
            binary->a = std::move(left);
            binary->b = std::move(right);
            left = std::move(binary);
        }
        return left;
    }

    ExprPtr parseUnary() {
        const int line = current().line;
        if (check(Tok::Not) || check(Tok::Minus)) {
            const std::string op = current().text;
            advance();
            auto operand = parseSubExpr(7);
            auto expr = std::make_unique<Expr>();
            expr->kind = Expr::Kind::Unary;
            expr->line = line;
            expr->text = op;
            expr->a = std::move(operand);
            return expr;
        }
        return parsePrimary();
    }

    ExprPtr parsePrimary() {
        const int line = current().line;
        ExprPtr expr;

        switch (current().type) {
            case Tok::Nil:
                advance();
                expr = std::make_unique<Expr>();
                expr->kind = Expr::Kind::Nil;
                expr->line = line;
                break;
            case Tok::True:
                advance();
                expr = std::make_unique<Expr>();
                expr->kind = Expr::Kind::True;
                expr->line = line;
                break;
            case Tok::False:
                advance();
                expr = std::make_unique<Expr>();
                expr->kind = Expr::Kind::False;
                expr->line = line;
                break;
            case Tok::Number:
                expr = std::make_unique<Expr>();
                expr->kind = Expr::Kind::Number;
                expr->line = line;
                expr->number = current().number;
                advance();
                break;
            case Tok::String:
                expr = std::make_unique<Expr>();
                expr->kind = Expr::Kind::String;
                expr->line = line;
                expr->text = current().text;
                advance();
                break;
            case Tok::Name:
                expr = std::make_unique<Expr>();
                expr->kind = Expr::Kind::Name;
                expr->line = line;
                expr->text = current().text;
                advance();
                break;
            case Tok::Function:
                advance();
                expr = parseFunctionBody(line);
                break;
            case Tok::LBrace:
                expr = parseTable();
                break;
            case Tok::LParen: {
                advance();
                expr = parseExpr();
                expect(Tok::RParen, "expected ')'");
                break;
            }
            default:
                fail(line, "unexpected token in an expression");
        }

        return parseSuffixes(std::move(expr));
    }

    ExprPtr parseSuffixes(ExprPtr expr) {
        while (true) {
            const int line = current().line;
            if (accept(Tok::Dot)) {
                auto index = std::make_unique<Expr>();
                index->kind = Expr::Kind::Index;
                index->line = line;
                index->a = std::move(expr);
                auto key = std::make_unique<Expr>();
                key->kind = Expr::Kind::String;
                key->line = line;
                key->text = expectName("expected a field name after '.'");
                index->b = std::move(key);
                expr = std::move(index);
                continue;
            }
            if (accept(Tok::LBracket)) {
                auto index = std::make_unique<Expr>();
                index->kind = Expr::Kind::Index;
                index->line = line;
                index->a = std::move(expr);
                index->b = parseExpr();
                expect(Tok::RBracket, "expected ']'");
                expr = std::move(index);
                continue;
            }
            if (accept(Tok::Colon)) {
                auto call = std::make_unique<Expr>();
                call->kind = Expr::Kind::MethodCall;
                call->line = line;
                call->text = expectName("expected a method name after ':'");
                call->a = std::move(expr);
                parseArgs(call->list);
                expr = std::move(call);
                continue;
            }
            if (check(Tok::LParen) || check(Tok::String) || check(Tok::LBrace)) {
                auto call = std::make_unique<Expr>();
                call->kind = Expr::Kind::Call;
                call->line = line;
                call->a = std::move(expr);
                parseArgs(call->list);
                expr = std::move(call);
                continue;
            }
            break;
        }
        return expr;
    }

    void parseArgs(std::vector<ExprPtr>& args) {
        if (accept(Tok::LParen)) {
            if (!check(Tok::RParen)) {
                args.push_back(parseExpr());
                while (accept(Tok::Comma)) args.push_back(parseExpr());
            }
            expect(Tok::RParen, "expected ')' after the arguments");
            return;
        }
        if (check(Tok::String)) {
            auto value = std::make_unique<Expr>();
            value->kind = Expr::Kind::String;
            value->line = current().line;
            value->text = current().text;
            advance();
            args.push_back(std::move(value));
            return;
        }
        if (check(Tok::LBrace)) {
            args.push_back(parseTable());
            return;
        }
        fail(current().line, "expected function arguments");
    }

    ExprPtr parseTable() {
        const int line = current().line;
        expect(Tok::LBrace, "expected '{'");
        auto expr = std::make_unique<Expr>();
        expr->kind = Expr::Kind::Table;
        expr->line = line;
        while (!check(Tok::RBrace)) {
            if (accept(Tok::LBracket)) {
                auto key = parseExpr();
                expect(Tok::RBracket, "expected ']' in a table key");
                expect(Tok::Eq, "expected '=' after a table key");
                expr->keyed.emplace_back(std::move(key), parseExpr());
            } else if (check(Tok::Name) && peek(1).type == Tok::Eq) {
                const std::string name = current().text;
                advance();
                advance();
                expr->named.emplace_back(name, parseExpr());
            } else {
                expr->list.push_back(parseExpr());
            }
            if (!accept(Tok::Comma) && !accept(Tok::Semi)) break;
        }
        expect(Tok::RBrace, "expected '}' to close the table");
        return expr;
    }

    ExprPtr parseFunctionBody(int line) {
        auto function = std::make_unique<Expr>();
        function->kind = Expr::Kind::Function;
        function->line = line;
        expect(Tok::LParen, "expected '(' after 'function'");
        if (!check(Tok::RParen)) {
            function->params.push_back(expectName("expected a parameter name"));
            while (accept(Tok::Comma)) {
                function->params.push_back(expectName("expected a parameter name"));
            }
        }
        expect(Tok::RParen, "expected ')' after the parameters");
        function->body = parseBlock();
        expect(Tok::End, "expected 'end' after the function body");
        return function;
    }

    std::vector<Token> tokens_;
    std::size_t index_ = 0;
};

std::string typeNameOf(const Value& value) {
    switch (value.type) {
        case Value::Type::Nil: return "nil";
        case Value::Type::Boolean: return "boolean";
        case Value::Type::Number: return "number";
        case Value::Type::String: return "string";
        case Value::Type::Table: return "table";
        case Value::Type::Function: return "function";
        case Value::Type::Object:
            return value.object ? value.object->typeName() : "object";
    }
    return "nil";
}

enum class Flow { Normal, Break, Return };

}  // namespace

struct Interpreter::Impl {
    Interpreter* owner = nullptr;
    EnvPtr globals;
    std::function<void(const std::string&, bool)> printHandler;
    long long steps = 0;
    long long stepLimit = 2000000;

    void step(int line) {
        if (++steps > stepLimit) {
            fail(line, "script exceeded the execution limit (possible infinite loop)");
        }
    }

    Value* findVar(const EnvPtr& env, const std::string& name) const {
        for (EnvPtr current = env; current; current = current->parent) {
            const auto found = current->vars.find(name);
            if (found != current->vars.end()) return &found->second;
        }
        return nullptr;
    }

    void assignName(const EnvPtr& env, const std::string& name, const Value& value) const {
        if (Value* slot = findVar(env, name)) {
            *slot = value;
            return;
        }
        globals->vars[name] = value;
    }

    Value eval(Expr& expr, const EnvPtr& env);
    Flow exec(Stmt& stmt, const EnvPtr& env, Value& returnValue);
    Flow execBlock(Block& block, const EnvPtr& env, Value& returnValue);
    Value callValue(const Value& callable, const std::vector<Value>& args, int line);
    Value evalIndex(Expr& expr, const EnvPtr& env);
    Value evalTable(Expr& expr, const EnvPtr& env);
    Value evalBinary(Expr& expr, const EnvPtr& env);
    void assignTo(Expr& target, const Value& value, const EnvPtr& env);

    Value lookupIndex(const Value& object, const Value& key, int line);
    void setIndex(const Value& object, const Value& key, const Value& value, int line);
};

Value Interpreter::Impl::lookupIndex(const Value& object, const Value& key, int line) {
    if (object.isTable()) return object.table->get(key);

    if (object.isObject()) {
        if (!key.isString()) {
            fail(line, "attempt to index " + typeNameOf(object) + " with a non-string key");
        }
        Value out;
        if (object.object->get(key.string, out)) return out;
        if (Signal* signalValue = object.object->signal(key.string)) {
            return makeSignalValue(*signalValue);
        }
        fail(line, typeNameOf(object) + " has no property '" + key.string + "'");
    }

    fail(line, "attempt to index a " + typeNameOf(object) + " value");
}

void Interpreter::Impl::setIndex(const Value& object, const Value& key, const Value& value,
                                 int line) {
    if (object.isTable()) {
        object.table->set(key, value);
        return;
    }
    if (object.isObject()) {
        if (!key.isString()) {
            fail(line, "attempt to set a field with a non-string key");
        }
        if (!object.object->set(key.string, value)) {
            fail(line, typeNameOf(object) + " has no writable property '" + key.string + "'");
        }
        return;
    }
    fail(line, "attempt to index a " + typeNameOf(object) + " value");
}

Value Interpreter::Impl::evalIndex(Expr& expr, const EnvPtr& env) {
    const Value object = eval(*expr.a, env);
    const Value key = eval(*expr.b, env);
    return lookupIndex(object, key, expr.line);
}

Value Interpreter::Impl::evalTable(Expr& expr, const EnvPtr& env) {
    auto table = std::make_shared<Table>();
    for (ExprPtr& item : expr.list) {
        table->elements.push_back(eval(*item, env));
    }
    for (auto& entry : expr.named) {
        table->fields[entry.first] = eval(*entry.second, env);
    }
    for (auto& entry : expr.keyed) {
        const Value key = eval(*entry.first, env);
        table->set(key, eval(*entry.second, env));
    }
    return Value::tableValue(table);
}

Value Interpreter::Impl::evalBinary(Expr& expr, const EnvPtr& env) {
    const std::string& op = expr.text;

    if (op == "and") {
        Value left = eval(*expr.a, env);
        return left.truthy() ? eval(*expr.b, env) : left;
    }
    if (op == "or") {
        Value left = eval(*expr.a, env);
        return left.truthy() ? left : eval(*expr.b, env);
    }

    const Value left = eval(*expr.a, env);
    const Value right = eval(*expr.b, env);

    if (op == "..") return Value::stringValue(left.toString() + right.toString());

    if (op == "==" || op == "~=") {
        bool equal = false;
        if (left.type == right.type) {
            switch (left.type) {
                case Value::Type::Nil: equal = true; break;
                case Value::Type::Boolean: equal = left.boolean == right.boolean; break;
                case Value::Type::Number: equal = left.number == right.number; break;
                case Value::Type::String: equal = left.string == right.string; break;
                case Value::Type::Table: equal = left.table == right.table; break;
                case Value::Type::Function: equal = left.function == right.function; break;
                case Value::Type::Object: equal = left.object == right.object; break;
            }
        }
        return Value::booleanValue(op == "==" ? equal : !equal);
    }

    if (op == "<" || op == "<=" || op == ">" || op == ">=") {
        bool less = false;
        bool equal = false;
        if (left.isNumber() && right.isNumber()) {
            less = left.number < right.number;
            equal = left.number == right.number;
        } else if (left.isString() && right.isString()) {
            less = left.string < right.string;
            equal = left.string == right.string;
        } else {
            fail(expr.line, "cannot compare " + typeNameOf(left) + " with " + typeNameOf(right));
        }
        if (op == "<") return Value::booleanValue(less);
        if (op == "<=") return Value::booleanValue(less || equal);
        if (op == ">") return Value::booleanValue(!less && !equal);
        return Value::booleanValue(!less);
    }

    if (!left.isNumber() || !right.isNumber()) {
        fail(expr.line, std::string("cannot use '") + op + "' on " + typeNameOf(left) + " and " +
                            typeNameOf(right));
    }

    const double a = left.number;
    const double b = right.number;
    if (op == "+") return Value::numberValue(a + b);
    if (op == "-") return Value::numberValue(a - b);
    if (op == "*") return Value::numberValue(a * b);
    if (op == "/") return Value::numberValue(a / b);
    if (op == "%") return Value::numberValue(std::fmod(a, b));
    if (op == "^") return Value::numberValue(std::pow(a, b));

    fail(expr.line, "unknown operator '" + op + "'");
}

Value Interpreter::Impl::eval(Expr& expr, const EnvPtr& env) {
    step(expr.line);

    switch (expr.kind) {
        case Expr::Kind::Nil: return Value::nil();
        case Expr::Kind::True: return Value::booleanValue(true);
        case Expr::Kind::False: return Value::booleanValue(false);
        case Expr::Kind::Number: return Value::numberValue(expr.number);
        case Expr::Kind::String: return Value::stringValue(expr.text);
        case Expr::Kind::Name: {
            if (Value* slot = findVar(env, expr.text)) return *slot;
            return Value::nil();
        }
        case Expr::Kind::Function: {
            auto function = std::make_shared<Function>();
            function->params = expr.params;
            function->body = expr.body;
            function->closure = env;
            return Value::functionValue(function);
        }
        case Expr::Kind::Table: return evalTable(expr, env);
        case Expr::Kind::Unary: {
            const Value operand = eval(*expr.a, env);
            if (expr.text == "not") return Value::booleanValue(!operand.truthy());
            if (!operand.isNumber()) {
                fail(expr.line, "cannot negate a " + typeNameOf(operand));
            }
            return Value::numberValue(-operand.number);
        }
        case Expr::Kind::Binary: return evalBinary(expr, env);
        case Expr::Kind::Index: return evalIndex(expr, env);
        case Expr::Kind::Call: {
            const Value callable = eval(*expr.a, env);
            std::vector<Value> args;
            args.reserve(expr.list.size());
            for (ExprPtr& argument : expr.list) args.push_back(eval(*argument, env));
            return callValue(callable, args, expr.line);
        }
        case Expr::Kind::MethodCall: {
            const Value object = eval(*expr.a, env);
            std::vector<Value> args;
            args.reserve(expr.list.size());
            for (ExprPtr& argument : expr.list) args.push_back(eval(*argument, env));

            // Host objects resolve method calls themselves; their methods take the
            // arguments as written, without an implicit self parameter.
            if (object.isObject() && object.object) {
                Value out;
                std::string error;
                if (!object.object->call(*owner, expr.text, args, out, error)) {
                    fail(expr.line, error);
                }
                return out;
            }

            const Value method =
                lookupIndex(object, Value::stringValue(expr.text), expr.line);
            args.insert(args.begin(), object);
            return callValue(method, args, expr.line);
        }
    }

    fail(expr.line, "unsupported expression");
}

void Interpreter::Impl::assignTo(Expr& target, const Value& value, const EnvPtr& env) {
    if (target.kind == Expr::Kind::Name) {
        assignName(env, target.text, value);
        return;
    }
    if (target.kind == Expr::Kind::Index) {
        const Value object = eval(*target.a, env);
        const Value key = eval(*target.b, env);
        setIndex(object, key, value, target.line);
        return;
    }
    fail(target.line, "cannot assign to this expression");
}

Flow Interpreter::Impl::execBlock(Block& block, const EnvPtr& env, Value& returnValue) {
    for (StmtPtr& statement : block.statements) {
        const Flow flow = exec(*statement, env, returnValue);
        if (flow != Flow::Normal) return flow;
    }
    return Flow::Normal;
}

Flow Interpreter::Impl::exec(Stmt& stmt, const EnvPtr& env, Value& returnValue) {
    step(stmt.line);

    switch (stmt.kind) {
        case Stmt::Kind::Local: {
            for (std::size_t i = 0; i < stmt.names.size(); ++i) {
                Value value = Value::nil();
                if (i < stmt.values.size()) value = eval(*stmt.values[i], env);
                env->vars[stmt.names[i]] = value;
            }
            return Flow::Normal;
        }
        case Stmt::Kind::Assign: {
            std::vector<Value> values;
            values.reserve(stmt.values.size());
            for (ExprPtr& value : stmt.values) values.push_back(eval(*value, env));
            for (std::size_t i = 0; i < stmt.targets.size(); ++i) {
                assignTo(*stmt.targets[i],
                         i < values.size() ? values[i] : Value::nil(), env);
            }
            return Flow::Normal;
        }
        case Stmt::Kind::Expr:
            eval(*stmt.values[0], env);
            return Flow::Normal;
        case Stmt::Kind::If: {
            for (auto& branch : stmt.branches) {
                if (eval(*branch.first, env).truthy()) {
                    EnvPtr scope = std::make_shared<Environment>();
                    scope->parent = env;
                    return execBlock(*branch.second, scope, returnValue);
                }
            }
            if (stmt.elseBody) {
                EnvPtr scope = std::make_shared<Environment>();
                scope->parent = env;
                return execBlock(*stmt.elseBody, scope, returnValue);
            }
            return Flow::Normal;
        }
        case Stmt::Kind::While: {
            while (eval(*stmt.cond, env).truthy()) {
                EnvPtr scope = std::make_shared<Environment>();
                scope->parent = env;
                const Flow flow = execBlock(*stmt.body, scope, returnValue);
                if (flow == Flow::Break) break;
                if (flow == Flow::Return) return flow;
            }
            return Flow::Normal;
        }
        case Stmt::Kind::Repeat: {
            while (true) {
                EnvPtr scope = std::make_shared<Environment>();
                scope->parent = env;
                const Flow flow = execBlock(*stmt.body, scope, returnValue);
                if (flow == Flow::Break) break;
                if (flow == Flow::Return) return flow;
                if (eval(*stmt.cond, env).truthy()) break;
            }
            return Flow::Normal;
        }
        case Stmt::Kind::ForNum: {
            const double start = eval(*stmt.start, env).asNumber();
            const double limit = eval(*stmt.limit, env).asNumber();
            const double step = stmt.step ? eval(*stmt.step, env).asNumber() : 1.0;
            if (step == 0.0) fail(stmt.line, "a for loop step cannot be zero");

            for (double i = start; step > 0 ? i <= limit : i >= limit; i += step) {
                EnvPtr scope = std::make_shared<Environment>();
                scope->parent = env;
                scope->vars[stmt.names[0]] = Value::numberValue(i);
                const Flow flow = execBlock(*stmt.body, scope, returnValue);
                if (flow == Flow::Break) break;
                if (flow == Flow::Return) return flow;
            }
            return Flow::Normal;
        }
        case Stmt::Kind::Do: {
            EnvPtr scope = std::make_shared<Environment>();
            scope->parent = env;
            return execBlock(*stmt.body, scope, returnValue);
        }
        case Stmt::Kind::Return: {
            returnValue = Value::nil();
            if (!stmt.values.empty()) returnValue = eval(*stmt.values[0], env);
            return Flow::Return;
        }
        case Stmt::Kind::Break:
            return Flow::Break;
        case Stmt::Kind::FunctionDecl: {
            auto function = std::make_shared<Function>();
            function->closure = env;
            function->body = stmt.functionBody;
            function->params = stmt.params;
            if (stmt.hasSelf) function->params.insert(function->params.begin(), "self");
            assignTo(*stmt.functionTarget, Value::functionValue(function), env);
            return Flow::Normal;
        }
    }

    return Flow::Normal;
}

Value Interpreter::Impl::callValue(const Value& callable, const std::vector<Value>& args,
                                   int line) {
    if (!callable.isFunction()) {
        fail(line, "attempt to call a " + typeNameOf(callable) + " value");
    }

    Function& function = *callable.function;
    if (function.native) {
        return function.nativeCall(*owner, args);
    }

    EnvPtr scope = std::make_shared<Environment>();
    scope->parent = function.closure;
    for (std::size_t i = 0; i < function.params.size(); ++i) {
        scope->vars[function.params[i]] =
            i < args.size() ? args[i] : Value::nil();
    }

    Value result = Value::nil();
    if (function.body) {
        execBlock(*function.body, scope, result);
    }
    return result;
}

Value makeNativeFunction(std::function<Value(Interpreter&, const std::vector<Value>&)> fn) {
    auto function = std::make_shared<Function>();
    function->native = true;
    function->nativeCall = std::move(fn);
    return Value::functionValue(function);
}

Value makeTable() { return Value::tableValue(std::make_shared<Table>()); }

Value Value::nil() { return Value{}; }

Value Value::booleanValue(bool value) {
    Value result;
    result.type = Type::Boolean;
    result.boolean = value;
    return result;
}

Value Value::numberValue(double value) {
    Value result;
    result.type = Type::Number;
    result.number = value;
    return result;
}

Value Value::stringValue(std::string value) {
    Value result;
    result.type = Type::String;
    result.string = std::move(value);
    return result;
}

Value Value::tableValue(std::shared_ptr<Table> value) {
    Value result;
    result.type = Type::Table;
    result.table = std::move(value);
    return result;
}

Value Value::functionValue(std::shared_ptr<Function> value) {
    Value result;
    result.type = Type::Function;
    result.function = std::move(value);
    return result;
}

Value Value::objectValue(std::shared_ptr<ScriptObject> value) {
    Value result;
    result.type = Type::Object;
    result.object = std::move(value);
    return result;
}

std::string Value::toString() const {
    switch (type) {
        case Type::Nil: return "nil";
        case Type::Boolean: return boolean ? "true" : "false";
        case Type::Number: return numberToString(number);
        case Type::String: return string;
        case Type::Table: return "table";
        case Type::Function: return "function";
        case Type::Object: return object ? object->typeName() : "object";
    }
    return "nil";
}

Value Table::get(const Value& key) const {
    if (key.isString()) {
        const auto found = fields.find(key.string);
        return found == fields.end() ? Value::nil() : found->second;
    }
    if (key.isNumber()) {
        const double raw = key.number;
        if (raw >= 1.0 && raw <= static_cast<double>(elements.size())) {
            return elements[static_cast<std::size_t>(raw - 1.0)];
        }
    }
    return Value::nil();
}

void Table::set(const Value& key, const Value& value) {
    if (key.isString()) {
        fields[key.string] = value;
        return;
    }
    if (key.isNumber()) {
        long long index = static_cast<long long>(key.number);
        if (index < 1) index = 1;
        if (static_cast<std::size_t>(index) > elements.size()) {
            elements.resize(static_cast<std::size_t>(index), Value::nil());
        }
        elements[static_cast<std::size_t>(index - 1)] = value;
    }
}

void Signal::connect(const Value& handler) {
    if (handler.isFunction()) handlers_.push_back(handler);
}

bool SignalHandle::call(Interpreter&, const std::string& method,
                        const std::vector<Value>& args, Value& out, std::string& error) {
    if (method == "Connect") {
        if (args.empty() || !args[0].isFunction()) {
            error = "Connect expects a function";
            return false;
        }
        signal_->connect(args[0]);
        out = Value::nil();
        return true;
    }
    error = "Signal has no method '" + method + "'";
    return false;
}

Value makeSignalValue(Signal& signal) {
    return Value::objectValue(std::make_shared<SignalHandle>(&signal));
}

void Signal::fire(Interpreter& interpreter, const std::vector<Value>& args) const {
    const std::vector<Value> handlers = handlers_;
    for (const Value& handler : handlers) {
        Value result;
        std::string error;
        if (!interpreter.callFunction(handler, args, result, error)) {
            interpreter.print(error, true);
        }
    }
}

Interpreter::Interpreter() : impl_(std::make_unique<Impl>()) {
    impl_->owner = this;
    impl_->globals = std::make_shared<Environment>();

    auto joinArgs = [](const std::vector<Value>& args, const char* separator) {
        std::string text;
        for (std::size_t i = 0; i < args.size(); ++i) {
            if (i > 0) text += separator;
            text += args[i].toString();
        }
        return text;
    };

    impl_->globals->vars["print"] = makeNativeFunction(
        [joinArgs](Interpreter& interpreter, const std::vector<Value>& args) {
            interpreter.print(joinArgs(args, "\t"), false);
            return Value::nil();
        });
    impl_->globals->vars["warn"] = makeNativeFunction(
        [joinArgs](Interpreter& interpreter, const std::vector<Value>& args) {
            interpreter.print(joinArgs(args, "\t"), true);
            return Value::nil();
        });
    impl_->globals->vars["type"] = makeNativeFunction(
        [](Interpreter&, const std::vector<Value>& args) {
            return Value::stringValue(args.empty() ? "nil" : typeNameOf(args[0]));
        });
    impl_->globals->vars["tostring"] = makeNativeFunction(
        [](Interpreter&, const std::vector<Value>& args) {
            return Value::stringValue(args.empty() ? "nil" : args[0].toString());
        });
    impl_->globals->vars["tonumber"] = makeNativeFunction(
        [](Interpreter&, const std::vector<Value>& args) {
            if (args.empty()) return Value::nil();
            if (args[0].isNumber()) return args[0];
            if (!args[0].isString()) return Value::nil();
            const std::string text = args[0].string;
            char* end = nullptr;
            const double parsed = std::strtod(text.c_str(), &end);
            if (end == text.c_str() || (end && *end != '\0')) return Value::nil();
            return Value::numberValue(parsed);
        });
    impl_->globals->vars["error"] = makeNativeFunction(
        [](Interpreter& interpreter, const std::vector<Value>& args) -> Value {
            interpreter.raise(args.empty() ? "script error" : args[0].toString());
            return Value::nil();
        });

    Value mathValue = makeTable();
    auto& math = mathValue.table->fields;
    math["pi"] = Value::numberValue(3.14159265358979323846);
    math["abs"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::fabs(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["ceil"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::ceil(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["floor"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::floor(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["sqrt"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::sqrt(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["sin"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::sin(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["cos"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::cos(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["tan"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        return Value::numberValue(std::tan(args.empty() ? 0.0 : args[0].asNumber()));
    });
    math["min"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        double value = args.empty() ? 0.0 : args[0].asNumber();
        for (std::size_t i = 1; i < args.size(); ++i) {
            value = std::min(value, args[i].asNumber());
        }
        return Value::numberValue(value);
    });
    math["max"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        double value = args.empty() ? 0.0 : args[0].asNumber();
        for (std::size_t i = 1; i < args.size(); ++i) {
            value = std::max(value, args[i].asNumber());
        }
        return Value::numberValue(value);
    });
    math["random"] = makeNativeFunction([](Interpreter&, const std::vector<Value>& args) {
        if (args.size() >= 2) {
            const long long low = static_cast<long long>(args[0].asNumber());
            const long long high = static_cast<long long>(args[1].asNumber());
            if (high <= low) return Value::numberValue(static_cast<double>(low));
            const long long span = high - low + 1;
            return Value::numberValue(static_cast<double>(low + (std::rand() % span)));
        }
        return Value::numberValue(static_cast<double>(std::rand()) / static_cast<double>(RAND_MAX));
    });
    impl_->globals->vars["math"] = mathValue;
}

Interpreter::~Interpreter() = default;

void Interpreter::setPrintHandler(std::function<void(const std::string&, bool)> handler) {
    impl_->printHandler = std::move(handler);
}

void Interpreter::print(const std::string& message, bool isError) const {
    if (impl_->printHandler) {
        impl_->printHandler(message, isError);
        return;
    }
    std::fprintf(isError ? stderr : stdout, "%s\n", message.c_str());
}

void Interpreter::setGlobal(const std::string& name, const Value& value) {
    impl_->globals->vars[name] = value;
}

Value Interpreter::getGlobal(const std::string& name) const {
    const auto found = impl_->globals->vars.find(name);
    return found == impl_->globals->vars.end() ? Value::nil() : found->second;
}

bool Interpreter::hasGlobal(const std::string& name) const {
    return impl_->globals->vars.find(name) != impl_->globals->vars.end();
}

void Interpreter::raise(const std::string& message) const {
    throw ScriptError{message, 0};
}

void Interpreter::setStepLimit(long long limit) { impl_->stepLimit = limit; }

namespace {

std::string describeError(const std::string& chunkName, const ScriptError& failure) {
    if (failure.line > 0) {
        return chunkName + ":" + std::to_string(failure.line) + ": " + failure.message;
    }
    return chunkName + ": " + failure.message;
}

}  // namespace

bool Interpreter::run(const std::string& source, const std::string& chunkName,
                      std::string& error) {
    std::vector<Token> tokens;
    std::string lexError;
    Lexer lexer(source);
    if (!lexer.lex(tokens, lexError)) {
        error = chunkName + ": " + lexError;
        return false;
    }

    Parser parser(std::move(tokens));
    auto block = parser.parseChunk(error);
    if (!block) {
        error = chunkName + ": " + error;
        return false;
    }

    impl_->steps = 0;
    EnvPtr scope = std::make_shared<Environment>();
    scope->parent = impl_->globals;
    Value returnValue;
    try {
        impl_->execBlock(*block, scope, returnValue);
    } catch (const ScriptError& failure) {
        error = describeError(chunkName, failure);
        return false;
    } catch (const std::exception& failure) {
        error = chunkName + ": " + failure.what();
        return false;
    }
    return true;
}

bool Interpreter::callFunction(const Value& function, const std::vector<Value>& args,
                               Value& result, std::string& error) {
    if (!function.isFunction()) {
        error = "attempt to call a non-function value";
        return false;
    }

    impl_->steps = 0;
    try {
        result = impl_->callValue(function, args, 0);
    } catch (const ScriptError& failure) {
        error = failure.line > 0 ? "line " + std::to_string(failure.line) + ": " + failure.message
                                 : failure.message;
        return false;
    } catch (const std::exception& failure) {
        error = failure.what();
        return false;
    }
    return true;
}

}  // namespace script
}  // namespace rblx
