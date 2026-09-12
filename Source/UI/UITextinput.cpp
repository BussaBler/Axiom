#include "axpch.h"

#include "UITextInput.h"

#include "Event/Event.h"
#include "Event/KeyCodes.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "UI/UIElement.h"

#include <algorithm>
#include <cctype>

namespace Axiom {

    Math::Vec2 UITextInput::getDesiredSize(const UIContext& context) {
        std::string displayText;
        if (currentState == TextInputState::Typing) {
            displayText = typingBuffer + "|";
        } else {
            displayText = valueGetter ? valueGetter() : "";
        }

        float textWidth = context.renderer->calculateTextWidth(displayText, context.theme->fontSize);

        float baseHeight = context.theme->fontSize * 1.8f;
        float minWidth = 100.0f;

        desiredSize.x() = std::max(minWidth, textWidth + (10.0f));
        desiredSize.y() = baseHeight;

        return desiredSize;
    }

    void UITextInput::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color backgroundColor = (isActive || currentState == TextInputState::Typing)
                                    ? context.theme->controlActiveColor
                                    : (isHovered ? context.theme->controlHoverColor : context.theme->controlNormalColor);

        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, context.theme->borderRadius, context.layer);

        std::string displayText;
        if (currentState == TextInputState::Typing) {
            displayText = typingBuffer + "|";
        } else {
            displayText = valueGetter ? valueGetter() : "";
        }

        float textHeight = context.renderer->calculateTextHeight(context.theme->fontSize);
        float textYOffset = (arrangedSize.y() - textHeight) * 0.5f;

        Math::Vec2 textPos = arrangedPosition + Math::Vec2(5.0f, textYOffset);

        context.renderer->addText(displayText, textPos, context.theme->fontSize, context.theme->textPrimary, context.layer);
    }

    bool UITextInput::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        EventDispatcher dispatcher(event);

        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
            if (e.getMouseButton() == KeyCode::LeftButton) {
                if (isHovered) {
                    isActive = true;
                    return true;
                }
            }
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) {
            if (isActive && e.getMouseButton() == KeyCode::LeftButton) {
                isActive = false;
                if (isHovered) {
                    currentState = TextInputState::Typing;
                    typingBuffer = valueGetter ? valueGetter() : "";
                    invalidateLayout();
                }
                return true;
            }
            return false;
        });

        dispatcher.dispatch<KeyPressedEvent>([this](const KeyPressedEvent& e) {
            if (currentState == TextInputState::Typing) {
                if (e.getKeyCode() == KeyCode::Return) {
                    commitTyping();
                    return true;
                } else if (e.getKeyCode() == KeyCode::Escape) {
                    currentState = TextInputState::Normal;
                    typingBuffer.clear();
                    invalidateLayout();
                    return true;
                } else if (e.getKeyCode() == KeyCode::Backspace) {
                    if (!typingBuffer.empty()) {
                        typingBuffer.pop_back();
                        invalidateLayout();
                    }
                    return true;
                }
            }
            return false;
        });

        dispatcher.dispatch<KeyTypedEvent>([this](const KeyTypedEvent& e) {
            if (currentState == TextInputState::Typing) {
                char typedChar = e.getKeyChar();
                if (std::isprint(typedChar)) {
                    typingBuffer.push_back(typedChar);
                    invalidateLayout();
                }
                return true;
            }
            return false;
        });

        return UIElement::onEvent(event);
    }

    void UITextInput::commitTyping() {
        if (currentState != TextInputState::Typing) {
            return;
        }

        if (valueSetter) {
            valueSetter(typingBuffer);
        }
        typingBuffer.clear();
        currentState = TextInputState::Normal;

        invalidateLayout();
    }
} // namespace Axiom
