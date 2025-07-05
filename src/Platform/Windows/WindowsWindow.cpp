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
			glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
			AX_CORE_ASSERT(success, "Could not initialize GLFW!");
			isGLFWInitialized = true;
		}

		window = glfwCreateWindow(static_cast<int>(data.width), static_cast<int>(data.height), data.title.c_str(), nullptr, nullptr);
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

		glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
				WindowData* wData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				
				switch (action) {
				case GLFW_PRESS: {
					KeyPressedEvent e(key, 0);
					wData->eventCallback(e);
					break;
				}
				case GLFW_RELEASE: {
					KeyReleassedEvent e(key);
					wData->eventCallback(e);
					break;
				}
				case GLFW_REPEAT: {
					KeyPressedEvent e(key, 1);
					wData->eventCallback(e);
					break;
				}
				default:
					break;
				}
			}
		);

		glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
				WindowData* wData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				
				switch (action) {
				case GLFW_PRESS: {
					MouseButtonPressedEvent e(button);
					wData->eventCallback(e);
					break;
				}
				case GLFW_RELEASE: {
					MouseButtonReleasedEvent e(button);
					wData->eventCallback(e);
					break;
				}
				default:
					break;
				}
			}
		);

		glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
				WindowData* wData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				MouseScrolledEvent e(static_cast<float>(xOffset), static_cast<float>(yOffset));
				wData->eventCallback(e);
			}
		);

		glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xPos, double yPos) {
				WindowData* wData = reinterpret_cast<WindowData*>(glfwGetWindowUserPointer(window));
				MouseMovedEvent e(static_cast<float>(xPos), static_cast<float>(yPos));
				wData->eventCallback(e);
			}
		);
	}

	void WindowsWindow::shutdown() {
		glfwDestroyWindow(window);
	}

	void WindowsWindow::onUpdate() {
		glfwPollEvents();
	}

	void WindowsWindow::setVSync(bool enabled) {
		glfwSwapInterval(enabled ? 1 : 0);
		data.vSync = enabled;
	}

	bool WindowsWindow::isVSync() const {
		return data.vSync;
	}
}
