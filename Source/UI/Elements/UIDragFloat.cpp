#include "UIDragFloat.h"
#include "Core/Application.h"

namespace Axiom {
    Math::Vec2 UIDragFloat::getDesiredSize() {
        desiredSize.x() = 60.0f + padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() = 20.0f + padding.top + padding.bottom + margin.top + margin.bottom;
        return desiredSize;
    }

    void UIDragFloat::onRender(UIRenderer* uiRenderer) {
        Color normalColor = overrideNormalColor.value_or(resolvedTheme->controlNormalColor);
        Color hoverColor = overrideHoverColor.value_or(resolvedTheme->controlHoverColor);
        Color activeColor = overrideActiveColor.value_or(resolvedTheme->controlActiveColor);
        Color backgroundColor = isActive ? activeColor : (isHovered ? hoverColor : normalColor);
        uiRenderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, resolvedTheme->borderRadius);

        float currentValue = getValue ? getValue() : 0.0f;
        std::string valueText = std::format("{:.2f}", currentValue);

        float dpiScale = Application::getWindow()->getWindowDPI() / 72.0f;

        float textWidth = uiRenderer->calculateTextWidth(valueText, resolvedTheme->fontSize, dpiScale);
        float textHeight = uiRenderer->calculateTextHeight(resolvedTheme->fontSize, dpiScale);

        float textX = arrangedPosition.x() + (arrangedSize.x() - textWidth) / 2.0f;
        float textY = arrangedPosition.y() + (arrangedSize.y() - textHeight) / 2.0f;

        uiRenderer->addText(valueText, Math::Vec2(textX, textY), resolvedTheme->fontSize, dpiScale, resolvedTheme->textColor);

        UIElement::onRender(uiRenderer);
    }

    bool UIDragFloat::onEvent(Event& event) {
        if (UIElement::onEvent(event)) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& event) {
            float mx = event.getMouseX();
            float my = event.getMouseY();

            if (isActive && getValue && setValue) {
                float deltaX = mx - lastMouseX;

                float newValue = getValue() + (deltaX * dragSpeed);
                if (isLimitted) {
                    newValue = std::clamp(newValue, floatLimit.x(), floatLimit.y());
                }
                setValue(newValue);

                lastMouseX = mx;
                return true;
            } else {
                isHovered = (mx >= arrangedPosition.x() && mx <= arrangedPosition.x() + arrangedSize.x() && my >= arrangedPosition.y() &&
                             my <= arrangedPosition.y() + arrangedSize.y());
                lastMouseX = mx;
            }
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
                return true;
            }
            return false;
        });

        return false;
    }
} // namespace Axiom