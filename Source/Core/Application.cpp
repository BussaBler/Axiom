#include "Application.h"

namespace Axiom {
    Application* Application::instance = nullptr;

    Application::Application(const ApplicationInfo& appInfo) {
        Log::init();
        FileSystem::setWorkingDirectory(appInfo.workingDirectory);
        AX_CORE_ASSERT(Log::getCoreLogger()->outputToFile(), "Failed to create log file!");

        AX_CORE_LOG_INFO("Application started: {0}", appInfo.name);
        AX_CORE_LOG_INFO("Current working directory: {0}", FileSystem::getWorkingDirectory().string());

        AX_CORE_ASSERT(!instance, "Application already exists!");
        instance = this;

        window = Window::create(WindowProps());
        window->setEventCallback(std::bind(&Application::onEvent, this, std::placeholders::_1));

        renderer = std::make_unique<Renderer>(window.get());
        uiContext = std::make_unique<UIContext>();
        UI::init(*uiContext);
    }

    Application::~Application() {
    }

    void Application::onEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowCloseEvent>(std::bind(&Application::onWindowClose, this, std::placeholders::_1));
        dispatcher.dispatch<WindowResizeEvent>(std::bind(&Application::onWindowResize, this, std::placeholders::_1));
        UI::onEvent(event);
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
        running = false;
        return true;
    }

    bool Application::onWindowResize(WindowResizeEvent& e) {
        if (e.getWidth() == 0 || e.getHeight() == 0) {
            return false;
        }
        renderer->recreateSwapChain();
        return true;
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
                layerAction.newLayer->onAttach();

                *it = std::move(layerAction.newLayer);
                break;

            case Layer::ActionType::Suspend:
                (*it)->onSuspend();
                (*it)->isSuspended = true;
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
        while (running) {
            window->beginFrame();
            UI::beginFrame();
            for (const auto& layer : layerStack) {
                layer->onUpdate();
            }
            for (const auto& layer : layerStack) {
                layer->onUIRender();
            }

            CommandBuffer* commandBuffer = renderer->beginFrame();
            for (const auto& layer : layerStack) {
                layer->onRender(commandBuffer);
            }
            UI::endFrame();
            UI::render(commandBuffer);
            renderer->endFrame();

            window->onUpdate();
            window->endFrame();
            processLayerActions();
        }
        renderer->waitIdle();
    }
} // namespace Axiom
