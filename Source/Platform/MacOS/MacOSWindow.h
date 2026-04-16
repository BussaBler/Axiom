#pragma once
#include "Core/Log.h"
#include "Core/Window.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"

#if defined(__OBJC__)
@class NSWindow;
@class MacOSWindowDelegate;
@class MacOSWindowView;
@class NSAutoreleasePool;
#else
typedef struct objc_object NSWindow;
typedef struct objc_object MacOSWindowDelegate;
typedef struct objc_object MacOSWindowView;
typedef struct objc_object NSAutoreleasePool;
#endif

namespace Axiom {
    class MacOSWindow : public Window {
      public:
        MacOSWindow(const WindowProps& props);
        ~MacOSWindow();

        void beginFrame() override;
        void endFrame() override;
        void onUpdate() override;
        uint32_t getWidth() const override { return data.width; }
        uint32_t getHeight() const override { return data.height; }
        uint32_t getFramebufferWidth() const override { return data.framebufferWidth; }
        uint32_t getFramebufferHeight() const override { return data.framebufferHeight; }
        void setEventCallback(const EventCallback& callback) override { data.eventCallback = callback; }

        void setVSync(bool enabled) override;
        bool isVSync() const override { return data.vSync; }

        void* getNativeWindow() override { return window; }
        void* getNativeDisplay() const override { return nullptr; }
        uint32_t getWindowDPI() const override;

        void attachMetalLayer(void* metalLayer);

      private:
        void init(const WindowProps& props);
        void shutdown();

      private:
        NSWindow* window = nullptr;
        MacOSWindowDelegate* delegate = nullptr;
        MacOSWindowView* view = nullptr;
        WindowData data;
        NSAutoreleasePool* autoreleasePool = nullptr;
    };
} // namespace Axiom