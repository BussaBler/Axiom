#include "axpch.h"

#include "UIHorizontalBox.h"

#include "Math/Vec.h"
#include "UI/UISlot.h"

#include <algorithm>

namespace Axiom {
    Math::Vec2 UIHorizontalBox::measure(const UIContext& context) {
        desiredSize = Math::Vec2::zero();
        for (auto& slot : slots) {
            slot.content->measure(context);
            const Math::Vec2 outer = slot.getOuterDesiredSize();
            desiredSize.x() += outer.x();
            desiredSize.y() = std::max(outer.y(), desiredSize.y());
        }
        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UIHorizontalBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);
        const Math::Rect content = getContentRect();

        float fixedWidth = 0.0f;
        float totalFill = 0.0f;
        for (const auto& slot : slots) {
            if (slot.fill > 0.0f) {
                totalFill += slot.fill;
            } else {
                fixedWidth += slot.getOuterDesiredSize().x();
            }
        }
        const float remaining = std::max(0.0f, content.width() - fixedWidth);

        float x = content.x();
        for (auto& slot : slots) {
            const float stripWidth = slot.fill > 0.0f ? remaining * (slot.fill / totalFill) : slot.getOuterDesiredSize().x();
            const Math::Rect strip(Math::Vec2(x, content.y()), Math::Vec2(stripWidth, content.height()));

            const Math::Rect rect = slot.alignInArea(strip);
            slot.content->arrange(context, rect.getPos(), rect.getSize());
            x += stripWidth;
        }
    }
} // namespace Axiom
