#pragma once
#include "Core/Assert.h"
#include "axpch.h"

namespace Axiom {
    class BinaryReader {
      public:
        BinaryReader(const std::filesystem::path& filePath, bool bigEndian = false);
        ~BinaryReader() = default;

        void skip(size_t bytes);
        void seek(size_t position);
        size_t tell();

        uint8_t readUInt8();
        uint16_t readUInt16();
        uint32_t readUInt32();
        uint64_t readUInt64();
        int8_t readInt8();
        int16_t readInt16();
        int32_t readInt32();
        int64_t readInt64();

        std::string readString(size_t length);

        inline static bool isFlagBitSet(uint16_t flag, uint16_t index) {
            return (flag & (1 << index)) != 0;
        }

      private:
        std::fstream fileStream;
        bool isBigEndian;
        uint16_t swap16(uint16_t value) const;
        uint32_t swap32(uint32_t value) const;
        uint64_t swap64(uint64_t value) const;
    };
} // namespace Axiom