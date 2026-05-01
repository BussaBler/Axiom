#pragma once
#include "UIElement.h"

namespace Axiom {
    class UIText : public UIElement {
      public:
        UIText(const std::string& text) : text(text) {}
        ~UIText() = default;

        Math::Vec2 getDesiredSize() override;

        void onRender(UIRenderer* uiRenderer) override;

        void setText(const std::string& newText) { text = newText; }
        void setColor(const Color& newColor) { overrideTextColor = newColor; }
        void setFontSize(float newFontSize) { overrideFontSize = newFontSize; }

        const std::string& getText() const { return text; }

      private:
        std::string text;
        std::optional<Color> overrideTextColor;
        std::optional<float> overrideFontSize;

        float textWidth = 0.0f;
        float textHeight = 0.0f;
    };
}