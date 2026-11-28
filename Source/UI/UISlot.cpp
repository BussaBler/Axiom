#include "axpch.h"

#include "UI/UISlot.h"

#include "Math/AxMath.h"
#include "Math/Vec.h"

#include <cmath>

namespace Axiom {
    namespace {
        struct AxisPlacement {
            float position;
            float size;
        };

        AxisPlacement alignAxis(UIAlignment alignment, bool hasFixedSize, float areaPos, float areaSize, float contentSize) {
            if (alignment == UIAlignment::Fill && !hasFixedSize) {
                return AxisPlacement{areaPos, areaSize};
            }

            switch (alignment) {
            case UIAlignment::Center:
                return AxisPlacement{areaPos + (areaSize - contentSize) * 0.5f, contentSize};
            case UIAlignment::End:
                return AxisPlacement{areaPos + areaSize - contentSize, contentSize};
            case UIAlignment::Fill:
            case UIAlignment::Start:
            default:
                return AxisPlacement{areaPos, contentSize};
            }
        }
    } // namespace

    Math::Vec2 UISlot::getContentSize() const {
        Math::Vec2 size = content->getDesiredSize();
        if (fixedSize.x() > 0.0f) {
            size.x() = fixedSize.x();
        }
        if (fixedSize.y() > 0.0f) {
            size.y() = fixedSize.y();
        }

        return size;
    }

    Math::Vec2 UISlot::getOuterDesiredSize() const {
        return getContentSize() + Math::Vec2(margin.left + margin.right, margin.top + margin.bottom);
    }

    Math::Rect UISlot::alignInArea(const Math::Rect& area) const {
        const Math::Vec2 innerPos = area.getPos() + Math::Vec2(margin.left, margin.top);
        const Math::Vec2 innerSize =
            Math::Vec2(std::max(0.0f, area.getSize().x() - margin.left - margin.right), std::max(0.0f, area.getSize().y() - margin.top - margin.bottom));
        const Math::Vec2 content = getContentSize();
        const AxisPlacement x = alignAxis(horizontalAlignment, fixedSize.x() > 0.0f, innerPos.x(), innerSize.x(), content.x());
        const AxisPlacement y = alignAxis(verticalAlignment, fixedSize.y() > 0.0f, innerPos.y(), innerSize.y(), content.y());

        const float left = std::round(x.position);
        const float top = std::round(y.position);
        const float right = std::round(x.position + x.size);
        const float bottom = std::round(y.position + y.size);
        return Math::Rect(Math::Vec2(left, top), Math::Vec2(right - left, bottom - top));
    }
} // namespace Axiom
