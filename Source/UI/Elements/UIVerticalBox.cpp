#include "UIVerticalBox.h"

namespace Axiom {
    Math::Vec2 UIVerticalBox::getDesiredSize() {
        Math::Vec2 desiredSize(0, 0);

        for (const auto& child : children) {
            Math::Vec2 childDesiredSize = child->getDesiredSize();
            desiredSize.x() = std::max(desiredSize.x(), childDesiredSize.x());
            desiredSize.y() += childDesiredSize.y();
        }

        desiredSize.x() += resolvedTheme->padding.left + resolvedTheme->padding.right + resolvedTheme->margin.left + resolvedTheme->margin.right;
        desiredSize.y() += resolvedTheme->padding.top + resolvedTheme->padding.bottom + resolvedTheme->margin.top + resolvedTheme->margin.bottom;

        return desiredSize;
    }

    void UIVerticalBox::arrange(const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float currentY = arrangedPosition.y() + resolvedTheme->padding.top;
        float startX = arrangedPosition.x() + resolvedTheme->padding.left;
        float availableWidth = arrangedSize.x() - resolvedTheme->padding.left - resolvedTheme->padding.right;

        for (const auto& child : children) {
            currentY += child->getTheme()->margin.top;
            float childDesiredWidth = child->getDesiredSize().x() - child->getTheme()->margin.left - child->getTheme()->margin.right;
            float childHeight = child->getDesiredSize().y() - child->getTheme()->margin.top - child->getTheme()->margin.bottom;

            float finalX = startX;
            float finalWidth = availableWidth;

            switch (child->getHorizontalAlignment()) {
            case UIAlignment::Fill:
                finalWidth = availableWidth - child->getTheme()->margin.left - child->getTheme()->margin.right;
                finalX = startX + child->getTheme()->margin.left;
                break;
            case UIAlignment::Start:
                finalWidth = childDesiredWidth;
                finalX = startX + child->getTheme()->margin.left;
                break;
            case UIAlignment::Center:
                finalWidth = childDesiredWidth;
                finalX = startX + (availableWidth / 2.0f) - (childDesiredWidth / 2.0f);
                break;
            case UIAlignment::End:
                finalWidth = childDesiredWidth;
                finalX = startX + availableWidth - childDesiredWidth - child->getTheme()->margin.right;
                break;
            }

            Math::Vec2 childPosition(finalX, currentY);
            Math::Vec2 childAllocSize(finalWidth, childHeight);

            child->arrange(childPosition, childAllocSize);

            currentY += childHeight + child->getTheme()->margin.bottom;
        }
    }
} // namespace Axiom