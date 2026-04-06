#pragma once
#include "Core/Log.h"
#include "Core/Window.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include <Cocoa/Cocoa.h>

#if defined(__OBJC__)
@class NSWindow;
#else
typedef struct objc_object NSWindow;
#endif

namespace Axiom {
    class MacOSWindow : public Window {
      public:
        MacOSWindow(const WindowProps& props);
        ~MacOSWindow();

        void onUpdate() override;
        uint32_t getWidth() const override {
            return data.width;
        }
        uint32_t getHeight() const override {
            return data.height;
        }
        void setEventCallback(const EventCallback& callback) override {
            data.eventCallback = callback;
        }

        void setVSync(bool enabled) override;
        bool isVSync() const override {
            return data.vSync;
        }

        void* getNativeWindow() override {
            return window;
        }
        void* getNativeDisplay() const override {
            return nullptr;
        }
        uint32_t getWindowDPI() const override;

        void attachMetalLayer(void* metalLayer);

      private:
        void init(const WindowProps& props);
        void shutdown();

      private:
        NSWindow* window;
        struct WindowData {
            std::string title;
            uint32_t width, height;
            bool vSync;
            EventCallback eventCallback;
        };
        WindowData data;
    };
} // namespace Axiom