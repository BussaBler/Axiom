#include "UIPanel.h"

namespace Axiom {
    Math::Vec2 UIPanel::getDesiredSize(const UIContext& context) {
        Math::Vec2 autoSize = Math::Vec2::zero();

        for (const auto& child : children) {
            Math::Vec2 childSize = child->getDesiredSize(context);
            autoSize.x() = std::max(autoSize.x(), childSize.x());
            autoSize.y() = std::max(autoSize.y(), childSize.y());
        }

        autoSize.x() += padding.left + padding.right + margin.left + margin.right;
        autoSize.y() += padding.top + padding.bottom + margin.top + margin.bottom;

        if (fixedSize.x() >= 0) {
            autoSize.x() = fixedSize.x();
        }
        if (fixedSize.y() >= 0) {
            autoSize.y() = fixedSize.y();
        }
        desiredSize = autoSize;

        return desiredSize;
    }

    void UIPanel::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float startX = position.x() + padding.left;
        float startY = position.y() + padding.top;
        float availableWidth = size.x() - padding.left - padding.right;
        float availableHeight = size.y() - padding.top - padding.bottom;

        for (const auto& child : children) {
            float childX = startX + child->getMargin().left;
            float childY = startY + child->getMargin().top;
            float childWidth = availableWidth - child->getMargin().left - child->getMargin().right;
            float childHeight = availableHeight - child->getMargin().top - child->getMargin().bottom;

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
                childX = startX + availableWidth - finalWidth - child->getMargin().right;
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
                childY = startY + availableHeight - finalHeight - child->getMargin().bottom;
                break;
            default:
                break;
            }

            Math::Vec2 childPosition(childX, childY);
            Math::Vec2 childAllocSize(finalWidth, finalHeight);

            child->arrange(context, childPosition, childAllocSize);
        }
    }

    void UIPanel::onRender(const UIContext& context) {
        Math::Vec4 radii = Math::Vec4::zero();
        switch (horizontalAlignment) {
        case UIAlignment::Fill:
            break;
        case UIAlignment::Start:
            radii.y() = resolvedTheme->borderRadius.y();
            radii.w() = resolvedTheme->borderRadius.w();
            break;
        case UIAlignment::Center:
            radii = resolvedTheme->borderRadius;
            break;
        case UIAlignment::End:
            radii.x() = resolvedTheme->borderRadius.x();
            radii.z() = resolvedTheme->borderRadius.z();
            break;
        default:
            break;
        }

        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, overridePanelBackgroundColor.value_or(resolvedTheme->panelBackgroundColor), radii);
        UIElement::onRender(context);
    }
} // namespace Axiom