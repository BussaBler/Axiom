#include "axpch.h"
#include "WindowsWindow.h"

namespace Axiom {
	static bool isGLFWInitialized = false;

	Window* Window::create(const WindowProps& props) {
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props) {
		init(props);
	}

	WindowsWindow::~WindowsWindow() {
		shutdown();
	}

	void WindowsWindow::init(const WindowProps& props) {
		data.title = props.title;
		data.width = props.width;
		data.height = props.height;

		AX_CORE_LOG_INFO("Creating window {0} ({1}, {2})", data.title, data.width, data.height);

		if (!isGLFWInitialized) {
			int success = glfwInit();
			AX_CORE_ASSERT(success, "Could not initialize GLFW!");
			isGLFWInitialized = true;
		}

		window = glfwCreateWindow(static_cast<int>(data.width), static_cast<int>(data.height), data.title.c_str(), nullptr, nullptr);
		glfwMakeContextCurrent(window);
		glfwSetWindowUserPointer(window, &data);
		setVSync(true);

		glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
				WindowData* wData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				WindowResizeEvent e(width, height);
				wData->width = width;
				wData->height = height;
				wData->eventCallback(e);
			}
		);

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window) {
				WindowData* wData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				WindowCloseEvent e;
				wData->eventCallback(e);
			}
		);
	}

	void WindowsWindow::shutdown() {
		glfwDestroyWindow(window);
	}

	void WindowsWindow::onUpdate() {
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	void WindowsWindow::setVSync(bool enabled) {
		glfwSwapInterval(enabled ? 1 : 0);
		data.vSync = enabled;
	}

	bool WindowsWindow::isVSync() const {
		return data.vSync;
	}
}
