#include "Font.h"
#include "MSDFGen/msdfgen.h"

namespace Axiom {
    static msdfgen::Shape createMSDFGenShape(const std::vector<Font::Contour> &contours) {
        msdfgen::Shape shape;

        for (const auto &contour : contours) {
            const auto &pts = contour.points;
            if (pts.size() < 3)
                continue;

            msdfgen::Contour &msdfContour = shape.addContour();

            auto it = pts.begin();
            while (it != pts.end()) {
                auto p0It = it;
                auto p1It = getNextWrapped(p0It, pts.end(), pts.begin());
                auto p2It = getNextWrapped(p1It, pts.end(), pts.begin());

                msdfgen::Point2 mp0(p0It->x(), p0It->y());
                msdfgen::Point2 mp1(p1It->x(), p1It->y());
                msdfgen::Point2 mp2(p2It->x(), p2It->y());

                float expectedMidX = (mp0.x + mp2.x) * 0.5f;
                float expectedMidY = (mp0.y + mp2.y) * 0.5f;

                float dx = mp1.x - expectedMidX;
                float dy = mp1.y - expectedMidY;

                if ((dx * dx + dy * dy) < 0.001f) {
                    msdfContour.addEdge(new msdfgen::LinearSegment(mp0, mp2));
                } else {
                    msdfContour.addEdge(new msdfgen::QuadraticSegment(mp0, mp1, mp2));
                }

                it++;
                if (it != pts.end()) {
                    it++;
                }
            }
        }

        shape.normalize();
        shape.orientContours();

        msdfgen::edgeColoringByDistance(shape, 3.0);

        return shape;
    }

    Font::Font(const std::filesystem::path &filePath) {
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

        reader.seek(tableOffsets["glyf"]);
        notFoundGlyph = readSimpleGlyph(reader);

        reader.seek(tableOffsets["maxp"] + 4);
        uint16_t numGlyphs = reader.readUInt16();
        reader.seek(tableOffsets["head"] + 18);
        uint16_t unitsPerEm = reader.readUInt16();
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

        reader.seek(tableOffsets["hhea"] + 34);
        uint16_t numOfLongHorMetrics = reader.readUInt16();
        uint32_t hmtxOffset = tableOffsets["hmtx"];
        auto getGlyphAdvance = [&](uint32_t glyphIndex) -> uint16_t {
            if (glyphIndex < numOfLongHorMetrics) {
                reader.seek(hmtxOffset + glyphIndex * 4);
                return reader.readUInt16();
            } else {
                reader.seek(hmtxOffset + (numOfLongHorMetrics - 1) * 4);
                return reader.readUInt16();
            }
        };

        uint32_t glyphSize = 64;
        uint16_t collumns = 16;
        uint16_t rows = 6; // 16 * 6 = 96 glyphs, which is enough for basic ASCII

        asciiAtlas = {.width = collumns * glyphSize, .height = rows * glyphSize, .channels = 4, .unitsPerEm = unitsPerEm};
        asciiAtlas.pixels.resize(asciiAtlas.width * asciiAtlas.height * asciiAtlas.channels);

        double pxRange = 4.0;
        for (uint32_t charCode = 32; charCode < 126; charCode++) {
            uint32_t glyphIndex = charCodeToGlyphIndex[charCode];

            uint32_t gridX = (charCode - 32) % collumns;
            uint32_t gridY = (charCode - 32) / collumns;
            uint32_t startPxX = gridX * glyphSize;
            uint32_t startPxY = gridY * glyphSize;

            GlyphMetrics metrics = {
                .uv0 = Math::Vec2(static_cast<float>(startPxX) / asciiAtlas.width, static_cast<float>(startPxY) / asciiAtlas.height),
                .uv1 = Math::Vec2(static_cast<float>(startPxX + glyphSize) / asciiAtlas.width, static_cast<float>(startPxY + glyphSize) / asciiAtlas.height),
                .quadMin = Math::Vec2(0.0f, 0.0f),
                .quadMax = Math::Vec2(0.0f, 0.0f),
                .advance = static_cast<float>(getGlyphAdvance(glyphIndex))};

            if (glyphIndex == 0 && charCode != 32) {
                asciiAtlas.glyphs[charCode] = metrics;
                continue;
            }

            uint32_t currentGlyphOffset = glyphLocations[glyphIndex];
            uint32_t nextGlyphOffset = glyphLocations[glyphIndex + 1];

            if (currentGlyphOffset == nextGlyphOffset) {
                asciiAtlas.glyphs[charCode] = metrics;
                continue;
            }

            reader.seek(currentGlyphOffset);
            Glyph glyph = readSimpleGlyph(reader);

            auto contours = createRawGlyphContours(glyph, 1.0f);
            if (contours.empty()) {
                asciiAtlas.glyphs[charCode] = metrics;
                continue;
            }

            msdfgen::Shape shape = createMSDFGenShape(contours);
            msdfgen::Shape::Bounds bounds = shape.getBounds();
            msdfgen::Bitmap<float, 3> localMsdf(glyphSize, glyphSize);

            double scale = std::min<double>((glyphSize - 2.0 * pxRange) / (bounds.r - bounds.l), (glyphSize - 2.0 * pxRange) / (bounds.t - bounds.b));

            msdfgen::Vector2 translate(0.5 * glyphSize / scale - 0.5 * (bounds.l + bounds.r), 0.5 * glyphSize / scale - 0.5 * (bounds.b + bounds.t));

            msdfgen::Projection projection(scale, translate);
            msdfgen::MSDFGeneratorConfig config;
            config.overlapSupport = true;
            config.errorCorrection.mode = msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY;
            msdfgen::generateMSDF(localMsdf, shape, projection, pxRange / scale, config);

            for (int y = 0; y < glyphSize; y++) {
                int flippedY = (glyphSize - 1) - y;
                for (int x = 0; x < glyphSize; x++) {
                    float r = localMsdf(x, y)[0];
                    float g = localMsdf(x, y)[1];
                    float b = localMsdf(x, y)[2];

                    uint8_t rByte = (uint8_t)std::max(0.0f, std::min(255.0f, r * 255.0f));
                    uint8_t gByte = (uint8_t)std::max(0.0f, std::min(255.0f, g * 255.0f));
                    uint8_t bByte = (uint8_t)std::max(0.0f, std::min(255.0f, b * 255.0f));
                    uint8_t aByte = 255; // padding

                    int atlasPxX = startPxX + x;
                    int atlasPxY = startPxY + flippedY;
                    int index = (atlasPxY * asciiAtlas.width + atlasPxX) * asciiAtlas.channels;

                    asciiAtlas.pixels[index + 0] = rByte;
                    asciiAtlas.pixels[index + 1] = gByte;
                    asciiAtlas.pixels[index + 2] = bByte;
                    asciiAtlas.pixels[index + 3] = aByte;
                }
            }

            float quadMinX = static_cast<float>(-translate.x);
            float quadMinY = static_cast<float>(-translate.y);

            float quadMaxX = quadMinX + static_cast<float>(glyphSize / scale);
            float quadMaxY = quadMinY + static_cast<float>(glyphSize / scale);

            metrics.quadMin = Math::Vec2(quadMinX, quadMinY);
            metrics.quadMax = Math::Vec2(quadMaxX, quadMaxY);

            asciiAtlas.glyphs[charCode] = metrics;
        }
    }

    Font::Glyph Font::readSimpleGlyph(BinaryReader &reader) const {
        int16_t numberOfContours = reader.readInt16();
        if (numberOfContours <= 0) {
            AX_CORE_LOG_WARN("Glyph with {} contours is not supported", numberOfContours);
            return notFoundGlyph;
        }

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
        Math::iVec2 boundsMin = Math::iVec2(xMin, yMin);
        Math::iVec2 boundsMax = Math::iVec2(xMax, yMax);
        bool hasOverlaps = BinaryReader::isFlagBitSet(pointsFlags[0], 6);

        return Glyph{boundsMax, boundsMin, std::move(points), std::move(contourEndPoints), hasOverlaps};
    }

    std::vector<Font::Point> Font::readPoints(BinaryReader &reader, const std::vector<uint8_t> &pointsFlags) const {
        std::vector<Point> points(pointsFlags.size());

        // 1. Read all X coordinates using a running sum
        int32_t currentX = 0;
        for (size_t i = 0; i < pointsFlags.size(); i++) {
            uint8_t flags = pointsFlags[i];
            points[i].onCurve = BinaryReader::isFlagBitSet(flags, 0);

            if (BinaryReader::isFlagBitSet(flags, 1)) { // X_SHORT
                uint8_t offset = reader.readUInt8();
                currentX += BinaryReader::isFlagBitSet(flags, 4) ? offset : -offset;
            } else if (!BinaryReader::isFlagBitSet(flags, 4)) { // NOT X_SAME
                currentX += reader.readInt16();
            }
            points[i].position.x() = currentX;
        }

        // 2. Read all Y coordinates using a running sum
        int32_t currentY = 0;
        for (size_t i = 0; i < pointsFlags.size(); i++) {
            uint8_t flags = pointsFlags[i];

            if (BinaryReader::isFlagBitSet(flags, 2)) { // Y_SHORT
                uint8_t offset = reader.readUInt8();
                currentY += BinaryReader::isFlagBitSet(flags, 5) ? offset : -offset;
            } else if (!BinaryReader::isFlagBitSet(flags, 5)) { // NOT Y_SAME
                currentY += reader.readInt16();
            }
            points[i].position.y() = currentY;
        }

        return points;
    }

    std::unordered_map<uint32_t, uint32_t> Font::createUnicodeToGlyphIndexMap(BinaryReader &reader) const {
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

                if (unicodeEncodingID == 3 && cmapSubtableOffset == std::numeric_limits<uint32_t>::max()) {
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
                idRangeOffsets[i].second = reader.tell();
                idRangeOffsets[i].first = reader.readUInt16();
            }

            for (size_t i = 0; i < startCodes.size(); i++) {
                uint32_t endCode = endCodes[i];
                uint32_t currCode = startCodes[i];

                while (currCode <= endCode) {
                    uint32_t glyphIndex = 0;
                    if (idRangeOffsets[i].first == 0) {
                        glyphIndex = (currCode + idDeltas[i]) % 65536;
                    } else {
                        uint32_t offset = idRangeOffsets[i].second + 2 * (currCode - startCodes[i]) + idRangeOffsets[i].first;
                        size_t currentPos = reader.tell();
                        reader.seek(offset);
                        uint16_t glyphIndexOffset = reader.readUInt16();
                        reader.seek(currentPos);

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

    Math::Vec2 Font::bezierInterpolation(Math::Vec2 p0, Math::Vec2 p1, Math::Vec2 p2, float t) const {
        Math::Vec2 interA = Math::linearInterpolation(p0, p1, t);
        Math::Vec2 interB = Math::linearInterpolation(p1, p2, t);
        return Math::linearInterpolation(interA, interB, t);
    }

    std::vector<Font::Contour> Font::createRawGlyphContours(const Glyph &glyph, float scale) const {
        std::vector<Contour> contours;
        uint32_t contourStart = 0;

        for (uint16_t contourEnd : glyph.contourEndPoints) {
            std::vector<Point> originalContour(glyph.points.begin() + contourStart, glyph.points.begin() + contourEnd + 1);
            uint32_t pointOffset;
            for (pointOffset = 0; pointOffset < originalContour.size(); pointOffset++) {
                if (originalContour[pointOffset].onCurve) {
                    break;
                }
            }

            Contour newContour;
            for (size_t i = 0; i < originalContour.size(); i++) {
                const Point &currPoint = originalContour[(pointOffset + i + 0) % originalContour.size()];
                const Point &nextPoint = originalContour[(pointOffset + i + 1) % originalContour.size()];
                newContour.points.push_back(Math::Vec2(currPoint.position) * scale);

                if (currPoint.onCurve == nextPoint.onCurve) {
                    Math::Vec2 midPoint = Math::Vec2(currPoint.position + nextPoint.position) * 0.5f * scale;
                    newContour.points.push_back(midPoint);
                }
            }
            contours.push_back(std::move(newContour));
            contourStart = contourEnd + 1;
        }

        return contours;
    }
} // namespace Axiom
