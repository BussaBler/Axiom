#include "axpch.h"
#include "Window.h"
#include "Platform/Windows/WindowsWindow.h"

namespace Axiom {
	Window* Window::create(WindowPlatform platform, const WindowProps& properties) {
		switch (platform) {
			case Axiom::WindowPlatform::None:
				AX_CORE_LOG_ERROR("WindowPlatform::None is not supported!");
				return nullptr;
				break;
			case Axiom::WindowPlatform::Windows:
				return new WindowsWindow(properties);
				break;
			case Axiom::WindowPlatform::Linux:
				// return new LinuxWindow(properties);
				break;
			case Axiom::WindowPlatform::MacOS:
				// return new MacOSWindow(properties);
				break;
			default:
				return nullptr;
				break;
		}
	}
}