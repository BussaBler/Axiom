#include "MacOSWindow.h"

namespace Axiom {
    std::unique_ptr<Window> Window::create(const WindowProps& properties) {
        return std::make_unique<MacOSWindow>(properties);
    }

    MacOSWindow::MacOSWindow(const WindowProps& properties) {
        init(properties);
    }

    MacOSWindow::~MacOSWindow() {
        shutdown();
    }

    void MacOSWindow::init(const WindowProps& properties) {
        data.title = properties.title;
        data.width = properties.width;
        data.height = properties.height;

        AX_CORE_LOG_INFO("Creating MacOS window {0} ({1}, {2})", data.title, data.width, data.height);

        [NSApplication sharedApplication];
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

        NSRect frame = NSMakeRect(0, 0, data.width, data.height);
        NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
        window = [[NSWindow alloc] initWithContentRect:frame
                                              styleMask:styleMask
                                                backing:NSBackingStoreBuffered
                                                  defer:NO];
        [window setTitle:[NSString stringWithUTF8String:data.title.c_str()]];
        [window center];
        [window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];
    }

    void MacOSWindow::shutdown() {
        [window close];
    }

    void MacOSWindow::onUpdate() {
        
    }

    void MacOSWindow::setVSync(bool enabled) {
        
    }

    uint32_t MacOSWindow::getWindowDPI() const {
        NSScreen* mainScreen = [NSScreen mainScreen];
        CGFloat scaleFactor = [mainScreen backingScaleFactor];
        return static_cast<uint32_t>(scaleFactor * 96);
    }
}