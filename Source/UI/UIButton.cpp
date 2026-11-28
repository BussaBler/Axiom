#include "axpch.h"

#include "UI/UIButton.h"

#include "Event/KeyCodes.h"
#include "Event/MouseEvent.h"
#include "Math/AxMath.h"
#include "Math/Vec.h"
#include "UI/UIElement.h"

#include <algorithm>

namespace Axiom {
    Math::Vec2 UIButton::measure(const UIContext& context) {
        float fontSize = overrideFontSize.value_or(context.theme->fontSize);
        textSize = Math::Vec2(context.renderer->calculateTextWidth(text, fontSize), context.renderer->calculateTextHeight(fontSize));
        const UITheme& theme = *context.theme;
        desiredSize.x() = textSize.x() + theme.panelPadding.left + theme.panelPadding.right;
        desiredSize.y() = std::max(textSize.y() + theme.containerPadding.top + theme.containerPadding.bottom, theme.defaultRowHeight);

        return desiredSize;
    }

    void UIButton::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color normalColor = overrideNormalColor.value_or(context.theme->controlNormalColor);
        Color hoverColor = overrideHoverColor.value_or(context.theme->controlHoverColor);
        Color activeColor = overrideActiveColor.value_or(context.theme->controlActiveColor);
        Color backgroundColor = isActive ? activeColor : (isHovered ? hoverColor : normalColor);
        float fontSize = overrideFontSize.value_or(context.theme->fontSize);
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, context.theme->borderRadius, context.layer);

        float textX = arrangedPosition.x() + (arrangedSize.x() - textSize.x()) / 2.0f;
        if (textAlignment == UIAlignment::Start) {
            textX = arrangedPosition.x() + context.theme->panelPadding.left;
        } else if (textAlignment == UIAlignment::End) {
            textX = arrangedPosition.x() + arrangedSize.x() - context.theme->panelPadding.right - textSize.x();
        }
        const float textY = arrangedPosition.y() + context.renderer->getCenteredTextOffset(arrangedSize.y(), fontSize);

        Color textColor = overrideTextColor.value_or(context.theme->textPrimary);
        context.renderer->addText(text, Math::Vec2(textX, textY), fontSize, textColor, context.layer);
    }

    bool UIButton::onMouseDown(const MouseButtonPressedEvent& event) {
        if (event.getMouseButton() != KeyCode::LeftButton) {
            return false;
        }

        isActive = true;
        return true;
    }

    bool UIButton::onMouseUp(const MouseButtonReleasedEvent& event) {
        if (!isActive) {
            return false;
        }

        isActive = false;
        if (isHovered && onClickCallback) {
            onClickCallback();
        }

        return true;
    }
} // namespace Axiom
