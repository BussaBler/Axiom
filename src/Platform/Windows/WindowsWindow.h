#pragma once
#include "Core/Assert.h"
#include "Core/Log.h"
#include "Core/Window.h"
#include "Event/ApplicationEvent.h"
#include <GLFW/glfw3.h>

namespace Axiom {
	class WindowsWindow : public Window {
	public:
		WindowsWindow(const WindowProps& props);
		~WindowsWindow();

		void onUpdate() override;
		uint32_t getWidth() const override { return data.width; }
		uint32_t getHeight() const override { return data.height; }

		void setEventCallback(const EventCallback& callback) override { data.eventCallback = callback; }
		void setVSync(bool enabled) override;
		bool isVSync() const override;

	private:
		virtual void init(const WindowProps& props);
		virtual void shutdown();

	private:
		GLFWwindow* window;
		struct WindowData {
			std::string title;
			uint32_t width, height;
			bool vSync;
			EventCallback eventCallback;
		};
		WindowData data;
	};
}

