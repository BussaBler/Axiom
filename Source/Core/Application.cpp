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
		renderer = std::make_unique<Renderer>();
		renderer->init(window.get());
	}

	Application::~Application() {
		renderer->shutdown();
	}

	void Application::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		//AX_CORE_LOG_TRACE("Event: {0}", event.toString());
		dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
		dispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::onWindowResize, this, std::placeholders::_1));

		for (Layer* layer : layerStack) {
			if (event.isHandled()) {
				break;
			}
			layer->onEvent(event);
		}
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

	bool Application::onWindowResize(WindowResizeEvent& e) {
		if (e.getWidth() == 0 || e.getHeight() == 0) {
			return false;
		}
		renderer->resize(e.getWidth(), e.getHeight());
		return true;
	}

	void Application::run() {
		while (running)
		{
			for (Layer* layer : layerStack) {
				layer->onUpdate();
			}
			renderer->draw();
			window->onUpdate();
		}
	}
}
