#include "axpch.h"
#include "Application.h"

namespace Axiom {
	Application* Application::instance = nullptr;

	Application::Application(const ApplicationInfo& appInfo) {
		AX_CORE_ASSERT(!instance, "Application already exists!");
		instance = this;
		
		FileSystem::setWorkingDirectory(appInfo.workingDirectory);
		AX_CORE_LOG_INFO("Application started: {0}", appInfo.name);
		AX_CORE_LOG_INFO("Current working directory: {0}", FileSystem::getWorkingDirectory().string());

		window = Window::create(WindowProps());
		window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

		graphicsDevice = GraphicsDevice::create(window.get(), GraphicsAPI::Vulkan);
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
