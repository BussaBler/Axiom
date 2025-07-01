#pragma once
#include "axpch.h"
#include "Event.h"

namespace Axiom {
	class MouseMovedEvent : public Event {
	public:
		MouseMovedEvent(float x, float y) : mouseX(x), mouseY(y) {}

		float getMouseX() const { return mouseX; }
		float getMouseY() const { return mouseY; }
		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseMovedEvent: " << mouseX << ", " << mouseY;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseMoved)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryApplicationInput)
	private:
		float mouseX, mouseY;
	};

	class MouseScrolledEvent : public Event {
	public:
		MouseScrolledEvent(float xOffset, float yOffset) : xOffset(xOffset), yOffset(yOffset) {}

		float getXOffset() const { return xOffset; }
		float getYOffset() const { return yOffset; }
		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseScrolledEvent: " << xOffset << ", " << yOffset;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseScrolled)
		EVENT_CLASS_CATEGORY(EventCategoryMouse | EventCategoryApplicationInput)
	private:
		float xOffset, yOffset;
	};

	class MouseButtonEvent : public Event {
	public:
		int getMouseButton() const { return button; }

	protected:
		MouseButtonEvent(int button) : button(button) {}

	protected:
		int button;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent {
	public:
		MouseButtonPressedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent {
	public:
		MouseButtonReleasedEvent(int button) : MouseButtonEvent(button) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "MouseButtonReleased: " << button;
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}
