#pragma once
#include "CommandRegistry.h"
#include "Event/KeyEvent.h"
#include "Layer.h"
#include "axpch.h"

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
        bool onKeyPressed(KeyPressedEvent& event);
        bool onKeyTyped(KeyTypedEvent& event);

      private:
        bool isOpen = false;
        std::string inputBuffer;
        bool checkboxValue = false;

        float consoleHeightRatio = 0.5f;
        uint16_t fontSize = 8;
    };
} // namespace Axiom
