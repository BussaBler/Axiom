#pragma once

#include "Event/Event.h"
#include "Event/KeyCodes.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Math/Color.h"
#include "Math/Vec.h"
#include "UI/UIElement.h"

#include <algorithm>
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

        Math::Vec2 getDesiredSize(const UIContext& context) override {
            desiredSize = Math::Vec2(60.0f, 20.0f);
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
            float textHeight = context.renderer->calculateTextHeight(context.theme->fontSize);
            Math::Vec2 textPos = arrangedPosition + (arrangedSize - Math::Vec2(textWidth, textHeight)) / 2.0f;

            context.renderer->addText(displayText, textPos, context.theme->fontSize, context.theme->textPrimary);
        }
        bool onEvent(Event& event) override {
            if (event.isHandled()) {
                return true;
            }

            EventDispatcher dispatcher(event);
            dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
                if (e.getMouseButton() == KeyCode::LeftButton) {
                    if (isHovered) {
                        isActive = true;
                        isDragging = false;
                        lastMousePosX = e.getMouseX();
                        return true;
                    }
                }
                return false;
            });
            dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) {
                if (isActive && e.getMouseButton() == KeyCode::LeftButton) {
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
                return false;
            });
            dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& e) {
                if (isActive && currentState != ScalarFieldState::Typing) {
                    float deltaX = e.getMouseX() - lastMousePosX;
                    if (std::abs(deltaX) > 0.0f) {
                        isDragging = true;
                        if (valueGetter && valueSetter) {
                            T newValue = std::clamp(valueGetter() + static_cast<T>(deltaX * dragSpeed), minLimit, maxLimit);
                            valueSetter(newValue);
                        }
                    }
                    lastMousePosX = e.getMouseX();
                    return true;
                }
                return false;
            });
            dispatcher.dispatch<KeyPressedEvent>([this](const KeyPressedEvent& event) {
                if (currentState == ScalarFieldState::Typing) {
                    if (event.getKeyCode() == KeyCode::Return) {
                        commitTyping();
                        return true;
                    } else if (event.getKeyCode() == KeyCode::Escape) {
                        currentState = ScalarFieldState::Normal;
                        typingBuffer.clear();
                        return true;
                    } else if (event.getKeyCode() == KeyCode::Backspace) {
                        if (!typingBuffer.empty()) {
                            typingBuffer.pop_back();
                        }
                        return true;
                    }
                }
                return false;
            });
            dispatcher.dispatch<KeyTypedEvent>([this](const KeyTypedEvent& event) {
                if (currentState == ScalarFieldState::Typing) {
                    char typedChar = event.getKeyChar();
                    if (std::isdigit(typedChar)) {
                        typingBuffer += typedChar;
                    } else if (typedChar == '-' && typingBuffer.empty()) {
                        typingBuffer += typedChar;
                    } else if constexpr (std::is_floating_point_v<T>) {
                        if (typedChar == '.' && typingBuffer.find('.') == std::string::npos) {
                            typingBuffer += typedChar;
                        }
                    }
                    return true;
                }
                return false;
            });

            return UIElement::onEvent(event);
        }

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
