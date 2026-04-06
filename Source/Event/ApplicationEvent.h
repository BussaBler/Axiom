#pragma once
#include "Event.h"
#include "axpch.h"

namespace Axiom {
    class WindowResizeEvent : public Event {
      public:
        WindowResizeEvent(uint32_t width, uint32_t height) : width(width), height(height) {
        }

        uint32_t getWidth() const {
            return width;
        }
        uint32_t getHeight() const {
            return height;
        }

        std::string toString() const override {
            std::stringstream ss;
            ss << "WindowResizeEvent: " << width << ", " << height;
            return ss.str();
        }

        EVENT_CLASS_TYPE(WindowResize)
        EVENT_CLASS_CATEGORY(EventCategory::EventCategoryApplication)
      private:
        uint32_t width, height;
    };

    class WindowCloseEvent : public Event {
      public:
        WindowCloseEvent() = default;

        EVENT_CLASS_TYPE(WindowClose)
        EVENT_CLASS_CATEGORY(EventCategory::EventCategoryApplication)
    };
} // namespace Axiom
