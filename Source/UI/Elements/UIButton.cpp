#include "UIButton.h"
#include "Core/Application.h"
#include "Event/MouseEvent.h"

namespace Axiom {
    Math::Vec2 UIButton::getDesiredSize() {
        desiredSize.x() = textWidth + resolvedTheme->padding.left + resolvedTheme->padding.right + resolvedTheme->margin.left + resolvedTheme->margin.right;
        desiredSize.y() = textHeight + resolvedTheme->padding.top + resolvedTheme->padding.bottom + resolvedTheme->margin.top + resolvedTheme->margin.bottom;

        return desiredSize;
    }

    void UIButton::arrange(const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;
    }

    void UIButton::onRender(UIRenderer* renderer) {
        Color normalColor = overrideNormalColor.value_or(resolvedTheme->controlNormalColor);
        Color hoverColor = overrideHoverColor.value_or(resolvedTheme->controlHoverColor);
        Color activeColor = overrideActiveColor.value_or(resolvedTheme->controlActiveColor);
        Color backgroundColor = isActive ? activeColor : (isHovered ? hoverColor : normalColor);

        renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, resolvedTheme->borderRadius);

        float dpiScale = Application::getWindow()->getWindowDPI() / 72.0f;

        textWidth = renderer->calculateTextWidth(text, resolvedTheme->fontSize, dpiScale);
        textHeight = renderer->calculateTextHeight(resolvedTheme->fontSize, dpiScale);

        float textX = arrangedPosition.x() + (arrangedSize.x() - textWidth) / 2.0f;
        float textY = arrangedPosition.y() + (arrangedSize.y() - textHeight) / 2.0f;

        renderer->addText(text, Math::Vec2(textX, textY), resolvedTheme->fontSize, dpiScale, resolvedTheme->textColor);

        UIElement::onRender(renderer);
    }

    bool UIButton::onEvent(Event& event) {
        if (UIElement::onEvent(event)) {
            return true;
        }

        EventDispatcher dispatcher(event);

        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& event) {
            float mx = event.getMouseX();
            float my = event.getMouseY();

            isHovered = (mx >= arrangedPosition.x() && mx <= arrangedPosition.x() + arrangedSize.x() && my >= arrangedPosition.y() &&
                         my <= arrangedPosition.y() + arrangedSize.y());
            return false;
        });

        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& event) {
            if (isHovered && event.getMouseButton() == KeyCode::LeftButton) {
                isActive = true;
                return true;
            }
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& event) {
            if (isActive && event.getMouseButton() == KeyCode::LeftButton) {
                isActive = false;
                if (isHovered && onClick) {
                    onClick();
                }
                return true;
            }
            return false;
        });

        return false;
    }
} // namespace Axiom