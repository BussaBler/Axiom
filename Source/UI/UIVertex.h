#pragma once
#include "Math/AxMath.h"
#include "axpch.h"

namespace Axiom {
    class UIVertex {
      public:
        UIVertex() = default;
        UIVertex(const Math::Vec2& pos, const Math::Vec2& uvCoords, const Math::Vec4& col, const Math::Vec4& params,
                 const Math::Vec4& radii = Math::Vec4::zero())
            : position(pos), uv(uvCoords), color(col), params(params), radii(radii) {}

      public:
        Math::Vec2 position;
        Math::Vec2 uv;
        Math::Vec4 color;
        Math::Vec4 params; // custom params
        Math::Vec4 radii;
    };
} // namespace Axiom
