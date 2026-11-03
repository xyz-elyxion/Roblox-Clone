#include "CxxInternal.h"

#include <algorithm>
#include <utility>

namespace rblx {
namespace cxx {
namespace detail {

namespace {

int roundUp(int value, int alignment) {
    if (alignment <= 1) return value;
    return (value + alignment - 1) / alignment * alignment;
}

struct ParseError {
    Diagnostic diagnostic;
};

bool isTypeKeyword(const std::string& text) {
    static const char* kWords[] = {"void",        "bool",   "char",    "int",     "long",
                                   "short",       "signed", "unsigned", "struct",  "const",
                                   "volatile",    "static", "extern",  "auto",    "register",
                                   "float",       "double", "typedef", "class",   "enum",
                                   "union",       "template"};
    for (const char* word : kWords) {
        if (text == word) return true;
    }
    return false;
}

class Parser {
public:
    Parser(const std::vector<Token>& tokens, const Options& options)
        : tokens_(tokens), options_(options) {}

    std::unique_ptr<Program> parseUnit() {
        program_ = std::unique_ptr<Program>(new Program());
        while (current().kind != TokenKind::End) {
            if (accept(";")) continue;
            topLevel();
        }
        if (program_->functions.find("main") == program_->functions.end()) {
            fail("no main function: a program needs int main()");
        }
        return std::move(program_);
    }

private:
    // ------------------------------------------------------------- tokens ----

    const Token& current() const { return tokens_[index_]; }
    const Token& look(int ahead) const {
        std::size_t position = index_ + static_cast<std::size_t>(ahead);
        return position < tokens_.size() ? tokens_[position] : tokens_.back();
    }
    bool at(const char* text) const {
        return current().kind != TokenKind::End && current().text == text;
    }
    bool atLook(int ahead, const char* text) const {
        const Token& token = look(ahead);
        return token.kind != TokenKind::End && token.text == text;
    }
    bool accept(const char* text) {
        if (!at(text)) return false;
        ++index_;
        return true;
    }
    const Token& advance() {
        const Token& token = tokens_[index_];
        if (index_ + 1 < tokens_.size()) ++index_;
        return token;
    }
    void expect(const char* text, const char* what) {
        if (accept(text)) return;
        fail(std::string("expected '") + text + "' " + what);
    }
    [[noreturn]] void fail(const std::string& message) const {
        ParseError error;
        error.diagnostic.line = current().line;
        error.diagnostic.column = current().column;
        error.diagnostic.message = message;
        throw error;
    }
    std::string expectName(const char* what) {
        if (current().kind != TokenKind::Identifier) {
            fail(std::string("expected a name ") + what);
        }
        return advance().text;
    }

    // -------------------------------------------------------------- types ----

    struct Specifiers {
        TypePtr type;
        bool isStatic = false;
        bool isExtern = false;
    };

    bool startsType() const {
        return current().kind == TokenKind::Identifier && isTypeKeyword(current().text);
    }

    Specifiers parseSpecifiers() {
        Specifiers specifiers;
        bool haveType = false;
        while (current().kind == TokenKind::Identifier) {
            const std::string word = current().text;
            if (word == "const" || word == "volatile" || word == "auto" || word == "register" ||
                word == "signed") {
                advance();
                continue;
            }
            if (word == "static") {
                specifiers.isStatic = true;
                advance();
                continue;
            }
            if (word == "extern") {
                specifiers.isExtern = true;
                advance();
                continue;
            }
            if (word == "unsigned") {
                fail("unsigned types are not supported yet; every integer here is signed");
            }
            if (word == "short") fail("short is not supported; use int");
            if (word == "float" || word == "double") {
                fail("floating point is not supported yet; use int or long");
            }
            if (word == "typedef") fail("typedef is not supported yet");
            if (word == "class" || word == "template" || word == "enum" || word == "union") {
                fail(word + " is not supported yet");
            }
            if (word == "struct") {
                if (haveType) break;
                specifiers.type = parseStruct();
                haveType = true;
                continue;
            }
            if (word == "void" || word == "bool" || word == "char" || word == "int" ||
                word == "long") {
                if (haveType) {
                    if (word == "long" && specifiers.type->kind == TypeKind::Long) {
                        advance();  // `long long` is one 64-bit integer
                        continue;
                    }
                    if (word == "int" && specifiers.type->kind == TypeKind::Long) {
                        advance();  // `long int`
                        continue;
                    }
                    break;
                }
                advance();
                if (word == "long") {
                    specifiers.type = primitiveType(TypeKind::Long);
                    if (at("long")) advance();
                    if (at("int")) advance();
                } else if (word == "void") {
                    specifiers.type = primitiveType(TypeKind::Void);
                } else if (word == "bool") {
                    specifiers.type = primitiveType(TypeKind::Bool);
                } else if (word == "char") {
                    specifiers.type = primitiveType(TypeKind::Char);
                } else {
                    specifiers.type = primitiveType(TypeKind::Int);
                }
                haveType = true;
                continue;
            }
            break;
        }
        return specifiers;
    }

    TypePtr parseStruct() {
        advance();  // struct
        std::string tag = expectName("after struct");
        if (!at("{")) {
            auto existing = structs_.find(tag);
            if (existing == structs_.end()) {
                fail("struct '" + tag + "' must be defined before it is used");
            }
            return existing->second;
        }

        advance();  // {
        auto type = std::make_shared<Type>();
        type->kind = TypeKind::Struct;
        type->name = tag;
        // Registered before the body is read so a field can point back at the
        // struct being defined - the next pointer of a list node.
        structs_[tag] = type;

        int offset = 0;
        int alignment = 1;
        while (!at("}")) {
            if (current().kind == TokenKind::End) fail("the struct body never closes");
            Specifiers fieldSpecifiers = parseSpecifiers();
            if (!fieldSpecifiers.type) fail("a struct field needs a type");
            do {
                std::string fieldName;
                TypePtr fieldType = parseDeclarator(fieldSpecifiers.type, fieldName, false);
                if (fieldType->isVoid()) fail("a struct field cannot be void");
                if (fieldType->isStruct()) {
                    fail("a struct cannot hold a struct by value; use a pointer");
                }
                for (const Field& existing : type->fields) {
                    if (existing.name == fieldName) {
                        fail("struct '" + tag + "' already has a field named '" + fieldName + "'");
                    }
                }
                offset = roundUp(offset, fieldType->align);
                Field field;
                field.name = fieldName;
                field.type = fieldType;
                field.offset = offset;
                type->fields.push_back(field);
                offset += fieldType->size;
                alignment = std::max(alignment, fieldType->align);
            } while (accept(","));
            expect(";", "after a struct field");
        }
        expect("}", "to close the struct body");

        type->size = roundUp(offset, alignment);
        type->align = alignment;
        if (type->size == 0) fail("struct '" + tag + "' has no fields");
        structs_[tag] = type;
        return type;
    }

    // Declarator tail: pointer stars, the name, then array dimensions.
    TypePtr parseDeclarator(const TypePtr& base, std::string& name, bool allowAbstract) {
        TypePtr type = base;
        while (accept("*")) type = pointerType(type);

        if (current().kind == TokenKind::Identifier && !isTypeKeyword(current().text)) {
            name = advance().text;
        } else if (!allowAbstract) {
            fail("expected a name in the declaration");
        }

        std::vector<long long> dimensions;
        while (at("[")) {
            advance();
            if (accept("]")) {
                dimensions.push_back(-1);  // only a parameter may leave it open
                continue;
            }
            ExprPtr size = parseAssignment();
            bool constant = true;
            long long value = fold(*size, constant);
            if (!constant) fail("an array length must be a constant");
            if (value <= 0) fail("an array length must be at least 1");
            dimensions.push_back(value);
            expect("]", "to close the array length");
        }
        for (std::size_t i = dimensions.size(); i-- > 0;) {
            long long length = dimensions[i];
            if (length < 0) {
                if (i != 0) fail("only the outermost array length may be left open");
                length = 0;
            }
            type = arrayType(type, length);
        }
        return type;
    }

    // ------------------------------------------------------------ scopes ----

    struct Variable {
        TypePtr type;
        bool global = false;
        int offset = 0;
    };

    const Variable* lookup(const std::string& name) const {
        for (std::size_t i = scopes_.size(); i-- > 0;) {
            auto found = scopes_[i].find(name);
            if (found != scopes_[i].end()) return &found->second;
        }
        auto global = globals_.find(name);
        if (global != globals_.end()) return &global->second;
        return nullptr;
    }

    int allocateLocal(const TypePtr& type) {
        int size = type->size > 0 ? type->size : 8;
        int alignment = type->align > 0 ? type->align : 1;
        localBytes_ = roundUp(localBytes_ + size, alignment);
        return -localBytes_;
    }

    void declareLocal(const std::string& name, const TypePtr& type, int offset) {
        if (scopes_.back().find(name) != scopes_.back().end()) {
            fail("'" + name + "' is already declared in this scope");
        }
        Variable variable;
        variable.type = type;
        variable.offset = offset;
        scopes_.back()[name] = variable;
    }

    // ------------------------------------------------------------ symbols ----

    void topLevel() {
        Specifiers specifiers = parseSpecifiers();
        if (!specifiers.type) fail("expected a declaration");
        if (accept(";")) return;  // `struct S { ... };` is a type, not a variable

        std::string name;
        TypePtr type = parseDeclarator(specifiers.type, name, false);
        if (at("(")) {
            parseFunction(specifiers, name, type);
            return;
        }
        parseGlobal(specifiers, name, type);
        while (accept(",")) {
            std::string nextName;
            TypePtr nextType = parseDeclarator(specifiers.type, nextName, false);
            parseGlobal(specifiers, nextName, nextType);
        }
        expect(";", "after a global declaration");
    }

    void parseGlobal(const Specifiers& specifiers, const std::string& name, const TypePtr& type) {
        if (type->isVoid()) fail("'" + name + "' cannot be void");
        if (globals_.find(name) != globals_.end()) fail("'" + name + "' is already declared");

        GlobalDef definition;
        definition.name = name;
        definition.type = type;
        definition.internal = specifiers.isStatic;
        definition.external = specifiers.isExtern;
        definition.line = current().line;

        if (accept("=")) {
            if (definition.external) fail("an extern global cannot have an initialiser");
            if (type->isAggregate()) {
                fail("a global array or struct cannot be initialised with '='; fill it at run time");
            }
            ExprPtr initialiser = parseAssignment();
            definition.hasInit = true;
            if (initialiser->kind == ExprKind::String && type->isPointer()) {
                if (!initialiser->type->isPointer() || initialiser->type->element->kind != TypeKind::Char) {
                    fail("a string can only initialise a char pointer");
                }
                definition.stringInit = true;
                definition.stringLabel = "Lstr" + std::to_string(initialiser->number);
            } else {
                if (!type->isScalar()) fail("a global must hold a number or a pointer");
                bool constant = true;
                definition.intInit = fold(*initialiser, constant);
                if (!constant) {
                    fail("a global initialiser must be a constant; assign it inside main instead");
                }
            }
        }

        Variable variable;
        variable.type = type;
        variable.global = true;
        globals_[name] = variable;
        program_->globals.push_back(definition);
    }

    void parseFunction(const Specifiers& specifiers, const std::string& name, const TypePtr& result) {
        advance();  // (
        FunctionDecl* declaration = nullptr;
        auto existing = program_->functions.find(name);
        if (existing != program_->functions.end()) {
            declaration = existing->second.get();
            if (typeName(declaration->result) != typeName(result)) {
                fail("'" + name + "' was already declared returning " + typeName(declaration->result));
            }
        } else {
            auto created = std::unique_ptr<FunctionDecl>(new FunctionDecl());
            created->name = name;
            created->result = result;
            created->internal = specifiers.isStatic;
            created->line = current().line;
            declaration = created.get();
            program_->functions[name] = std::move(created);
        }

        std::vector<Parameter> parameters;
        bool variadic = false;
        if (!at(")")) {
            if (at("void") && look(1).is(")")) {
                advance();
            } else {
                do {
                    if (accept("...")) {
                        variadic = true;
                        break;
                    }
                    Specifiers parameterSpecifiers = parseSpecifiers();
                    if (!parameterSpecifiers.type) fail("a parameter needs a type");
                    std::string parameterName;
                    TypePtr parameterType =
                        parseDeclarator(parameterSpecifiers.type, parameterName, true);
                    if (parameterType->isArray()) {
                        if (parameterType->elements != 0) {
                            fail("an array parameter is a pointer; write it as a pointer");
                        }
                        parameterType = pointerType(parameterType->element);
                    }
                    if (parameterType->isVoid()) fail("a parameter cannot be void");
                    if (parameterType->isStruct()) {
                        fail("pass a struct by pointer rather than by value");
                    }
                    Parameter parameter;
                    parameter.name = parameterName;
                    parameter.type = parameterType;
                    parameters.push_back(parameter);
                } while (accept(","));
            }
        }
        expect(")", "to close the parameter list");

        if (!declaration->defined) {
            declaration->parameters = parameters;
            declaration->variadic = variadic;
        } else if (declaration->parameters.size() != parameters.size()) {
            fail("'" + name + "' is defined with a different parameter count");
        }

        if (accept(";")) return;  // a declaration only

        if (declaration->defined) fail("'" + name + "' is defined twice");
        declaration->defined = true;
        declaration->parameters = parameters;
        declaration->variadic = variadic;

        // A function body gets its own scope stack: nothing outside is visible
        // except globals and other functions.
        std::vector<std::map<std::string, Variable>> outerScopes;
        outerScopes.swap(scopes_);
        int outerBytes = localBytes_;
        localBytes_ = 0;
        int outerLoops = loopDepth_;
        loopDepth_ = 0;
        FunctionDecl* outerFunction = currentFunction_;
        currentFunction_ = declaration;

        scopes_.push_back(std::map<std::string, Variable>());
        for (Parameter& parameter : declaration->parameters) {
            if (parameter.name.empty()) continue;
            int offset = allocateLocal(parameter.type);
            parameter.offset = offset;
            declareLocal(parameter.name, parameter.type, offset);
        }

        StmtPtr body = parseBlock();

        FunctionDef definition;
        definition.declaration = declaration;
        definition.body = std::move(body);
        definition.localBytes = localBytes_;
        definition.parameterCount = static_cast<int>(declaration->parameters.size());
        program_->definitions.push_back(std::move(definition));

        scopes_.clear();
        scopes_.swap(outerScopes);
        localBytes_ = outerBytes;
        loopDepth_ = outerLoops;
        currentFunction_ = outerFunction;
    }

    // --------------------------------------------------------- statements ----

    StmtPtr makeStatement(StmtKind kind) {
        auto statement = StmtPtr(new Stmt());
        statement->kind = kind;
        statement->line = current().line;
        statement->column = current().column;
        return statement;
    }

    StmtPtr parseBlock() {
        expect("{", "to open a block");
        auto block = makeStatement(StmtKind::Block);
        scopes_.push_back(std::map<std::string, Variable>());
        while (!at("}")) {
            if (current().kind == TokenKind::End) fail("a block is missing its closing }");
            block->body.push_back(parseStatement());
        }
        scopes_.pop_back();
        expect("}", "to close the block");
        return block;
    }

    StmtPtr parseStatement() {
        if (at("{")) return parseBlock();
        if (at(";")) {
            StmtPtr empty = makeStatement(StmtKind::Empty);
            advance();
            return empty;
        }
        if (at("if")) return parseIf();
        if (at("while")) return parseWhile();
        if (at("do")) return parseDoWhile();
        if (at("for")) return parseFor();
        if (at("return")) return parseReturn();
        if (at("break") || at("continue")) {
            StmtPtr statement = makeStatement(at("break") ? StmtKind::Break : StmtKind::Continue);
            if (loopDepth_ == 0) {
                fail(statement->kind == StmtKind::Break ? "'break' outside a loop"
                                                        : "'continue' outside a loop");
            }
            advance();
            expect(";", "after the statement");
            return statement;
        }
        if (startsType()) {
            StmtPtr declaration = makeStatement(StmtKind::Declaration);
            parseLocalDeclaration(*declaration);
            return declaration;
        }
        StmtPtr statement = makeStatement(StmtKind::Expression);
        statement->expr = parseAssignment();
        expect(";", "after the expression");
        return statement;
    }

    // `int a = 1, b;` - consumes the trailing semicolon.
    void parseLocalDeclaration(Stmt& statement) {
        Specifiers specifiers = parseSpecifiers();
        if (!specifiers.type) fail("expected a type");
        if (specifiers.isStatic) {
            fail("static locals are not supported yet; use a global");
        }
        if (specifiers.isExtern) fail("extern is only allowed at file scope");
        if (accept(";")) return;  // `struct Point { int x; int y; };` declares only a type
        do {
            std::string name;
            TypePtr type = parseDeclarator(specifiers.type, name, false);
            if (type->isVoid()) fail("'" + name + "' cannot be void");

            LocalDecl declaration;
            declaration.name = name;
            declaration.type = type;
            declaration.offset = allocateLocal(type);

            if (accept("=")) {
                ExprPtr initialiser = parseAssignment();
                checkAssignment(type, initialiser, "'" + name + "'");
                declaration.init = std::move(initialiser);
            }
            declareLocal(name, type, declaration.offset);
            statement.declarations.push_back(std::move(declaration));
        } while (accept(","));
        expect(";", "after the declaration");
    }

    StmtPtr parseIf() {
        advance();
        expect("(", "after if");
        StmtPtr statement = makeStatement(StmtKind::If);
        statement->expr = parseAssignment();
        requireScalar(statement->expr, "an if condition");
        expect(")", "after the condition");
        statement->thenBranch = parseStatement();
        if (accept("else")) statement->elseBranch = parseStatement();
        return statement;
    }

    StmtPtr parseWhile() {
        advance();
        expect("(", "after while");
        StmtPtr statement = makeStatement(StmtKind::While);
        statement->expr = parseAssignment();
        requireScalar(statement->expr, "a while condition");
        expect(")", "after the condition");
        ++loopDepth_;
        statement->thenBranch = parseStatement();
        --loopDepth_;
        return statement;
    }

    StmtPtr parseDoWhile() {
        advance();
        StmtPtr statement = makeStatement(StmtKind::DoWhile);
        ++loopDepth_;
        statement->thenBranch = parseStatement();
        --loopDepth_;
        expect("while", "after the body of do");
        expect("(", "after while");
        statement->expr = parseAssignment();
        requireScalar(statement->expr, "a do-while condition");
        expect(")", "after the condition");
        expect(";", "after do { } while (...);");
        return statement;
    }

    StmtPtr parseFor() {
        advance();
        expect("(", "after for");
        StmtPtr statement = makeStatement(StmtKind::For);
        scopes_.push_back(std::map<std::string, Variable>());
        if (at(";")) {
            advance();
        } else if (startsType()) {
            StmtPtr init = makeStatement(StmtKind::Declaration);
            parseLocalDeclaration(*init);
            statement->init = std::move(init);
        } else {
            StmtPtr init = makeStatement(StmtKind::Expression);
            init->expr = parseAssignment();
            expect(";", "after the for initialiser");
            statement->init = std::move(init);
        }
        if (!at(";")) {
            statement->expr = parseAssignment();
            requireScalar(statement->expr, "a for condition");
        }
        expect(";", "after the for condition");
        if (!at(")")) statement->step = parseAssignment();
        expect(")", "after the for header");
        ++loopDepth_;
        statement->thenBranch = parseStatement();
        --loopDepth_;
        scopes_.pop_back();
        return statement;
    }

    StmtPtr parseReturn() {
        StmtPtr statement = makeStatement(StmtKind::Return);
        advance();
        if (accept(";")) {
            if (currentFunction_ && !currentFunction_->result->isVoid()) {
                fail("this function returns " + typeName(currentFunction_->result) +
                     "; return a value");
            }
            return statement;
        }
        if (!currentFunction_) fail("return outside a function");
        if (currentFunction_->result->isVoid()) fail("this function is void; return nothing");
        statement->expr = parseAssignment();
        checkAssignment(currentFunction_->result, statement->expr, "the return value");
        expect(";", "after the return value");
        return statement;
    }

    // -------------------------------------------------------- expressions ----

    ExprPtr makeExpr(ExprKind kind) {
        auto expression = ExprPtr(new Expr());
        expression->kind = kind;
        expression->line = current().line;
        expression->column = current().column;
        return expression;
    }

    void requireScalar(const ExprPtr& expression, const char* what) const {
        if (!expression->type || !expression->type->isScalar()) {
            fail(std::string(what) + " must be a number or a pointer");
        }
    }

    void checkAssignment(const TypePtr& to, const ExprPtr& value, const std::string& target) {
        if (!value->type) return;
        if (to->isStruct() || value->type->isStruct()) {
            if (typeName(to) != typeName(value->type)) {
                fail(target + " cannot hold " + typeName(value->type));
            }
            return;
        }
        if (!to->isScalar()) {
            if (to->isArray()) fail(target + " is an array and cannot be assigned");
            fail(target + " cannot hold a value");
        }
        if (!value->type->isScalar() && !value->type->isArray()) {
            fail(target + " cannot hold " + typeName(value->type));
        }
        if (to->isPointer() && value->type->isInteger() && value->kind != ExprKind::Integer &&
            value->kind != ExprKind::Character) {
            fail(target + " is a pointer; assign a pointer, not a number");
        }
        if (to->isInteger() && value->type->isPointer()) {
            fail(std::string("cannot put a pointer into ") + typeName(to) + " without a cast");
        }
    }

    ExprPtr parseAssignment() {
        ExprPtr left = parseConditional();
        static const char* kAssignments[] = {"=",  "+=", "-=", "*=",  "/=", "%=",
                                             "&=", "|=", "^=", "<<=", ">>="};
        for (const char* op : kAssignments) {
            if (!at(op)) continue;
            advance();
            ExprPtr right = parseAssignment();
            if (!left->lvalue) fail("the left side of an assignment must be a variable, an element or a field");
            if (left->type->isArray()) fail("an array cannot be assigned");

            bool compound = std::string(op) != "=";
            if (compound) {
                std::string binaryOp = std::string(op).substr(0, std::string(op).size() - 1);
                if (left->type->isStruct()) fail("a struct cannot be changed with " + std::string(op));
                checkBinary(binaryOp, left->type, right->type, left->line);
            } else {
                checkAssignment(left->type, right, "this variable");
            }

            auto expression = ExprPtr(new Expr());
            expression->kind = ExprKind::Assign;
            expression->text = op;
            expression->line = left->line;
            expression->column = left->column;
            expression->type = left->type;
            expression->a = std::move(left);
            expression->b = std::move(right);
            left = std::move(expression);
            break;
        }
        return left;
    }

    ExprPtr parseConditional() {
        ExprPtr condition = parseBinary(0);
        if (!at("?")) return condition;
        advance();
        ExprPtr whenTrue = parseAssignment();
        expect(":", "in a ?: expression");
        ExprPtr whenFalse = parseConditional();
        requireScalar(condition, "a ?: condition");

        auto expression = ExprPtr(new Expr());
        expression->kind = ExprKind::Conditional;
        expression->line = condition->line;
        expression->column = condition->column;
        expression->a = std::move(condition);
        expression->b = std::move(whenTrue);
        expression->c = std::move(whenFalse);
        expression->type = unify(expression->b->type, expression->c->type);
        return expression;
    }

    static int precedenceOf(const std::string& text, const char*& operation) {
        static const struct {
            const char* op;
            int level;
        } kOperators[] = {{"||", 0}, {"&&", 1}, {"|", 2},  {"^", 3},  {"&", 4},
                          {"==", 5}, {"!=", 5}, {"<", 6},  {">", 6},  {"<=", 6},
                          {">=", 6}, {"<<", 7}, {">>", 7}, {"+", 8},  {"-", 8},
                          {"*", 9},  {"/", 9},  {"%", 9}};
        for (const auto& entry : kOperators) {
            if (text == entry.op) {
                operation = entry.op;
                return entry.level;
            }
        }
        return -1;
    }

    ExprPtr parseBinary(int level) {
        if (level > 9) return parseUnary();
        ExprPtr left = parseBinary(level + 1);
        for (;;) {
            if (current().kind != TokenKind::Punctuation) break;
            const char* operation = nullptr;
            if (precedenceOf(current().text, operation) != level) break;
            advance();
            ExprPtr right = parseBinary(level + 1);
            left = buildBinary(operation, std::move(left), std::move(right));
        }
        return left;
    }

    ExprPtr buildBinary(const std::string& op, ExprPtr left, ExprPtr right) {
        if (op == "&&" || op == "||") {
            requireScalar(left, "the left side");
            requireScalar(right, "the right side");
        } else {
            checkBinary(op, left->type, right->type, left->line);
        }

        auto expression = ExprPtr(new Expr());
        expression->kind = ExprKind::Binary;
        expression->text = op;
        expression->line = left->line;
        expression->column = left->column;

        const bool comparison = op == "==" || op == "!=" || op == "<" || op == ">" ||
                                op == "<=" || op == ">=" || op == "&&" || op == "||";
        const bool leftPointer = left->type->isPointerLike();
        const bool rightPointer = right->type->isPointerLike();

        if (comparison) {
            expression->type = primitiveType(TypeKind::Int);
        } else if ((op == "+" || op == "-") && (leftPointer || rightPointer)) {
            if (leftPointer && rightPointer) {
                expression->type = primitiveType(TypeKind::Long);
            } else {
                expression->type = leftPointer ? left->type : right->type;
            }
        } else if (left->type->kind == TypeKind::Long || right->type->kind == TypeKind::Long) {
            expression->type = primitiveType(TypeKind::Long);
        } else {
            expression->type = primitiveType(TypeKind::Int);
        }

        expression->a = std::move(left);
        expression->b = std::move(right);
        return expression;
    }

    void checkBinary(const std::string& op, const TypePtr& left, const TypePtr& right,
                     int line) const {
        (void)line;
        if (!left || !right) return;
        if (left->isStruct() || right->isStruct()) {
            fail("a struct cannot be used with '" + op + "'");
        }
        // Arrays are allowed on the pointer-like paths below, where they behave
        // as a pointer to their first element; every other operator rejects them
        // through the integer checks.

        if (op == "*" || op == "/" || op == "%" || op == "<<" || op == ">>" || op == "&" ||
            op == "|" || op == "^") {
            if (!left->isInteger() || !right->isInteger()) {
                fail("'" + op + "' needs numbers on both sides, not " + typeName(left) + " and " +
                     typeName(right));
            }
            return;
        }
        if (op == "+" || op == "-") {
            if (left->isPointerLike() && right->isPointerLike()) {
                if (op == "+") fail("two pointers cannot be added; subtract them instead");
                return;
            }
            if (left->isPointerLike()) {
                if (!right->isInteger()) {
                    fail(std::string("a pointer can only be moved by a number, not ") +
                         typeName(right));
                }
                return;
            }
            if (right->isPointerLike()) {
                if (!left->isInteger()) {
                    fail(std::string("a pointer can only be moved by a number, not ") +
                         typeName(left));
                }
                return;
            }
            if (!left->isInteger() || !right->isInteger()) {
                fail("'" + op + "' needs numbers on both sides");
            }
            return;
        }
        if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" || op == ">=") {
            if (!left->isScalar() || !right->isScalar()) {
                fail("a comparison needs numbers or pointers on both sides");
            }
            return;
        }
    }

    static TypePtr unify(const TypePtr& left, const TypePtr& right) {
        if (!left) return right;
        if (!right) return left;
        if (left->isPointer() || right->isPointer()) {
            return left->isPointer() ? left : right;
        }
        if (left->kind == TypeKind::Long || right->kind == TypeKind::Long) {
            return primitiveType(TypeKind::Long);
        }
        return primitiveType(TypeKind::Int);
    }

    ExprPtr parseUnary() {
        if (at("sizeof")) return parseSizeOf();

        // A cast starts with '(' and a type keyword; anything else in parens is
        // an ordinary grouped expression.
        if (at("(") && look(1).kind == TokenKind::Identifier &&
            isTypeKeyword(look(1).text)) {
            advance();
            TypePtr target = parseTypeName();
            expect(")", "after the cast type");
            ExprPtr operand = parseUnary();
            if (!target->isScalar()) fail("a cast can only target a number or a pointer");
            if (!operand->type->isScalar()) {
                fail("cannot cast " + typeName(operand->type) + " with a cast");
            }
            auto expression = ExprPtr(new Expr());
            expression->kind = ExprKind::Cast;
            expression->line = operand->line;
            expression->column = operand->column;
            expression->type = target;
            expression->a = std::move(operand);
            return expression;
        }

        if (at("-") || at("+") || at("!") || at("~") || at("*") || at("&")) {
            std::string op = advance().text;
            ExprPtr operand = parseUnary();
            if (op == "*") {
                if (!operand->type->isPointer()) {
                    fail("'*' needs a pointer, not " + typeName(operand->type));
                }
                auto expression = ExprPtr(new Expr());
                expression->kind = ExprKind::Unary;
                expression->text = op;
                expression->line = operand->line;
                expression->column = operand->column;
                expression->type = operand->type->element;
                expression->lvalue = !expression->type->isVoid();
                expression->a = std::move(operand);
                return expression;
            }
            if (op == "&") {
                if (!operand->lvalue) fail("'&' needs a variable, an element or a field");
                auto expression = ExprPtr(new Expr());
                expression->kind = ExprKind::Unary;
                expression->text = op;
                expression->line = operand->line;
                expression->column = operand->column;
                expression->type = pointerType(operand->type);
                expression->a = std::move(operand);
                return expression;
            }
            if (op == "!") {
                requireScalar(operand, "the operand of '!'");
                auto expression = ExprPtr(new Expr());
                expression->kind = ExprKind::Unary;
                expression->text = op;
                expression->type = primitiveType(TypeKind::Int);
                expression->a = std::move(operand);
                return expression;
            }
            if (!operand->type->isInteger()) {
                fail("'" + op + "' needs a number, not " + typeName(operand->type));
            }
            auto expression = ExprPtr(new Expr());
            expression->kind = ExprKind::Unary;
            expression->text = op;
            expression->type = operand->type->kind == TypeKind::Long
                                   ? operand->type
                                   : primitiveType(TypeKind::Int);
            expression->a = std::move(operand);
            return expression;
        }

        if (at("++") || at("--")) {
            std::string op = advance().text;
            ExprPtr operand = parseUnary();
            if (!operand->lvalue) fail("'" + op + "' needs a variable, an element or a field");
            if (!operand->type->isScalar()) fail("'" + op + "' needs a number or a pointer");
            auto expression = ExprPtr(new Expr());
            expression->kind = ExprKind::Unary;
            expression->text = op;
            expression->type = operand->type;
            expression->a = std::move(operand);
            return expression;
        }

        return parsePostfix();
    }

    TypePtr parseTypeName() {
        Specifiers specifiers = parseSpecifiers();
        if (!specifiers.type) fail("expected a type");
        TypePtr type = specifiers.type;
        while (accept("*")) type = pointerType(type);
        return type;
    }

    ExprPtr parseSizeOf() {
        advance();  // sizeof
        TypePtr target;
        if (at("(") && look(1).kind == TokenKind::Identifier && isTypeKeyword(look(1).text)) {
            advance();
            target = parseTypeName();
            expect(")", "after the sizeof type");
        } else {
            ExprPtr operand = parseUnary();
            target = operand->type;
        }
        auto expression = makeExpr(ExprKind::SizeOf);
        expression->type = primitiveType(TypeKind::Long);
        expression->number = target->size;
        return expression;
    }

    ExprPtr parsePostfix() {
        ExprPtr expression = parsePrimary();
        for (;;) {
            if (at("(")) {
                advance();
                auto call = ExprPtr(new Expr());
                call->kind = ExprKind::Call;
                call->line = expression->line;
                call->column = expression->column;
                if (expression->kind != ExprKind::Function) {
                    fail("only a named function can be called; declare it first");
                }
                auto function = program_->functions.find(expression->text);
                if (function == program_->functions.end()) {
                    fail("'" + expression->text +
                         "' is not declared; declare it before calling it, e.g. "
                         "int " +
                         expression->text + "(int value);");
                }
                call->text = expression->text;
                while (!at(")")) {
                    ExprPtr argument = parseAssignment();
                    if (!argument->type->isScalar() && !argument->type->isArray()) {
                        fail("declare the parameter as a pointer rather than passing " +
                             typeName(argument->type) + " by value");
                    }
                    call->args.push_back(std::move(argument));
                    if (!accept(",")) break;
                }
                expect(")", "to close the call");

                FunctionDecl* declaration = function->second.get();
                std::size_t given = call->args.size();
                std::size_t expected = declaration->parameters.size();
                if (declaration->variadic ? given < expected : given != expected) {
                    fail("'" + declaration->name + "' takes " +
                         std::to_string(expected) + " argument" + (expected == 1 ? "" : "s") +
                         (declaration->variadic ? " or more" : "") + " but was given " +
                         std::to_string(given));
                }
                for (std::size_t i = 0; i < expected; ++i) {
                    const Parameter& parameter = declaration->parameters[i];
                    if (parameter.type->isPointer() && parameter.type->element->isScalar() &&
                        call->args[i]->type->isInteger() &&
                        call->args[i]->kind == ExprKind::Integer &&
                        call->args[i]->number != 0) {
                        // Passing a whole number as a pointer is almost always a
                        // mistake, but 0 means null and is fine.
                        fail("argument " + std::to_string(i + 1) + " of '" +
                             declaration->name + "' is a pointer, not a number");
                    }
                }
                call->type = declaration->result;
                expression = std::move(call);
                continue;
            }
            if (at("[")) {
                advance();
                ExprPtr index = parseAssignment();
                expect("]", "to close the subscript");
                if (!index->type->isInteger()) fail("a subscript must be a number");
                TypePtr base = expression->type;
                if (!base->isPointer() && !base->isArray()) {
                    fail("'" + typeName(base) + "' cannot be indexed");
                }
                auto subscript = ExprPtr(new Expr());
                subscript->kind = ExprKind::Index;
                subscript->line = expression->line;
                subscript->column = expression->column;
                subscript->type = base->isArray() ? base->element : base->element;
                subscript->lvalue = !subscript->type->isVoid();
                subscript->a = std::move(expression);
                subscript->b = std::move(index);
                expression = std::move(subscript);
                continue;
            }
            if (at(".") || at("->")) {
                bool arrow = at("->");
                advance();
                std::string fieldName = expectName("after '.' or '->'");
                TypePtr base = expression->type;
                if (arrow) {
                    if (!base->isPointer() || !base->element->isStruct()) {
                        fail("'->' needs a pointer to a struct, not " + typeName(base));
                    }
                    base = base->element;
                } else if (!base->isStruct()) {
                    fail("'.' needs a struct, not " + typeName(base));
                }
                int index = -1;
                for (std::size_t i = 0; i < base->fields.size(); ++i) {
                    if (base->fields[i].name == fieldName) index = static_cast<int>(i);
                }
                if (index < 0) {
                    fail("struct " + base->name + " has no field named '" + fieldName + "'");
                }
                auto member = ExprPtr(new Expr());
                member->kind = ExprKind::Member;
                member->text = fieldName;
                member->line = expression->line;
                member->column = expression->column;
                member->arrow = arrow;
                member->memberIndex = index;
                member->type = base->fields[static_cast<std::size_t>(index)].type;
                member->lvalue = true;
                member->a = std::move(expression);
                expression = std::move(member);
                continue;
            }
            if (at("++") || at("--")) {
                std::string op = advance().text;
                if (!expression->lvalue) {
                    fail("'" + op + "' needs a variable, an element or a field");
                }
                if (!expression->type->isScalar()) fail("'" + op + "' needs a number or a pointer");
                auto postfix = ExprPtr(new Expr());
                postfix->kind = ExprKind::Postfix;
                postfix->text = op;
                postfix->line = expression->line;
                postfix->column = expression->column;
                postfix->type = expression->type;
                postfix->a = std::move(expression);
                expression = std::move(postfix);
                continue;
            }
            break;
        }
        if (expression->kind == ExprKind::Function) {
            fail("'" + expression->text + "' is a function; call it with () to use its result");
        }
        return expression;
    }

    ExprPtr parsePrimary() {
        const Token& token = current();
        if (token.kind == TokenKind::Number || token.kind == TokenKind::Character) {
            auto expression = makeExpr(token.kind == TokenKind::Number ? ExprKind::Integer
                                                                      : ExprKind::Character);
            expression->number = token.number;
            // A literal that fits in 32 bits is an int; anything larger is long.
            const bool fitsInt = token.number >= -2147483648LL && token.number <= 2147483647LL;
            expression->type = primitiveType(token.kind == TokenKind::Number && !fitsInt
                                                 ? TypeKind::Long
                                                 : TypeKind::Int);
            advance();
            return expression;
        }
        if (token.kind == TokenKind::String) {
            std::string contents;
            while (current().kind == TokenKind::String) {
                contents += current().string;
                advance();
            }
            auto expression = ExprPtr(new Expr());
            expression->kind = ExprKind::String;
            expression->line = token.line;
            expression->column = token.column;
            expression->string = contents;
            expression->number = 0;
            expression->type = pointerType(primitiveType(TypeKind::Char));
            if (!contents.empty()) {
                program_->strings.push_back(contents);
                expression->number = static_cast<long long>(program_->strings.size()) - 1;
            }
            return expression;
        }
        if (token.kind == TokenKind::Identifier) {
            if (token.text == "true" || token.text == "false") {
                auto expression = makeExpr(ExprKind::Integer);
                expression->number = token.text == "true" ? 1 : 0;
                expression->type = primitiveType(TypeKind::Bool);
                advance();
                return expression;
            }
            if (token.text == "nullptr") {
                auto expression = makeExpr(ExprKind::Integer);
                expression->number = 0;
                expression->type = pointerType(primitiveType(TypeKind::Void));
                advance();
                return expression;
            }
            if (isTypeKeyword(token.text)) {
                fail("'" + token.text + "' cannot be used as a value");
            }
            auto expression = makeExpr(ExprKind::Name);
            expression->text = token.text;
            advance();
            const Variable* variable = lookup(expression->text);
            if (!variable) {
                // A function name is only meaningful in front of its argument
                // list; parsePostfix turns it into a call and refuses it
                // anywhere else.
                if (program_->functions.find(expression->text) != program_->functions.end()) {
                    expression->kind = ExprKind::Function;
                    return expression;
                }
                fail("'" + expression->text + "' is not declared");
            }
            expression->type = variable->type;
            expression->globalRef = variable->global;
            expression->offset = variable->offset;
            expression->lvalue = true;
            return expression;
        }
        if (token.is("(")) {
            advance();
            ExprPtr inner = parseAssignment();
            expect(")", "to close the group");
            return inner;
        }
        if (token.kind == TokenKind::End) fail("the source ends in the middle of an expression");
        fail("unexpected '" + token.text + "' where a value belongs");
    }

    // ------------------------------------------------------------ folding ----

    // Compile-time integer value, used for array lengths and global initialisers.
    long long fold(const Expr& expression, bool& constant) const {
        switch (expression.kind) {
            case ExprKind::Integer:
            case ExprKind::Character:
            case ExprKind::SizeOf:
                return expression.number;
            case ExprKind::Cast: {
                if (!expression.a) {
                    constant = false;
                    return 0;
                }
                return fold(*expression.a, constant);
            }
            case ExprKind::Unary: {
                if (!expression.a) {
                    constant = false;
                    return 0;
                }
                long long value = fold(*expression.a, constant);
                if (!constant) return 0;
                if (expression.text == "-") return -value;
                if (expression.text == "+") return value;
                if (expression.text == "!") return value == 0 ? 1 : 0;
                if (expression.text == "~") return ~value;
                constant = false;
                return 0;
            }
            case ExprKind::Conditional: {
                if (!expression.a || !expression.b || !expression.c) {
                    constant = false;
                    return 0;
                }
                long long condition = fold(*expression.a, constant);
                if (!constant) return 0;
                return condition != 0 ? fold(*expression.b, constant)
                                      : fold(*expression.c, constant);
            }
            case ExprKind::Binary: {
                if (!expression.a || !expression.b) {
                    constant = false;
                    return 0;
                }
                long long left = fold(*expression.a, constant);
                if (!constant) return 0;
                long long right = fold(*expression.b, constant);
                if (!constant) return 0;
                const std::string& op = expression.text;
                if (op == "+") return left + right;
                if (op == "-") return left - right;
                if (op == "*") return left * right;
                if (op == "/") return right == 0 ? 0 : left / right;
                if (op == "%") return right == 0 ? 0 : left % right;
                if (op == "<<") return left << right;
                if (op == ">>") return left >> right;
                if (op == "&") return left & right;
                if (op == "|") return left | right;
                if (op == "^") return left ^ right;
                if (op == "==") return left == right ? 1 : 0;
                if (op == "!=") return left != right ? 1 : 0;
                if (op == "<") return left < right ? 1 : 0;
                if (op == ">") return left > right ? 1 : 0;
                if (op == "<=") return left <= right ? 1 : 0;
                if (op == ">=") return left >= right ? 1 : 0;
                if (op == "&&") return (left != 0 && right != 0) ? 1 : 0;
                if (op == "||") return (left != 0 || right != 0) ? 1 : 0;
                constant = false;
                return 0;
            }
            default:
                constant = false;
                return 0;
        }
    }

    const std::vector<Token>& tokens_;
    const Options& options_;
    std::size_t index_ = 0;

    std::unique_ptr<Program> program_;
    std::map<std::string, TypePtr> structs_;
    std::map<std::string, Variable> globals_;
    std::vector<std::map<std::string, Variable>> scopes_;
    int localBytes_ = 0;
    int loopDepth_ = 0;
    FunctionDecl* currentFunction_ = nullptr;
};

}  // namespace

ParseResult parse(const std::vector<Token>& tokens, const Options& options) {
    ParseResult result;
    Parser parser(tokens, options);
    try {
        result.program = parser.parseUnit();
    } catch (const ParseError& error) {
        result.diagnostics.push_back(error.diagnostic);
    }
    return result;
}

}  // namespace detail
}  // namespace cxx
}  // namespace rblx
