#pragma once
#include "axpch.h"
#include "Event/Event.h"

namespace Axiom {
	struct WindowProps {
		std::string title;
		uint32_t width;
		uint32_t height;

		WindowProps(const std::string& title = "Axiom Engine", uint32_t width = 1280, uint32_t height = 720)
			: title(title), width(width), height(height) {
		}
	};

	class Window {
	public:
		using EventCallback = std::function<void(Event&)>;
		Window() = default;
		virtual ~Window() = default;

		virtual void onUpdate() = 0;
		virtual uint32_t getWidth() const = 0;
		virtual uint32_t getHeight() const = 0;

		virtual void setEventCallback(const EventCallback& callback) = 0;
		virtual void setVSync(bool enabled) = 0;
		virtual bool isVSync() const = 0;

		virtual void* getNativeWindow() const = 0;

		static Window* create(const WindowProps& props = WindowProps());
	private:

	};
}