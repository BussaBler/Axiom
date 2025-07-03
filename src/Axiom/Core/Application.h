#pragma once
#include "Event/ApplicationEvent.h"
#include "LayerStack.h"
#include "Log.h"
#include "Window.h"

namespace Axiom {
	class Application {
	public:
		Application();
		virtual ~Application() = default;

		void run();

		void onEvent(Event& event);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);

	private:
		bool onWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> window;
		bool running = true;
		LayerStack layerStack;
	};

	Application* createApplication();
}

