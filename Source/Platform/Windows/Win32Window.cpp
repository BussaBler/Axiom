#include "axpch.h"

#include "Win32Window.h"

#include "Event/ApplicationEvent.h"

#include <cstdint>

namespace Axiom {
    LRESULT CALLBACK Win32Window::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        WindowData* wData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
        float dpiScale = 1.0f;
        if (wData) {
            dpiScale = GetDpiForWindow(hWnd) / 96.0f;
            if (dpiScale <= 0.0f) {
                dpiScale = 1.0f;
            }
        }

        switch (uMsg) {
        case WM_CLOSE: {
            WindowCloseEvent e;
            wData->eventCallback(e);
            DestroyWindow(hWnd);
            break;
        }
        case WM_SIZE: {
            uint32_t fbWidth = LOWORD(lParam);
            uint32_t fbHeight = HIWORD(lParam);

            wData->framebufferWidth = fbWidth;
            wData->framebufferHeight = fbHeight;

            wData->width = static_cast<uint32_t>(fbWidth / dpiScale);
            wData->height = static_cast<uint32_t>(fbHeight / dpiScale);

            WindowResizeEvent e(fbWidth, fbHeight);
            if (wData->eventCallback)
                wData->eventCallback(e);
            break;
        }
        case WM_KEYDOWN: {
            bool isRepeat = (lParam & (1 << 30)) != 0;
            if (isRepeat) {
                KeyPressedEvent e(static_cast<KeyCode>(wParam), 1);
                wData->eventCallback(e);
            } else {
                KeyPressedEvent e(static_cast<KeyCode>(wParam), 0);
                wData->eventCallback(e);
            }
            break;
        }
        case WM_KEYUP: {
            KeyReleasedEvent e(static_cast<KeyCode>(wParam));
            wData->eventCallback(e);
            break;
        }
        case WM_CHAR: {
            KeyTypedEvent e(static_cast<char>(wParam));
            wData->eventCallback(e);
            break;
        }
        case WM_LBUTTONDOWN: {
            float logicalX = static_cast<float>(LOWORD(lParam)) / dpiScale;
            float logicalY = static_cast<float>(HIWORD(lParam)) / dpiScale;
            MouseButtonPressedEvent e(KeyCode::LeftButton, logicalX, logicalY);
            wData->eventCallback(e);
            break;
        }
        case WM_LBUTTONUP: {
            float logicalX = static_cast<float>(LOWORD(lParam)) / dpiScale;
            float logicalY = static_cast<float>(HIWORD(lParam)) / dpiScale;
            MouseButtonReleasedEvent e(KeyCode::LeftButton, logicalX, logicalY);
            wData->eventCallback(e);
            break;
        }
        case WM_MOUSEMOVE: {
            float logicalX = static_cast<float>(LOWORD(lParam)) / dpiScale;
            float logicalY = static_cast<float>(HIWORD(lParam)) / dpiScale;
            MouseMovedEvent e(logicalX, logicalY);
            wData->eventCallback(e);
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        default:
            break;
        }

        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    std::unique_ptr<Window> Window::create(const WindowProps& props) {
        return std::make_unique<Win32Window>(props);
    }

    Win32Window::Win32Window(const WindowProps& props) {
        init(props);
    }

    Win32Window::~Win32Window() {
        shutdown();
    }

    void Win32Window::init(const WindowProps& props) {
        UINT systemDpi = GetDpiForSystem();
        float dpiScale = systemDpi / 96.0f;
        if (dpiScale < 0.0f) {
            dpiScale = 1.0f;
        }

        uint32_t physicalWidth = static_cast<uint32_t>(props.width * dpiScale);
        uint32_t physicalHeight = static_cast<uint32_t>(props.height * dpiScale);

        data.title = props.title;
        data.width = props.width;
        data.height = props.height;
        data.framebufferWidth = physicalWidth;
        data.framebufferHeight = physicalHeight;

        AX_CORE_LOG_INFO("Creating Win32 window {0} (Logical: {1}x{2} | Physical: {3}x{4} | DPI: {5})", data.title, data.width, data.height,
                         data.framebufferWidth, data.framebufferHeight, systemDpi);

        hInstance = GetModuleHandle(nullptr);
        const wchar_t* className = L"AxiomWindowClass";
        WNDCLASS wndClass = {};
        wndClass.lpszClassName = className;
        wndClass.hInstance = hInstance;
        wndClass.hIcon = LoadIcon(nullptr, IDI_WINLOGO);
        wndClass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wndClass.lpfnWndProc = windowProc;

        RegisterClass(&wndClass);

        DWORD style = WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU | WS_OVERLAPPEDWINDOW;
        RECT windowRect = {0, 0, static_cast<LONG>(data.framebufferWidth), static_cast<LONG>(data.framebufferHeight)};
        AdjustWindowRectExForDpi(&windowRect, style, FALSE, 0, systemDpi);

        window = CreateWindowEx(0, className, std::wstring(data.title.begin(), data.title.end()).c_str(), style, CW_USEDEFAULT, CW_USEDEFAULT,
                                windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, nullptr, nullptr, hInstance, nullptr);

        SetWindowLongPtr(window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(&data));

        ShowWindow(window, SW_SHOW);
    }

    void Win32Window::shutdown() {
        if (window) {
            DestroyWindow(window);
            window = nullptr;
        }
        UnregisterClass(L"AxiomWindowClass", hInstance);
    }

    void Win32Window::processMessages() {
        MSG msg{};

        while (PeekMessage(&msg, nullptr, 0u, 0u, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    void Win32Window::onUpdate() {
        processMessages();
    }

    void Win32Window::setVSync(bool enabled) {
        data.vSync = enabled;
    }
} // namespace Axiom
