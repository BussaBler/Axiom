#pragma once
#include "Core/Window.h"
#include "Event/ApplicationEvent.h"
#include "Core/Assert.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>

namespace Axiom
{
	class XLibWindow : public Window {
	public:
		XLibWindow(const WindowProps &props);
		~XLibWindow();
		void onUpdate() override;

		void setEventCallback(const EventCallback &callback) override { data.eventCallback = callback; }
		void setVSync(bool enabled) override {}
		bool isVSync() const override { return false; }

		void *getNativeWindow() override { return &window; }
		void *getNativeDisplay() const override { return display; }
		uint32_t getWidth() const override { return data.width; }
		uint32_t getHeight() const override { return data.height; }

	private:
		void init(const WindowProps &props);

	private:
		Display *display;
		int screen;
		::Window rootWindow, window;
		XEvent xEvent;

		struct WindowData {
			std::string title;
			uint32_t width, height;
			bool vSync;
			EventCallback eventCallback;
		};
		WindowData data;
	};
}