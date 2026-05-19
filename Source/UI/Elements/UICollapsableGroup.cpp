#include "axpch.h"

#include "UICollapsableGroup.h"

#include "Event/MouseEvent.h"

namespace Axiom {
    Math::Vec2 UICollapsableGroup::getDesiredSize(const UIContext& context) {
        desiredSize.y() = HEADER_HEIGHT;

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        float textWidth = context.renderer->calculateTextWidth(displayTitle, resolvedTheme->fontSize, context.dpiScale);
        desiredSize.x() = textWidth + 20.0f;

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
            float currentY = arrangedPosition.y() + HEADER_HEIGHT + padding.top;
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

    void UICollapsableGroup::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Math::Vec2 headerPos = arrangedPosition + Math::Vec2(padding.left, padding.top);
        Math::Vec2 headerSize = Math::Vec2(arrangedSize.x() - padding.left - padding.right, HEADER_HEIGHT);

        Color headerColor = isHeaderHovered ? resolvedTheme->controlHoverColor : resolvedTheme->controlNormalColor;
        context.renderer->addBasicQuad(headerPos, headerSize, headerColor, resolvedTheme->borderRadius, context.layer);

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        float textHeight = context.renderer->calculateTextHeight(resolvedTheme->fontSize, context.dpiScale);

        float textYOffset = (HEADER_HEIGHT - textHeight) * 0.5f;
        Math::Vec2 textPos = headerPos + Math::Vec2(5.0f, textYOffset);

        context.renderer->addText(displayTitle, textPos, resolvedTheme->fontSize, context.dpiScale, resolvedTheme->textColor, context.layer);

        if (isOpen) {
            for (const auto& child : children) {
                child->onRender(context, scissorRect);
            }
        }
    }

    bool UICollapsableGroup::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        if (isOpen) {
            for (auto it = children.rbegin(); it != children.rend(); ++it) {
                if ((*it)->onEvent(event)) {
                    return true;
                }
            }
        }

        EventDispatcher dispatcher(event);

        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& e) {
            float mouseX = e.getMouseX();
            float mouseY = e.getMouseY();

            isHeaderHovered = false;
            if (mouseX >= arrangedPosition.x() && mouseX <= arrangedPosition.x() + arrangedSize.x() && mouseY >= arrangedPosition.y() &&
                mouseY <= arrangedPosition.y() + HEADER_HEIGHT) {
                isHeaderHovered = true;
            }
            return false;
        });

        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& event) {
            if (isHeaderHovered && event.getMouseButton() == KeyCode::LeftButton) {
                isActive = true;
                return true;
            }
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& event) {
            if (isActive && event.getMouseButton() == KeyCode::LeftButton) {
                isActive = false;
                if (isHeaderHovered) {
                    isOpen = !isOpen;
                }
                return true;
            }
            return false;
        });

        return UIElement::onEvent(event);
    }
} // namespace Axiom
