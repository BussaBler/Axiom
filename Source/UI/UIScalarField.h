#pragma once

#include "Event/KeyCodes.h"
#include "Math/Color.h"
#include "Math/Vec.h"
#include "UI/UIElement.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <format>
#include <functional>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>

namespace Axiom {
    enum class ScalarFieldState { Normal, Hovered, Active, Typing };

    template <typename T>
        requires std::is_arithmetic_v<T>
    class UIScalarField : public UIElement {
      public:
        UIScalarField() = default;
        ~UIScalarField() = default;

        Math::Vec2 measure(const UIContext& context) override {
            desiredSize = Math::Vec2(60.0f, context.theme->defaultRowHeight);
            return desiredSize;
        }

        void onRender(const UIContext& context, const Math::Rect& scissorRect) override {
            Color color = isActive ? context.theme->controlActiveColor
                                   : (isHovered ? context.theme->controlHoverColor : normalColor.value_or(context.theme->controlNormalColor));
            context.renderer->addBasicQuad(arrangedPosition, arrangedSize, color, context.theme->borderRadius, context.layer);

            std::string displayText;
            if (currentState == ScalarFieldState::Typing) {
                displayText = typingBuffer + "|";
            } else {
                displayText = std::to_string(valueGetter());
                if constexpr (std::is_floating_point_v<T>) {
                    displayText = std::format("{:.2f}", valueGetter());
                }
            }

            float textWidth = context.renderer->calculateTextWidth(displayText, context.theme->fontSize);
            Math::Vec2 textPos = arrangedPosition + Math::Vec2((arrangedSize.x() - textWidth) / 2.0f,
                                                               context.renderer->getCenteredTextOffset(arrangedSize.y(), context.theme->fontSize));

            context.renderer->addText(displayText, textPos, context.theme->fontSize, context.theme->textPrimary, context.layer);
        }

        bool onMouseDown(const MouseButtonPressedEvent& event) override {
            if (event.getMouseButton() != KeyCode::LeftButton || currentState == ScalarFieldState::Typing) {
                return currentState == ScalarFieldState::Typing;
            }

            isActive = true;
            isDragging = false;
            lastMousePosX = event.getMouseX();
            return true;
        }

        bool onMouseUp(const MouseButtonReleasedEvent& event) override {
            if (!isActive) {
                return false;
            }

            isActive = false;
            if (isHovered && !isDragging) {
                currentState = ScalarFieldState::Typing;
                if (valueGetter) {
                    if constexpr (std::is_floating_point_v<T>) {
                        typingBuffer = std::format("{:.2f}", valueGetter());
                    } else {
                        typingBuffer = std::to_string(valueGetter());
                    }
                }
            }
            isDragging = false;
            return true;
        }

        bool onMouseMove(const MouseMovedEvent& event) override {
            if (!isActive) {
                return false;
            }

            const float deltaX = event.getMouseX() - lastMousePosX;
            if (std::abs(deltaX) > 1e-6) {
                isDragging = true;
                if (valueGetter && valueSetter) {
                    valueSetter(std::clamp(valueGetter() + static_cast<T>(deltaX * dragSpeed), minLimit, maxLimit));
                }
            }
            lastMousePosX = event.getMouseX();
            return true;
        }

        bool onKeyDown(const KeyPressedEvent& event) override {
            if (currentState != ScalarFieldState::Typing) {
                return false;
            }

            switch (event.getKeyCode()) {
            case KeyCode::Return:
                commitTyping();
                return true;
            case KeyCode::Escape:
                currentState = ScalarFieldState::Normal;
                typingBuffer.clear();
                return true;
            case KeyCode::Backspace:
                if (!typingBuffer.empty()) {
                    typingBuffer.pop_back();
                }
                return true;
            default:
                return true;
            }
        }

        bool onTextInput(const KeyTypedEvent& event) override {
            if (currentState != ScalarFieldState::Typing) {
                return false;
            }

            const char c = event.getKeyChar();
            if (std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-') {
                typingBuffer += c;
            }
            return true;
        }

        bool isFocusable() const override { return true; }

        UIScalarField<T>& setDragSpeed(float speed) {
            dragSpeed = speed;
            return *this;
        }
        UIScalarField<T>& setLimits(T min, T max) {
            minLimit = min;
            maxLimit = max;
            return *this;
        }
        UIScalarField<T>& setValueSetter(std::function<void(T)> setter) {
            valueSetter = std::move(setter);
            return *this;
        }
        UIScalarField<T>& setValueGetter(std::function<T()> getter) {
            valueGetter = std::move(getter);
            return *this;
        }
        UIScalarField<T>& setNormalColor(const Color& color) {
            normalColor = color;
            return *this;
        }

      private:
        void commitTyping() {
            if (currentState != ScalarFieldState::Typing) {
                return;
            }

            try {
                T newValue;
                if constexpr (std::is_floating_point_v<T>) {
                    newValue = static_cast<T>(std::stof(typingBuffer));
                } else {
                    newValue = static_cast<T>(std::stoi(typingBuffer));
                }
                newValue = std::clamp(newValue, minLimit, maxLimit);

                if (valueSetter) {
                    valueSetter(newValue);
                }
            } catch (...) {
            }

            currentState = ScalarFieldState::Normal;
            typingBuffer.clear();
        }

      private:
        ScalarFieldState currentState = ScalarFieldState::Normal;

        float dragSpeed = 0.1f;
        float lastMousePosX = 0.0f;
        bool isActive = false;
        bool isDragging = false;

        T minLimit = std::numeric_limits<T>::lowest();
        T maxLimit = std::numeric_limits<T>::max();

        std::function<void(T)> valueSetter = nullptr;
        std::function<T()> valueGetter = nullptr;

        std::string typingBuffer;

        std::optional<Color> normalColor;
    };
} // namespace Axiom
