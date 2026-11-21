#pragma once

#include "Asset/UUID.h"
#include "Core/Assert.h"

#include <array>
#include <charconv>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <system_error>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

namespace Axiom {
    template <typename T>
    concept NonBoolInteger = std::integral<T> && !std::is_same_v<std::remove_cv_t<T>, bool>;

    class JSONValue {
      public:
        using Array = std::vector<JSONValue>;
        using Objects = std::vector<std::pair<std::string, JSONValue>>;

      public:
        JSONValue() = default;
        JSONValue(std::nullptr_t) {}
        JSONValue(bool v) : value(v) {}
        template <NonBoolInteger T> JSONValue(T v) : value(int64_t(v)) {}
        JSONValue(double v) : value(v) {}
        JSONValue(std::string v) : value(std::move(v)) {}
        JSONValue(const char* v) : value(std::string(v)) {}
        JSONValue(Array v) : value(std::move(v)) {}
        JSONValue(Objects v) : value(std::move(v)) {}

        template <typename T> bool is() const { return std::holds_alternative<T>(value); }
        template <typename T> T& get() {
            AX_CORE_ASSERT(is<T>(), "Cannot get JSONValue type");
            return std::get<T>(value);
        }
        template <typename T> const T& get() const {
            AX_CORE_ASSERT(is<T>(), "Cannot get JSONValue type");
            return std::get<T>(value);
        }

        JSONValue& operator[](const std::string& key) {
            if (is<std::nullptr_t>()) {
                value = Objects();
            }
            AX_CORE_ASSERT(is<Objects>(), "operator [] used on a JSON value that is not an object");

            Objects& members = std::get<Objects>(value);
            for (auto& [name, child] : members) {
                if (name == key) {
                    return child;
                }
            }
            return members.emplace_back(key, nullptr).second;
        }
        const JSONValue* find(const std::string& key) const {
            if (is<Objects>()) {
                for (const auto& [name, child] : std::get<Objects>(value)) {
                    if (key == name) {
                        return &child;
                    }
                }
            }
            return nullptr;
        }
        const JSONValue& operator[](const std::string& key) const {
            static const JSONValue nullValue;
            const JSONValue* child = find(key);
            return child ? *child : nullValue;
        }

        void append(JSONValue jsonValue) {
            if (is<std::nullptr_t>()) {
                value = Array();
            }
            AX_CORE_ASSERT(is<Array>(), "Cannot append element to a non Array JSONValue");

            std::get<Array>(value).push_back(std::move(jsonValue));
        }
        template <typename F> decltype(auto) visit(F&& f) const { return std::visit(std::forward<F>(f), value); }

        // helper functions
        std::optional<double> number() const {
            if (is<double>()) {
                return get<double>();
            }
            if (is<int64_t>()) {
                return static_cast<double>(get<int64_t>());
            }
            return std::nullopt;
        }
        std::optional<std::string> text() const {
            if (!is<std::string>()) {
                return std::nullopt;
            }
            return get<std::string>();
        }
        std::optional<UUID> uuid() const {
            auto str = text();
            if (!str) {
                return std::nullopt;
            }
            uint64_t value = 0;
            auto [ptr, ec] = std::from_chars(str->data(), str->data() + str->size(), value);
            if (ec != std::errc() || ptr != str->data() + str->size()) {
                return std::nullopt;
            }
            return UUID(value);
        }
        template <size_t N> std::optional<std::array<float, N>> floats() const {
            if (!is<JSONValue::Array>()) {
                return std::nullopt;
            }
            const auto& array = get<JSONValue::Array>();
            if (array.size() != N) {
                return std::nullopt;
            }

            std::array<float, N> result{};
            for (size_t i = 0; i < N; i++) {
                auto n = array[i].number();
                if (!n) {
                    return std::nullopt;
                }
                result[i] = static_cast<float>(n.value());
            }
            return result;
        }

      private:
        std::variant<std::nullptr_t, bool, int64_t, double, std::string, Array, Objects> value;
    };
} // namespace Axiom
