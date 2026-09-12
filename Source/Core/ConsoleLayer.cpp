#include "axpch.h"

#include "ConsoleLayer.h"

#include "CommandRegistry.h"
#include "Renderer/RenderGraph.h"
#include "UI/UIPanel.h"
#include "UI/UISlot.h"
#include "UI/UIText.h"
#include "UI/UITextInput.h"
#include "UI/UIVerticalBox.h"
#include "Window.h"

namespace Axiom {
    void ConsoleLayer::onAttach() {
        AX_CORE_LOG_DEBUG("ConsoleLayer attached");
        mainUiContext = {
            .renderer = Locator::getUIRenderer(),
            .layer = 0,
        };
        uiRoot = std::make_shared<UICanvas>();

        auto consolePanel = std::make_shared<UIPanel>();
        Color panelColor = Color::darkGray();
        panelColor.a() = 0.8f;
        consolePanel->setBackgroundColor(panelColor);

        auto verticalBox = std::make_shared<UIVerticalBox>();

        auto consoleText = std::make_shared<UIText>("Console");
        verticalBox->addSlot(consoleText)
            .setMargin({0.0f, 0.0f, 0.0f, 5.0f})
            .setHorizontalAlignment(UIAlignment::Start)
            .setVerticalAlignment(UIAlignment::Start);

        consoleScrollBox = std::make_shared<UIScrollBox>();
        verticalBox->addSlot(consoleScrollBox).setHorizontalAlignment(UIAlignment::Fill).setVerticalAlignment(UIAlignment::Fill);

        auto consoleInput = std::make_shared<UITextInput>();
        consoleInput->setValueGetter([this]() { return ""; }).setValueSetter([this](const std::string& input) {
            consoleInputBuffer = input;
            CommandRegistry::executeCommand(input);
            consoleInputBuffer.clear();
            shouldRefreshHistory = true;
        });
        verticalBox->addSlot(consoleInput).setHorizontalAlignment(UIAlignment::Fill).setVerticalAlignment(UIAlignment::Start);

        consolePanel->addSlot(verticalBox).setHorizontalAlignment(UIAlignment::Fill).setVerticalAlignment(UIAlignment::Fill);
        uiRoot->addSlot(consolePanel).setHorizontalAlignment(UIAlignment::Fill).setVerticalAlignment(UIAlignment::Fill);
    }

    void ConsoleLayer::onDetach() {
    }

    void ConsoleLayer::onUpdate() {
        if (isOpen) {
            Math::Vec2 winSize = Math::Vec2(Locator::getWindow()->getWidth(), Locator::getWindow()->getHeight());
            Math::Vec2 consolePos = Math::Vec2(0.0f, winSize.y() * (1.0f - CONSOLE_HEIGHT_RATIO));
            Math::Vec2 consoleSize = Math::Vec2(winSize.x(), winSize.y() * CONSOLE_HEIGHT_RATIO);

            uiRoot->updateLayout(mainUiContext, consolePos, consoleSize);
        }
        if (shouldRefreshHistory) {
            refreshConsoleHistory();
            shouldRefreshHistory = false;
        }
    }

    void ConsoleLayer::onEvent(Event& event) {
        EventDispatcher dispatcher(event);

        dispatcher.dispatch<KeyPressedEvent>(std::bind(&ConsoleLayer::onKeyPressed, this, std::placeholders::_1));

        if (isOpen && !event.isHandled()) {
            event.handled |= uiRoot->onEvent(event);
            if ((event.getCategoryFlags() & EventCategory::EventCategoryApplicationInput) != EventCategory::Empty) {
                event.handled = true;
            }
        }
    }

    void ConsoleLayer::onUIRender() {
        if (isOpen) {
            Math::Rect logicalScreen({0, 0}, {Locator::getWindow()->getWidth(), Locator::getWindow()->getHeight()});
            mainUiContext.renderer->pushScissorRect(logicalScreen, mainUiContext.layer);
            uiRoot->onRender(mainUiContext, logicalScreen);
            mainUiContext.renderer->popScissorRect(mainUiContext.layer);
        }
    }

    void ConsoleLayer::onRender(RenderGraph& rendeGraph) {
    }

    void ConsoleLayer::refreshConsoleHistory() {
        consoleScrollBox->clearSlots();
        for (const auto& entry : CommandRegistry::getCommandHistory()) {
            auto text = std::make_shared<UIText>(entry);
            consoleScrollBox->addSlot(text)
                .setHorizontalAlignment(UIAlignment::Fill)
                .setVerticalAlignment(UIAlignment::Start)
                .setMargin({0.0f, 0.0f, 0.0f, 2.0f});
        }
    }

    bool ConsoleLayer::onKeyPressed(KeyPressedEvent& event) {
        if (event.getKeyCode() == KeyCode::Grave) {
            isOpen = !isOpen;
            return true;
        }

        return false;
    }
} // namespace Axiom
