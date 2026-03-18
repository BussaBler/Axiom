#pragma once
#include "axpch.h"
#include "Event.h"
#include "KeyCodes.h"

namespace Axiom {
	class KeyEvent : public Event {
	public:
		KeyCode getKeyCode() const { return keycode; }
		EVENT_CLASS_CATEGORY(EventCategory::EventCategoryKeyboard | EventCategory::EventCategoryApplicationInput)
	protected:
		KeyEvent(KeyCode keycode) : keycode(keycode) {};
	protected:
		KeyCode keycode;
	};

	class KeyPressedEvent : public KeyEvent {
	public:
		KeyPressedEvent(KeyCode keycode, int repeatCount) : KeyEvent(keycode), repeatCount(repeatCount) {}

		int getRepeatCount() const { return repeatCount; }
		std::string toString() const override {
			std::stringstream ss;
			ss << "KeyPressedEvent: " << static_cast<uint32_t>(keycode) << " (" << repeatCount << " repeats)";
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyPressed)
	private:
		int repeatCount;
	};

	class KeyReleassedEvent : public KeyEvent {
	public:
		KeyReleassedEvent(KeyCode keycode) : KeyEvent(keycode) {}

		std::string toString() const override {
			std::stringstream ss;
			ss << "KeyReleasedEvent: " << static_cast<uint32_t>(keycode);
			return ss.str();
		}
		EVENT_CLASS_TYPE(KeyReleased)
	private:

	};
}
