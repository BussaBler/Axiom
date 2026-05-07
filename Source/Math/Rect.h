#include "Vec.h"

namespace Math {
    class Rect {
      public:
        Rect() = default;
        Rect(const Vec2& position, const Vec2& size) : position(position), size(size) {}

        inline Vec2 getPos() const { return position; }
        inline Vec2 getSize() const { return size; }
        inline Vec2 getCenter() const { return position + (size * 0.5f); }
        inline float x() const { return position.x(); }
        inline float y() const { return position.y(); }
        inline float width() const { return size.x(); }
        inline float height() const { return size.y(); }

        Math::Rect getIntersection(const Rect& other) const {
            float x1 = std::max(position.x(), other.position.x());
            float y1 = std::max(position.y(), other.position.y());
            float x2 = std::min(position.x() + size.x(), other.position.x() + other.size.x());
            float y2 = std::min(position.y() + size.y(), other.position.y() + other.size.y());

            if (x2 >= x1 && y2 >= y1) {
                return Rect(Vec2(x1, y1), Vec2(x2 - x1, y2 - y1));
            } else {
                return Rect(Vec2(0, 0), Vec2(0, 0)); // No intersection
            }
        }

      private:
        Vec2 position;
        Vec2 size;
    };
} // namespace Math