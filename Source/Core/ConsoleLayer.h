#pragma once
#include "axpch.h"
#include "Layer.h"
#include "Event/KeyEvent.h"

namespace Axiom {
	class ConsoleLayer : public Layer {
	public:
		ConsoleLayer() : Layer("ConsoleLayer") {}
		~ConsoleLayer() = default;

		void onAttach() override;
		void onDetach() override;
		void onUpdate() override;
		void onEvent(Event& event) override;
		void onUIRender() override;
		void onRender(CommandBuffer* commandBuffer) override;

		void log(const std::string& message);
		using CommandCallback = std::function<void(const std::vector<std::string>& args)>;
		void registerCommand(const std::string& command, CommandCallback callback);
		void executeCommand(const std::string& commandLine);

	private:
		bool onKeyPressed(KeyPressedEvent& event);
		bool onKeyTyped(KeyTypedEvent& event);

	private:
		bool isOpen = false;
		std::string inputBuffer;
		std::deque<std::string> commandHistory;
		std::unordered_map<std::string, CommandCallback> commandRegistry;

		float consoleHeightRatio = 0.5f;
		uint16_t fontSize = 16;
	};
}
