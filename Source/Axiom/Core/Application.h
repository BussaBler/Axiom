#pragma once
#include "Assert.h"
#include "Event/ApplicationEvent.h"
#include "Input.h"
#include "LayerStack.h"
#include "Log.h"
#include "Renderer/GraphicsDevice.h"
#include "Utils/FileSystem.h"
#include "Window.h"

namespace Axiom {
	struct ApplicationInfo {
		std::string name;
		std::filesystem::path workingDirectory;
	};

	class Application {
	public:
		Application(const ApplicationInfo& appInfo);
		virtual ~Application() = default;

		void run();

		void onEvent(Event& event);

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* overlay);

		static Application& get() { return *instance; }
		Window& getWindow() const { return *window.get(); }

	private:
		bool onWindowClose(WindowCloseEvent& e);

	private:
		std::unique_ptr<Window> window;
		std::unique_ptr<GraphicsDevice> graphicsDevice;
		bool running = true;
		LayerStack layerStack;

	private:
		static Application* instance; 
	};

	Application* createApplication(const ApplicationInfo& appInfo);
}

