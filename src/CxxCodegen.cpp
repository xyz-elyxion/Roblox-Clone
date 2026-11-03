#include "CxxInternal.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace rblx {
namespace cxx {

namespace {

int roundUp(int value, int alignment) {
    if (alignment <= 1) return value;
    return (value + alignment - 1) / alignment * alignment;
}

// Every value lives in a 64-bit register: loads sign-extend whatever the
// declared width is, and stores write back only that width. The declared type
// therefore decides what is kept, and an `int` wraps at 32 bits the way the
// language says it should.
class Emitter {
public:
    Emitter(const detail::Program& program, const Options& options)
        : program_(program), options_(options) {}

    std::string run() {
        text_ += "    .text\n";
        for (const detail::FunctionDef& definition : program_.definitions) {
            function(&definition);
        }
        globals();
        strings();
#if !defined(__APPLE__)
        // Without this the linker warns that the program needs an executable
        // stack, which it does not: nothing here writes to the stack.
        text_ += "\n    .section .note.GNU-stack,\"\",@progbits\n";
#endif
        return text_;
    }

private:
    // ------------------------------------------------------------ output ----

    void emit(const std::string& line) { *out_ += "    " + line + "\n"; }
    void emitLabel(const std::string& label) { *out_ += label + ":\n"; }
    void emitRaw(const std::string& line) { *out_ += line + "\n"; }
    void annotate(int line) {
        if (options_.annotate && line > 0) {
            *out_ += "    # line " + std::to_string(line) + "\n";
        }
    }
    std::string freshLabel(const char* prefix) {
        return std::string(".L") + prefix + std::to_string(labelCounter_++);
    }
    std::string symbol(const std::string& name) const { return options_.symbolPrefix + name; }

    // ------------------------------------------------------------- temps ----

    std::string tempAt(int index) const {
        return std::to_string(-(tempBase_ + 8 * (index + 1))) + "(%rbp)";
    }
    int allocTemp() {
        int index = tempDepth_++;
        maxTemp_ = std::max(maxTemp_, tempDepth_);
        return index;
    }
    void releaseTemps(int count) { tempDepth_ -= count; }

    // ------------------------------------------------------- load / store ----

    // Value is at the address in %rax.
    void loadValue(const detail::TypePtr& type) {
        if (!type) return;
        if (type->isVoid()) return;
        if (type->isAggregate()) return;  // aggregates are handled by address
        switch (type->size) {
            case 1: emit("movsbq (%rax), %rax"); break;
            case 2: emit("movswq (%rax), %rax"); break;
            case 4: emit("movslq (%rax), %rax"); break;
            default: emit("movq (%rax), %rax"); break;
        }
    }

    // Value in %rax, address in %rcx; the value is left in %rax afterwards.
    void storeValue(const detail::TypePtr& type) {
        if (type->isBool()) {
            emit("cmpq $0, %rax");
            emit("setne %al");
            emit("movzbq %al, %rax");
        }
        switch (type->size) {
            case 1:
                emit("movb %al, (%rcx)");
                emit("movsbq %al, %rax");
                break;
            case 2:
                emit("movw %ax, (%rcx)");
                emit("movswq %ax, %rax");
                break;
            case 4:
                emit("movl %eax, (%rcx)");
                emit("movslq %eax, %rax");
                break;
            default:
                emit("movq %rax, (%rcx)");
                break;
        }
    }

    // Trims a computed value back to the width of its declared type.
    void narrow(const detail::TypePtr& type) {
        if (!type) return;
        if (type->isBool()) {
            emit("cmpq $0, %rax");
            emit("setne %al");
            emit("movzbq %al, %rax");
            return;
        }
        if (type->kind == detail::TypeKind::Char) {
            emit("movsbq %al, %rax");
        } else if (type->kind == detail::TypeKind::Int) {
            emit("movslq %eax, %rax");
        }
    }

    void storeAt(const detail::TypePtr& type, int offset) {
        std::string memory = std::to_string(offset) + "(%rbp)";
        switch (type->size) {
            case 1: emit("movb %al, " + memory); break;
            case 2: emit("movw %ax, " + memory); break;
            case 4: emit("movl %eax, " + memory); break;
            default: emit("movq %rax, " + memory); break;
        }
    }

    // Copies `size` bytes from (%rsi) to (%rdi). Only %rax is clobbered.
    void copyBytes(int size) {
        int offset = 0;
        while (size - offset >= 8) {
            emit("movq " + std::to_string(offset) + "(%rsi), %rax");
            emit("movq %rax, " + std::to_string(offset) + "(%rdi)");
            offset += 8;
        }
        if (size - offset >= 4) {
            emit("movl " + std::to_string(offset) + "(%rsi), %eax");
            emit("movl %eax, " + std::to_string(offset) + "(%rdi)");
            offset += 4;
        }
        while (size - offset >= 1) {
            emit("movb " + std::to_string(offset) + "(%rsi), %al");
            emit("movb %al, " + std::to_string(offset) + "(%rdi)");
            ++offset;
        }
    }

    // ---------------------------------------------------------- addresses ----

    void genAddress(const detail::Expr& expression) {
        switch (expression.kind) {
            case detail::ExprKind::Name:
                if (expression.globalRef) {
                    emit("leaq " + symbol(expression.text) + "(%rip), %rax");
                } else {
                    emit("leaq " + std::to_string(expression.offset) + "(%rbp), %rax");
                }
                return;

            case detail::ExprKind::Unary:
                if (expression.text == "*") {
                    genValue(*expression.a);  // the pointer is the address
                    return;
                }
                break;

            case detail::ExprKind::Index: {
                genValue(*expression.a);  // base address (an array decays to one)
                int base = allocTemp();
                emit("movq %rax, " + tempAt(base));
                genValue(*expression.b);
                int elementSize = expression.a->type->element->size;
                if (elementSize != 1) emit("imulq $" + std::to_string(elementSize) + ", %rax");
                emit("movq " + tempAt(base) + ", %rcx");
                emit("addq %rcx, %rax");
                releaseTemps(1);
                return;
            }

            case detail::ExprKind::Member: {
                if (expression.arrow) {
                    genValue(*expression.a);
                } else {
                    genAddress(*expression.a);
                }
                const detail::TypePtr& base =
                    expression.arrow ? expression.a->type->element : expression.a->type;
                int offset = base->fields[static_cast<std::size_t>(expression.memberIndex)].offset;
                if (offset != 0) emit("addq $" + std::to_string(offset) + ", %rax");
                return;
            }

            default:
                break;
        }
        // The parser guarantees this never happens; emit zero rather than
        // producing assembly that silently reads the wrong address.
        emit("xorl %eax, %eax");
    }

    // ---------------------------------------------------------- expressions ----

    void genValue(const detail::Expr& expression) {
        annotate(expression.line);
        switch (expression.kind) {
            case detail::ExprKind::Integer:
            case detail::ExprKind::Character:
                emit("movq $" + std::to_string(expression.number) + ", %rax");
                return;

            case detail::ExprKind::SizeOf:
                emit("movq $" + std::to_string(expression.number) + ", %rax");
                return;

            case detail::ExprKind::String:
                emit("leaq .Lstring" + std::to_string(expression.number) + "(%rip), %rax");
                return;

            case detail::ExprKind::Name: {
                genAddress(expression);
                loadValue(expression.type);
                return;
            }

            case detail::ExprKind::Function:
                // The parser rejects an uncalled function name before code
                // generation, so this is only reachable if that check regresses.
                emit("xorl %eax, %eax");
                return;

            case detail::ExprKind::Cast: {
                genValue(*expression.a);
                narrow(expression.type);
                return;
            }

            case detail::ExprKind::Unary:
                unary(expression);
                return;

            case detail::ExprKind::Postfix:
                increment(expression, /*prefix=*/false);
                return;

            case detail::ExprKind::Binary:
                binary(expression);
                return;

            case detail::ExprKind::Conditional: {
                std::string other = freshLabel("cond");
                std::string done = freshLabel("end");
                genValue(*expression.a);
                emit("cmpq $0, %rax");
                emit("je " + other);
                genValue(*expression.b);
                emit("jmp " + done);
                emitLabel(other);
                genValue(*expression.c);
                emitLabel(done);
                return;
            }

            case detail::ExprKind::Assign:
                assign(expression);
                return;

            case detail::ExprKind::Call:
                call(expression);
                return;

            case detail::ExprKind::Index:
            case detail::ExprKind::Member:
                genAddress(expression);
                loadValue(expression.type);
                return;
        }
    }

    void unary(const detail::Expr& expression) {
        const std::string& op = expression.text;
        if (op == "&") {
            genAddress(*expression.a);
            return;
        }
        if (op == "*") {
            genValue(*expression.a);
            loadValue(expression.type);
            return;
        }
        if (op == "++" || op == "--") {
            increment(expression, /*prefix=*/true);
            return;
        }
        genValue(*expression.a);
        if (op == "-") {
            emit("negq %rax");
            narrow(expression.type);
        } else if (op == "~") {
            emit("notq %rax");
            narrow(expression.type);
        } else if (op == "!") {
            emit("cmpq $0, %rax");
            emit("sete %al");
            emit("movzbq %al, %rax");
        }
        // Unary '+' has no effect.
    }

    // Steps the value at the address in %rax by one element.
    void step(const detail::TypePtr& type, bool forward) {
        int size = type->isPointer() ? type->element->size : 1;
        if (size <= 0) size = 1;
        emit(std::string(forward ? "addq $" : "subq $") + std::to_string(size) + ", %rax");
    }

    void increment(const detail::Expr& expression, bool prefix) {
        const bool forward = expression.text == "++";
        const detail::TypePtr& type = expression.type;

        genAddress(*expression.a);
        int address = allocTemp();
        emit("movq %rax, " + tempAt(address));

        loadValue(type);
        if (prefix) {
            step(type, forward);
            narrow(type);
            emit("movq " + tempAt(address) + ", %rcx");
            storeValue(type);
            releaseTemps(1);
            return;
        }
        int previous = allocTemp();
        emit("movq %rax, " + tempAt(previous));
        step(type, forward);
        narrow(type);
        emit("movq " + tempAt(address) + ", %rcx");
        storeValue(type);
        emit("movq " + tempAt(previous) + ", %rax");
        releaseTemps(2);
    }

    // Left value in %rax, right value in %rcx; the result is left in %rax.
    void combine(const detail::Expr& expression) {
        const std::string& op = expression.text;
        const detail::TypePtr& left = expression.a->type;
        const detail::TypePtr& right = expression.b->type;

        if (op == "+" || op == "-") {
            if (left->isPointerLike() && right->isPointerLike()) {
                emit("subq %rcx, %rax");
                int stride = left->element->size;
                if (stride > 1) {
                    emit("cqto");
                    emit("movq $" + std::to_string(stride) + ", %rcx");
                    emit("idivq %rcx");
                }
                return;
            }
            if (left->isPointerLike()) {
                int stride = left->element->size;
                if (stride > 1) emit("imulq $" + std::to_string(stride) + ", %rcx");
                emit(std::string(op == "+" ? "addq" : "subq") + " %rcx, %rax");
                return;
            }
            if (right->isPointerLike()) {
                int stride = right->element->size;
                if (stride > 1) emit("imulq $" + std::to_string(stride) + ", %rax");
                emit("addq %rcx, %rax");
                return;
            }
        }

        if (op == "*") {
            emit("imulq %rcx, %rax");
        } else if (op == "/" || op == "%") {
            emit("cqto");
            emit("idivq %rcx");
            if (op == "%") emit("movq %rdx, %rax");
        } else if (op == "<<") {
            emit("salq %cl, %rax");
        } else if (op == ">>") {
            emit("sarq %cl, %rax");
        } else if (op == "&") {
            emit("andq %rcx, %rax");
        } else if (op == "|") {
            emit("orq %rcx, %rax");
        } else if (op == "^") {
            emit("xorq %rcx, %rax");
        } else if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" ||
                   op == ">=") {
            emit("cmpq %rcx, %rax");
            if (op == "==") emit("sete %al");
            else if (op == "!=") emit("setne %al");
            else if (op == "<") emit("setl %al");
            else if (op == ">") emit("setg %al");
            else if (op == "<=") emit("setle %al");
            else emit("setge %al");
            emit("movzbq %al, %rax");
            return;
        } else if (op == "+") {
            emit("addq %rcx, %rax");
        } else if (op == "-") {
            emit("subq %rcx, %rax");
        }
        narrow(expression.type);
    }

    void binary(const detail::Expr& expression) {
        const std::string& op = expression.text;
        if (op == "&&" || op == "||") {
            std::string shortCircuit = freshLabel("sc");
            std::string done = freshLabel("scend");
            genValue(*expression.a);
            emit("cmpq $0, %rax");
            emit(op == "&&" ? "je " + shortCircuit : "jne " + shortCircuit);
            genValue(*expression.b);
            emit("cmpq $0, %rax");
            emit(op == "&&" ? "je " + shortCircuit : "jne " + shortCircuit);
            emit(op == "&&" ? "movl $1, %eax" : "xorl %eax, %eax");
            emit("jmp " + done);
            emitLabel(shortCircuit);
            emit(op == "&&" ? "xorl %eax, %eax" : "movl $1, %eax");
            emitLabel(done);
            return;
        }

        genValue(*expression.a);
        int left = allocTemp();
        emit("movq %rax, " + tempAt(left));
        genValue(*expression.b);
        emit("movq %rax, %rcx");
        emit("movq " + tempAt(left) + ", %rax");
        releaseTemps(1);
        combine(expression);
    }

    void assign(const detail::Expr& expression) {
        const detail::TypePtr& type = expression.a->type;

        if (type->isStruct() && expression.text == "=") {
            genAddress(*expression.a);
            emit("movq %rax, %rdi");
            genValue(*expression.b);  // a struct expression yields its address
            emit("movq %rax, %rsi");
            copyBytes(type->size);
            emit("movq %rdi, %rax");
            return;
        }

        genAddress(*expression.a);
        int address = allocTemp();
        emit("movq %rax, " + tempAt(address));

        if (expression.text == "=") {
            genValue(*expression.b);
        } else {
            loadValue(type);
            int previous = allocTemp();
            emit("movq %rax, " + tempAt(previous));
            genValue(*expression.b);
            emit("movq %rax, %rcx");
            emit("movq " + tempAt(previous) + ", %rax");
            releaseTemps(1);

            detail::Expr operation;
            operation.text = expression.text.substr(0, expression.text.size() - 1);
            operation.type = type;
            operation.a = std::unique_ptr<detail::Expr>(new detail::Expr());
            operation.a->type = type;
            operation.b = std::unique_ptr<detail::Expr>(new detail::Expr());
            operation.b->type = expression.b->type;
            combine(operation);
        }

        emit("movq " + tempAt(address) + ", %rcx");
        storeValue(type);
        releaseTemps(1);
    }

    void call(const detail::Expr& expression) {
        auto found = program_.functions.find(expression.text);
        const detail::FunctionDecl* declaration =
            found == program_.functions.end() ? nullptr : found->second.get();

        std::vector<int> arguments;
        for (const detail::ExprPtr& argument : expression.args) {
            genValue(*argument);
            int slot = allocTemp();
            emit("movq %rax, " + tempAt(slot));
            arguments.push_back(slot);
        }

        const std::size_t count = arguments.size();
        const std::size_t stackCount = count > 6 ? count - 6 : 0;
        const bool pad = (stackCount % 2) != 0;
        if (pad) emit("subq $8, %rsp");
        for (std::size_t i = count; i-- > 6;) {
            emit("pushq " + tempAt(arguments[i]));
        }
        static const char* kRegisters[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        const std::size_t registerCount = std::min<std::size_t>(count, 6);
        for (std::size_t i = 0; i < registerCount; ++i) {
            emit("movq " + tempAt(arguments[i]) + ", " + kRegisters[i]);
        }
        // printf and friends read this to know whether to touch the vector
        // registers; zero is always safe.
        emit("movb $0, %al");

        const bool external = !declaration || !declaration->defined;
        emit("call " + symbol(expression.text) + (external && options_.usePlt ? "@PLT" : ""));

        if (stackCount > 0 || pad) {
            emit("addq $" + std::to_string(8 * (stackCount + (pad ? 1 : 0))) + ", %rsp");
        }
        releaseTemps(static_cast<int>(count));

        if (expression.type && !expression.type->isVoid()) {
            if (expression.type->isBool()) {
                emit("cmpq $0, %rax");
                emit("setne %al");
                emit("movzbq %al, %rax");
            } else if (expression.type->kind == detail::TypeKind::Char) {
                emit("movsbq %al, %rax");
            } else if (expression.type->kind == detail::TypeKind::Int) {
                emit("movslq %eax, %rax");
            }
        }
    }

    // ---------------------------------------------------------- statements ----

    void genStatement(const detail::Stmt& statement) {
        annotate(statement.line);
        switch (statement.kind) {
            case detail::StmtKind::Empty:
                return;

            case detail::StmtKind::Block:
                for (const detail::StmtPtr& child : statement.body) genStatement(*child);
                return;

            case detail::StmtKind::Declaration:
                for (const detail::LocalDecl& declaration : statement.declarations) {
                    if (!declaration.init) continue;
                    if (declaration.type->isStruct()) {
                        genValue(*declaration.init);
                        emit("movq %rax, %rsi");
                        emit("leaq " + std::to_string(declaration.offset) + "(%rbp), %rdi");
                        copyBytes(declaration.type->size);
                        continue;
                    }
                    genValue(*declaration.init);
                    emit("leaq " + std::to_string(declaration.offset) + "(%rbp), %rcx");
                    storeValue(declaration.type);
                }
                return;

            case detail::StmtKind::Expression:
                genValue(*statement.expr);
                return;

            case detail::StmtKind::If: {
                std::string other = freshLabel("else");
                std::string done = freshLabel("endif");
                genValue(*statement.expr);
                emit("cmpq $0, %rax");
                emit("je " + (statement.elseBranch ? other : done));
                genStatement(*statement.thenBranch);
                if (statement.elseBranch) {
                    emit("jmp " + done);
                    emitLabel(other);
                    genStatement(*statement.elseBranch);
                }
                emitLabel(done);
                return;
            }

            case detail::StmtKind::While: {
                std::string top = freshLabel("while");
                std::string done = freshLabel("endwhile");
                emitLabel(top);
                genValue(*statement.expr);
                emit("cmpq $0, %rax");
                emit("je " + done);
                breakLabels_.push_back(done);
                continueLabels_.push_back(top);
                genStatement(*statement.thenBranch);
                breakLabels_.pop_back();
                continueLabels_.pop_back();
                emit("jmp " + top);
                emitLabel(done);
                return;
            }

            case detail::StmtKind::DoWhile: {
                std::string top = freshLabel("do");
                std::string test = freshLabel("dotest");
                std::string done = freshLabel("enddo");
                emitLabel(top);
                breakLabels_.push_back(done);
                continueLabels_.push_back(test);
                genStatement(*statement.thenBranch);
                breakLabels_.pop_back();
                continueLabels_.pop_back();
                emitLabel(test);
                genValue(*statement.expr);
                emit("cmpq $0, %rax");
                emit("jne " + top);
                emitLabel(done);
                return;
            }

            case detail::StmtKind::For: {
                if (statement.init) genStatement(*statement.init);
                std::string top = freshLabel("for");
                std::string next = freshLabel("forstep");
                std::string done = freshLabel("endfor");
                emitLabel(top);
                if (statement.expr) {
                    genValue(*statement.expr);
                    emit("cmpq $0, %rax");
                    emit("je " + done);
                }
                breakLabels_.push_back(done);
                continueLabels_.push_back(next);
                genStatement(*statement.thenBranch);
                breakLabels_.pop_back();
                continueLabels_.pop_back();
                emitLabel(next);
                if (statement.step) genValue(*statement.step);
                emit("jmp " + top);
                emitLabel(done);
                return;
            }

            case detail::StmtKind::Return: {
                if (statement.expr) {
                    genValue(*statement.expr);
                    const detail::TypePtr& result = function_->declaration->result;
                    if (result->isBool()) {
                        emit("cmpq $0, %rax");
                        emit("setne %al");
                        emit("movzbq %al, %rax");
                    } else if (result->kind == detail::TypeKind::Char) {
                        emit("movsbq %al, %rax");
                    } else if (result->kind == detail::TypeKind::Int) {
                        emit("movslq %eax, %rax");
                    }
                }
                emit("jmp " + returnLabel_);
                return;
            }

            case detail::StmtKind::Break:
                if (!breakLabels_.empty()) emit("jmp " + breakLabels_.back());
                return;

            case detail::StmtKind::Continue:
                if (!continueLabels_.empty()) emit("jmp " + continueLabels_.back());
                return;
        }
    }

    // ----------------------------------------------------------- functions ----

    void storeParameters(const detail::FunctionDef& definition) {
        static const char* kRegisters[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        const std::vector<detail::Parameter>& parameters = definition.declaration->parameters;
        for (std::size_t i = 0; i < parameters.size(); ++i) {
            const detail::Parameter& parameter = parameters[i];
            if (parameter.name.empty()) continue;
            if (i < 6) {
                emit("movq " + std::string(kRegisters[i]) + ", %rax");
            } else {
                emit("movq " + std::to_string(16 + 8 * static_cast<int>(i - 6)) + "(%rbp), %rax");
            }
            storeAt(parameter.type, parameter.offset);
        }
    }

    void function(const detail::FunctionDef* definition) {
        function_ = definition;
        tempDepth_ = 0;
        maxTemp_ = 0;
        tempBase_ = roundUp(definition->localBytes, 8);
        returnLabel_ = ".Lreturn_" + definition->declaration->name;
        breakLabels_.clear();
        continueLabels_.clear();

        std::string parameters;
        out_ = &parameters;
        storeParameters(*definition);

        std::string body;
        out_ = &body;
        genStatement(*definition->body);
        emit("movl $0, %eax");  // falling off the end returns 0
        emitLabel(returnLabel_);
        emit("movq %rbp, %rsp");
        emit("popq %rbp");
        emit("ret");

        const int frame = roundUp(tempBase_ + 8 * maxTemp_, 16);

        text_ += "\n";
        if (!definition->declaration->internal) {
            text_ += "    .globl " + symbol(definition->declaration->name) + "\n";
        }
        text_ += symbol(definition->declaration->name) + ":\n";
        text_ += "    pushq %rbp\n";
        text_ += "    movq %rsp, %rbp\n";
        if (frame > 0) text_ += "    subq $" + std::to_string(frame) + ", %rsp\n";
        text_ += parameters;
        text_ += body;

        function_ = nullptr;
        out_ = &text_;
    }

    // ------------------------------------------------------- data sections ----

    void globals() {
        for (const detail::GlobalDef& definition : program_.globals) {
            if (definition.external) continue;
            const int size = definition.type->size;
            const int alignment = definition.type->align;

            text_ += "\n";
            text_ += definition.hasInit ? "    .data\n" : "    .bss\n";
            if (!definition.internal) {
                text_ += "    .globl " + symbol(definition.name) + "\n";
            }
            text_ += "    .balign " + std::to_string(std::max(alignment, 1)) + "\n";
            text_ += symbol(definition.name) + ":\n";

            if (!definition.hasInit) {
                text_ += "    .zero " + std::to_string(std::max(size, 1)) + "\n";
                continue;
            }
            if (definition.stringInit) {
                text_ += "    .quad .Lstring" + std::to_string(stringIndex(definition.stringLabel)) +
                         "\n";
                continue;
            }
            const std::string value = std::to_string(definition.intInit);
            switch (size) {
                case 1: text_ += "    .byte " + value + "\n"; break;
                case 2: text_ += "    .short " + value + "\n"; break;
                case 4: text_ += "    .long " + value + "\n"; break;
                default: text_ += "    .quad " + value + "\n"; break;
            }
        }
    }

    // Labels arrive as "Lstr<index>"; the pool index is the digits after it.
    static int stringIndex(const std::string& label) {
        std::size_t digits = label.find_first_of("0123456789");
        if (digits == std::string::npos) return 0;
        return std::atoi(label.c_str() + digits);
    }

    void strings() {
        if (program_.strings.empty()) return;
        text_ += "\n    .section .rodata\n";
        for (std::size_t i = 0; i < program_.strings.size(); ++i) {
            text_ += ".Lstring" + std::to_string(i) + ":\n";
            text_ += "    .string \"" + escape(program_.strings[i]) + "\"\n";
        }
    }

    static std::string escape(const std::string& text) {
        std::string escaped;
        for (char c : text) {
            switch (c) {
                case '\\': escaped += "\\\\"; break;
                case '"': escaped += "\\\""; break;
                case '\n': escaped += "\\n"; break;
                case '\t': escaped += "\\t"; break;
                case '\r': escaped += "\\r"; break;
                default:
                    if (static_cast<unsigned char>(c) < 32) {
                        char buffer[8];
                        std::snprintf(buffer, sizeof(buffer), "\\%03o",
                                      static_cast<unsigned char>(c));
                        escaped += buffer;
                    } else {
                        escaped += c;
                    }
                    break;
            }
        }
        return escaped;
    }

    const detail::Program& program_;
    const Options& options_;
    std::string text_;
    std::string* out_ = &text_;
    int labelCounter_ = 0;
    const detail::FunctionDef* function_ = nullptr;
    std::string returnLabel_;
    int tempDepth_ = 0;
    int maxTemp_ = 0;
    int tempBase_ = 0;
    std::vector<std::string> breakLabels_;
    std::vector<std::string> continueLabels_;
};

}  // namespace

namespace detail {

GenerateResult generate(const Program& program, const Options& options) {
    GenerateResult result;
    Emitter emitter(program, options);
    result.assembly = emitter.run();
    return result;
}

}  // namespace detail

std::string Result::report() const {
    std::string text;
    for (const Diagnostic& diagnostic : diagnostics) {
        text += std::to_string(diagnostic.line) + ":" + std::to_string(diagnostic.column) + ": " +
                diagnostic.message + "\n";
    }
    return text;
}

Result compile(const std::string& source, const Options& options) {
    Result result;

    detail::LexResult lexed =
        detail::lex(source, options.includePrelude ? detail::defaultPreludeText() : std::string());
    if (!lexed.diagnostics.empty()) {
        result.diagnostics = lexed.diagnostics;
        return result;
    }

    detail::ParseResult parsed = detail::parse(lexed.tokens, options);
    if (!parsed.program || !parsed.diagnostics.empty()) {
        result.diagnostics = parsed.diagnostics;
        if (result.diagnostics.empty()) {
            Diagnostic diagnostic;
            diagnostic.message = "the program could not be parsed";
            result.diagnostics.push_back(diagnostic);
        }
        return result;
    }

    detail::GenerateResult generated = detail::generate(*parsed.program, options);
    result.diagnostics = generated.diagnostics;
    result.assembly = generated.assembly;
    result.ok = generated.diagnostics.empty();
    return result;
}

std::string supportedSummary() {
    return
        "Types: void, bool, char, int, long (64-bit), pointers, arrays and structs by pointer.\n"
        "Statements: blocks, declarations, if/else, while, do/while, for, return, break, continue.\n"
        "Expressions: arithmetic, comparisons, && || !, ?:, casts, sizeof, ++/--, compound\n"
        "assignment, calls, subscripts and field access.\n"
        "Preprocessor: object-like #define, #undef, #ifdef/#ifndef/#else/#endif; #include is\n"
        "ignored, so declare what you call.\n"
        "Not supported yet: classes, templates, exceptions, unsigned and floating point,\n"
        "structs or arrays by value, function pointers, static locals.";
}

}  // namespace cxx
}  // namespace rblx
