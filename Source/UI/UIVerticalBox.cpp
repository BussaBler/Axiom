#include "axpch.h"

#include "UI/UIVerticalBox.h"

#include "Math/Vec.h"

namespace Axiom {
    Math::Vec2 UIVerticalBox::measure(const UIContext& context) {
        desiredSize = Math::Vec2::zero();
        for (auto& slot : slots) {
            slot.content->measure(context);
            const Math::Vec2 outer = slot.getOuterDesiredSize();
            desiredSize.x() = std::max(desiredSize.x(), outer.x());
            desiredSize.y() += outer.y();
        }
        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UIVerticalBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);
        const Math::Rect content = getContentRect();

        float fixedHeight = 0.0f;
        float totalFill = 0.0f;
        for (const auto& slot : slots) {
            if (slot.fill > 0.0f) {
                totalFill += slot.fill;
            } else {
                fixedHeight += slot.getOuterDesiredSize().y();
            }
        }
        const float remaining = std::max(0.0f, content.height() - fixedHeight);

        float y = content.y();
        for (auto& slot : slots) {
            const float stripHeight = slot.fill > 0.0f ? remaining * (slot.fill / totalFill) : slot.getOuterDesiredSize().y();
            const Math::Rect strip(Math::Vec2(content.x(), y), Math::Vec2(content.width(), stripHeight));

            const Math::Rect rect = slot.alignInArea(strip);
            slot.content->arrange(context, rect.getPos(), rect.getSize());
            y += stripHeight;
        }
    }
} // namespace Axiom
