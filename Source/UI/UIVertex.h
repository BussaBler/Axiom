#pragma once
#include "Math/AxMath.h"
#include "axpch.h"

namespace Axiom {
    class UIVertex {
      public:
        UIVertex() = default;
        UIVertex(const Math::Vec2& pos, const Math::Vec2& uvCoords, const Math::Vec4& col) : position(pos), uv(uvCoords), color(col) {
        }

      public:
        Math::Vec2 position;
        Math::Vec2 uv;
        Math::Vec4 color;
    };
} // namespace Axiom
