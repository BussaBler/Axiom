#pragma once

#include "UI/UIElement.h"

#include <functional>
#include <string>
#include <utility>

namespace Axiom {
    enum class TextInputState { Normal, Hovered, Active, Typing };

    class UITextInput : public UIElement {
      public:
        UITextInput() = default;
        ~UITextInput() = default;

        Math::Vec2 measure(const UIContext& context) override;

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;

        bool onMouseDown(const MouseButtonPressedEvent& event) override;
        bool onKeyDown(const KeyPressedEvent& event) override;
        bool onTextInput(const KeyTypedEvent& event) override;

        void onFocusChanged(bool focused) override;
        bool isFocusable() const override { return true; }

        UITextInput& setValueSetter(std::function<void(const std::string&)> setter) {
            valueSetter = std::move(setter);
            return *this;
        }
        UITextInput& setValueGetter(std::function<std::string()> getter) {
            valueGetter = std::move(getter);
            return *this;
        }

      private:
        void commitTyping();

      private:
        std::string typingBuffer;

        bool isActive = false;
        TextInputState currentState = TextInputState::Normal;

        Math::Vec2 textSize = Math::Vec2::zero();

        std::function<void(const std::string&)> valueSetter = nullptr;
        std::function<std::string()> valueGetter = nullptr;
    };
} // namespace Axiom
