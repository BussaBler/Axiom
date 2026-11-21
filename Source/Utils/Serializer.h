#pragma once

#include "Utils/JSONValue.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <string>
#include <string_view>
#include <utility>

namespace Axiom {
    template <typename... Fs> struct Overloaded : Fs... {
        using Fs::operator()...;
    };

    class Serializer {
      public:
        struct ParseError {
            std::string error;
            size_t line = 1;
            size_t column = 1;
        };

      private:
        template <typename T> struct Parser {
            std::string_view text;
            size_t pos;
            size_t line = 1;
            size_t column = 1;

            bool atEnd() const { return pos >= text.size(); }
            char peek() const { return atEnd() ? '\0' : text[pos]; }

            char advance() {
                char c = text[pos++];
                if (c == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                return c;
            }
            void skipWhitespace() {
                while (!atEnd() && (peek() == ' ' || peek() == '\t' || peek() == '\n' || peek() == '\r')) {
                    advance();
                }
            }

            std::unexpected<ParseError> error(std::string message) const { return std::unexpected<ParseError>(ParseError{std::move(message), line, column}); }

            virtual std::expected<T, ParseError> parseValue() = 0;
            virtual std::expected<T, ParseError> parseLiteral(std::string_view word, T result) = 0;
            virtual std::expected<T, ParseError> parseNumber() = 0;
            virtual std::expected<T, ParseError> parseString() = 0;
            virtual std::expected<T, ParseError> parseArray() = 0;
            virtual std::expected<T, ParseError> parseObject() = 0;
        };

        struct JSONParser : Parser<JSONValue> {
            std::expected<JSONValue, ParseError> parseValue() override;
            std::expected<JSONValue, ParseError> parseLiteral(std::string_view word, JSONValue result) override;
            std::expected<JSONValue, ParseError> parseNumber() override;
            std::expected<JSONValue, ParseError> parseString() override;
            std::expected<JSONValue, ParseError> parseArray() override;
            std::expected<JSONValue, ParseError> parseObject() override;
            std::expected<std::string, ParseError> parseRawString();
            std::expected<uint32_t, ParseError> parseHex();
        };

      public:
        static std::string serializeJSON(const JSONValue& value, int ident = 2);
        static std::expected<JSONValue, ParseError> deserializeJSON(std::string_view content);

      private:
        static void writeNewLine(std::string& out, int indent, int depth);

        static void writeJSONValue(std::string& out, const JSONValue& value, int indent, int depth);
        static void writeJSONDouble(std::string& out, double d);
        static void writeJSONString(std::string& out, std::string_view s);
        static void writeJSONArray(std::string& out, const JSONValue::Array& array, int indent, int depth);
        static void writeJSONObject(std::string& out, const JSONValue::Objects& objects, int indent, int depth);
    };
} // namespace Axiom

template <> struct std::formatter<Axiom::Serializer::ParseError> : std::formatter<std::string_view> {
    auto format(const Axiom::Serializer::ParseError& error, std::format_context& ctx) const {
        return std::formatter<std::string_view>::format(std::format("Error {} at line {} column {}", error.error, error.line, error.column), ctx);
    }
};
