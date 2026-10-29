#pragma once

namespace Axiom {
    class Window;
    class RenderHardwareInterface;
    class UIRenderer;
    class TaskManager;

    class Locator {
        friend class Application;

      public:
        inline static RenderHardwareInterface* getRHI() { return rhi; }
        inline static UIRenderer* getUIRenderer() { return uiRenderer; }
        inline static Window* getWindow() { return window; }
        inline static TaskManager* getTaskManager() { return taskManager; }

      private:
        inline static void provideRenderer(RenderHardwareInterface* r) { rhi = r; }
        inline static void provideUIRenderer(UIRenderer* r) { uiRenderer = r; }
        inline static void provideWindow(Window* w) { window = w; }
        inline static void provideTaskManager(TaskManager* t) { taskManager = t; }

      private:
        inline static RenderHardwareInterface* rhi = nullptr;
        inline static UIRenderer* uiRenderer = nullptr;
        inline static Window* window = nullptr;
        inline static TaskManager* taskManager = nullptr;
    };
} // namespace Axiom
