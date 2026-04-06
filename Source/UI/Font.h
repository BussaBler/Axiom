#pragma once
#include "Core/Assert.h"
#include "Math/AxMath.h"
#include "Utils/BinaryReader.h"
#include "Utils/ContainerUtils.h"
#include "axpch.h"

namespace Axiom {
    struct GlyphMetrics {
        Math::Vec2 uv0;
        Math::Vec2 uv1;
        Math::Vec2 quadMin;
        Math::Vec2 quadMax;
        float advance = 0.0f;
    };

    class Font {
      public:
        struct Atlas {
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t channels = 4;
            uint16_t unitsPerEm = 2048;
            std::vector<uint8_t> pixels;
            std::unordered_map<uint32_t, GlyphMetrics> glyphs;
        };

        struct Contour {
            std::list<Math::Vec2> points;
        };

      public:
        Font(const std::filesystem::path &filePath);
        ~Font() = default;

        Atlas &getAsciiAtlas() {
            return asciiAtlas;
        }

      private:
        struct Point {
            Math::iVec2 position = Math::iVec2(0);
            bool onCurve = true;
        };

        struct Glyph {
            Math::iVec2 boundsMax;
            Math::iVec2 boundsMin;
            std::vector<Point> points;
            std::vector<uint16_t> contourEndPoints;
            bool hasOverlaps = false;
        };

      private:
        Glyph readSimpleGlyph(BinaryReader &reader) const;
        std::vector<Point> readPoints(BinaryReader &reader, const std::vector<uint8_t> &pointsFlags) const;
        std::unordered_map<uint32_t, uint32_t> createUnicodeToGlyphIndexMap(BinaryReader &reader) const;

        Math::Vec2 bezierInterpolation(Math::Vec2 p0, Math::Vec2 p1, Math::Vec2 p2, float t) const;
        std::vector<Contour> createRawGlyphContours(const Glyph &glyph, float scale) const;

      private:
        Glyph notFoundGlyph;
        Atlas asciiAtlas;
    };
} // namespace Axiom
