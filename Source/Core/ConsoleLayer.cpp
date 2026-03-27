#include "ConsoleLayer.h"
#include "UI/UI.h"

namespace Axiom {
	void ConsoleLayer::onAttach() {
		registerCommand("echo", [this](const std::vector<std::string>& args) {
			std::string output;
			for (const auto& arg : args) {
				output += arg + " ";
			}
			log(output);
			}
		);
		registerCommand("clear", [this](const std::vector<std::string>& args) {
			commandHistory.clear();
			}
		);
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
		if (UI::button("Toggle Console", Math::Vec2(0.0f, 0.0f), Math::Vec2(120.0f, 30.0f))) {
			isOpen = !isOpen;
		}
		UI::text("Open Console", Math::Vec2(2.5f, 20.0f), Math::Vec4(0.0f, 0.0f, 0.0f, 1.0f), 13);

		if (isOpen) {
			float width = Application::getWindow()->getWidth();
			float height = Application::getWindow()->getHeight();
			float consoleHeight = height * consoleHeightRatio;

			UI::button("Console test", Math::Vec2(0.0f, height - consoleHeight), Math::Vec2(width, consoleHeight));
			float cursorY = (height - consoleHeight) + 20.0f;
			UI::text("> " + inputBuffer + "_", Math::Vec2(10.0f, cursorY), Math::Vec4(1.0f, 0.0f, 0.0f, 1.0f), fontSize);

			float lineHeight = fontSize * 1.2f;
			cursorY += lineHeight + 5.0f;

			for (auto it = commandHistory.rbegin(); it != commandHistory.rend(); ++it) {
				if (cursorY > height) break;
				UI::text(*it, Math::Vec2(10.0f, cursorY), Math::Vec4(0.9f, 0.9f, 0.9f, 1.0f), fontSize);
				cursorY += lineHeight;
			}
		}
	}

	void ConsoleLayer::onRender(CommandBuffer* commandBuffer) {

	}

	void ConsoleLayer::registerCommand(const std::string& command, CommandCallback callback) {
		commandRegistry[command] = callback;
	}

	void ConsoleLayer::executeCommand(const std::string& commandLine) {
		std::istringstream iss(commandLine);
		std::string command;
		iss >> command;

		std::vector<std::string> args;
		std::string arg;
		while (iss >> arg) {
			args.push_back(arg);
		}

		auto it = commandRegistry.find(command);
		if (it != commandRegistry.end()) {
			it->second(args);
		} else {
			log("Unknown command: " + command);
		}
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
		}
		else if (event.getKeyCode() == KeyCode::Return) {
			executeCommand(inputBuffer);
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

	void ConsoleLayer::log(const std::string& message) {
		commandHistory.push_back(message);
		if (commandHistory.size() > 100) {
			commandHistory.pop_front();
		}
	}
}
