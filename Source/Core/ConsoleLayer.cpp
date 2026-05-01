#include "ConsoleLayer.h"
#include "Core/Application.h"

namespace Axiom {
    void ConsoleLayer::onAttach() {
    }

    void ConsoleLayer::onDetach() {
    }

    void ConsoleLayer::onUpdate() {
    }

    void ConsoleLayer::onEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<KeyPressedEvent>(std::bind(&ConsoleLayer::onKeyPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyTypedEvent>(std::bind(&ConsoleLayer::onKeyTyped, this, std::placeholders::_1));
    }

    void ConsoleLayer::onUIRender(UIRenderer* uiRenderer) {
    }

    void ConsoleLayer::onRender(CommandBuffer* commandBuffer) {
    }

    bool ConsoleLayer::onKeyPressed(KeyPressedEvent& event) {
        if (event.getKeyCode() == KeyCode::Grave) {
            isOpen = !isOpen;
            return true;
        }

        if (!isOpen) {
            return false;
        }

        if (event.getKeyCode() == KeyCode::Backspace && !inputBuffer.empty()) {
            inputBuffer.pop_back();
        } else if (event.getKeyCode() == KeyCode::Return) {
            CommandRegistry::executeCommand(inputBuffer);
            inputBuffer.clear();
        }

        return true;
    }

    bool ConsoleLayer::onKeyTyped(KeyTypedEvent& event) {
        char c = event.getKeyChar();

        if (c >= 32 && c <= 126 && c != '`') {
            inputBuffer += c;
        }

        return true;
    }
} // namespace Axiom
