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

		Renderer::init(window.get());

		uiLayer = new UILayer();
		pushOverlay(uiLayer);
	}

	Application::~Application() {
		popOverlay(uiLayer);
		delete uiLayer;
		Renderer::shutdown();
	}

	void Application::onEvent(Event& event) {
		EventDispatcher dispatcher(event);
		dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
		dispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::onWindowResize, this, std::placeholders::_1));

		for (auto it = layerStack.rbegin(); it != layerStack.rend(); it++) {
			if (event.isHandled()) {
				break;
			}
			(*it)->onEvent(event);
		}
	}

	void Application::pushLayer(Layer* layer) {
		layerStack.pushLayer(layer);
	}

	void Application::pushOverlay(Layer* overlay) {
		layerStack.pushOverlay(overlay);
	}

	void Application::popLayer(Layer* layer) {
		layerStack.popLayer(layer);
	}

	void Application::popOverlay(Layer* overlay) {
		layerStack.popOverlay(overlay);
	}

	bool Application::onWindowClose(WindowCloseEvent& e) {
		running = false;
		return true;
	}

	bool Application::onWindowResize(WindowResizeEvent& e) {
		if (e.getWidth() == 0 || e.getHeight() == 0) {
			return false;
		}
		return true;
	}

	void Application::run() {
		while (running)
		{
			for (Layer* layer : layerStack) {
				layer->onUpdate();
			}
			uiLayer->begin();
			for (Layer* layer : layerStack) {
				layer->onUIRender();
			}
			uiLayer->end();

			CommandBuffer* commandBuffer = Renderer::beginFrame();
			for (Layer* layer : layerStack) {
				layer->onRender(commandBuffer);
			}
			Renderer::endFrame();

			window->onUpdate();
		}
		Renderer::waitIdle();
	}
}
