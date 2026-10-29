#include "RblxScript.h"

#include <cstdio>
#include <string>
#include <vector>

using namespace rblx::script;

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

class TestObject : public ScriptObject {
public:
    const char* typeName() const override { return "TestObject"; }

    bool get(const std::string& name, Value& out) override {
        if (name != "value") return false;
        out = Value::numberValue(value);
        return true;
    }

    bool set(const std::string& name, const Value& newValue) override {
        if (name != "value") return false;
        value = newValue.asNumber();
        return true;
    }

    double value = 1.0;
};

struct Harness {
    Interpreter interpreter;
    std::vector<std::string> printed;
    std::vector<std::string> errors;

    Harness() {
        interpreter.setPrintHandler([this](const std::string& message, bool isError) {
            if (isError) {
                errors.push_back(message);
            } else {
                printed.push_back(message);
            }
        });
    }

    bool run(const std::string& source, std::string& error) {
        return interpreter.run(source, "test", error);
    }

    std::string lastPrinted() const { return printed.empty() ? std::string() : printed.back(); }
};

void testArithmeticAndStrings() {
    Harness harness;
    std::string error;
    const bool ok = harness.run(
        "local a = 2 + 3 * 4\n"
        "local b = (2 + 3) * 4\n"
        "local c = 2 ^ 3 ^ 2\n"
        "local d = -2 ^ 2\n"
        "local text = \"hello\" .. \" \" .. \"world\"\n"
        "local half = 10 / 4\n"
        "print(a, b, c, d, text, half)\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(harness.lastPrinted() == "14\t20\t512\t-4\thello world\t2.5");
}

void testControlFlow() {
    Harness harness;
    std::string error;
    const bool ok = harness.run(
        "local total = 0\n"
        "for i = 1, 10 do\n"
        "    total = total + i\n"
        "end\n"
        "local n = 0\n"
        "while n < 3 do\n"
        "    n = n + 1\n"
        "end\n"
        "local r = 0\n"
        "repeat\n"
        "    r = r + 1\n"
        "until r == 4\n"
        "if total == 55 and n == 3 then\n"
        "    print(\"ok\", r)\n"
        "else\n"
        "    print(\"bad\")\n"
        "end\n"
        "local fallback = nil\n"
        "print(fallback or \"used\")\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(harness.printed.size() == 2);
    if (harness.printed.size() == 2) {
        CHECK(harness.printed[0] == "ok\t4");
        CHECK(harness.printed[1] == "used");
    }
}

void testFunctionsAndClosures() {
    Harness harness;
    std::string error;
    const bool ok = harness.run(
        "local function fact(n)\n"
        "    if n <= 1 then return 1 end\n"
        "    return n * fact(n - 1)\n"
        "end\n"
        "local function makeCounter()\n"
        "    local count = 0\n"
        "    return function()\n"
        "        count = count + 1\n"
        "        return count\n"
        "    end\n"
        "end\n"
        "local counter = makeCounter()\n"
        "counter()\n"
        "counter()\n"
        "print(fact(5), counter())\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(harness.lastPrinted() == "120\t3");
}

void testTablesAndMethods() {
    Harness harness;
    std::string error;
    const bool ok = harness.run(
        "local t = { 10, 20, 30, label = \"parts\" }\n"
        "t[2] = 25\n"
        "t.extra = true\n"
        "local total = 0\n"
        "for i = 1, 3 do\n"
        "    total = total + t[i]\n"
        "end\n"
        "local counter = { value = 0 }\n"
        "function counter:bump(amount)\n"
        "    self.value = self.value + amount\n"
        "    return self.value\n"
        "end\n"
        "counter:bump(5)\n"
        "print(t.label, total, t.extra, t.missing, counter:bump(2), counter.value)\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(harness.lastPrinted() == "parts\t65\ttrue\tnil\t7\t7");
}

void testHostObjects() {
    Harness harness;
    auto object = std::make_shared<TestObject>();
    harness.interpreter.setGlobal("obj", Value::objectValue(object));

    std::string error;
    const bool ok = harness.run(
        "obj.value = 5\n"
        "obj.value = obj.value * 2\n"
        "print(obj.value)\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(harness.lastPrinted() == "10");
    CHECK(object->value == 10.0);
}

void testSignals() {
    Harness harness;
    Signal ticker;
    harness.interpreter.setGlobal("ticker", makeSignalValue(ticker));

    std::string error;
    const bool ok = harness.run(
        "local received = 0\n"
        "ticker:Connect(function(amount)\n"
        "    received = received + amount\n"
        "    print(\"tick\", received)\n"
        "end)\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(ticker.size() == 1);

    ticker.fire(harness.interpreter, {Value::numberValue(2.5)});
    ticker.fire(harness.interpreter, {Value::numberValue(1.0)});
    CHECK(harness.printed.size() == 2);
    if (harness.printed.size() == 2) {
        CHECK(harness.printed[0] == "tick\t2.5");
        CHECK(harness.printed[1] == "tick\t3.5");
    }
    CHECK(harness.errors.empty());
}

void testComparisonAndLogic() {
    Harness harness;
    std::string error;
    const bool ok = harness.run(
        "print(10 .. \"x\", 3 ~= 4, 3 == 3, \"a\" < \"b\", not false, not nil)\n",
        error);
    CHECK(ok);
    CHECK(error.empty());
    CHECK(harness.lastPrinted() == "10x\ttrue\ttrue\ttrue\ttrue\ttrue");
}

void testErrors() {
    Harness harness;
    std::string error;
    CHECK(!harness.run("local value = nil\nprint(value.field)\n", error));
    CHECK(error.find("test:2:") != std::string::npos);

    error.clear();
    CHECK(!harness.run("local = 5\n", error));
    CHECK(error.find("test:") != std::string::npos);

    error.clear();
    harness.interpreter.setStepLimit(2000);
    CHECK(!harness.run("while true do end\n", error));
    CHECK(error.find("execution limit") != std::string::npos);
    harness.interpreter.setStepLimit(2000000);

    error.clear();
    CHECK(!harness.run("local function broken()\n    error(\"boom\")\nend\nbroken()\n", error));
    CHECK(error.find("boom") != std::string::npos);
}

}  // namespace

int main() {
    testArithmeticAndStrings();
    testControlFlow();
    testFunctionsAndClosures();
    testTablesAndMethods();
    testHostObjects();
    testSignals();
    testComparisonAndLogic();
    testErrors();

    std::printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
