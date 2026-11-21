#include "Serializer.h"

#include "Core/Log.h"
#include "Utils/JSONValue.h"

#include <cctype>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <system_error>
#include <utility>
#include <vector>

void appendUtf8(std::string& out, uint32_t codepoint) {
    if (codepoint < 0x80) {
        out += static_cast<char>(codepoint);
    } else if (codepoint < 0x800) {
        out += static_cast<char>(0xC0 | (codepoint >> 6));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint < 0x10000) {
        out += static_cast<char>(0xE0 | (codepoint >> 12));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else {
        out += static_cast<char>(0xF0 | (codepoint >> 18));
        out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

namespace Axiom {
    std::expected<JSONValue, Serializer::ParseError> Serializer::JSONParser::parseValue() {
        skipWhitespace();
        switch (peek()) {
        case '{':
            return parseObject();
        case '[':
            return parseArray();
        case '"':
            return parseString();
        case 't':
            return parseLiteral("true", JSONValue(true));
        case 'f':
            return parseLiteral("false", JSONValue(false));
        case 'n':
            return parseLiteral("null", JSONValue(nullptr));
        default:
            if (peek() == ' ' || std::isdigit(static_cast<unsigned char>(peek()))) {
                return parseNumber();
            }
            return error(atEnd() ? "Unexpected end of input" : std::format("Unexpected character: '{}'", peek()));
        }
    }

    std::expected<JSONValue, Serializer::ParseError> Serializer::JSONParser::parseLiteral(std::string_view word, JSONValue result) {
        if (!text.substr(pos).starts_with(word)) {
            return error(std::format("Expected: '{}'", word));
        }
        for (size_t i = 0; i < word.size(); i++) {
            advance();
        }
        return result;
    }

    std::expected<JSONValue, Serializer::ParseError> Serializer::JSONParser::parseNumber() {
        const size_t start = pos;
        bool isFloat = false;
        while (!atEnd()) {
            const char c = peek();
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '-' || c == '+') {
                advance();
            } else if (c == '.' || c == 'e' || c == 'E') {
                isFloat = true;
                advance();
            } else {
                break;
            }
        }

        const std::string_view numberStr = text.substr(start, pos - start);
        const char* first = numberStr.data();
        const char* last = first + numberStr.size();
        if (isFloat) {
            double result{};
            auto [ptr, ec] = std::from_chars(first, last, result);
            if (ec != std::errc() || ptr != last) {
                return error(std::format("Invalid number: {}", numberStr));
            }
            return JSONValue(result);
        }

        int64_t result{};
        auto [ptr, ec] = std::from_chars(first, last, result);
        if (ec == std::errc::result_out_of_range) {
            return error(std::format("Integer out of range: {}", numberStr));
        }
        if (ec != std::errc() || ptr != last) {
            return error(std::format("Invalid number: {}", numberStr));
        }
        return JSONValue(result);
    }

    std::expected<uint32_t, Serializer::ParseError> Serializer::JSONParser::parseHex() {
        if (text.size() - pos < 4) {
            return error("Incomplete \\u escape");
        }
        const char* first = text.data() + pos;
        const char* last = first + 4;
        uint32_t result{};
        auto [ptr, ec] = std::from_chars(first, last, result, 16);
        if (ec != std::errc() || ptr != last) {
            return error("Incomplete \\u escape");
        }
        for (int i = 0; i < 4; i++) {
            advance();
        }
        return result;
    }

    std::expected<std::string, Serializer::ParseError> Serializer::JSONParser::parseRawString() {
        if (peek() != '"') {
            return error("Expected '\"'");
        }
        advance();

        std::string result;
        while (true) {
            if (atEnd()) {
                return error("Unterminated string");
            }

            const char c = advance();
            if (c == '"') {
                return result;
            }
            if (static_cast<unsigned char>(c) < 0x20) {
                return error("Unexpected control character in string");
            }
            if (c != '\\') {
                result += c;
                continue;
            }

            if (atEnd()) {
                return error("Unterminated escape sequence");
            }
            const char escape = advance();
            switch (escape) {
            case '"':
                result += '"';
                break;
            case '\\':
                result += '\\';
                break;
            case '/':
                result += '/';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'u': {
                auto high = parseHex();
                if (!high) {
                    return std::unexpected(high.error());
                }
                uint32_t codepoint = *high;
                if (codepoint >= 0xD800 && codepoint <= 0xDBFF) {
                    if (!text.substr(pos).starts_with(R"(\u)")) {
                        return error("Unpaired high surrogate in \\u escape");
                    }
                    advance();
                    advance();
                    auto low = parseHex();
                    if (!low) {
                        return std::unexpected(low.error());
                    }
                    if (*low < 0xDC00 || *low > 0xDFFF) {
                        return error("Invalid low surrogate in \\u escape");
                    }
                    codepoint = 0x10000 + ((codepoint - 0xD800) << 10) + (*low - 0xDC00);
                } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
                    return error("Unpaired low surrogate in \\u escape");
                }

                appendUtf8(result, codepoint);
                break;
            }
            default:
                return error(std::format("Invalid escape sequence: '\\{}'", escape));
            }
        }
    }

    std::expected<JSONValue, Serializer::ParseError> Serializer::JSONParser::parseString() {
        std::expected<std::string, ParseError> str = parseRawString();
        if (!str) {
            return std::unexpected<ParseError>(str.error());
        }
        return JSONValue(std::move(str.value()));
    }

    std::expected<JSONValue, Serializer::ParseError> Serializer::JSONParser::parseArray() {
        advance();
        JSONValue array = JSONValue::Array{};

        skipWhitespace();
        if (peek() == ']') {
            advance();
            return array;
        }

        while (true) {
            std::expected<JSONValue, Serializer::ParseError> element = parseValue();
            if (!element) {
                return element;
            }
            array.append(std::move(element.value()));

            skipWhitespace();
            if (peek() == ',') {
                advance();
                continue;
            }
            if (peek() == ']') {
                advance();
                return array;
            }
            return error(atEnd() ? "Unterminated array" : "Expected ',' or ']' in array");
        }
    }

    std::expected<JSONValue, Serializer::ParseError> Serializer::JSONParser::parseObject() {
        advance();
        JSONValue object = JSONValue::Objects{};

        skipWhitespace();
        if (peek() == '}') {
            advance();
            return object;
        }

        while (true) {
            skipWhitespace();
            std::expected<std::string, Serializer::ParseError> key = parseRawString();
            if (!key) {
                return std::unexpected<ParseError>(key.error());
            }

            skipWhitespace();
            if (peek() != ':') {
                return error("Expected ':' after object key");
            }
            advance();

            std::expected<JSONValue, Serializer::ParseError> member = parseValue();
            if (!member) {
                return member;
            }
            object[key.value()] = std::move(member.value());

            skipWhitespace();
            if (peek() == ',') {
                advance();
                continue;
            }
            if (peek() == '}') {
                advance();
                return object;
            }
            return error(atEnd() ? "Unterminated object" : "Expected ':' or '}' in object");
        }
    }

    std::string Serializer::serializeJSON(const JSONValue& value, int indent) {
        std::string output;
        writeJSONValue(output, value, indent, 0);
        return output;
    }

    std::expected<JSONValue, Serializer::ParseError> Serializer::deserializeJSON(std::string_view content) {
        JSONParser parser{};
        parser.text = content;

        std::expected<JSONValue, ParseError> result = parser.parseValue();
        if (!result) {
            return result;
        }

        parser.skipWhitespace();
        if (!parser.atEnd()) {
            return parser.error("Unexpected characters after JSON value");
        }
        return result;
    }

    void Serializer::writeNewLine(std::string& out, int indent, int depth) {
        if (indent > 0) {
            out += '\n';
            out.append(static_cast<size_t>(indent * depth), ' ');
        }
    }

    void Serializer::writeJSONValue(std::string& out, const JSONValue& value, int indent, int depth) {
        value.visit(Overloaded{[&](std::nullptr_t) { out += "null"; }, [&](bool b) { out += b ? "true" : "false"; },
                               [&](int64_t i) { out += std::to_string(i); }, [&](double d) { writeJSONDouble(out, d); },
                               [&](const std::string& s) { writeJSONString(out, s); },
                               [&](const JSONValue::Array& array) { writeJSONArray(out, array, indent, depth); },
                               [&](const JSONValue::Objects& objects) { writeJSONObject(out, objects, indent, depth); }});
    }

    void Serializer::writeJSONDouble(std::string& out, double d) {
        if (!std::isfinite(d)) {
            AX_CORE_LOG_WARN("Cannot write NaN or infinity to JSON");
            out += "null";
            return;
        }

        const size_t bufferSize = 32;
        char buffer[bufferSize]{};
        std::to_chars_result result = std::to_chars(buffer, buffer + bufferSize, d);
        if (result.ec != std::errc()) {
            AX_CORE_LOG_ERROR("Failed to format double: {}", std::make_error_code(result.ec).message());
            out += "null";
            return;
        }

        std::string_view text(buffer, result.ptr - buffer);
        out += text;
        if (text.find_first_of(".eE") == std::string_view::npos) {
            out += ".0";
        }
    }

    void Serializer::writeJSONString(std::string& out, std::string_view s) {
        out += '"';

        for (char c : s) {
            switch (c) {
            case '"':
                out += R"(\")";
                break;
            case '\\':
                out += R"(\\)";
                break;
            case '\n':
                out += R"(\n)";
                break;
            case '\r':
                out += R"(\r)";
                break;
            case '\t':
                out += R"(\t)";
                break;
            case '\b':
                out += R"(\b)";
                break;
            case '\f':
                out += R"(\f)";
                break;
            default: {
                const auto byte = static_cast<unsigned char>(c);
                if (byte < 0x20) {
                    out += std::format(R"(\u{:04x})", byte);
                } else {
                    out += c;
                }
            }
            }
        }

        out += '"';
    }

    void Serializer::writeJSONArray(std::string& out, const JSONValue::Array& array, int indent, int depth) {
        if (array.empty()) {
            out += "[]";
            return;
        }
        out += "[";
        for (size_t i = 0; i < array.size(); i++) {
            if (i > 0) {
                out += ",";
            }
            writeNewLine(out, indent, depth + 1);
            writeJSONValue(out, array[i], indent, depth + 1);
        }
        writeNewLine(out, indent, depth);
        out += "]";
    }

    void Serializer::writeJSONObject(std::string& out, const JSONValue::Objects& objects, int indent, int depth) {
        if (objects.empty()) {
            out += "{}";
            return;
        }
        out += "{";
        for (size_t i = 0; i < objects.size(); i++) {
            if (i > 0) {
                out += ",";
            }
            writeNewLine(out, indent, depth + 1);
            writeJSONString(out, objects[i].first);
            out += ": ";
            writeJSONValue(out, objects[i].second, indent, depth + 1);
        }
        writeNewLine(out, indent, depth);
        out += "}";
    }

} // namespace Axiom
