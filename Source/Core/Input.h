#pragma once
#include "Event/KeyCodes.h"
#include "Math/AxMath.h"
#include <array>

namespace Axiom {

    class Input {
      public:
        friend class Application;

        inline static bool isKeyPressed(KeyCode keyCode) { return keyStates[std::to_underlying(keyCode)]; }
        inline static bool isMouseButtonPressed(KeyCode button) { return mouseButtonStates[std::to_underlying(button)]; }
        inline static Math::Vec2 getMousePosition() { return mousePosition; }
        inline static float getMouseX() { return mousePosition.x(); }
        inline static float getMouseY() { return mousePosition.y(); }

      private:
        static void setKeyPressed(KeyCode keyCode, bool isPressed) { keyStates[std::to_underlying(keyCode)] = isPressed; }
        static void setMouseButtonPressed(KeyCode button, bool isPressed) { mouseButtonStates[std::to_underlying(button)] = isPressed; }
        static void setMousePosition(float x, float y) { mousePosition = Math::Vec2(x, y); }

      private:
        static std::array<bool, 512> keyStates;
        static std::array<bool, 32> mouseButtonStates;
        static Math::Vec2 mousePosition;
    };

} // namespace Axiom