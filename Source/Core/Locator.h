#pragma once

namespace Axiom {
    class Window;
    class Renderer;
    class UIRenderer;

    class Locator {
        friend class Application;

      public:
        inline static Renderer* getRenderer() { return renderer; }
        inline static UIRenderer* getUIRenderer() { return uiRenderer; }
        inline static Window* getWindow() { return window; }

      private:
        inline static void provideRenderer(Renderer* r) { renderer = r; }
        inline static void provideUIRenderer(UIRenderer* r) { uiRenderer = r; }
        inline static void provideWindow(Window* w) { window = w; }

      private:
        inline static Renderer* renderer = nullptr;
        inline static UIRenderer* uiRenderer = nullptr;
        inline static Window* window = nullptr;
    };
} // namespace Axiom