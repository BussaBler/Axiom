#include "axpch.h"

#include "UIText.h"

namespace Axiom {
    Math::Vec2 UIText::measure(const UIContext& context) {
        textSize.x() = context.renderer->calculateTextWidth(text, context.theme->fontSize);
        textSize.y() = context.renderer->calculateTextHeight(context.theme->fontSize);
        desiredSize = textSize;

        return desiredSize;
    }

    void UIText::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        if (text.empty()) {
            return;
        }

        const float yOffset = context.renderer->getCenteredTextOffset(arrangedSize.y(), context.theme->fontSize);
        context.renderer->addText(text, arrangedPosition + Math::Vec2(0.0f, yOffset), context.theme->fontSize,
                                  textColor.value_or(context.theme->textPrimary), context.layer);
    }
} // namespace Axiom
