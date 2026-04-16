#pragma once
#include "Event.h"
#include "KeyCodes.h"
#include "axpch.h"

namespace Axiom {
    class KeyEvent : public Event {
      public:
        KeyCode getKeyCode() const {
            return keycode;
        }
        EVENT_CLASS_CATEGORY(EventCategory::EventCategoryKeyboard | EventCategory::EventCategoryApplicationInput)
      protected:
        KeyEvent(KeyCode keycode) : keycode(keycode) {};

      protected:
        KeyCode keycode;
    };

    class KeyPressedEvent : public KeyEvent {
      public:
        KeyPressedEvent(KeyCode keycode, int repeatCount) : KeyEvent(keycode), repeatCount(repeatCount) {
        }

        int getRepeatCount() const {
            return repeatCount;
        }
        std::string toString() const override {
            std::stringstream ss;
            ss << "KeyPressedEvent: " << static_cast<uint32_t>(keycode) << " (" << repeatCount << " repeats)";
            return ss.str();
        }
        EVENT_CLASS_TYPE(KeyPressed)
      private:
        int repeatCount;
    };

    class KeyReleasedEvent : public KeyEvent {
      public:
        KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {
        }

        std::string toString() const override {
            std::stringstream ss;
            ss << "KeyReleasedEvent: " << static_cast<uint32_t>(keycode);
            return ss.str();
        }
        EVENT_CLASS_TYPE(KeyReleased)
      private:
    };

    class KeyTypedEvent : public KeyEvent {
      public:
        KeyTypedEvent(char keyChar) : KeyEvent(KeyCode::None), keyChar(keyChar) {
        }

        std::string toString() const override {
            std::stringstream ss;
            ss << "KeyTypedEvent: " << keyChar;
            return ss.str();
        }
        EVENT_CLASS_TYPE(KeyTyped)

        char getKeyChar() const {
            return keyChar;
        }

      private:
        char keyChar;
    };
} // namespace Axiom
