#include "BinaryReader.h"

namespace Axiom {
    BinaryReader::BinaryReader(const std::filesystem::path& filePath, bool bigEndian) : isBigEndian(bigEndian) {
        fileStream.open(filePath, std::ios::binary | std::ios::in);
        AX_CORE_ASSERT(fileStream.is_open(), "Failed to open file: {0}", filePath.string());
    }

    void BinaryReader::skip(size_t bytes) {
        fileStream.seekg(bytes, std::ios::cur);
    }

    void BinaryReader::seek(size_t position) {
        fileStream.seekg(position, std::ios::beg);
    }

    size_t BinaryReader::tell() {
        return static_cast<size_t>(fileStream.tellg());
    }

    uint8_t BinaryReader::readUInt8() {
        uint8_t value;
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return value;
    }

    uint16_t BinaryReader::readUInt16() {
        uint16_t value;
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return isBigEndian ? swap16(value) : value;
    }

    uint32_t BinaryReader::readUInt32() {
        uint32_t value;
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(value));
        return isBigEndian ? swap32(value) : value;
    }

    uint64_t BinaryReader::readUInt64() {
        uint64_t value;
        fileStream.read(reinterpret_cast<char*>(&value), sizeof(value));
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
        std::string str(length, '\0');
        fileStream.read(&str[0], length);
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
