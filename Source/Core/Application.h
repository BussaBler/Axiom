#pragma once
#include "Assert.h"
#include "Asset/AssetManager.h"
#include "ECS/Scene.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Input.h"
#include "LayerStack.h"
#include "Log.h"
#include "Math/AxMath.h"
#include "Renderer/Renderer.h"
#include "UI/UI.h"
#include "Utils/FileSystem.h"
#include "Window.h"
#include "axpch.h"

namespace Axiom {
    struct ApplicationInfo {
        std::string name;
        std::filesystem::path workingDirectory;
    };

    class Application {
      public:
        Application(const ApplicationInfo& appInfo);
        virtual ~Application();

        void run();
        void onEvent(Event& event);

        template <typename T>
            requires std::derived_from<T, Layer>
        void pushLayer() {
            layerStack.pushLayer<T>();
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void pushOverlay() {
            layerStack.pushOverlay<T>();
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void popLayer() {
            layerStack.popLayer<T>();
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void popOverlay() {
            layerStack.popOverlay<T>();
        }

        void queueLayerAction(Layer* requester, std::unique_ptr<Layer> newLayer, Layer::ActionType action);

        static Application* get() { return instance; }
        static Renderer* getRenderer() { return instance->renderer.get(); }
        static Window* getWindow() { return instance->window.get(); }

      private:
        bool onWindowClose(WindowCloseEvent& e);
        bool onWindowResize(WindowResizeEvent& e);
        bool onKeyPressed(KeyPressedEvent& e);
        bool onKeyReleased(KeyReleasedEvent& e);
        bool onMouseButtonPressed(MouseButtonPressedEvent& e);
        bool onMouseButtonReleased(MouseButtonReleasedEvent& e);
        bool onMouseMoved(MouseMovedEvent& e);

        void processLayerActions();

      private:
        struct LayerAction {
            Layer* requester;
            std::unique_ptr<Layer> newLayer;
            Layer::ActionType action;
        };

      private:
        std::unique_ptr<Window> window;
        std::unique_ptr<Renderer> renderer;
        bool running = true;
        LayerStack layerStack;
        std::vector<LayerAction> layerActionQueue;
        std::unique_ptr<UIContext> uiContext;

      private:
        static Application* instance;
    };

    Application* createApplication(const ApplicationInfo& appInfo);
} // namespace Axiom
