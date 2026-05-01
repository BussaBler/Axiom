#include "UIPanel.h"

namespace Axiom {
    Math::Vec2 UIPanel::getDesiredSize() {
        Math::Vec2 autoSize = Math::Vec2::zero();

        for (const auto& child : children) {
            Math::Vec2 childSize = child->getDesiredSize();
            autoSize.x() = std::max(autoSize.x(), childSize.x());
            autoSize.y() = std::max(autoSize.y(), childSize.y());
        }

        autoSize.x() += resolvedTheme->padding.left + resolvedTheme->padding.right + resolvedTheme->margin.left + resolvedTheme->margin.right;
        autoSize.y() += resolvedTheme->padding.top + resolvedTheme->padding.bottom + resolvedTheme->margin.top + resolvedTheme->margin.bottom;

        if (fixedSize.x() >= 0) {
            autoSize.x() = fixedSize.x();
        }
        if (fixedSize.y() >= 0) {
            autoSize.y() = fixedSize.y();
        }
        desiredSize = autoSize;

        return desiredSize;
    }

    void UIPanel::arrange(const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        float startX = position.x() + resolvedTheme->padding.left;
        float startY = position.y() + resolvedTheme->padding.top;
        float availableWidth = size.x() - resolvedTheme->padding.left - resolvedTheme->padding.right;
        float availableHeight = size.y() - resolvedTheme->padding.top - resolvedTheme->padding.bottom;

        for (const auto& child : children) {
            float childX = startX + child->getTheme()->margin.left;
            float childY = startY + child->getTheme()->margin.top;
            float childWidth = availableWidth - child->getTheme()->margin.left - child->getTheme()->margin.right;
            float childHeight = availableHeight - child->getTheme()->margin.top - child->getTheme()->margin.bottom;

            float finalWidth = childWidth;
            float finalHeight = childHeight;

            switch (child->getHorizontalAlignment()) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalWidth = child->getDesiredSize().x();
                break;
            case UIAlignment::Center:
                finalWidth = child->getDesiredSize().x();
                childX = startX + (availableWidth / 2.0f) - (finalWidth / 2.0f);
                break;
            case UIAlignment::End:
                finalWidth = child->getDesiredSize().x();
                childX = startX + availableWidth - finalWidth - child->getTheme()->margin.right;
                break;
            default:
                break;
            }

            switch (child->getVerticalAlignment()) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalHeight = child->getDesiredSize().y();
                break;
            case UIAlignment::Center:
                finalHeight = child->getDesiredSize().y();
                childY = startY + (availableHeight / 2.0f) - (finalHeight / 2.0f);
                break;
            case UIAlignment::End:
                finalHeight = child->getDesiredSize().y();
                childY = startY + availableHeight - finalHeight - child->getTheme()->margin.bottom;
                break;
            default:
                break;
            }

            Math::Vec2 childPosition(childX, childY);
            Math::Vec2 childAllocSize(finalWidth, finalHeight);

            child->arrange(childPosition, childAllocSize);
        }
    }

    void UIPanel::onRender(UIRenderer* uiRenderer) {
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

        uiRenderer->addBasicQuad(arrangedPosition, arrangedSize, overridePanelBackgroundColor.value_or(resolvedTheme->panelBackgroundColor), radii);
        UIElement::onRender(uiRenderer);
    }
} // namespace Axiom