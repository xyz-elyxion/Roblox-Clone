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
        reals();
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

    // ------------------------------------------------------- conversions ----

    // Integers and pointers travel in %rax; floating point values in %xmm0.
    static bool isFloating(const detail::TypePtr& type) { return type && type->isFloating(); }

    // The instruction suffix for a floating point type: ss is float, sd is double.
    static std::string floatOp(const detail::TypePtr& type) {
        return type && type->kind == detail::TypeKind::Float ? "ss" : "sd";
    }

    // The moves spell it out: movss and movsd, not movs + ss.
    static std::string moveOp(const detail::TypePtr& type) {
        return type && type->kind == detail::TypeKind::Float ? "movss" : "movsd";
    }

    // The type both sides of a binary operation are computed in.
    static detail::TypePtr common(const detail::TypePtr& left, const detail::TypePtr& right) {
        if (left && left->kind == detail::TypeKind::Double) return left;
        if (right && right->kind == detail::TypeKind::Double) return right;
        if (left && left->kind == detail::TypeKind::Float) return left;
        if (right && right->kind == detail::TypeKind::Float) return right;
        if (left && left->kind == detail::TypeKind::Long) return left;
        if (right && right->kind == detail::TypeKind::Long) return right;
        return left;
    }

    // Converts the value in %rax/%xmm0 between types, the way an assignment, a
    // cast and an argument all may have to.
    void convert(const detail::TypePtr& from, const detail::TypePtr& to) {
        if (!from || !to || from->isVoid() || to->isVoid()) return;
        if (from->kind == to->kind) {
            // Same width, different signedness: the value has to be trimmed the
            // way the target reads it, so -1 becomes 0xFFFFFFFF for an unsigned
            // int instead of staying sign-extended.
            if (from->isUnsigned != to->isUnsigned && to->isInteger()) narrow(to);
            return;
        }
        if (from->isFloating() && to->isFloating()) {
            if (from->kind == detail::TypeKind::Float) emit("cvtss2sd %xmm0, %xmm0");
            else emit("cvtsd2ss %xmm0, %xmm0");
            return;
        }
        if (from->isFloating()) {
            if (from->kind == detail::TypeKind::Float) emit("cvttss2siq %xmm0, %rax");
            else emit("cvttsd2siq %xmm0, %rax");
            narrow(to);
            return;
        }
        if (to->isFloating()) {
            if (to->kind == detail::TypeKind::Float) emit("cvtsi2ssq %rax, %xmm0");
            else emit("cvtsi2sdq %rax, %xmm0");
            return;
        }
        narrow(to);
    }

    // Turns the value in %xmm0 into 0 or 1 in %rax. NaN is true, like every other
    // value that is not zero.
    void toBool(const detail::TypePtr& type) {
        if (!isFloating(type)) return;
        const std::string suffix = floatOp(type);
        emit("xorpd %xmm1, %xmm1");
        emit("ucomi" + suffix + " %xmm1, %xmm0");
        emit("setne %al");
        emit("setp %cl");
        emit("orb %cl, %al");
        emit("movzbq %al, %rax");
    }

    void spill(const detail::TypePtr& type, int slot) {
        if (isFloating(type)) emit(moveOp(type) + " %xmm0, " + tempAt(slot));
        else emit("movq %rax, " + tempAt(slot));
    }

    void fillValue(const detail::TypePtr& type, int slot) {
        if (isFloating(type)) emit(moveOp(type) + " " + tempAt(slot) + ", %xmm0");
        else emit("movq " + tempAt(slot) + ", %rax");
    }

    // ------------------------------------------------------- load / store ----

    // Value is at the address in %rax.
    void loadValue(const detail::TypePtr& type) {
        if (!type) return;
        if (type->isVoid()) return;
        if (type->isAggregate()) return;  // aggregates are handled by address
        if (type->isFloating()) {
            emit(moveOp(type) + " (%rax), %xmm0");
            return;
        }
        switch (type->size) {
            case 1: emit(type->isUnsigned ? "movzbq (%rax), %rax" : "movsbq (%rax), %rax"); break;
            case 2: emit(type->isUnsigned ? "movzwq (%rax), %rax" : "movswq (%rax), %rax"); break;
            case 4: emit(type->isUnsigned ? "movl (%rax), %eax" : "movslq (%rax), %rax"); break;
            default: emit("movq (%rax), %rax"); break;
        }
    }

    // Value in %rax (or %xmm0 for a floating point type), address in %rcx; the
    // value is left where it was afterwards.
    void storeValue(const detail::TypePtr& type) {
        if (type->isFloating()) {
            emit(moveOp(type) + " %xmm0, (%rcx)");
            return;
        }
        if (type->isBool()) {
            emit("cmpq $0, %rax");
            emit("setne %al");
            emit("movzbq %al, %rax");
            emit("movb %al, (%rcx)");
            return;
        }
        switch (type->size) {
            case 1: emit("movb %al, (%rcx)"); break;
            case 2: emit("movw %ax, (%rcx)"); break;
            case 4: emit("movl %eax, (%rcx)"); break;
            default: emit("movq %rax, (%rcx)"); break;
        }
        // The assignment keeps the value, trimmed the way reading it back gives
        // it: a store to an unsigned char leaves 0..255 in %rax.
        narrow(type);
    }

    // Trims a computed value back to the width and signedness of its declared
    // type. An unsigned value is zero-extended and a signed one sign-extended,
    // which is why `unsigned char` 250 plus 10 lands on 4, and why an unsigned
    // int never looks negative to a division or a comparison.
    void narrow(const detail::TypePtr& type) {
        if (!type || type->isFloating()) return;  // floating values carry their width
        switch (type->kind) {
            case detail::TypeKind::Bool:
                emit("cmpq $0, %rax");
                emit("setne %al");
                emit("movzbq %al, %rax");
                return;
            case detail::TypeKind::Char:
                emit(type->isUnsigned ? "movzbq %al, %rax" : "movsbq %al, %rax");
                return;
            case detail::TypeKind::Short:
                emit(type->isUnsigned ? "movzwq %ax, %rax" : "movswq %ax, %rax");
                return;
            case detail::TypeKind::Int:
                // movl %eax, %eax is the zero-extend: a 32-bit move clears the
                // top half of the register.
                emit(type->isUnsigned ? "movl %eax, %eax" : "movslq %eax, %rax");
                return;
            default:
                return;  // long, pointers and aggregates already hold their width
        }
    }

    void storeAt(const detail::TypePtr& type, int offset) {
        std::string memory = std::to_string(offset) + "(%rbp)";
        if (type->isFloating()) {
            emit(moveOp(type) + " %xmm0, " + memory);
            return;
        }
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

            case detail::ExprKind::Floating: {
                const detail::RealLiteral& literal =
                    program_.reals[static_cast<std::size_t>(expression.number)];
                emit(std::string(literal.isFloat ? "movss" : "movsd") + " .Lreal" +
                     std::to_string(expression.number) + "(%rip), %xmm0");
                return;
            }

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
                convert(expression.a->type, expression.type);
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
                toBool(expression.a->type);
                emit("cmpq $0, %rax");
                emit("je " + other);
                genValue(*expression.b);
                convert(expression.b->type, expression.type);
                emit("jmp " + done);
                emitLabel(other);
                genValue(*expression.c);
                convert(expression.c->type, expression.type);
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
        if (op == "!" && isFloating(expression.a->type)) {
            toBool(expression.a->type);
            emit("cmpq $0, %rax");
            emit("sete %al");
            emit("movzbq %al, %rax");
            return;
        }
        if (op == "-" && isFloating(expression.type)) {
            // Zero minus the value flips the sign, including for zero itself.
            emit("xorpd %xmm1, %xmm1");
            emit("sub" + floatOp(expression.type) + " %xmm0, %xmm1");
            emit("movapd %xmm1, %xmm0");
            return;
        }
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
        if (isFloating(type)) {
            const std::string suffix = floatOp(type);
            emit(moveOp(type) + " .Lone" + (suffix == "ss" ? "f" : "") + "(%rip), %xmm1");
            emit(std::string(forward ? "add" : "sub") + suffix + " %xmm1, %xmm0");
            return;
        }
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
        spill(type, previous);
        step(type, forward);
        narrow(type);
        emit("movq " + tempAt(address) + ", %rcx");
        storeValue(type);
        fillValue(type, previous);
        releaseTemps(2);
    }

    // Left value in %rax, right value in %rcx; the result is left in %rax.
    void combine(const detail::Expr& expression) {
        const std::string& op = expression.text;
        const detail::TypePtr& left = expression.a->type;
        const detail::TypePtr& right = expression.b->type;

        if (isFloating(left) || isFloating(right) || isFloating(expression.type)) {
            // Floating point operands arrive as the common type in %xmm0 (left)
            // and %xmm1 (right).
            const std::string suffix = floatOp(common(left, right));
            if (op == "==" || op == "!=" || op == "<" || op == ">" || op == "<=" ||
                op == ">=") {
                // ucomi sets the parity flag for NaN, which is unequal to
                // everything, including itself.
                emit("ucomi" + suffix + " %xmm1, %xmm0");
                if (op == "==") {
                    emit("sete %al");
                    emit("setnp %cl");
                    emit("andb %cl, %al");
                } else if (op == "!=") {
                    emit("setne %al");
                    emit("setp %cl");
                    emit("orb %cl, %al");
                } else if (op == "<") {
                    emit("setb %al");
                    emit("setnp %cl");
                    emit("andb %cl, %al");
                } else if (op == "<=") {
                    emit("setbe %al");
                    emit("setnp %cl");
                    emit("andb %cl, %al");
                } else if (op == ">") {
                    emit("seta %al");
                } else {
                    emit("setae %al");
                }
                emit("movzbq %al, %rax");
                return;
            }
            if (op == "+") emit("add" + suffix + " %xmm1, %xmm0");
            else if (op == "-") emit("sub" + suffix + " %xmm1, %xmm0");
            else if (op == "*") emit("mul" + suffix + " %xmm1, %xmm0");
            else if (op == "/") emit("div" + suffix + " %xmm1, %xmm0");
            return;
        }

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

        // Whether the operation is signed comes from the usual arithmetic
        // conversions, not from the instruction: `divq` rather than `idivq` and
        // `shrq` rather than `sarq` are the difference between 4000000000/2
        // being 2000000000 and being -147483648.
        const bool unsignedResult = detail::arithmeticType(left, right)->isUnsigned;
        if (op == "*") {
            emit("imulq %rcx, %rax");
        } else if (op == "/" || op == "%") {
            if (unsignedResult) {
                emit("xorl %edx, %edx");  // divq divides rdx:rax, so clear the top
                emit("divq %rcx");
            } else {
                emit("cqto");
                emit("idivq %rcx");
            }
            if (op == "%") emit("movq %rdx, %rax");
        } else if (op == "<<") {
            emit("salq %cl, %rax");
        } else if (op == ">>") {
            emit(detail::promotedType(left)->isUnsigned ? "shrq %cl, %rax" : "sarq %cl, %rax");
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
            else if (op == "<") emit(unsignedResult ? "setb %al" : "setl %al");
            else if (op == ">") emit(unsignedResult ? "seta %al" : "setg %al");
            else if (op == "<=") emit(unsignedResult ? "setbe %al" : "setle %al");
            else emit(unsignedResult ? "setae %al" : "setge %al");
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
            toBool(expression.a->type);
            emit("cmpq $0, %rax");
            emit(op == "&&" ? "je " + shortCircuit : "jne " + shortCircuit);
            genValue(*expression.b);
            toBool(expression.b->type);
            emit("cmpq $0, %rax");
            emit(op == "&&" ? "je " + shortCircuit : "jne " + shortCircuit);
            emit(op == "&&" ? "movl $1, %eax" : "xorl %eax, %eax");
            emit("jmp " + done);
            emitLabel(shortCircuit);
            emit(op == "&&" ? "xorl %eax, %eax" : "movl $1, %eax");
            emitLabel(done);
            return;
        }

        if (isFloating(expression.a->type) || isFloating(expression.b->type)) {
            const detail::TypePtr commonType = common(expression.a->type, expression.b->type);
            genValue(*expression.a);
            convert(expression.a->type, commonType);
            const int left = allocTemp();
            spill(commonType, left);
            genValue(*expression.b);
            convert(expression.b->type, commonType);
            emit("movapd %xmm0, %xmm1");
            fillValue(commonType, left);
            releaseTemps(1);
            combine(expression);
            return;
        }

        // Both sides are computed in the type the usual arithmetic conversions
        // pick. A pointer is never converted - pointer arithmetic scales the
        // integer instead - so only plain integer operands take this path.
        const bool pointerArithmetic =
            expression.a->type->isPointerLike() || expression.b->type->isPointerLike();
        const detail::TypePtr commonType =
            pointerArithmetic ? nullptr
                              : detail::arithmeticType(expression.a->type, expression.b->type);
        genValue(*expression.a);
        if (commonType) convert(expression.a->type, commonType);
        int left = allocTemp();
        emit("movq %rax, " + tempAt(left));
        genValue(*expression.b);
        if (commonType) convert(expression.b->type, commonType);
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
            convert(expression.b->type, type);
        } else {
            loadValue(type);
            int previous = allocTemp();
            spill(type, previous);
            genValue(*expression.b);
            convert(expression.b->type, type);
            if (isFloating(type)) {
                emit("movapd %xmm0, %xmm1");
                fillValue(type, previous);
            } else {
                emit("movq %rax, %rcx");
                emit("movq " + tempAt(previous) + ", %rax");
            }
            releaseTemps(1);

            detail::Expr operation;
            operation.text = expression.text.substr(0, expression.text.size() - 1);
            operation.type = type;
            operation.a = std::unique_ptr<detail::Expr>(new detail::Expr());
            operation.a->type = type;
            operation.b = std::unique_ptr<detail::Expr>(new detail::Expr());
            operation.b->type = type;
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

        struct Argument {
            int slot = 0;
            detail::TypePtr type;
        };
        std::vector<Argument> arguments;
        const std::size_t parameterCount = declaration ? declaration->parameters.size() : 0;
        // A member call passes the object as the hidden first argument, so it
        // takes the first integer register and every other argument follows it.
        const std::size_t hidden = expression.a ? 1 : 0;
        if (expression.a) {
            if (expression.a->type->isPointer()) {
                genValue(*expression.a);
            } else {
                genAddress(*expression.a);
            }
            Argument self;
            self.type = nullptr;
            self.slot = allocTemp();
            spill(nullptr, self.slot);
            arguments.push_back(self);
        }
        for (std::size_t i = 0; i < expression.args.size(); ++i) {
            const detail::ExprPtr& argument = expression.args[i];
            genValue(*argument);
            detail::TypePtr type = argument->type;
            if (declaration && i + hidden < parameterCount) {
                type = declaration->parameters[i + hidden].type;
                convert(argument->type, type);
            } else if (declaration && declaration->variadic && isFloating(argument->type) &&
                       argument->type->kind == detail::TypeKind::Float) {
                // The default argument promotion: printf reads a double.
                emit("cvtss2sd %xmm0, %xmm0");
                type = detail::primitiveType(detail::TypeKind::Double);
            }
            Argument stored;
            stored.type = type;
            stored.slot = allocTemp();
            spill(type, stored.slot);
            arguments.push_back(stored);
        }

        // The System V calling convention: integers and pointers travel in
        // %rdi..%r9, floating point values in %xmm0..%xmm7, each counted
        // separately, and whatever does not fit goes on the stack in order.
        static const char* kRegisters[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
        std::vector<Argument> integerArguments;
        std::vector<Argument> floatArguments;
        std::vector<Argument> stackArguments;
        for (const Argument& argument : arguments) {
            if (isFloating(argument.type)) {
                if (floatArguments.size() < 8) floatArguments.push_back(argument);
                else stackArguments.push_back(argument);
            } else {
                if (integerArguments.size() < 6) integerArguments.push_back(argument);
                else stackArguments.push_back(argument);
            }
        }

        const bool pad = (stackArguments.size() % 2) != 0;
        if (pad) emit("subq $8, %rsp");
        for (std::size_t i = stackArguments.size(); i-- > 0;) {
            emit("pushq " + tempAt(stackArguments[i].slot));
        }
        for (std::size_t i = 0; i < integerArguments.size(); ++i) {
            emit("movq " + tempAt(integerArguments[i].slot) + ", " + kRegisters[i]);
        }
        for (std::size_t i = 0; i < floatArguments.size(); ++i) {
            emit(moveOp(floatArguments[i].type) + " " + tempAt(floatArguments[i].slot) + ", %xmm" +
                 std::to_string(i));
        }
        // printf and friends read this to know how many vector registers are live.
        emit("movb $" + std::to_string(floatArguments.size()) + ", %al");

        const bool external = !declaration || !declaration->defined;
        emit("call " + symbol(expression.text) + (external && options_.usePlt ? "@PLT" : ""));

        const std::size_t stackCount = stackArguments.size();
        if (stackCount > 0 || pad) {
            emit("addq $" + std::to_string(8 * (stackCount + (pad ? 1 : 0))) + ", %rsp");
        }
        releaseTemps(static_cast<int>(arguments.size()));

        narrow(expression.type);
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
                    convert(declaration.init->type, declaration.type);
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
                toBool(statement.expr->type);
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
                toBool(statement.expr->type);
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
                toBool(statement.expr->type);
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
                    toBool(statement.expr->type);
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
                    convert(statement.expr->type, function_->declaration->result);
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
        std::size_t integerIndex = 0;
        std::size_t floatIndex = 0;
        std::size_t stackIndex = 0;
        for (const detail::Parameter& parameter : parameters) {
            const bool floating = isFloating(parameter.type);
            const std::string memory =
                std::to_string(16 + 8 * static_cast<int>(stackIndex)) + "(%rbp)";
            if (floating && floatIndex < 8) {
                if (!parameter.name.empty()) {
                    emit(moveOp(parameter.type) + " %xmm" + std::to_string(floatIndex) + ", " +
                         std::to_string(parameter.offset) + "(%rbp)");
                }
                ++floatIndex;
                continue;
            }
            if (!floating && integerIndex < 6) {
                if (!parameter.name.empty()) {
                    emit("movq " + std::string(kRegisters[integerIndex]) + ", %rax");
                    storeAt(parameter.type, parameter.offset);
                }
                ++integerIndex;
                continue;
            }
            // Out of registers: the value is on the stack, in argument order.
            if (!parameter.name.empty()) {
                if (floating) {
                    emit(moveOp(parameter.type) + " " + memory + ", %xmm0");
                    emit(moveOp(parameter.type) + " %xmm0, " +
                         std::to_string(parameter.offset) + "(%rbp)");
                } else {
                    emit("movq " + memory + ", %rax");
                    storeAt(parameter.type, parameter.offset);
                }
            }
            ++stackIndex;
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
            if (definition.realInit) {
                text_ += std::string("    ") +
                         (definition.type->kind == detail::TypeKind::Float ? ".float " : ".double ") +
                         realText(definition.realValue) + "\n";
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

    // A floating point constant has to be spelled so the assembler keeps it a
    // floating point value: `1` is an integer, `1.0` is a double.
    static std::string realText(double value) {
        char buffer[64];
        std::snprintf(buffer, sizeof(buffer), "%.17g", value);
        std::string text = buffer;
        if (text.find('.') == std::string::npos && text.find('e') == std::string::npos &&
            text.find("inf") == std::string::npos && text.find("nan") == std::string::npos) {
            text += ".0";
        }
        return text;
    }

    void reals() {
        text_ += "\n    .section .rodata\n";
        for (std::size_t i = 0; i < program_.reals.size(); ++i) {
            const detail::RealLiteral& literal = program_.reals[i];
            text_ += ".Lreal" + std::to_string(i) + ":\n";
            text_ += std::string("    ") + (literal.isFloat ? ".float " : ".double ") +
                     realText(literal.value) + "\n";
        }
        // ++ and -- on a floating point value add and subtract one.
        text_ += ".Lone:\n    .double 1.0\n.Lonef:\n    .float 1.0\n";
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
        if (!diagnostic.file.empty()) text += diagnostic.file + ":";
        text += std::to_string(diagnostic.line) + ":" + std::to_string(diagnostic.column) + ": " +
                diagnostic.message + "\n";
    }
    return text;
}

Result compile(const std::string& source, const Options& options) {
    Result result;

    detail::LexResult lexed = detail::lex(
        source, options.includePrelude ? detail::defaultPreludeText() : std::string(), options);
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

    if (options.requireEntryPoint &&
        parsed.program->functions.find("main") == parsed.program->functions.end()) {
        Diagnostic diagnostic;
        diagnostic.message = "no main function: a program needs int main()";
        result.diagnostics.push_back(diagnostic);
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
        "Types: void, bool, char, short, int, long (64-bit) and their unsigned forms,\n"
        "float, double, pointers, arrays and structs by pointer.\n"
        "Statements: blocks, declarations, if/else, while, do/while, for, return, break, continue.\n"
        "Expressions: arithmetic, comparisons, && || !, ?:, casts, sizeof, ++/--, compound\n"
        "assignment, calls, subscripts and field access.\n"
        "Member functions: declared or defined in a struct, or defined as Rect::area();\n"
        "the object is passed as the hidden `this` argument.\n"
        "Preprocessor: object-like #define, #undef, #ifdef/#ifndef/#else/#endif and #include\n"
        "(\"...\" beside the source or on -I, <...> on -I, #pragma once).\n"
        "Storage: globals, static locals and static functions.\n"
        "Not supported yet: inheritance, virtual functions, constructors, templates,\n"
        "exceptions, long double, structs or arrays by value, function pointers.";
}

}  // namespace cxx
}  // namespace rblx
