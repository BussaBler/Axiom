#include "axpch.h"

#include "UIText.h"

namespace Axiom {
    Math::Vec2 UIText::getDesiredSize(const UIContext& context) {
        textSize.x() = context.renderer->calculateTextWidth(text, context.theme->fontSize);
        textSize.y() = context.renderer->calculateTextHeight(context.theme->fontSize);
        desiredSize = textSize;

        return desiredSize;
    }

    void UIText::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        if (text.empty()) {
            return;
        }

        context.renderer->addText(text, arrangedPosition, context.theme->fontSize, textColor.value_or(context.theme->textPrimary));
    }
} // namespace Axiom
