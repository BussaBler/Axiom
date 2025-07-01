#include "axpch.h"
#include "Application.h"

namespace Axiom {
	Application::Application() {
		Log::init();
		window = std::unique_ptr<Window>(Window::create());
		window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));
	}

	void Application::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		running = false;
		return true;
	}

	void Application::run() {
		while (running)
		{
			window->onUpdate();
		}
	}
}
