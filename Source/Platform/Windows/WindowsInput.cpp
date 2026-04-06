#include "WindowsInput.h"
#include "axpch.h"

namespace Axiom {
    Input* Input::instance = new WindowsInput();

    bool WindowsInput::internalIsKeyPressed(KeyCode keyCode) {
        SHORT result = GetAsyncKeyState(keyCodeToWindowsKey(keyCode));
        if (result & 0x8000) {
            return true;
        }
        return false;
    }

    float WindowsInput::internalGetMouseX() {
        POINT pt;
        HWND hwnd = reinterpret_cast<HWND>(Application::getWindow()->getNativeWindow());
        if (!hwnd || !GetCursorPos(&pt))
            return 0.0f;

        if (!ScreenToClient(hwnd, &pt))
            return 0.0f;

        RECT r;
        GetClientRect(hwnd, &r);

        float x = static_cast<float>(pt.x);
        return std::clamp(x, static_cast<float>(r.left), static_cast<float>(r.right));
    }

    float WindowsInput::internalgetMouseY() {
        POINT pt;
        HWND hwnd = reinterpret_cast<HWND>(Application::getWindow()->getNativeWindow());
        if (!hwnd || !GetCursorPos(&pt))
            return 0.0f;

        if (!ScreenToClient(hwnd, &pt))
            return 0.0f;

        RECT r;
        GetClientRect(hwnd, &r);

        float y = static_cast<float>(pt.y);
        return std::clamp(y, static_cast<float>(r.top), static_cast<float>(r.bottom));
    }

} // namespace Axiom