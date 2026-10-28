#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

namespace rblx {
namespace web {

// Minimal JSON value used by the native website backend. Objects keep their
// insertion order so responses stay stable and readable.
class Json {
public:
    enum class Type { Null, Bool, Number, String, Array, Object };

    Json();
    explicit Json(bool value);
    explicit Json(double value);
    explicit Json(int value);
    explicit Json(long long value);
    explicit Json(const std::string& value);
    explicit Json(const char* value);

    static Json makeObject();
    static Json makeArray();
    // Returns a Null value and fills `error` when the document is malformed.
    static Json parse(const std::string& text, std::string* error);

    Type type() const { return type_; }
    bool isNull() const { return type_ == Type::Null; }
    bool isBool() const { return type_ == Type::Bool; }
    bool isNumber() const { return type_ == Type::Number; }
    bool isString() const { return type_ == Type::String; }
    bool isArray() const { return type_ == Type::Array; }
    bool isObject() const { return type_ == Type::Object; }

    bool asBool(bool fallback = false) const;
    double asNumber(double fallback = 0.0) const;
    long long asInteger(long long fallback = 0) const;
    std::string asString(const std::string& fallback = std::string()) const;

    bool has(const std::string& key) const;
    // Returns a Null value when the key is absent or this is not an object.
    const Json& member(const std::string& key) const;

    Json& set(const std::string& key, Json value);
    Json& set(const std::string& key, const std::string& value);
    Json& set(const std::string& key, const char* value);
    Json& set(const std::string& key, double value);
    Json& set(const std::string& key, int value);
    Json& set(const std::string& key, long long value);
    Json& set(const std::string& key, bool value);
    Json& setNull(const std::string& key);

    void push(Json value);
    std::size_t size() const;
    const std::vector<Json>& elements() const { return array_; }
    const std::vector<std::pair<std::string, Json>>& members() const { return object_; }

    std::string dump() const;
    void dumpTo(std::string& out) const;

private:
    Type type_ = Type::Null;
    bool bool_ = false;
    double number_ = 0.0;
    std::string string_;
    std::vector<Json> array_;
    std::vector<std::pair<std::string, Json>> object_;
};

std::string jsonEscape(const std::string& value);
std::string formatJsonNumber(double value);

}  // namespace web
}  // namespace rblx
