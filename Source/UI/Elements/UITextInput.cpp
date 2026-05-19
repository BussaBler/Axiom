#include "axpch.h"

#include "UITextInput.h"

#include "Core/Application.h"

namespace Axiom {
    Math::Vec2 UITextInput::getDesiredSize(const UIContext& context) {
        std::string displayText;
        if (state == TextInputState::Typing) {
            displayText = buffer + "|";
        } else {
            displayText = getValue ? getValue() : "";
        }

        float fontSize = overrideFontSize.value_or(resolvedTheme->fontSize);
        textWidth = context.renderer->calculateTextWidth(displayText, fontSize, context.dpiScale);
        textHeight = context.renderer->calculateTextHeight(fontSize, context.dpiScale);
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

    void UITextInput::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color backgroundColor =
            isActive ? resolvedTheme->controlActiveColor : (isHovered ? resolvedTheme->controlHoverColor : resolvedTheme->controlNormalColor);
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, resolvedTheme->borderRadius, context.layer);
        Color textColor = overrideTextColor.value_or(resolvedTheme->textColor);

        float fontSize = overrideFontSize.value_or(resolvedTheme->fontSize);

        std::string displayText;
        if (state == TextInputState::Typing) {
            displayText = buffer + "|";
        } else {
            displayText = getValue ? getValue() : "";
        }

        Math::Vec2 textPos = arrangedPosition + Math::Vec2(margin.left + padding.left + 5.0f, margin.top + padding.top);
        context.renderer->addText(displayText, textPos, fontSize, context.dpiScale, textColor, context.layer);
    }

    bool UITextInput::onEvent(Event& event) {
        if (UIElement::onEvent(event)) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& event) {
            if (event.getMouseButton() == KeyCode::LeftButton) {
                if (isHovered) {
                    isActive = true;
                    return true;
                } else {
                    if (state == TextInputState::Typing) {
                        commitTyping();
                    }
                }
            }
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& event) {
            if (isActive && event.getMouseButton() == KeyCode::LeftButton) {
                isActive = false;
                if (isHovered) {
                    state = TextInputState::Typing;
                    buffer = getValue ? getValue() : "";
                }
                return true;
            }
            return false;
        });

        dispatcher.dispatch<KeyPressedEvent>([this](const KeyPressedEvent& event) {
            if (state == TextInputState::Typing) {
                if (event.getKeyCode() == KeyCode::Return) {
                    commitTyping();
                    return true;
                } else if (event.getKeyCode() == KeyCode::Escape) {
                    state = TextInputState::Normal;
                    buffer.clear();
                    return true;
                } else if (event.getKeyCode() == KeyCode::Backspace) {
                    if (!buffer.empty()) {
                        buffer.pop_back();
                    }
                    return true;
                }
            }
            return false;
        });

        dispatcher.dispatch<KeyTypedEvent>([this](const KeyTypedEvent& event) {
            if (state == TextInputState::Typing) {
                char typedChar = event.getKeyChar();
                if (std::isprint(typedChar)) {
                    buffer += typedChar;
                }
                return true;
            }
            return false;
        });

        return UIElement::onEvent(event);
    }

    void UITextInput::commitTyping() {
        if (state != TextInputState::Typing) {
            return;
        }

        if (setValue) {
            setValue(buffer);
        }
        buffer.clear();
        state = TextInputState::Normal;
    }

} // namespace Axiom
