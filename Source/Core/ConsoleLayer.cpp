#include "ConsoleLayer.h"
#include "Core/Application.h"
#include "UI/UI.h"

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

    void ConsoleLayer::onUIRender() {
        if (isOpen) {
            float width = Application::getWindow()->getWidth();
            float height = Application::getWindow()->getHeight();
            float consoleHeight = height * consoleHeightRatio;

            UI::beginPanel("Console", Math::Vec2(0.0f, height - consoleHeight), Math::Vec2(width, consoleHeight));

            UI::text("> " + inputBuffer + "_", Color::white(), fontSize);

            auto commandHistory = CommandRegistry::getCommandHistory();
            for (auto it = commandHistory.rbegin(); it != commandHistory.rend(); ++it) {
                UI::text(*it, Color::white(), fontSize);
            }
            UI::endPanel();
        }
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
