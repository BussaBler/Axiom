#include "UIText.h"
#include "Core/Application.h"

namespace Axiom {
    Math::Vec2 UIText::getDesiredSize() {
        desiredSize.x() = textWidth + padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() = textHeight + padding.top + padding.bottom + margin.top + margin.bottom;
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

        Math::Vec2 textPos = arrangedPosition + Math::Vec2(margin.left + padding.left, margin.top + padding.top);
        uiRenderer->addText(text, textPos, overrideFontSize.value_or(theme->fontSize), dpiScale, overrideTextColor.value_or(theme->textColor));
    }
} // namespace Axiom