#include "axpch.h"

#include "BinaryReader.h"

#include "Utils/FileSystem.h"

#include <cstring>

namespace Axiom {
    BinaryReader::BinaryReader(const std::filesystem::path& filePath, bool bigEndian) : isBigEndian(bigEndian) {
        buffer = FileSystem::readFile(filePath);
    }

    void BinaryReader::skip(size_t bytes) {
        cursor += bytes;
    }

    void BinaryReader::seek(size_t position) {
        cursor = position;
    }

    size_t BinaryReader::tell() {
        return cursor;
    }

    uint8_t BinaryReader::readUInt8() {
        checkBounds(1);
        return buffer[cursor++];
    }

    uint16_t BinaryReader::readUInt16() {
        checkBounds(2);
        uint16_t value;
        std::memcpy(&value, &buffer[cursor], 2);
        cursor += 2;
        return isBigEndian ? swap16(value) : value;
    }

    uint32_t BinaryReader::readUInt32() {
        checkBounds(4);
        uint32_t value;
        std::memcpy(&value, &buffer[cursor], 4);
        cursor += 4;
        return isBigEndian ? swap32(value) : value;
    }

    uint64_t BinaryReader::readUInt64() {
        checkBounds(8);
        uint64_t value;
        std::memcpy(&value, &buffer[cursor], 8);
        cursor += 8;
        return isBigEndian ? swap64(value) : value;
    }

    int8_t BinaryReader::readInt8() {
        return static_cast<int8_t>(readUInt8());
    }

    int16_t BinaryReader::readInt16() {
        return static_cast<int16_t>(readUInt16());
    }

    int32_t BinaryReader::readInt32() {
        return static_cast<int32_t>(readUInt32());
    }

    int64_t BinaryReader::readInt64() {
        return static_cast<int64_t>(readUInt64());
    }

    std::string BinaryReader::readString(size_t length) {
        checkBounds(length);
        std::string str(reinterpret_cast<const char*>(&buffer[cursor]), length);
        cursor += length;
        return str;
    }

    uint16_t BinaryReader::swap16(uint16_t value) const {
        return (value >> 8) | (value << 8);
    }

    uint32_t BinaryReader::swap32(uint32_t value) const {
        return ((value >> 24) & 0x000000FF) | ((value >> 8) & 0x0000FF00) | ((value << 8) & 0x00FF0000) | ((value << 24) & 0xFF000000);
    }

    uint64_t BinaryReader::swap64(uint64_t value) const {
        return ((value >> 56) & 0x00000000000000FF) | ((value >> 40) & 0x000000000000FF00) | ((value >> 24) & 0x0000000000FF0000) |
               ((value >> 8) & 0x00000000FF000000) | ((value << 8) & 0x000000FF00000000) | ((value << 24) & 0x0000FF0000000000) |
               ((value << 40) & 0x00FF000000000000) | ((value << 56) & 0xFF00000000000000);
    }
} // namespace Axiom
