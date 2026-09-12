#include "axpch.h"

#include "UI/UIButton.h"

#include "Event/Event.h"
#include "Event/KeyCodes.h"
#include "Event/MouseEvent.h"
#include "Math/AxMath.h"
#include "Math/Vec.h"
#include "UI/UIElement.h"

namespace Axiom {
    Math::Vec2 UIButton::getDesiredSize(const UIContext& context) {
        float fontSize = overrideFontSize.value_or(context.theme->fontSize);
        textSize = Math::Vec2(context.renderer->calculateTextWidth(text, fontSize), context.renderer->calculateTextHeight(fontSize));
        desiredSize.x() = textSize.x();
        desiredSize.y() = textSize.y();

        return desiredSize;
    }

    void UIButton::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color normalColor = overrideNormalColor.value_or(context.theme->controlNormalColor);
        Color hoverColor = overrideHoverColor.value_or(context.theme->controlHoverColor);
        Color activeColor = overrideActiveColor.value_or(context.theme->controlActiveColor);
        Color backgroundColor = isActive ? activeColor : (isHovered ? hoverColor : normalColor);
        float fontSize = overrideFontSize.value_or(context.theme->fontSize);
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, context.theme->borderRadius, context.layer);

        Math::Vec2 textPos = arrangedPosition + (arrangedSize - textSize) / 2.0f;
        context.renderer->addText(text, textPos, fontSize, context.theme->textPrimary, context.layer);
    }

    bool UIButton::onEvent(Event& event) {
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
                if (isHovered && onClickCallback) {
                    onClickCallback();
                }
                return true;
            }
            return false;
        });

        return UIElement::onEvent(event);
    }
} // namespace Axiom
