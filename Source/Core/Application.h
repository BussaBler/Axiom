#pragma once
#include "Asset/AssetManager.h"
#include "Core/EngineContext.h"
#include "Core/TaskManager.h"
#include "ECS/ComponentRegistry.h"
#include "Event/ApplicationEvent.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "LayerStack.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/SceneRenderer.h"
#include "UI/UIRenderer.h"
#include "Window.h"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

namespace Axiom {
    struct RendererConfig {
        // TODO: think this through
    };

    struct ApplicationInfo {
        std::string name;
        std::filesystem::path engineWorkingDirectory;
        std::filesystem::path applicationWorkingDirectory;
        uint32_t width, height;
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
            layerStack.pushLayer<T>(&engineContext);
        }
        template <typename T>
            requires std::derived_from<T, Layer>
        void pushOverlay() {
            layerStack.pushOverlay<T>(&engineContext);
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

        inline static Application* get() { return instance; }

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
        std::unique_ptr<TaskManager> taskManager;
        std::unique_ptr<RenderHardwareInterface> rhi;
        std::unique_ptr<PSOManager> psoManager;
        std::unique_ptr<AssetManager> assetManager;
        std::unique_ptr<ComponentRegistry> componentRegistry;
        std::unique_ptr<UIRenderer> uiRenderer;
        std::unique_ptr<SceneRenderer> sceneRenderer;

        EngineContext engineContext;

        LayerStack layerStack;
        std::vector<LayerAction> layerActionQueue;
        bool isRunning = true;

      private:
        static Application* instance;
    };

    Application* createApplication(int argc, char** argv, const std::filesystem::path& axiomRootDir);
} // namespace Axiom
