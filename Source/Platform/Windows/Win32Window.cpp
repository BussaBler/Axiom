#include "Win32Window.h"
#include "axpch.h"

namespace Axiom {
    LRESULT CALLBACK Win32Window::windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        WindowData* wData = reinterpret_cast<WindowData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

        switch (uMsg) {
        case WM_CLOSE: {
            WindowCloseEvent e;
            wData->eventCallback(e);
            DestroyWindow(hWnd);
            break;
        }
        case WM_SIZE: {
            WindowResizeEvent e(LOWORD(lParam), HIWORD(lParam));
            wData->width = LOWORD(lParam);
            wData->height = HIWORD(lParam);
            wData->framebufferWidth = LOWORD(lParam);
            wData->framebufferHeight = HIWORD(lParam);
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
            MouseButtonPressedEvent e(KeyCode::LeftButton);
            wData->eventCallback(e);
            break;
        }
        case WM_LBUTTONUP: {
            MouseButtonReleasedEvent e(KeyCode::LeftButton);
            wData->eventCallback(e);
            break;
        }
        case WM_MOUSEMOVE: {
            MouseMovedEvent e(LOWORD(lParam), HIWORD(lParam));
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
        data.title = props.title;
        data.width = props.width;
        data.height = props.height;
        data.framebufferWidth = props.width;
        data.framebufferHeight = props.height;

        AX_CORE_LOG_INFO("Creating a Win32 window {0} ({1}, {2})", data.title, data.width, data.height);

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
        RECT windowRect = {0, 0, static_cast<LONG>(data.width), static_cast<LONG>(data.height)};
        AdjustWindowRect(&windowRect, style, FALSE);

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
