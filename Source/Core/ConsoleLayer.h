#pragma once
#include "CommandRegistry.h"
#include "Event/KeyEvent.h"
#include "Layer.h"
#include "Locator.h"
#include "Math/AxMath.h"
#include "UI/Elements/UIButton.h"
#include "UI/Elements/UICanvas.h"
#include "UI/Elements/UIPanel.h"
#include "UI/Elements/UIScrollBox.h"
#include "UI/Elements/UIText.h"
#include "UI/Elements/UITextInput.h"
#include "UI/Elements/UIVerticalBox.h"

#include <memory>
#include <string>

namespace Axiom {
    class ConsoleLayer : public Layer {
      public:
        ConsoleLayer() : Layer("ConsoleLayer") {}
        ~ConsoleLayer() = default;

        void onAttach() override;
        void onDetach() override;
        void onUpdate() override;
        void onEvent(Event& event) override;
        void onUIRender() override;
        void onRender(CommandBuffer* commandBuffer) override;

      private:
        void refreshConsoleHistory();

        bool onKeyPressed(KeyPressedEvent& event);

      private:
        static constexpr float CONSOLE_HEIGHT_RATIO = 0.4f;

        bool isOpen = false;

        UIContext mainUiContext;
        std::shared_ptr<UICanvas> uiRoot;
        std::shared_ptr<UIScrollBox> consoleScrollBox;
        bool shouldRefreshHistory = false;

        std::string consoleInputBuffer;
    };
} // namespace Axiom
