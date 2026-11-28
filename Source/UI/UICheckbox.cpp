#include "axpch.h"

#include "UICheckbox.h"

#include "Event/KeyCodes.h"
#include "Event/MouseEvent.h"
#include "UI/UIElement.h"

namespace Axiom {
    Math::Vec2 UICheckbox::measure(const UIContext& context) {
        const float boxSize = context.theme->defaultRowHeight - 6.0f;
        desiredSize = Math::Vec2(boxSize);

        return desiredSize;
    }

    void UICheckbox::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color normalColor = overrideNormalColor.value_or(context.theme->controlNormalColor);
        Color hoverColor = overrideHoverColor.value_or(context.theme->controlHoverColor);
        Color activeColor = overrideActiveColor.value_or(context.theme->controlActiveColor);
        Color backgroundColor = isActive ? activeColor : (isHovered ? hoverColor : normalColor);

        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, context.theme->borderRadius, context.layer);

        if (valueGetter && valueGetter()) {
            float innerPadding = arrangedSize.x() * 0.25f;
            Math::Vec2 innerPos = arrangedPosition + Math::Vec2(innerPadding);
            Math::Vec2 innerSize = arrangedSize - Math::Vec2(innerPadding) * 2.0f;

            context.renderer->addBasicQuad(innerPos, innerSize, context.theme->accentColor, context.theme->borderRadius * 0.5f, context.layer);
        }
    }

    bool UICheckbox::onMouseDown(const MouseButtonPressedEvent& event) {
        if (event.getMouseButton() != KeyCode::LeftButton) {
            return false;
        }

        isActive = true;
        return true;
    }

    bool UICheckbox::onMouseUp(const MouseButtonReleasedEvent& event) {
        if (!isActive) {
            return false;
        }

        isActive = false;
        if (isHovered && valueGetter && valueSetter) {
            valueSetter(!valueGetter());
        }
        return true;
    }
} // namespace Axiom
