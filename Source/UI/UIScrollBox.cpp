#include "axpch.h"

#include "UIScrollBox.h"

#include "Event/Event.h"
#include "Event/MouseEvent.h"
#include "Math/AxMath.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UISlot.h"

#include <algorithm>

namespace Axiom {
    Math::Vec2 UIScrollBox::getDesiredSize(const UIContext& context) {
        for (const auto& slot : slots) {
            Math::Vec2 slotSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.x() > 0) {
                slotSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotSize.y() = slot.fixedSize.y();
            }

            slotSize.x() += slot.margin.left + slot.margin.right;
            slotSize.y() += slot.margin.top + slot.margin.bottom;

            desiredSize.x() = std::max(desiredSize.x(), slotSize.x());
            desiredSize.y() += slotSize.y();
        }

        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UIScrollBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIContainer::arrange(context, position, size);

        Math::Vec2 start = arrangedPosition + Math::Vec2(padding.left, padding.top);
        Math::Vec2 availableSpace = arrangedSize - Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);

        float totalContentHeight = 0.0f;
        for (const auto& slot : slots) {
            Math::Vec2 slotSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.y() > 0) {
                slotSize.y() = slot.fixedSize.y();
            }
            totalContentHeight += slotSize.y() + slot.margin.top + slot.margin.bottom;
        }

        float maxScroll = std::max(0.0f, totalContentHeight - availableSpace.y());
        scrollOffset.y() = std::clamp(scrollOffset.y(), 0.0f, maxScroll);

        float currentY = start.y() - scrollOffset.y();
        for (const auto& slot : slots) {
            currentY += slot.margin.top;

            Math::Vec2 slotDesiredSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.x() > 0)
                slotDesiredSize.x() = slot.fixedSize.x();
            if (slot.fixedSize.y() > 0)
                slotDesiredSize.y() = slot.fixedSize.y();

            float finalWidth = availableSpace.x();
            float finalX = start.x();

            switch (slot.horizontalAlignment) {
            case UIAlignment::Fill:
                finalWidth = availableSpace.x() - slot.margin.left - slot.margin.right;
                finalX = start.x() + slot.margin.left;
                break;
            case UIAlignment::Start:
                finalWidth = slotDesiredSize.x();
                finalX = start.x() + slot.margin.left;
                break;
            case UIAlignment::Center:
                finalWidth = slotDesiredSize.x();
                finalX = start.x() + (availableSpace.x() - finalWidth) / 2.0f;
                break;
            case UIAlignment::End:
                finalWidth = slotDesiredSize.x();
                finalX = start.x() + availableSpace.x() - finalWidth - slot.margin.right;
                break;
            }
            slot.content->arrange(context, Math::Vec2(finalX, currentY), Math::Vec2(finalWidth, slotDesiredSize.y()));

            currentY += slotDesiredSize.y() + slot.margin.bottom;
        }
    }

    void UIScrollBox::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Math::Rect contentRect(arrangedPosition, arrangedSize);
        Math::Rect visibleRect = contentRect.getIntersection(scissorRect);

        if (visibleRect.width() < 0.0f || visibleRect.height() < 0.0f) {
            return;
        }

        context.renderer->pushScissorRect(visibleRect, context.layer);
        UIContainer::onRender(context, visibleRect);
        context.renderer->popScissorRect(context.layer);
    }

    bool UIScrollBox::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseScrolledEvent>([this](const MouseScrolledEvent& e) {
            if (isHovered) {
                scrollOffset.y() -= e.getYOffset() * 10.0f;
                invalidateLayout();

                return true;
            }
            return false;
        });

        return UIContainer::onEvent(event);
    }
} // namespace Axiom
