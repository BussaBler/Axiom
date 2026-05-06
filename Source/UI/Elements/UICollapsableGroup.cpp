#include "UICollapsableGroup.h"
#include "Event/MouseEvent.h"

namespace Axiom {
    Math::Vec2 UICollapsableGroup::getDesiredSize(const UIContext& context) {
        desiredSize.y() = headerHeight;

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        float textWidth = context.renderer->calculateTextWidth(displayTitle, resolvedTheme->fontSize, context.dpiScale);
        desiredSize.x() = textWidth + HEADER_TEXT_PADDING_X + 20.0f;

        if (isOpen) {
            for (const auto& child : children) {
                Math::Vec2 childSize = child->getDesiredSize(context);
                desiredSize.x() = std::max(desiredSize.x(), childSize.x());
                desiredSize.y() += childSize.y();
            }
        }

        desiredSize.x() += padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() += padding.top + padding.bottom + margin.top + margin.bottom;
        return desiredSize;
    }

    void UICollapsableGroup::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;

        if (isOpen && !children.empty()) {
            float currentY = arrangedPosition.y() + headerHeight + padding.top;
            float startX = arrangedPosition.x() + padding.left;
            float availableWidth = arrangedSize.x() - padding.left - padding.right;

            for (const auto& child : children) {
                currentY += child->getMargin().top;

                float childDesiredWidth = child->getDesiredSize(context).x() - child->getMargin().left - child->getMargin().right;
                float childHeight = child->getDesiredSize(context).y() - child->getMargin().top - child->getMargin().bottom;

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

                child->arrange(context, Math::Vec2(finalX, currentY), Math::Vec2(finalWidth, childHeight));
                currentY += childHeight + child->getMargin().bottom;
            }
        }
    }

    void UICollapsableGroup::onRender(const UIContext& context) {
        Math::Vec2 headerPos = arrangedPosition + Math::Vec2(padding.left, padding.top);
        Math::Vec2 headerSize = Math::Vec2(arrangedSize.x() - padding.left - padding.right, headerHeight);

        Color headerColor = isHovered ? resolvedTheme->controlHoverColor : resolvedTheme->controlNormalColor;
        context.renderer->addBasicQuad(headerPos, headerSize, headerColor, resolvedTheme->borderRadius, context.layer);

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        Math::Vec2 textPos = headerPos + Math::Vec2(HEADER_TEXT_PADDING_X, HEADER_TEXT_PADDING_Y);
        context.renderer->addText(displayTitle, textPos, resolvedTheme->fontSize, context.dpiScale, resolvedTheme->textColor, context.layer);

        if (isOpen) {
            for (const auto& child : children) {
                child->onRender(context);
            }
        }
    }

    bool UICollapsableGroup::onEvent(Event& event) {
        if (isOpen) {
            if (UIElement::onEvent(event)) {
                return true;
            }
        }

        EventDispatcher dispatcher(event);

        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& event) {
            float mx = event.getMouseX();
            float my = event.getMouseY();
            isHovered = mx >= arrangedPosition.x() + padding.left && mx <= arrangedPosition.x() + arrangedSize.x() - padding.right &&
                        my >= arrangedPosition.y() + padding.top && my <= arrangedPosition.y() + padding.top + headerHeight;
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& event) {
            if (isHovered && event.getMouseButton() == KeyCode::LeftButton) {
                isOpen = !isOpen;
                return true;
            }
            return false;
        });

        return false;
    }
} // namespace Axiom
