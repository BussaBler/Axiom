#pragma once
#include "Core/Application.h"
#include "Core/Input.h"
#define UNICODE
#define _UNICODE
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Axiom {
    class WindowsInput : public Input {
      protected:
        bool internalIsKeyPressed(KeyCode keyCode) override;
        float internalGetMouseX() override;
        float internalgetMouseY() override;
    };
} // namespace Axiom
