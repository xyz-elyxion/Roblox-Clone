#include "WebJson.h"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace rblx {
namespace web {
namespace {

const std::size_t kMaxDepth = 64;

void appendUtf8(std::string& out, unsigned int codePoint) {
    if (codePoint <= 0x7F) {
        out.push_back(static_cast<char>(codePoint));
    } else if (codePoint <= 0x7FF) {
        out.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
        out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else if (codePoint <= 0xFFFF) {
        out.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
        out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    } else {
        out.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
        out.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
    }
}

class Parser {
public:
    explicit Parser(const std::string& text) : text_(text) {}

    bool parse(Json& out) {
        skipWhitespace();
        if (!parseValue(out, 0)) {
            return false;
        }
        skipWhitespace();
        if (position_ != text_.size()) {
            error_ = "Unexpected trailing characters.";
            return false;
        }
        return true;
    }

    const std::string& error() const { return error_; }

private:
    void skipWhitespace() {
        while (position_ < text_.size()) {
            const char c = text_[position_];
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                ++position_;
            } else {
                break;
            }
        }
    }

    bool fail(const std::string& message) {
        if (error_.empty()) {
            error_ = message;
        }
        return false;
    }

    bool parseValue(Json& out, std::size_t depth) {
        if (depth > kMaxDepth) {
            return fail("Document is nested too deeply.");
        }
        if (position_ >= text_.size()) {
            return fail("Unexpected end of document.");
        }

        const char c = text_[position_];
        if (c == '{') {
            return parseObject(out, depth);
        }
        if (c == '[') {
            return parseArray(out, depth);
        }
        if (c == '"') {
            std::string value;
            if (!parseString(value)) {
                return false;
            }
            out = Json(value);
            return true;
        }
        if (c == 't' && text_.compare(position_, 4, "true") == 0) {
            position_ += 4;
            out = Json(true);
            return true;
        }
        if (c == 'f' && text_.compare(position_, 5, "false") == 0) {
            position_ += 5;
            out = Json(false);
            return true;
        }
        if (c == 'n' && text_.compare(position_, 4, "null") == 0) {
            position_ += 4;
            out = Json();
            return true;
        }
        return parseNumber(out);
    }

    bool parseObject(Json& out, std::size_t depth) {
        ++position_;  // '{'
        out = Json::makeObject();
        skipWhitespace();
        if (position_ < text_.size() && text_[position_] == '}') {
            ++position_;
            return true;
        }

        while (true) {
            skipWhitespace();
            if (position_ >= text_.size() || text_[position_] != '"') {
                return fail("Expected an object key.");
            }
            std::string key;
            if (!parseString(key)) {
                return false;
            }
            skipWhitespace();
            if (position_ >= text_.size() || text_[position_] != ':') {
                return fail("Expected ':' after an object key.");
            }
            ++position_;
            skipWhitespace();
            Json value;
            if (!parseValue(value, depth + 1)) {
                return false;
            }
            out.set(key, value);
            skipWhitespace();
            if (position_ >= text_.size()) {
                return fail("Unterminated object.");
            }
            if (text_[position_] == ',') {
                ++position_;
                continue;
            }
            if (text_[position_] == '}') {
                ++position_;
                return true;
            }
            return fail("Expected ',' or '}' in an object.");
        }
    }

    bool parseArray(Json& out, std::size_t depth) {
        ++position_;  // '['
        out = Json::makeArray();
        skipWhitespace();
        if (position_ < text_.size() && text_[position_] == ']') {
            ++position_;
            return true;
        }

        while (true) {
            skipWhitespace();
            Json value;
            if (!parseValue(value, depth + 1)) {
                return false;
            }
            out.push(value);
            skipWhitespace();
            if (position_ >= text_.size()) {
                return fail("Unterminated array.");
            }
            if (text_[position_] == ',') {
                ++position_;
                continue;
            }
            if (text_[position_] == ']') {
                ++position_;
                return true;
            }
            return fail("Expected ',' or ']' in an array.");
        }
    }

    bool parseHex4(unsigned int* value) {
        if (position_ + 4 > text_.size()) {
            return fail("Truncated unicode escape.");
        }
        unsigned int result = 0;
        for (int i = 0; i < 4; ++i) {
            const char c = text_[position_ + static_cast<std::size_t>(i)];
            result <<= 4;
            if (c >= '0' && c <= '9') {
                result |= static_cast<unsigned int>(c - '0');
            } else if (c >= 'a' && c <= 'f') {
                result |= static_cast<unsigned int>(c - 'a' + 10);
            } else if (c >= 'A' && c <= 'F') {
                result |= static_cast<unsigned int>(c - 'A' + 10);
            } else {
                return fail("Invalid unicode escape.");
            }
        }
        position_ += 4;
        *value = result;
        return true;
    }

    bool parseString(std::string& out) {
        ++position_;  // opening quote
        out.clear();
        while (true) {
            if (position_ >= text_.size()) {
                return fail("Unterminated string.");
            }
            const char c = text_[position_++];
            if (c == '"') {
                return true;
            }
            if (c != '\\') {
                out.push_back(c);
                continue;
            }
            if (position_ >= text_.size()) {
                return fail("Unterminated escape sequence.");
            }
            const char escape = text_[position_++];
            switch (escape) {
                case '"': out.push_back('"'); break;
                case '\\': out.push_back('\\'); break;
                case '/': out.push_back('/'); break;
                case 'b': out.push_back('\b'); break;
                case 'f': out.push_back('\f'); break;
                case 'n': out.push_back('\n'); break;
                case 'r': out.push_back('\r'); break;
                case 't': out.push_back('\t'); break;
                case 'u': {
                    unsigned int codePoint = 0;
                    if (!parseHex4(&codePoint)) {
                        return false;
                    }
                    if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                        if (position_ + 1 < text_.size() && text_[position_] == '\\' &&
                            text_[position_ + 1] == 'u') {
                            position_ += 2;
                            unsigned int low = 0;
                            if (!parseHex4(&low)) {
                                return false;
                            }
                            if (low >= 0xDC00 && low <= 0xDFFF) {
                                codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                            } else {
                                codePoint = 0xFFFD;
                            }
                        } else {
                            codePoint = 0xFFFD;
                        }
                    }
                    appendUtf8(out, codePoint);
                    break;
                }
                default:
                    return fail("Unsupported escape sequence.");
            }
        }
    }

    bool parseNumber(Json& out) {
        const std::size_t start = position_;
        if (position_ < text_.size() && (text_[position_] == '-' || text_[position_] == '+')) {
            ++position_;
        }
        while (position_ < text_.size()) {
            const char c = text_[position_];
            const bool numeric = (c >= '0' && c <= '9') || c == '.' || c == 'e' || c == 'E' ||
                                 c == '+' || c == '-';
            if (!numeric) {
                break;
            }
            ++position_;
        }
        if (position_ == start) {
            return fail("Expected a JSON value.");
        }

        const std::string token = text_.substr(start, position_ - start);
        char* end = nullptr;
        const double value = std::strtod(token.c_str(), &end);
        if (end == nullptr || *end != '\0') {
            position_ = start;
            return fail("Invalid number.");
        }
        if (!std::isfinite(value)) {
            position_ = start;
            return fail("Number is out of range.");
        }
        out = Json(value);
        return true;
    }

    const std::string& text_;
    std::size_t position_ = 0;
    std::string error_;
};

}  // namespace

Json::Json() = default;

Json::Json(bool value) : type_(Type::Bool), bool_(value) {}

Json::Json(double value) : type_(Type::Number), number_(value) {}

Json::Json(int value) : type_(Type::Number), number_(static_cast<double>(value)) {}

Json::Json(long long value) : type_(Type::Number), number_(static_cast<double>(value)) {}

Json::Json(const std::string& value) : type_(Type::String), string_(value) {}

Json::Json(const char* value) : type_(Type::String), string_(value ? value : "") {}

Json Json::makeObject() {
    Json value;
    value.type_ = Type::Object;
    return value;
}

Json Json::makeArray() {
    Json value;
    value.type_ = Type::Array;
    return value;
}

Json Json::parse(const std::string& text, std::string* error) {
    Parser parser(text);
    Json value;
    if (!parser.parse(value)) {
        if (error) {
            *error = parser.error();
        }
        return Json();
    }
    if (error) {
        error->clear();
    }
    return value;
}

bool Json::asBool(bool fallback) const {
    if (type_ == Type::Bool) {
        return bool_;
    }
    if (type_ == Type::Number) {
        return number_ != 0.0;
    }
    return fallback;
}

double Json::asNumber(double fallback) const {
    if (type_ == Type::Number) {
        return number_;
    }
    return fallback;
}

long long Json::asInteger(long long fallback) const {
    if (type_ != Type::Number) {
        return fallback;
    }
    if (!std::isfinite(number_)) {
        return fallback;
    }
    return static_cast<long long>(number_);
}

std::string Json::asString(const std::string& fallback) const {
    if (type_ == Type::String) {
        return string_;
    }
    return fallback;
}

bool Json::has(const std::string& key) const {
    if (type_ != Type::Object) {
        return false;
    }
    for (const auto& entry : object_) {
        if (entry.first == key) {
            return true;
        }
    }
    return false;
}

const Json& Json::member(const std::string& key) const {
    static const Json nullValue;
    if (type_ != Type::Object) {
        return nullValue;
    }
    for (const auto& entry : object_) {
        if (entry.first == key) {
            return entry.second;
        }
    }
    return nullValue;
}

Json& Json::set(const std::string& key, Json value) {
    if (type_ != Type::Object) {
        type_ = Type::Object;
        object_.clear();
    }
    for (auto& entry : object_) {
        if (entry.first == key) {
            entry.second = std::move(value);
            return *this;
        }
    }
    object_.emplace_back(key, std::move(value));
    return *this;
}

Json& Json::set(const std::string& key, const std::string& value) {
    return set(key, Json(value));
}

Json& Json::set(const std::string& key, const char* value) {
    return set(key, Json(value));
}

Json& Json::set(const std::string& key, double value) {
    return set(key, Json(value));
}

Json& Json::set(const std::string& key, int value) {
    return set(key, Json(value));
}

Json& Json::set(const std::string& key, long long value) {
    return set(key, Json(value));
}

Json& Json::set(const std::string& key, bool value) {
    return set(key, Json(value));
}

Json& Json::setNull(const std::string& key) {
    return set(key, Json());
}

void Json::push(Json value) {
    if (type_ != Type::Array) {
        type_ = Type::Array;
        array_.clear();
    }
    array_.push_back(std::move(value));
}

std::size_t Json::size() const {
    if (type_ == Type::Array) {
        return array_.size();
    }
    if (type_ == Type::Object) {
        return object_.size();
    }
    return 0;
}

std::string Json::dump() const {
    std::string out;
    out.reserve(128);
    dumpTo(out);
    return out;
}

void Json::dumpTo(std::string& out) const {
    switch (type_) {
        case Type::Null:
            out += "null";
            break;
        case Type::Bool:
            out += bool_ ? "true" : "false";
            break;
        case Type::Number:
            out += formatJsonNumber(number_);
            break;
        case Type::String:
            out += jsonEscape(string_);
            break;
        case Type::Array: {
            out.push_back('[');
            for (std::size_t i = 0; i < array_.size(); ++i) {
                if (i > 0) {
                    out.push_back(',');
                }
                array_[i].dumpTo(out);
            }
            out.push_back(']');
            break;
        }
        case Type::Object: {
            out.push_back('{');
            for (std::size_t i = 0; i < object_.size(); ++i) {
                if (i > 0) {
                    out.push_back(',');
                }
                out += jsonEscape(object_[i].first);
                out.push_back(':');
                object_[i].second.dumpTo(out);
            }
            out.push_back('}');
            break;
        }
    }
}

std::string jsonEscape(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 2);
    out.push_back('"');
    for (const char rawChar : value) {
        const unsigned char c = static_cast<unsigned char>(rawChar);
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buffer[8];
                    std::snprintf(buffer, sizeof(buffer), "\\u%04x", static_cast<unsigned int>(c));
                    out += buffer;
                } else {
                    out.push_back(static_cast<char>(c));
                }
                break;
        }
    }
    out.push_back('"');
    return out;
}

std::string formatJsonNumber(double value) {
    if (!std::isfinite(value)) {
        return "null";
    }
    char buffer[40];
    std::snprintf(buffer, sizeof(buffer), "%.15g", value);
    return std::string(buffer);
}

}  // namespace web
}  // namespace rblx
