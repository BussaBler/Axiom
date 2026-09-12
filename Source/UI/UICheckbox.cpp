#include "axpch.h"

#include "UICheckbox.h"

#include "Event/Event.h"
#include "Event/MouseEvent.h"
#include "UI/UIElement.h"

namespace Axiom {
    Math::Vec2 UICheckbox::getDesiredSize(const UIContext& context) {
        float boxSize = context.theme->fontSize * 1.2f;
        desiredSize = Math::Vec2(boxSize);

        return desiredSize;
    }

    void UICheckbox::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color normalColor = overrideNormalColor.value_or(context.theme->controlNormalColor);
        Color hoverColor = overrideHoverColor.value_or(context.theme->controlHoverColor);
        Color activeColor = overrideActiveColor.value_or(context.theme->controlActiveColor);
        Color backgroundColor = isActive ? activeColor : (isHovered ? hoverColor : normalColor);

        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor);

        if (valueGetter && valueGetter()) {
            float innerPadding = arrangedSize.x() * 0.25f;
            Math::Vec2 innerPos = arrangedPosition + Math::Vec2(innerPadding);
            Math::Vec2 innerSize = arrangedSize - Math::Vec2(innerPadding) * 2.0f;

            context.renderer->addBasicQuad(innerPos, innerSize, context.theme->accentColor, context.theme->borderRadius * 0.5f);
        }
    }

    bool UICheckbox::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
            if (isHovered && e.getMouseButton() == KeyCode::LeftButton) {
                isActive = true;
                return true;
            }
            return false;
        });
        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) {
            if (isActive && e.getMouseButton() == KeyCode::LeftButton) {
                isActive = false;
                if (isHovered && valueSetter) {
                    bool currValue = valueGetter ? valueGetter() : false;
                    valueSetter(!currValue);
                }
                return true;
            }
            return false;
        });

        return UIElement::onEvent(event);
    }
} // namespace Axiom
