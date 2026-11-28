#include "axpch.h"

#include "UITextInput.h"

#include "Event/KeyCodes.h"
#include "UI/UICanvas.h"
#include "UI/UIElement.h"

#include <algorithm>
#include <cctype>

namespace Axiom {

    Math::Vec2 UITextInput::measure(const UIContext& context) {
        std::string displayText;
        if (currentState == TextInputState::Typing) {
            displayText = typingBuffer + "|";
        } else {
            displayText = valueGetter ? valueGetter() : "";
        }

        float textWidth = context.renderer->calculateTextWidth(displayText, context.theme->fontSize);

        const float minWidth = 100.0f;
        desiredSize.x() = std::max(minWidth, textWidth + context.theme->panelPadding.left + context.theme->panelPadding.right);
        desiredSize.y() = context.theme->defaultRowHeight;

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

        const float textYOffset = context.renderer->getCenteredTextOffset(arrangedSize.y(), context.theme->fontSize);
        Math::Vec2 textPos = arrangedPosition + Math::Vec2(context.theme->panelPadding.left, textYOffset);

        context.renderer->addText(displayText, textPos, context.theme->fontSize, context.theme->textPrimary, context.layer);
    }

    bool UITextInput::onMouseDown(const MouseButtonPressedEvent& event) {
        return event.getMouseButton() == KeyCode::LeftButton;
    }

    bool UITextInput::onKeyDown(const KeyPressedEvent& event) {
        switch (event.getKeyCode()) {
        case KeyCode::Backspace:
            if (!typingBuffer.empty()) {
                typingBuffer.pop_back();
            }
            return true;
        case KeyCode::Return:
            getParentCanvas()->clearFocus();
            return true;
        case KeyCode::Escape:
            typingBuffer = valueGetter ? valueGetter() : "";
            getParentCanvas()->clearFocus();
            return true;
        default:
            return false;
        }
    }

    bool UITextInput::onTextInput(const KeyTypedEvent& event) {
        typingBuffer += event.getKeyChar();
        return true;
    }

    void UITextInput::onFocusChanged(bool focused) {
        UIElement::onFocusChanged(focused);
        if (focused) {
            typingBuffer = valueGetter ? valueGetter() : "";
            currentState = TextInputState::Typing;
            invalidateLayout();
        } else {
            commitTyping();
        }
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
