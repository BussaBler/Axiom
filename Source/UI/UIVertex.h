#pragma once
#include "Math/AxMath.h"

#include <cstdint>

namespace Axiom {
    enum class UIPrimitive : uint32_t { Rect = 0, Glyph = 1, Image = 2 };

    class UIVertex {
      public:
        UIVertex() = default;
        UIVertex(const Math::Vec2& pos, const Math::Vec2& uvCoords, const Math::Vec4& col, const Math::Vec4& params,
                 const Math::Vec4& radii = Math::Vec4::zero(), uint32_t texIndex = 0, UIPrimitive type = UIPrimitive::Rect)
            : position(pos), uv(uvCoords), color(col), params(params), radii(radii), textureIndex(texIndex), primitive(static_cast<uint32_t>(type)) {}

      public:
        Math::Vec2 position;
        Math::Vec2 uv;
        Math::Vec4 color;
        Math::Vec4 params;
        Math::Vec4 radii;
        uint32_t textureIndex;
        uint32_t primitive;
    };
} // namespace Axiom
