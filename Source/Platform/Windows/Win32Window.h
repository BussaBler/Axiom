#pragma once
#include "Core/Window.h"
#include "Core/Log.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#define UNICODE
#define _UNICODE
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Axiom {
	class Win32Window : public Window {
	public:
		Win32Window(const WindowProps& props);
		~Win32Window();

		void onUpdate() override;

		void setEventCallback(const EventCallback& callback) override { data.eventCallback = callback; }
		void setVSync(bool enabled) override;
		bool isVSync() const override { return data.vSync; }

		void* getNativeWindow() override { return window; }
		void* getNativeDisplay() const override { return hInstance; }
		uint32_t getWidth() const override { return data.width; }
		uint32_t getHeight() const override { return data.height; }

	private:
		void init(const WindowProps& props);
		void shutdown();
		void processMessages();

		static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	private:
		HINSTANCE hInstance;
		HWND window;

		struct WindowData {
			std::string title;
			uint32_t width, height;
			bool vSync;
			EventCallback eventCallback;
		};
		WindowData data;
	};
}

