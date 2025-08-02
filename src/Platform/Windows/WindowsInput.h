#pragma once
#include "Core/Application.h"
#include "Core/Input.h"
#include <Windows.h>

namespace Axiom {
	class WindowsInput : public Input {
	protected:
		bool internalIsKeyPressed(KeyCode keyCode) override;
		float internalGetMouseX() override;
		float internalgetMouseY() override;
	};
}

