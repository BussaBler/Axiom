#include "axpch.h"

#include "UICanvas.h"

namespace Axiom {
    void UICanvas::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float startX = position.x();
        float startY = position.y();
        float availableWidth = size.x();
        float availableHeight = size.y();

        for (const auto& child : children) {
            float childX = startX;
            float childY = startY;
            float childWidth = availableWidth;
            float childHeight = availableHeight;

            float finalWidth = childWidth;
            float finalHeight = childHeight;

            switch (child->getHorizontalAlignment()) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalWidth = child->getDesiredSize(context).x();
                break;
            case UIAlignment::Center:
                finalWidth = child->getDesiredSize(context).x();
                childX = startX + (availableWidth / 2.0f) - (finalWidth / 2.0f);
                break;
            case UIAlignment::End:
                finalWidth = child->getDesiredSize(context).x();
                childX = startX + availableWidth - finalWidth;
                break;
            default:
                break;
            }

            switch (child->getVerticalAlignment()) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalHeight = child->getDesiredSize(context).y();
                break;
            case UIAlignment::Center:
                finalHeight = child->getDesiredSize(context).y();
                childY = startY + (availableHeight / 2.0f) - (finalHeight / 2.0f);
                break;
            case UIAlignment::End:
                finalHeight = child->getDesiredSize(context).y();
                childY = startY + availableHeight - finalHeight;
                break;
            default:
                break;
            }

            Math::Vec2 childPosition(childX, childY);
            Math::Vec2 childAllocSize(finalWidth, finalHeight);

            child->arrange(context, childPosition, childAllocSize);
        }
    }
} // namespace Axiom