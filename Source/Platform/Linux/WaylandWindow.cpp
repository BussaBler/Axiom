#include "WaylandWindow.h"

namespace Axiom {
	WaylandWindow::WaylandWindow(const WindowProps& props) {

	}

	WaylandWindow::~WaylandWindow() {

	}

	void WaylandWindow::onUpdate() {

	}

	void WaylandWindow::init(const WindowProps& props) {

	}

	std::unique_ptr<Window> Window::create(const WindowProps& props) {
		return std::make_unique<WaylandWindow>(props);
	}
}