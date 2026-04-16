#include "MacOSWindow.h"
#include <Cocoa/Cocoa.h>
#include <QuartzCore/QuartzCore.h>
#include <Carbon/Carbon.h>

namespace Axiom {
    static KeyCode macOSKeyCodeToAxiomKeyCode(unsigned short macKeyCode) {
        switch (macKeyCode) {
            // Alphabet
            case kVK_ANSI_A: return KeyCode::A;
            case kVK_ANSI_S: return KeyCode::S;
            case kVK_ANSI_D: return KeyCode::D;
            case kVK_ANSI_F: return KeyCode::F;
            case kVK_ANSI_H: return KeyCode::H;
            case kVK_ANSI_G: return KeyCode::G;
            case kVK_ANSI_Z: return KeyCode::Z;
            case kVK_ANSI_X: return KeyCode::X;          
            case kVK_ANSI_C: return KeyCode::C;           
            case kVK_ANSI_V: return KeyCode::V;           
            case kVK_ANSI_B: return KeyCode::B;           
            case kVK_ANSI_Q: return KeyCode::Q;           
            case kVK_ANSI_W: return KeyCode::W;           
            case kVK_ANSI_E: return KeyCode::E;           
            case kVK_ANSI_R: return KeyCode::R;           
            case kVK_ANSI_Y: return KeyCode::Y;           
            case kVK_ANSI_T: return KeyCode::T; 
            case kVK_ANSI_O: return KeyCode::O;           
            case kVK_ANSI_U: return KeyCode::U; 
            case kVK_ANSI_I: return KeyCode::I;
            case kVK_ANSI_P: return KeyCode::P;
            case kVK_ANSI_L: return KeyCode::L;
            case kVK_ANSI_J: return KeyCode::J;
            case kVK_ANSI_K: return KeyCode::K;
            case kVK_ANSI_N: return KeyCode::N;
            case kVK_ANSI_M: return KeyCode::M;

            // Numbers
            case kVK_ANSI_1: return KeyCode::Num1;           
            case kVK_ANSI_2: return KeyCode::Num2;           
            case kVK_ANSI_3: return KeyCode::Num3;           
            case kVK_ANSI_4: return KeyCode::Num4;           
            case kVK_ANSI_5: return KeyCode::Num5;           
            case kVK_ANSI_6: return KeyCode::Num6;           
            case kVK_ANSI_7: return KeyCode::Num7;           
            case kVK_ANSI_8: return KeyCode::Num8;     
            case kVK_ANSI_9: return KeyCode::Num9;           
            case kVK_ANSI_0: return KeyCode::Num0;

            // Punctuation
            case kVK_ANSI_Equal: return KeyCode::Equals;       
            case kVK_ANSI_Minus: return KeyCode::Minus;
            case kVK_ANSI_Semicolon: return KeyCode::Semicolon;
            case kVK_ANSI_Comma: return KeyCode::Comma;
            case kVK_ANSI_Period: return KeyCode::Period;
            case kVK_ANSI_Grave: return KeyCode::Grave;
            case kVK_ANSI_RightBracket: return KeyCode::OEM6; 
            case kVK_ANSI_LeftBracket: return KeyCode::OEM4;
            case kVK_ANSI_Quote: return KeyCode::OEM7;        
            case kVK_ANSI_Backslash: return KeyCode::OEM5;    
            case kVK_ANSI_Slash: return KeyCode::OEM2;      

            // Numpad
            case kVK_ANSI_Keypad0: return KeyCode::Numpad0;
            case kVK_ANSI_Keypad1: return KeyCode::Numpad1;
            case kVK_ANSI_Keypad2: return KeyCode::Numpad2;
            case kVK_ANSI_Keypad3: return KeyCode::Numpad3;
            case kVK_ANSI_Keypad4: return KeyCode::Numpad4;
            case kVK_ANSI_Keypad5: return KeyCode::Numpad5;
            case kVK_ANSI_Keypad6: return KeyCode::Numpad6;
            case kVK_ANSI_Keypad7: return KeyCode::Numpad7;
            case kVK_ANSI_Keypad8: return KeyCode::Numpad8;
            case kVK_ANSI_Keypad9: return KeyCode::Numpad9;
            case kVK_ANSI_KeypadDecimal: return KeyCode::Decimal;
            case kVK_ANSI_KeypadMultiply: return KeyCode::Multiply;
            case kVK_ANSI_KeypadPlus: return KeyCode::Add;
            case kVK_ANSI_KeypadClear: return KeyCode::Clear;
            case kVK_ANSI_KeypadDivide: return KeyCode::Divide;
            case kVK_ANSI_KeypadMinus: return KeyCode::Subtract;
            case kVK_ANSI_KeypadEnter: return KeyCode::Return; // Maps to main Return for now. TODO: Consider adding NumpadEnter
            case kVK_ANSI_KeypadEquals: return KeyCode::Equals;// Maps to main Equals for now. TODO: Consider adding NumpadEquals

            // Core Controls
            case kVK_Return: return KeyCode::Return; 
            case kVK_Tab: return KeyCode::Tab;          
            case kVK_Space: return KeyCode::Space;        
            case kVK_Delete: return KeyCode::Backspace; // Apple "Delete" is Backspace
            case kVK_ForwardDelete: return KeyCode::Delete; // Apple "Forward Delete" is standard Delete
            case kVK_Escape: return KeyCode::Escape;       
            
            // Modifiers
            case kVK_Command: return KeyCode::LeftSuper; // Apple Command maps to Windows/Super key
            case kVK_Shift: return KeyCode::Shift;        
            case kVK_CapsLock: return KeyCode::Capital; // Your enum uses Capital for CapsLock
            case kVK_Option: return KeyCode::Menu;       
            case kVK_Control: return KeyCode::Control;      
            case kVK_RightShift: return KeyCode::RightShift;
            case kVK_RightOption: return KeyCode::RightMenu;
            case kVK_RightControl: return KeyCode::RightControl;
            
            // Navigation
            case kVK_Home: return KeyCode::Home;
            case kVK_PageUp: return KeyCode::PageUp;
            case kVK_End: return KeyCode::End;
            case kVK_PageDown: return KeyCode::PageDown;
            case kVK_LeftArrow: return KeyCode::Left;
            case kVK_RightArrow: return KeyCode::Right;
            case kVK_DownArrow: return KeyCode::Down;
            case kVK_UpArrow: return KeyCode::Up;
            
            // Function Keys
            case kVK_F1: return KeyCode::F1;
            case kVK_F2: return KeyCode::F2;
            case kVK_F3: return KeyCode::F3;
            case kVK_F4: return KeyCode::F4;
            case kVK_F5: return KeyCode::F5;
            case kVK_F6: return KeyCode::F6;
            case kVK_F7: return KeyCode::F7;
            case kVK_F8: return KeyCode::F8;
            case kVK_F9: return KeyCode::F9;
            case kVK_F10: return KeyCode::F10;
            case kVK_F11: return KeyCode::F11;
            case kVK_F12: return KeyCode::F12;
            case kVK_F13: return KeyCode::F13;
            case kVK_F14: return KeyCode::F14;
            case kVK_F15: return KeyCode::F15;
            case kVK_F16: return KeyCode::F16;
            case kVK_F17: return KeyCode::F17;
            case kVK_F18: return KeyCode::F18;
            case kVK_F19: return KeyCode::F19;
            case kVK_F20: return KeyCode::F20;
            
            case kVK_Help: return KeyCode::Help;
            case kVK_Function: return KeyCode::Function;
            case kVK_VolumeUp: return KeyCode::VolumeUp;
            case kVK_VolumeDown: return KeyCode::VolumeDown;
            case kVK_Mute: return KeyCode::VolumeMute;

            default: return KeyCode::None;
        }
    }

    static KeyCode macOSMouseButtonToAxiomMouseButton(unsigned short macButton) {
        switch (macButton) {
            case 0: return KeyCode::LeftButton;
            case 1: return KeyCode::RightButton;
            case 2: return KeyCode::MiddleButton;
            default: return KeyCode::None;
        }
    }
}

@interface MacOSWindowDelegate : NSObject <NSWindowDelegate>
@property (nonatomic, assign) Axiom::WindowData* wData;
@end

@implementation MacOSWindowDelegate
- (void)windowWillClose:(NSNotification *)notification {
    Axiom::WindowCloseEvent event;
    _wData->eventCallback(event);
}

- (void)windowDidResize:(NSNotification *)notification {
    NSWindow* window = [notification object];
    CGSize logicalSize = [[window contentView] bounds].size;
    CGFloat scaleFactor = [window backingScaleFactor];

    uint32_t fbWidth = static_cast<uint32_t>(logicalSize.width * scaleFactor);
    uint32_t fbHeight = static_cast<uint32_t>(logicalSize.height * scaleFactor);

    Axiom::WindowResizeEvent event(fbWidth, fbHeight);
    _wData->eventCallback(event);

    _wData->width = static_cast<uint32_t>(logicalSize.width);
    _wData->height = static_cast<uint32_t>(logicalSize.height);
    _wData->framebufferWidth = fbWidth;
    _wData->framebufferHeight = fbHeight;
}
@end

@interface MacOSWindowView : NSView
@property (nonatomic, assign) Axiom::WindowData* wData;
@end

@interface MacOSWindowView () {
    NSTrackingArea* trackingArea;
}
@end

@implementation MacOSWindowView
- (BOOL)acceptsFirstResponder {
    return YES;
}

- (void)keyDown:(NSEvent *)event {
    Axiom::KeyPressedEvent keyEvent(Axiom::macOSKeyCodeToAxiomKeyCode([event keyCode]), event.isARepeat);
    _wData->eventCallback(keyEvent);

    NSString* characters = [event characters];
    if (characters != nil && [characters length] > 0) {
        for (NSUInteger i = 0; i < [characters length]; i++) {
            unichar singleChar = [characters characterAtIndex:i];
            
            if (singleChar >= 32 && singleChar != 127 && singleChar < 0xF700) {
                Axiom::KeyTypedEvent typedEvent(static_cast<char>(singleChar));
                _wData->eventCallback(typedEvent);
            }
        }
    }
}

- (void)keyUp:(NSEvent *)event {
    Axiom::KeyReleasedEvent keyEvent(Axiom::macOSKeyCodeToAxiomKeyCode([event keyCode]));
    _wData->eventCallback(keyEvent);
}

- (void)mouseDown:(NSEvent *)event {
    Axiom::MouseButtonPressedEvent mouseEvent(Axiom::macOSMouseButtonToAxiomMouseButton([event buttonNumber]));
    _wData->eventCallback(mouseEvent);
}

- (void)mouseUp:(NSEvent *)event {
    Axiom::MouseButtonReleasedEvent mouseEvent(Axiom::macOSMouseButtonToAxiomMouseButton([event buttonNumber]));
    _wData->eventCallback(mouseEvent);
}

- (void)rightMouseDown:(NSEvent *)event {
    Axiom::MouseButtonPressedEvent mouseEvent(Axiom::macOSMouseButtonToAxiomMouseButton([event buttonNumber]));
    _wData->eventCallback(mouseEvent);
}

- (void)rightMouseUp:(NSEvent *)event {
    Axiom::MouseButtonReleasedEvent mouseEvent(Axiom::macOSMouseButtonToAxiomMouseButton([event buttonNumber]));
    _wData->eventCallback(mouseEvent);
}

- (void)otherMouseDown:(NSEvent *)event {
    Axiom::MouseButtonPressedEvent mouseEvent(Axiom::macOSMouseButtonToAxiomMouseButton([event buttonNumber]));
    _wData->eventCallback(mouseEvent);
}

- (void)otherMouseUp:(NSEvent *)event {
    Axiom::MouseButtonReleasedEvent mouseEvent(Axiom::macOSMouseButtonToAxiomMouseButton([event buttonNumber]));
    _wData->eventCallback(mouseEvent);
}

- (void)mouseMoved:(NSEvent *)event {
    NSPoint location = [event locationInWindow];
    
    CGFloat contentHeight = [[[self window] contentView] bounds].size.height;
    
    float flippedY = static_cast<float>(contentHeight - location.y);
    float x = static_cast<float>(location.x);
    
    Axiom::MouseMovedEvent mouseEvent(x, flippedY);
    _wData->eventCallback(mouseEvent);
}

- (void)mouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)rightMouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)otherMouseDragged:(NSEvent *)event {
    [self mouseMoved:event];
}

- (void)updateTrackingAreas {
    [super updateTrackingAreas];
    
    if (trackingArea != nil) {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
        trackingArea = nil;
    }
    
    NSTrackingAreaOptions options = (NSTrackingActiveAlways | NSTrackingInVisibleRect | NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved);
                                     
    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds] options:options owner:self userInfo:nil];
    [self addTrackingArea:trackingArea];
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

        NSApplication* app = [NSApplication sharedApplication];
        [app setActivationPolicy:NSApplicationActivationPolicyRegular];
        [app finishLaunching];

        NSRect frame = NSMakeRect(0, 0, data.width, data.height);
        NSUInteger styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable;
        
        window = [[NSWindow alloc] initWithContentRect:frame styleMask:styleMask backing:NSBackingStoreBuffered defer:NO];
                                                
        [window setTitle:[NSString stringWithUTF8String:data.title.c_str()]];
        [window center];
        [window makeKeyAndOrderFront:nil];
        [NSApp activateIgnoringOtherApps:YES];

        delegate = [[MacOSWindowDelegate alloc] init];
        delegate.wData = &data;
        [window setDelegate:delegate];

        view = [[MacOSWindowView alloc] initWithFrame:frame];
        view.wData = &data;
        
        CGFloat scaleFactor = [window backingScaleFactor];
        
        [view setWantsLayer:YES];
        
        view.layer.contentsScale = scaleFactor;
        data.framebufferWidth = static_cast<uint32_t>(data.width * scaleFactor);
        data.framebufferHeight = static_cast<uint32_t>(data.height * scaleFactor);

        [window setContentView:view];
        [window setAcceptsMouseMovedEvents:YES];
        [window makeFirstResponder:view];
    }

    void MacOSWindow::shutdown() {
        [window close];
        [NSApp terminate:nil];
    }

    void MacOSWindow::beginFrame() {
        autoreleasePool = [[NSAutoreleasePool alloc] init];
    }

    void MacOSWindow::endFrame() {
        [autoreleasePool release];
        autoreleasePool = nil;
    }

    void MacOSWindow::onUpdate() {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny untilDate:[NSDate distantPast] inMode:NSDefaultRunLoopMode dequeue:YES])) {
            [NSApp sendEvent:event];
            [NSApp updateWindows];
        }
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
        layer.contentsScale = [window backingScaleFactor];
        [view setWantsLayer:YES];
        [view setLayer:layer];
    }
}