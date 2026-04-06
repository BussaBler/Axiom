#include "MacOSWindow.h"

@interface MacOSWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) Axiom::WindowData* wData;
@end

@implementation MacOSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
    Axiom::WindowCloseEvent event;
    wData->eventCallback(event);
    [NSApp terminate:nil];
}

- (void)windowDidResize:(NSNotification *)notification {
    NSWindow* window = [notification object];
    NSRect frame = [window frame];
    Axiom::WindowResizeEvent event(static_cast<uint32_t>(frame.size.width), static_cast<uint32_t>(frame.size.height));
    wData->eventCallback(event);
}
@end

@interface MacOSWindowView : NSView
@property (nonatomic, assign) Axiom::WindowData* wData;
@end

@implementation MacOSWindowView
- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)event {
    Axiom::KeyPressedEvent keyEvent(static_cast<uint32_t>(event.keyCode), event.isARepeat);
    wData->eventCallback(keyEvent);
}

- (void)keyUp:(NSEvent *)event {
    Axiom::KeyReleasedEvent keyEvent(static_cast<uint32_t>(event.keyCode));
    wData->eventCallback(keyEvent);
}

- (void)mouseDown:(NSEvent *)event {
    Axiom::MouseButtonPressedEvent mouseEvent(static_cast<uint32_t>(event.buttonNumber));
    wData->eventCallback(mouseEvent);
}

- (void)mouseUp:(NSEvent *)event {
    Axiom::MouseButtonReleasedEvent mouseEvent(static_cast<uint32_t>(event.buttonNumber));
    wData->eventCallback(mouseEvent);
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint location = [event locationInWindow];
    Axiom::MouseMovedEvent mouseEvent(static_cast<float>(location.x), static_cast<float>(location.y));
    wData->eventCallback(mouseEvent);
}
@end


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

        delegate = [[MacOSWindowDelegate alloc] init];
        delegate.wData = &data;
        [window setDelegate:delegate];

        view = [[MacOSWindowView alloc] initWithFrame:frame];
        view.wData = &data;
        [window setContentView:view];

        [window makeFirstResponder:view];
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

    void MacOSWindow::attachMetalLayer(void* metalLayer) {
        CAMetalLayer* layer = (__bridge CAMetalLayer*)metalLayer;
        [view setWantsLayer:YES];
        [view setLayer:layer];
    }
}