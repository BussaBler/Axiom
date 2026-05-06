#include "UIText.h"
#include "Core/Application.h"

namespace Axiom {
    Math::Vec2 UIText::getDesiredSize(const UIContext& context) {
        textWidth = context.renderer->calculateTextWidth(text, overrideFontSize.value_or(resolvedTheme->fontSize), context.dpiScale);
        textHeight = context.renderer->calculateTextHeight(overrideFontSize.value_or(resolvedTheme->fontSize), context.dpiScale);
        desiredSize.x() = textWidth + padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() = textHeight + padding.top + padding.bottom + margin.top + margin.bottom;

        if (fixedSize.x() > 0.0f) {
            desiredSize.x() = fixedSize.x();
        }
        if (fixedSize.y() > 0.0f) {
            desiredSize.y() = fixedSize.y();
        }
        return desiredSize;
    }

    void UIText::onRender(const UIContext& context) {
        if (text.empty()) {
            return;
        }

        Math::Vec2 textPos = arrangedPosition + Math::Vec2(margin.left + padding.left, margin.top + padding.top);
        context.renderer->addText(text, textPos, overrideFontSize.value_or(resolvedTheme->fontSize), context.dpiScale,
                                  overrideTextColor.value_or(resolvedTheme->textColor), context.layer);
    }
} // namespace Axiom