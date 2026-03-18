#pragma once
#include "axpch.h"
#include "Core/Assert.h"
#include "Math/AxMath.h"
#include "Utils/BinaryReader.h"

namespace Axiom {
    struct Point {
        Math::iVec2 position;
        bool onCurve;
    };

    struct Glyph {
        Math::iVec2 boundsMax;
        Math::iVec2 boundsMin;
        std::vector<Point> points;
        std::vector<uint16_t> contourEndPoints;
    };

    class Font {
    public:
        Font(const std::filesystem::path& filePath);
        ~Font() = default;

    private:
        Glyph readSimpleGlyph(BinaryReader& reader) const;
        std::vector<Point> readPoints(BinaryReader& reader, const std::vector<uint8_t>& pointsFlags) const;
        std::unordered_map<uint32_t, uint32_t> createUnicodeToGlyphIndexMap(BinaryReader& reader);

    private:
        Glyph notFoundGlyph;
        std::unordered_map<uint32_t, Glyph> glyphs;
    };
} // namespace Axiom
