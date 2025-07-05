#pragma once
#include "Core/Application.h"
#include "Core/Input.h"
#include <GLFW/glfw3.h>

namespace Axiom {
	class WindowsInput : public Input {
	protected:
		bool isKeyPressedImpl(int keyCode) override;
		bool isMouseButtonPressedImpl(int buttonCode) override;
		float getMouseXImpl() override;
		float getMouseYImpl() override;
	};
}

