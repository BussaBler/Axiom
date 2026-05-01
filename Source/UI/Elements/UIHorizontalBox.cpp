#include "UIHorizontalBox.h"

namespace Axiom {
    Math::Vec2 UIHorizontalBox::getDesiredSize() {
        Math::Vec2 desiredSize(0, 0);

        for (const auto& child : children) {
            Math::Vec2 childDesiredSize = child->getDesiredSize();
            desiredSize.x() += childDesiredSize.x();
            desiredSize.y() = std::max(desiredSize.y(), childDesiredSize.y());
        }

        desiredSize.x() += resolvedTheme->padding.left + resolvedTheme->padding.right + resolvedTheme->margin.left + resolvedTheme->margin.right;
        desiredSize.y() += resolvedTheme->padding.top + resolvedTheme->padding.bottom + resolvedTheme->margin.top + resolvedTheme->margin.bottom;

        return desiredSize;
    }

    void UIHorizontalBox::arrange(const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float currentX = arrangedPosition.x() + resolvedTheme->padding.left;
        float startY = arrangedPosition.y() + resolvedTheme->padding.top;
        float availableHeight = arrangedSize.y() - resolvedTheme->padding.top - resolvedTheme->padding.bottom;

        for (const auto& child : children) {
            currentX += child->getTheme()->margin.left;
            float childDesiredWidth = child->getDesiredSize().x() - child->getTheme()->padding.left - child->getTheme()->margin.right;
            float childHeight = child->getDesiredSize().y() - child->getTheme()->margin.top - child->getTheme()->margin.bottom;

            float finalY = startY;
            float finalHeight = availableHeight;

            switch (child->getVerticalAlignment()) {
            case UIAlignment::Fill:
                finalHeight = availableHeight - child->getTheme()->margin.top - child->getTheme()->margin.bottom;
                finalY = startY + child->getTheme()->margin.top;
                break;
            case UIAlignment::Start:
                finalHeight = childHeight;
                finalY = startY + child->getTheme()->margin.top;
                break;
            case UIAlignment::Center:
                finalHeight = childHeight;
                finalY = startY + (availableHeight / 2.0f) - (childHeight / 2.0f);
                break;
            case UIAlignment::End:
                finalHeight = childHeight;
                finalY = startY + availableHeight - childHeight - child->getTheme()->margin.bottom;
                break;
            }

            Math::Vec2 childPosition(currentX, finalY);
            Math::Vec2 childAllocSize(childDesiredWidth, finalHeight);

            child->arrange(childPosition, childAllocSize);

            currentX += childDesiredWidth + child->getTheme()->margin.right;
        }
    }
} // namespace Axiom