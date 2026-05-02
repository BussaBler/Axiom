#include "UIVerticalBox.h"

namespace Axiom {
    Math::Vec2 UIVerticalBox::getDesiredSize() {
        Math::Vec2 desiredSize(0, 0);

        for (const auto& child : children) {
            Math::Vec2 childDesiredSize = child->getDesiredSize();
            desiredSize.x() = std::max(desiredSize.x(), childDesiredSize.x());
            desiredSize.y() += childDesiredSize.y();
        }

        desiredSize.x() += padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() += padding.top + padding.bottom + margin.top + margin.bottom;

        return desiredSize;
    }

    void UIVerticalBox::arrange(const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float currentY = arrangedPosition.y() + padding.top;
        float startX = arrangedPosition.x() + padding.left;
        float availableWidth = arrangedSize.x() - padding.left - padding.right;

        for (const auto& child : children) {
            currentY += child->getMargin().top;
            float childDesiredWidth = child->getDesiredSize().x() - child->getMargin().left - child->getMargin().right;
            float childHeight = child->getDesiredSize().y() - child->getMargin().top - child->getMargin().bottom;

            float finalX = startX;
            float finalWidth = availableWidth;

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

            Math::Vec2 childPosition(finalX, currentY);
            Math::Vec2 childAllocSize(finalWidth, childHeight);

            child->arrange(childPosition, childAllocSize);

            currentY += childHeight + child->getMargin().bottom;
        }
    }
} // namespace Axiom