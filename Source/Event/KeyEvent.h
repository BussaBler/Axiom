#pragma once
#include "axpch.h"
#include "Event.h"

namespace Axiom {
	class KeyEvent : public Event {
	public:
		int getKeyCode() const { return keycode; }
		EVENT_CLASS_CATEGORY(EventCategoryKeyboard | EventCategoryApplicationInput)
	protected:
		KeyEvent(int keycode) : keycode(keycode) {};
	protected:
		int keycode;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(int keycode, int repeatCount) : KeyEvent(keycode), repeatCount(repeatCount) {}

		int getRepeatCount() const { return repeatCount; }
		std::string toString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << keycode << " (" << repeatCount << " repeats)";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int repeatCount;
	};

	class KeyReleassedEvent : public KeyEvent {
	public:
		KeyReleassedEvent(int keycode) : KeyEvent(keycode) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << keycode;
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	private:

	};
}
