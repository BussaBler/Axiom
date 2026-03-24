#include "axpch.h"
#include "Application.h"
#include "UI/UILayer.h"

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

		renderer = std::make_unique<Renderer>(window.get());

		pushOverlay<UILayer>();
	}

	Application::~Application() {

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
			for (const auto& layer : layerStack) {
				layer->onUpdate();
			}
			for (const auto& layer : layerStack) {
				layer->onUIRender();
			}

			CommandBuffer* commandBuffer = renderer->beginFrame();
			for (const auto& layer : layerStack) {
				layer->onRender(commandBuffer);
			}
			renderer->endFrame();

			window->onUpdate();
		}
		renderer->waitIdle();
	}
}
