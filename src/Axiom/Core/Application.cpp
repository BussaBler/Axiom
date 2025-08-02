#include "axpch.h"
#include "Application.h"

namespace Axiom {
	Application* Application::instance = nullptr;

	Application::Application() {
		AX_CORE_ASSERT(!instance, "Application already exists!");
		instance = this;

		window = std::unique_ptr<Window>(Window::create(WindowPlatform::Windows, { "Axiom", 1280, 720 }));
		window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

		//graphicsDevice = std::unique_ptr<GraphicsDevice>(GraphicsDevice::create(window.get(), GraphicsAPI::Vulkan));
	}

	void Application::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		AX_CORE_LOG_TRACE("Event: {0}", event.toString());
		dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
	}

	void Application::pushLayer(Layer* layer) {
		layerStack.pushLayer(layer);
	}

	void Application::pushOverlay(Layer* overlay) {
		layerStack.pushOverlay(overlay);
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		running = false;
		return true;
	}

	void Application::run() {
		while (running)
		{
			for (Layer* layer : layerStack) {
				layer->onUpdate();
			}
			window->onUpdate();
		}
	}
}
