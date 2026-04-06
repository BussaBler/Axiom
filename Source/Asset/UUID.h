#pragma once
#include "axpch.h"

namespace Axiom {
    class UUID {
      public:
        UUID() : value(distribution(generator)) {
        }
        UUID(uint64_t value) : value(value) {
        }
        ~UUID() = default;

        operator uint64_t() const {
            return value;
        }

      private:
        static std::random_device randomDevice;
        static std::mt19937_64 generator;
        static std::uniform_int_distribution<uint64_t> distribution;

        uint64_t value;
    };
} // namespace Axiom

namespace std {
    template <> struct hash<Axiom::UUID> {
        size_t operator()(const Axiom::UUID& uuid) const {
            return hash<uint64_t>()(static_cast<uint64_t>(uuid));
        }
    };
} // namespace std