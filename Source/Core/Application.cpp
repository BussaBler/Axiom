#include "axpch.h"

#include "Application.h"

#include "Asset/AssetManager.h"
#include "Core/Input.h"
#include "Core/TaskManager.h"
#include "Profiler.h"
#include "Renderer/PSOManager.h"
#include "Renderer/SceneRenderer.h"
#include "Utils/FileSystem.h"

#include <memory>

namespace Axiom {
    Application* Application::instance = nullptr;

    Application::Application(const ApplicationInfo& appInfo) {
        Log::init();

        AX_CORE_ASSERT(!instance, "Application already exists!");
        instance = this;
        AX_CORE_LOG_INFO("Application started: {0}", appInfo.name);

        AX_CORE_LOG_INFO("Current working directory: {0}", FileSystem::getWorkingDirectory().string());
        FileSystem::mount("axiom://", appInfo.engineWorkingDirectory);
        FileSystem::mount("app://", appInfo.applicationWorkingDirectory);
        bool logFileOutput = Log::getCoreLogger()->outputToFile();
        AX_CORE_ASSERT(logFileOutput, "Failed to create log file!");

        WindowProps windowProps{appInfo.name, appInfo.width, appInfo.height};
        window = Window::create(windowProps);
        window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

        taskManager = std::make_unique<TaskManager>();

        rhi = std::make_unique<RenderHardwareInterface>(window.get());
        psoManager = std::make_unique<PSOManager>(*rhi);

        assetManager = std::make_unique<AssetManager>(*rhi, *taskManager);

        componentRegistry = std::make_unique<ComponentRegistry>();
        componentRegistry->registerReflectedComponents();

        uiRenderer = std::make_unique<UIRenderer>(*rhi, *window, *assetManager, *taskManager);
        sceneRenderer = std::make_unique<SceneRenderer>(*rhi, *psoManager, *assetManager);

        engineContext = {window.get(), taskManager.get(), rhi.get(), assetManager.get(), componentRegistry.get(), uiRenderer.get(), sceneRenderer.get()};
    }

    Application::~Application() {
    }

    void Application::onEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
        dispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::onWindowResize, this, std::placeholders::_1));

        dispatcher.dispatch<KeyPressedEvent>(std::bind(&Application::onKeyPressed, this, std::placeholders::_1));
        dispatcher.dispatch<KeyReleasedEvent>(std::bind(&Application::onKeyReleased, this, std::placeholders::_1));
        dispatcher.dispatch<MouseButtonPressedEvent>(std::bind(&Application::onMouseButtonPressed, this, std::placeholders::_1));
        dispatcher.dispatch<MouseButtonReleasedEvent>(std::bind(&Application::onMouseButtonReleased, this, std::placeholders::_1));
        dispatcher.dispatch<MouseMovedEvent>(std::bind(&Application::onMouseMoved, this, std::placeholders::_1));

        for (auto it = layerStack.rbegin(); it != layerStack.rend(); it++) {
            if (event.isHandled()) {
                break;
            }
            (*it)->onEvent(event);
        }
    }

    void Application::queueLayerAction(Layer* requester, std::unique_ptr<Layer> newLayer, Layer::ActionType action) {
        layerActionQueue.emplace_back(LayerAction{requester, std::move(newLayer), action});
    }

    bool Application::onWindowClose(WindowCloseEvent& e) {
        isRunning = false;
        return true;
    }

    bool Application::onWindowResize(WindowResizeEvent& e) {
        if (e.getWidth() == 0 || e.getHeight() == 0) {
            return false;
        }
        rhi->recreateSwapChain();
        return true;
    }

    bool Application::onKeyPressed(KeyPressedEvent& e) {
        Input::setKeyPressed(e.getKeyCode(), true);
        return false;
    }

    bool Application::onKeyReleased(KeyReleasedEvent& e) {
        Input::setKeyPressed(e.getKeyCode(), false);
        return false;
    }

    bool Application::onMouseButtonPressed(MouseButtonPressedEvent& e) {
        Input::setMouseButtonPressed(e.getMouseButton(), true);
        return false;
    }

    bool Application::onMouseButtonReleased(MouseButtonReleasedEvent& e) {
        Input::setMouseButtonPressed(e.getMouseButton(), false);
        return false;
    }

    bool Application::onMouseMoved(MouseMovedEvent& e) {
        Input::setMousePosition(e.getMouseX(), e.getMouseY());
        return false;
    }

    void Application::processLayerActions() {
        for (auto& layerAction : layerActionQueue) {

            auto it = std::find_if(layerStack.begin(), layerStack.end(),
                                   [&layerAction](const std::unique_ptr<Layer>& layerPtr) { return layerPtr.get() == layerAction.requester; });

            if (it == layerStack.end()) {
                AX_CORE_LOG_ERROR("Layer action requester not found in layer stack!");
                continue;
            }

            switch (layerAction.action) {
            case Layer::ActionType::Transition:
                (*it)->onDetach();
                layerAction.newLayer->engineContext = &engineContext;
                layerAction.newLayer->onAttach();

                *it = std::move(layerAction.newLayer);
                break;

            case Layer::ActionType::Suspend:
                (*it)->onSuspend();
                (*it)->isSuspended = true;
                layerAction.newLayer->engineContext = &engineContext;
                layerAction.newLayer->onAttach();

                layerStack.insertLayer(it + 1, std::move(layerAction.newLayer));
                break;

            case Layer::ActionType::Pop:
                (*it)->onDetach();

                if (it != layerStack.begin()) {
                    auto prevIt = std::prev(it);
                    (*prevIt)->isSuspended = false;
                    (*prevIt)->onResume();
                }
                layerStack.eraseLayer(it);
                break;
            }
        }

        layerActionQueue.clear();
    }

    void Application::run() {
        while (isRunning) {
            Profiler::beginFrame();
            window->beginFrame();
            for (const auto& layer : layerStack) {
                layer->onUpdate();
            }

            taskManager->executeTasks();

            uiRenderer->beginFrame();
            for (const auto& layer : layerStack) {
                // TODO: add a delta time input parameter
                layer->onUIRender();
            }

            CommandBuffer* commandBuffer = rhi->beginFrame();
            if (commandBuffer) {
                sceneRenderer->beginFrame();
                for (const auto& layer : layerStack) {
                    layer->onRender();
                }
                // TODO: add the delta time
                sceneRenderer->execute(commandBuffer);
                uiRenderer->onRender(commandBuffer, rhi->getCurrentRenderTarget());
                rhi->endFrame();
            }

            window->onUpdate();
            window->endFrame();
            processLayerActions();
        }
        rhi->waitIdle();
    }
} // namespace Axiom
