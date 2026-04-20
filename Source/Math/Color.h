#pragma once
#include "AxMath.h"
#include "axpch.h"

namespace Axiom {
    class Color {
      public:
        Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f) : rgba(r, g, b, a) {}
        Color(const Math::Vec4& vec) : rgba(vec) {}
        ~Color() = default;

        inline float r() const { return rgba.r(); }
        inline float g() const { return rgba.g(); }
        inline float b() const { return rgba.b(); }
        inline float a() const { return rgba.a(); }

        static Color white() { return Color(); }
        static Color black() { return Color(0.0f, 0.0f, 0.0f); }
        static Color red() { return Color(1.0f, 0.0f, 0.0f); }
        static Color green() { return Color(0.0f, 1.0f, 0.0f); }
        static Color blue() { return Color(0.0f, 0.0f, 1.0f); }
        static Color magenta() { return Color(1.0f, 0.0f, 1.0f); }
        static Color cyan() { return Color(0.0f, 1.0f, 1.0f); }
        static Color yellow() { return Color(1.0f, 1.0f, 0.0f); }
        static Color transparent() { return Color(0.0f, 0.0f, 0.0f, 0.0f); }
        static Color gray() { return Color(0.5f, 0.5f, 0.5f); }

        operator Math::Vec4() const { return rgba; }

      private:
        Math::Vec4 rgba;
    };
} // namespace Axiom