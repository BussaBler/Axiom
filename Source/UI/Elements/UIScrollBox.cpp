#include "axpch.h"

#include "UIScrollBox.h"

namespace Axiom {
    Math::Vec2 UIScrollBox::getDesiredSize(const UIContext& context) {
        Math::Vec2 desiredSize(0.0f, 0.0f);

        for (const auto& child : children) {
            Math::Vec2 childDesiredSize = child->getDesiredSize(context);
            desiredSize.x() = std::max(desiredSize.x(), childDesiredSize.x());
            desiredSize.y() += childDesiredSize.y();
        }

        desiredSize.x() += padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() += padding.top + padding.bottom + margin.top + margin.bottom;

        return desiredSize;
    }

    void UIScrollBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float startX = arrangedPosition.x() + padding.left + margin.left;
        float startY = arrangedPosition.y() + padding.top + margin.top;

        float availableWidth = arrangedSize.x() - padding.left - padding.right;
        float availableHeight = arrangedSize.y() - padding.top - padding.bottom;

        float fixedHeightUsed = 0.0f;
        int fillChildCount = 0;

        for (const auto& child : children) {
            if (child->getVerticalAlignment() == UIAlignment::Fill && child->getFixedSize().y() < 0.0f) {
                fillChildCount++;
            } else {
                fixedHeightUsed += child->getDesiredSize(context).y() + child->getMargin().top + child->getMargin().bottom;
            }
        }

        float maxScroll = std::max(0.0f, fixedHeightUsed - availableHeight);
        scrollOffset.y() = std::clamp(scrollOffset.y(), 0.0f, maxScroll);

        float remainingHeight = std::max(0.0f, availableHeight - fixedHeightUsed);
        float heightPerFillChild = fillChildCount > 0 ? (remainingHeight / fillChildCount) : 0.0f;

        float currentY = startY;

        for (const auto& child : children) {
            currentY += child->getMargin().top;
            float childHeight = 0.0f;
            if (child->getVerticalAlignment() == UIAlignment::Fill && child->getFixedSize().y() < 0.0f) {
                childHeight = heightPerFillChild - child->getMargin().top - child->getMargin().bottom;
            } else {
                childHeight = child->getDesiredSize(context).y();
            }

            float childDesiredWidth = child->getDesiredSize(context).x() - child->getMargin().left - child->getMargin().right;
            float finalWidth = availableWidth;
            float finalX = startX;

            switch (child->getHorizontalAlignment()) {
            case UIAlignment::Fill:
                finalWidth = availableWidth - child->getMargin().left - child->getMargin().right;
                finalX = startX + child->getMargin().left;
                break;
            case UIAlignment::Start:
                finalWidth = childDesiredWidth;
                finalX = startX + child->getMargin().left;
                break;
            case UIAlignment::Center:
                finalWidth = childDesiredWidth;
                finalX = startX + (availableWidth / 2.0f) - (childDesiredWidth / 2.0f);
                break;
            case UIAlignment::End:
                finalWidth = childDesiredWidth;
                finalX = startX + availableWidth - childDesiredWidth - child->getMargin().right;
                break;
            }

            float finalY = currentY - scrollOffset.y();

            child->arrange(context, Math::Vec2(finalX, finalY), Math::Vec2(finalWidth, childHeight));
            currentY += childHeight + child->getMargin().bottom;
        }
    }

    void UIScrollBox::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Math::Rect contentRect(arrangedPosition, arrangedSize);
        Math::Rect visibleRect = contentRect.getIntersection(scissorRect);

        if (visibleRect.width() <= 0.0f || visibleRect.height() <= 0.0f) {
            return;
        }

        context.renderer->pushScissorRect(visibleRect, context.layer);
        UIElement::onRender(context, visibleRect);
        context.renderer->popScissorRect(context.layer);
    }

    bool UIScrollBox::onEvent(Event& event) {
        if (UIElement::onEvent(event)) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseScrolledEvent>([this](const MouseScrolledEvent& e) {
            scrollOffset.y() -= e.getYOffset();
            return true;
        });

        return UIElement::onEvent(event);
    }
} // namespace Axiom