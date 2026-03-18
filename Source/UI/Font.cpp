#include "Font.h"

namespace Axiom {
    Font::Font(const std::filesystem::path& filePath) {
        BinaryReader reader(filePath, true);

        reader.skip(4);
        uint16_t numTables = reader.readUInt16();
        reader.skip(6);

        // tables offsets relative to the beginning of the file indexed by their 4-character tag
        std::unordered_map<std::string, uint32_t> tableOffsets;

        for (uint16_t i = 0; i < numTables; i++) {
            std::string tag = reader.readString(4);
            reader.skip(4);
            uint32_t offset = reader.readUInt32();
            reader.skip(4);
            tableOffsets[tag] = offset;
        }

        reader.seek(tableOffsets["glyph"]);
        notFoundGlyph = readSimpleGlyph(reader);

        reader.seek(tableOffsets["maxp"] + 4);
        uint16_t numGlyphs = reader.readUInt16();
        reader.seek(tableOffsets["head"] + 50);
        bool isTwoByteEntry = reader.readInt16() == 0;

        uint32_t locationTableOffset = tableOffsets["loca"];
        uint32_t glyphTableOffset = tableOffsets["glyf"];
        std::vector<uint32_t> glyphLocations(numGlyphs + 1);

        for (uint16_t i = 0; i <= numGlyphs; i++) {
            reader.seek(locationTableOffset + i * (isTwoByteEntry ? 2 : 4));
            uint32_t glyphOffset = isTwoByteEntry ? reader.readUInt16() * 2 : reader.readUInt32();
            glyphLocations[i] = glyphTableOffset + glyphOffset;
        }

        reader.seek(tableOffsets["cmap"]);
        auto charCodeToGlyphIndex = createUnicodeToGlyphIndexMap(reader);
    }

    Glyph Font::readSimpleGlyph(BinaryReader& reader) const {
        int16_t numberOfContours = reader.readInt16();
        AX_CORE_ASSERT(numberOfContours > 0, "Only simple glyphs are supported");

        int16_t xMin = reader.readInt16();
        int16_t yMin = reader.readInt16();
        int16_t xMax = reader.readInt16();
        int16_t yMax = reader.readInt16();

        std::vector<uint16_t> contourEndPoints(numberOfContours);
        for (int i = 0; i < numberOfContours; i++) {
            contourEndPoints[i] = reader.readUInt16();
        }
        reader.skip(reader.readUInt16());

        uint16_t numPoints = contourEndPoints.back() + 1;
        std::vector<uint8_t> pointsFlags(numPoints);
        for (uint16_t i = 0; i < numPoints; i++) {
            uint8_t flags = reader.readUInt8();
            pointsFlags[i] = flags;
            if (BinaryReader::isFlagBitSet(flags, 3)) {
                uint8_t repeatCount = reader.readUInt8();
                for (uint8_t j = 0; j < repeatCount; j++) {
                    pointsFlags[++i] = flags;
                }
            }
        }

        std::vector<Point> points = readPoints(reader, pointsFlags);
    }

    std::vector<Point> Font::readPoints(BinaryReader& reader, const std::vector<uint8_t>& pointsFlags) const {
        std::vector<Point> points(pointsFlags.size());

        // first read X coords
        int offsetSizeFlagBitPos = 1;
        int offsetSignFlagBitPos = 4;

        for (size_t i = 0; i < pointsFlags.size(); i++) {
            points[i].position.x() = points[std::max<size_t>(0, i - 1)].position.x();
            uint8_t flags = pointsFlags[i];

            points[i].onCurve = BinaryReader::isFlagBitSet(flags, 0);

            if (BinaryReader::isFlagBitSet(flags, offsetSizeFlagBitPos)) {
                uint8_t offset = reader.readUInt8();
                points[i].position.x() += BinaryReader::isFlagBitSet(flags, offsetSignFlagBitPos) ? offset : -offset;
            } else if (!BinaryReader::isFlagBitSet(flags, offsetSignFlagBitPos)) {
                points[i].position.x() += reader.readInt16();
            }
        }

        offsetSizeFlagBitPos = 2;
        offsetSignFlagBitPos = 5;

        for (size_t i = 0; i < pointsFlags.size(); i++) {
            points[i].position.y() = points[std::max<size_t>(0, i - 1)].position.y();
            uint8_t flags = pointsFlags[i];

            if (BinaryReader::isFlagBitSet(flags, offsetSizeFlagBitPos)) {
                uint8_t offset = reader.readUInt8();
                points[i].position.y() += BinaryReader::isFlagBitSet(flags, offsetSignFlagBitPos) ? offset : -offset;
            } else if (!BinaryReader::isFlagBitSet(flags, offsetSignFlagBitPos)) {
                points[i].position.y() += reader.readInt16();
            }
        }

        return points;
    }

    std::unordered_map<uint32_t, uint32_t> Font::createUnicodeToGlyphIndexMap(BinaryReader& reader) {
        size_t cmapPos = reader.tell();
        std::unordered_map<uint32_t, uint32_t> charCodeToGlyphIndex;
        uint16_t cmapVersion = reader.readUInt16();
        uint16_t numSubtables = reader.readUInt16();
        uint32_t cmapSubtableOffset = std::numeric_limits<uint32_t>::max();

        for (uint16_t i = 0; i < numSubtables; i++) {
            uint16_t platformID = reader.readUInt16();
            uint16_t encodingID = reader.readUInt16();
            uint32_t subtableOffset = reader.readUInt32();

            if (platformID == 0) {
                uint16_t unicodeEncodingID = encodingID;
                if (unicodeEncodingID == 4) {
                    cmapSubtableOffset = subtableOffset;
                }

                if (unicodeEncodingID == 3 && subtableOffset == UINT32_MAX) {
                    cmapSubtableOffset = subtableOffset;
                }
            }
        }

        reader.seek(cmapPos + cmapSubtableOffset);
        uint16_t format = reader.readUInt16();

        if (format != 12 && format != 4) {
            AX_CORE_LOG_ERROR("Unsupported cmap format {}", format);
        }

        if (format == 12) {
            uint16_t reserved = reader.readUInt16();
            uint32_t subtableLength = reader.readUInt32();
            uint32_t languageCode = reader.readUInt32();
            uint32_t numGroups = reader.readUInt32();

            for (uint32_t i = 0; i < numGroups; i++) {
                uint32_t startCharCode = reader.readUInt32();
                uint32_t endCharCode = reader.readUInt32();
                uint32_t startGlyphIndex = reader.readUInt32();

                uint32_t numChars = endCharCode - startCharCode + 1;
                for (uint32_t charCodeOffset = 0; charCodeOffset < numChars; charCodeOffset++) {
                    uint32_t charCode = startCharCode + charCodeOffset;
                    uint32_t glyphIndex = startGlyphIndex + charCodeOffset;
                    charCodeToGlyphIndex[charCode] = glyphIndex;
                }
            }
        } else if (format == 4) {
            reader.skip(4); // skip length and languageCode

            uint16_t segCountX2 = reader.readUInt16();
            uint16_t segCount = segCountX2 / 2;
            reader.skip(6); // skip searchRange, entrySelector, rangeShift

            std::vector<uint16_t> endCodes(segCount);
            for (uint16_t i = 0; i < segCount; i++) {
                endCodes[i] = reader.readUInt16();
            }
            reader.skip(2); // skip reservedPad

            std::vector<uint16_t> startCodes(segCount);
            for (uint16_t i = 0; i < segCount; i++) {
                startCodes[i] = reader.readUInt16();
            }

            std::vector<int16_t> idDeltas(segCount);
            for (uint16_t i = 0; i < segCount; i++) {
                idDeltas[i] = reader.readInt16();
            }

            std::vector<std::pair<uint16_t, uint16_t>> idRangeOffsets(segCount);
            for (uint16_t i = 0; i < segCount; i++) {
                idRangeOffsets[i].first = reader.readUInt16();
                idRangeOffsets[i].second = reader.tell();
            }

            for (size_t i = 0; i < startCodes.size(); i++) {
                uint16_t endCode = endCodes[i];
                uint16_t currCode = startCodes[i];

                while (currCode <= endCode) {
                    uint32_t glyphIndex = 0;
                    if (idRangeOffsets[i].first == 0) {
                        glyphIndex = (currCode + idDeltas[i]) % 65536;
                    } else {
                        uint32_t offset =
                            idRangeOffsets[i].second + 2 * (currCode - startCodes[i]) + idRangeOffsets[i].first;
                        size_t currentPos = reader.tell();
                        reader.seek(offset);
                        uint16_t glyphIndexOffset = reader.readUInt16();

                        if (glyphIndexOffset != 0) {
                            glyphIndex = (glyphIndexOffset + idDeltas[i]) % 65536;
                        }
                    }

                    charCodeToGlyphIndex[currCode] = glyphIndex;
                    currCode++;
                }
            }
        }

        return charCodeToGlyphIndex;
    }
} // namespace Axiom
