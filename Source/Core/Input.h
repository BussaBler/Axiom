#pragma once
#include "Event/KeyCodes.h"

namespace Axiom {
    class Input {
      public:
        static bool isKeyPressed(KeyCode keyCode) {
            return instance->internalIsKeyPressed(keyCode);
        }
        static float getMouseX() {
            return instance->internalGetMouseX();
        }
        static float getMouseY() {
            return instance->internalgetMouseY();
        }
        // TODO: change this to a more platform-agnostic key code system
        static int keyCodeToWindowsKey(KeyCode keyCode) {
            return static_cast<int>(keyCode);
        }

      protected:
        virtual bool internalIsKeyPressed(KeyCode keyCode) = 0;
        virtual float internalGetMouseX() = 0;
        virtual float internalgetMouseY() = 0;

      private:
        static Input* instance;
    };
} // namespace Axiom
