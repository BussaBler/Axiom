#pragma once

namespace Axiom {
	class Input {
	public:
		static bool isKeyPressed(int keyCode) { instance->isKeyPressedImpl(keyCode); }
		static bool isMouseButtonPressed(int buttonCode) { return instance->isMouseButtonPressedImpl(buttonCode); }
		static float getMouseX() { return instance->getMouseXImpl(); }
		static float getMouseY() { return instance->getMouseYImpl(); }

	protected:
		virtual bool isKeyPressedImpl(int keyCode) = 0; 
		virtual bool isMouseButtonPressedImpl(int buttonCode) = 0;
		virtual float getMouseXImpl() = 0;
		virtual float getMouseYImpl() = 0;

	private:
		static Input* instance;
	};
}
