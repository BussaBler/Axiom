#include "UIText.h"
#include "Core/Application.h"

namespace Axiom {
    Math::Vec2 UIText::getDesiredSize() {
        desiredSize.x() = textWidth + resolvedTheme->padding.left + resolvedTheme->padding.right + resolvedTheme->margin.left + resolvedTheme->margin.right;
        desiredSize.y() = textHeight + resolvedTheme->padding.top + resolvedTheme->padding.bottom + resolvedTheme->margin.top + resolvedTheme->margin.bottom;
        return desiredSize;
    }

    void UIText::onRender(UIRenderer* uiRenderer) {
        if (text.empty()) {
            return;
        }
        std::shared_ptr<UITheme> theme = getTheme();

        float dpiScale = Application::getWindow()->getWindowDPI() / 72.0f;
        textWidth = uiRenderer->calculateTextWidth(text, overrideFontSize.value_or(theme->fontSize), dpiScale);
        textHeight = uiRenderer->calculateTextHeight(overrideFontSize.value_or(theme->fontSize), dpiScale);

        Math::Vec2 textPos = arrangedPosition + Math::Vec2(theme->margin.left + theme->padding.left, theme->margin.top + theme->padding.top);
        uiRenderer->addText(text, textPos, overrideFontSize.value_or(theme->fontSize), dpiScale, overrideTextColor.value_or(theme->textColor));
    }
} // namespace Axiom