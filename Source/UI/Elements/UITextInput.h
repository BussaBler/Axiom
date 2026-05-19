#pragma once
#include "UIElement.h"

namespace Axiom {
    enum class TextInputState { Normal, Hovered, Active, Typing };

    class UITextInput : public UIElement {
      public:
        UITextInput() = default;
        ~UITextInput() = default;

        Math::Vec2 getDesiredSize(const UIContext& context) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        bool onEvent(Event& event) override;

        void setColor(const Color& newColor) { overrideTextColor = newColor; }
        void setFontSize(float newFontSize) { overrideFontSize = newFontSize; }
        void setValueGetter(std::function<std::string()> getter) { getValue = std::move(getter); }
        void setValueSetter(std::function<void(const std::string&)> setter) { setValue = std::move(setter); }

      private:
        void commitTyping();

      private:
        std::string buffer;

        bool isActive = false;
        TextInputState state = TextInputState::Normal;

        std::optional<Color> overrideTextColor;
        std::optional<float> overrideFontSize;

        float textWidth = 0.0f;
        float textHeight = 0.0f;

        std::function<void(const std::string&)> setValue = nullptr;
        std::function<std::string()> getValue = nullptr;
    };
} // namespace Axiom