#pragma once

#include <cstddef>
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace rblx {
namespace script {

class Interpreter;
struct Function;
struct Table;

// A dynamically typed script value. Scripts never see this struct directly:
// parts, vectors and signals are exposed as ScriptObject userdata.
struct Value {
    enum class Type { Nil, Boolean, Number, String, Table, Function, Object };

    Type type = Type::Nil;
    bool boolean = false;
    double number = 0.0;
    std::string string;
    std::shared_ptr<Table> table;
    std::shared_ptr<Function> function;
    std::shared_ptr<class ScriptObject> object;

    static Value nil();
    static Value booleanValue(bool value);
    static Value numberValue(double value);
    static Value stringValue(std::string value);
    static Value tableValue(std::shared_ptr<Table> value);
    static Value functionValue(std::shared_ptr<Function> value);
    static Value objectValue(std::shared_ptr<ScriptObject> value);

    bool isNil() const { return type == Type::Nil; }
    bool isNilOrFalse() const { return type == Type::Nil || (type == Type::Boolean && !boolean); }
    bool isNumber() const { return type == Type::Number; }
    bool isString() const { return type == Type::String; }
    bool isTable() const { return type == Type::Table; }
    bool isFunction() const { return type == Type::Function; }
    bool isObject() const { return type == Type::Object; }
    bool truthy() const { return !isNilOrFalse(); }
    double asNumber() const { return type == Type::Number ? number : 0.0; }
    bool asBool() const { return type == Type::Boolean ? boolean : truthy(); }
    std::string asString() const { return type == Type::String ? string : std::string(); }

    // Lua-style textual form, used by print, tostring and error messages.
    std::string toString() const;
};

struct Table {
    std::map<std::string, Value> fields;
    std::vector<Value> elements;  // 1-based array part

    Value get(const Value& key) const;
    void set(const Value& key, const Value& value);
};

// An event scripts can subscribe to with `signal:Connect(function(...) ... end)`.
// Handlers keep their captured script state alive through the recorded Value.
class Signal {
public:
    void connect(const Value& handler);
    bool empty() const { return handlers_.empty(); }
    std::size_t size() const { return handlers_.size(); }

    // Calls every handler, reporting handler errors to the interpreter instead of
    // letting one broken listener take down the caller.
    void fire(Interpreter& interpreter, const std::vector<Value>& args) const;

private:
    std::vector<Value> handlers_;
};

// Host object exposed to scripts (a part, a Vector3, the workspace, a signal...).
class ScriptObject {
public:
    virtual ~ScriptObject() = default;

    virtual const char* typeName() const = 0;

    virtual bool get(const std::string& name, Value& out) {
        (void)name;
        (void)out;
        return false;
    }
    virtual bool set(const std::string& name, const Value& value) {
        (void)name;
        (void)value;
        return false;
    }
    virtual bool call(Interpreter& interpreter, const std::string& method,
                      const std::vector<Value>& args, Value& out, std::string& error) {
        (void)interpreter;
        (void)method;
        (void)args;
        (void)out;
        error = std::string(typeName()) + " has no method '" + method + "'";
        return false;
    }
    virtual Signal* signal(const std::string& name) {
        (void)name;
        return nullptr;
    }
};

// Wraps a Signal so scripts can subscribe with `signal:Connect(function(...) ... end)`.
class SignalHandle : public ScriptObject {
public:
    explicit SignalHandle(Signal* signal) : signal_(signal) {}

    const char* typeName() const override { return "Signal"; }
    bool call(Interpreter& interpreter, const std::string& method,
              const std::vector<Value>& args, Value& out, std::string& error) override;

private:
    Signal* signal_;
};

Value makeSignalValue(Signal& signal);

Value makeNativeFunction(std::function<Value(Interpreter&, const std::vector<Value>&)> fn);
Value makeTable();

class Interpreter {
public:
    Interpreter();
    ~Interpreter();

    void setPrintHandler(std::function<void(const std::string&, bool)> handler);
    void print(const std::string& message, bool isError) const;

    void setGlobal(const std::string& name, const Value& value);
    Value getGlobal(const std::string& name) const;
    bool hasGlobal(const std::string& name) const;

    // Runs a chunk. Returns false and fills `error` (with a line number) on failure.
    bool run(const std::string& source, const std::string& chunkName, std::string& error);

    bool callFunction(const Value& function, const std::vector<Value>& args, Value& result,
                      std::string& error);

    // Native functions use this to abort the current script with a message.
    [[noreturn]] void raise(const std::string& message) const;

    void setStepLimit(long long limit);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

}  // namespace script
}  // namespace rblx
