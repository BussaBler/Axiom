#include "WaylandWindow.h"

namespace Axiom {
	XLibWindow::XLibWindow(const WindowProps& props) {
		init(props);
	}

	XLibWindow::~XLibWindow() {

	}

	void XLibWindow::onUpdate() {
		Atom wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", false);
		XSetWMProtocols(display, window, &wmDeleteMessage, 1);

		if (XNextEvent(display, &xEvent) == 0) {
			switch (xEvent.type) {
				case ClientMessage: {
					if (xEvent.xclient.data.l[0] == static_cast<long>(wmDeleteMessage)) {
						WindowCloseEvent e;
						data.eventCallback(e);
						XDestroyWindow(display, window);
						XCloseDisplay(display);
					}
					break;	
				}
				case ConfigureNotify: {
					if (xEvent.xconfigure.width != static_cast<int>(data.width) || xEvent.xconfigure.height != static_cast<int>(data.height)) {
						data.width = xEvent.xconfigure.width;
						data.height = xEvent.xconfigure.height;
						WindowResizeEvent e(data.width, data.height);
						data.eventCallback(e);
					}
					break;
				}
			}
		}
	}

	void XLibWindow::init(const WindowProps& props) {
		AX_CORE_LOG_INFO("Initializing X11 window {0} ({1}, {2})", props.title, props.width, props.height);
		data.title = props.title;
		data.width = props.width;
		data.height = props.height;

		if ((display = XOpenDisplay(NULL)) == nullptr) {
			AX_CORE_ASSERT(false, "Failed to open X display");
		}
		screen = DefaultScreen(display);
		rootWindow = RootWindow(display, screen);

		window = XCreateSimpleWindow(display, rootWindow, 0, 0, data.width, data.height, 1, BlackPixel(display, screen), BlackPixel(display, screen));
		if (!window) {
			AX_CORE_ASSERT(false, "Failed to create X window");
		}
		XSetStandardProperties(display, window, data.title.c_str(), data.title.c_str(), None, NULL, 0, NULL);
		XSelectInput(display, window, ExposureMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask | StructureNotifyMask);
		XkbSetDetectableAutoRepeat(display, true, nullptr);
		XMapWindow(display, window);
	}

	std::unique_ptr<Window> Window::create(const WindowProps& props) {
		return std::make_unique<XLibWindow>(props);
	}
}