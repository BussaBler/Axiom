#pragma once
#include "Core/Window.h"
#include <X11/Xlib.h>

namespace Axiom {
	class WaylandWindow : public Window {
	public:
		WaylandWindow(const WindowProps& props);
		~WaylandWindow();
		void onUpdate() override;

		void setEventCallback(const EventCallback& callback) override {}
		void setVSync(bool enabled) override {}
		bool isVSync() const override { return false; }

		void* getNativeWindow() const override { return nullptr; }
		void* getNativeDisplay() const override { return nullptr; }
		uint32_t getWidth() const override { return 0; }
		uint32_t getHeight() const override { return 0; }

	private:
		void init(const WindowProps& props);

	private:
		Display* display;
		int screen;
		::Window rootWindow, window;
		XEvent xEvent;

		struct WindowData {
			std::string title;
			uint32_t width, height;
			bool vSync;
			EventCallback eventCallback;
		};
	};
}