#include "axpch.h"

#include "UIHorizontalBox.h"

namespace Axiom {
    Math::Vec2 UIHorizontalBox::getDesiredSize(const UIContext& context) {
        Math::Vec2 desiredSize(0, 0);

        for (const auto& child : children) {
            Math::Vec2 childDesiredSize = child->getDesiredSize(context);
            desiredSize.x() += childDesiredSize.x();
            desiredSize.y() = std::max(desiredSize.y(), childDesiredSize.y());
        }

        desiredSize.x() += padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() += padding.top + padding.bottom + margin.top + margin.bottom;

        return desiredSize;
    }

    void UIHorizontalBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float startX = arrangedPosition.x() + padding.left + margin.left;
        float startY = arrangedPosition.y() + padding.top + margin.top;

        float availableWidth = arrangedSize.x() - padding.left - padding.right;
        float availableHeight = arrangedSize.y() - padding.top - padding.bottom;

        float fixedWidthUsed = 0.0f;
        int fillChildCount = 0;

        for (const auto& child : children) {
            if (child->getHorizontalAlignment() == UIAlignment::Fill && child->getFixedSize().x() < 0.0f) {
                fillChildCount++;
            } else {
                fixedWidthUsed += child->getDesiredSize(context).x() + child->getMargin().left + child->getMargin().right;
            }
        }

        float remainingWidth = std::max(0.0f, availableWidth - fixedWidthUsed);
        float widthPerFillChild = fillChildCount > 0 ? (remainingWidth / fillChildCount) : 0.0f;

        float currentX = startX;

        for (auto& child : children) {
            currentX += child->getMargin().left;
            float childWidth = 0.0f;
            if (child->getHorizontalAlignment() == UIAlignment::Fill && child->getFixedSize().x() < 0.0f) {
                childWidth = widthPerFillChild - child->getMargin().left - child->getMargin().right;
            } else {
                childWidth = child->getDesiredSize(context).x();
            }

            float childDesiredHeight = child->getDesiredSize(context).y() - child->getMargin().top - child->getMargin().bottom;
            float finalHeight = availableHeight;
            float finalY = startY;

            switch (child->getVerticalAlignment()) {
            case UIAlignment::Fill:
                finalHeight = availableHeight - child->getMargin().top - child->getMargin().bottom;
                finalY = startY + child->getMargin().top;
                break;
            case UIAlignment::Start:
                finalHeight = childDesiredHeight;
                finalY = startY + child->getMargin().top;
                break;
            case UIAlignment::Center:
                finalHeight = childDesiredHeight;
                finalY = startY + (availableHeight / 2.0f) - (childDesiredHeight / 2.0f);
                break;
            case UIAlignment::End:
                finalHeight = childDesiredHeight;
                finalY = startY + availableHeight - childDesiredHeight - child->getMargin().bottom;
                break;
            }

            Math::Vec2 childAllocSize(childWidth, finalHeight);
            Math::Vec2 childPosition(currentX, finalY);

            child->arrange(context, childPosition, childAllocSize);

            currentX += childWidth + child->getMargin().right;
        }
    }
} // namespace Axiom