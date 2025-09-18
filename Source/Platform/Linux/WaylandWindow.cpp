#include "WaylandWindow.h"

namespace Axiom {
	WaylandWindow::WaylandWindow(const WindowProps& props) {
		init(props);
	}

	WaylandWindow::~WaylandWindow() {

	}

	void WaylandWindow::onUpdate() {

	}

	void WaylandWindow::init(const WindowProps& props) {
		data.title = props.title;
		data.width = props.width;
		data.height = props.height;

		display = XOpenDisplay(NULL);
		screen = DefaultScreen(display);
		rootWindow = RootWindow(display, screen);

		window = XCreateSimpleWindow(display, rootWindow, 0, 0, data.width, data.height, 1, BlackPixel(display, screen), BlackPixel(display, screen));
	}

	std::unique_ptr<Window> Window::create(const WindowProps& props) {
		return std::make_unique<WaylandWindow>(props);
	}
}