#include "axpch.h"

#include "UIScrollBox.h"

#include "Event/MouseEvent.h"
#include "Math/AxMath.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UISlot.h"

#include <algorithm>

namespace Axiom {
    Math::Vec2 UIScrollBox::measure(const UIContext& context) {
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

    void UIScrollBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);
        const Math::Rect content = getContentRect();
        const float contentHeight = desiredSize.y() - padding.top - padding.bottom;
        scrollOffset.y() = std::clamp(scrollOffset.y(), 0.0f, std::max(0.0f, contentHeight - content.height()));

        float y = content.y() - scrollOffset.y();
        for (auto& slot : slots) {
            const float stripHeight = slot.getOuterDesiredSize().y();
            const Math::Rect strip(Math::Vec2(content.x(), y), Math::Vec2(content.width(), stripHeight));

            const Math::Rect rect = slot.alignInArea(strip);
            slot.content->arrange(context, rect.getPos(), rect.getSize());
            y += stripHeight;
        }
    }

    void UIScrollBox::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Math::Rect contentRect(arrangedPosition, arrangedSize);
        Math::Rect visibleRect = contentRect.getIntersection(scissorRect);

        if (visibleRect.width() <= 0.0f || visibleRect.height() < 0.0f) {
            return;
        }

        context.renderer->pushScissorRect(visibleRect, context.layer);
        UIContainer::onRender(context, visibleRect);
        context.renderer->popScissorRect(context.layer);
    }

    bool UIScrollBox::onScroll(const MouseScrolledEvent& event) {
        scrollOffset.y() -= event.getYOffset() * 10.0f;
        invalidateLayout();
        return true;
    }
} // namespace Axiom
