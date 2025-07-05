#include "axpch.h"
#include "WindowsInput.h"

namespace Axiom {
	Input* Input::instance = new WindowsInput();

	bool WindowsInput::isKeyPressedImpl(int keyCode) {
		auto& window = Application::get().getWindow();
		auto* glfwWindow = static_cast<GLFWwindow*>(window.getNativeWindow());

		int state = glfwGetKey(glfwWindow, keyCode); 

		return state == GLFW_PRESS || state == GLFW_REPEAT;
	}

	bool WindowsInput::isMouseButtonPressedImpl(int buttonCode) {
		auto& window = Application::get().getWindow();
		auto* glfwWindow = static_cast<GLFWwindow*>(window.getNativeWindow());

		int state = glfwGetMouseButton(glfwWindow, buttonCode);

		return state == GLFW_PRESS;
	}

	float WindowsInput::getMouseXImpl() {
		auto& window = Application::get().getWindow();
		auto* glfwWindow = static_cast<GLFWwindow*>(window.getNativeWindow());

		double xPos;
		glfwGetCursorPos(glfwWindow, &xPos, nullptr);

		return static_cast<float>(xPos);
	}

	float WindowsInput::getMouseYImpl() {
		auto& window = Application::get().getWindow();
		auto* glfwWindow = static_cast<GLFWwindow*>(window.getNativeWindow());

		double yPos;
		glfwGetCursorPos(glfwWindow, nullptr, &yPos);

		return static_cast<float>(yPos);
	}

}