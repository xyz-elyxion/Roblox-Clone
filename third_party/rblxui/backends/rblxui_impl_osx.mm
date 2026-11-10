// dear rblxui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan, Metal..)
// - Not well tested. If you want a portable application, prefer using the RBLXWIN or SDL platform Backends on Mac.
// - Requires linking with the GameController framework ("-framework GameController").

// Implemented features:
//  [X] Platform: Clipboard support is part of core Dear RblxUI (no specific code in this backend).
//  [X] Platform: Mouse support. Can discriminate Mouse/Pen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass RblxUIKey values to all key functions e.g. RblxUI::IsKeyPressed(RblxUIKey_Space). [Legacy kVK_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (RblxUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= RblxUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: IME support.
//  [x] Platform: Multi-viewport / platform windows.
// Missing features or Issues:
//  [ ] Platform: Multi-viewport: Window size not correctly reported when enabling io.ConfigViewportsNoDecoration
//  [ ] Platform: Multi-viewport: Missing RblxUIBackendFlags_HasParentViewport support. The viewport->ParentViewportID field is ignored, and therefore io.ConfigViewportsNoDefaultParent has no effect either.

// You can use unmodified rblxui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire rblxui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

#import "rblxui.h"
#ifndef RBLXUI_DISABLE
#import "rblxui_impl_osx.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <GameController/GameController.h>
#import <time.h>

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Added support for multiple windows via the RblxUIPlatformIO interface.
//  2025-09-18: Call platform_io.ClearPlatformHandlers() on shutdown.
//  2025-07-08: [Docking] Fixed multi-viewport handling broken on 2025-06-02. (#8644, #8777)
//  2025-06-27: Added RblxUIMouseCursor_Wait and RblxUIMouseCursor_Progress mouse cursor support.
//  2025-06-12: RblxUI_ImplOSX_HandleEvent() only process event for window containing our view. (#8644)
//  2025-05-15: [Docking] Add Platform_GetWindowFramebufferScale() handler, to allow varying Retina display density on multiple monitors.
//  2025-03-21: Fill gamepad inputs and set RblxUIBackendFlags_HasGamepad regardless of RblxUIConfigFlags_NavEnableGamepad being set.
//  2025-01-20: Removed notification observer when shutting down. (#8331)
//  2024-08-22: moved some OS/backend related function pointers from RblxUIIO to RblxUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformSetImeDataFn  -> platform_io.Platform_SetImeDataFn
//  2024-07-02: Update for io.SetPlatformImeDataFn() -> io.PlatformSetImeDataFn() renaming in main library.
//  2023-10-05: Inputs: Added support for extra RblxUIKey values: F13 to F20 function keys. Stopped mapping F13 into PrintScreen.
//  2023-04-09: Inputs: Added support for io.AddMouseSourceEvent() to discriminate RblxUIMouseSource_Mouse/RblxUIMouseSource_Pen.
//  2023-02-01: Fixed scroll wheel scaling for devices emitting events with hasPreciseScrollingDeltas==false (e.g. non-Apple mice).
//  2022-11-02: Fixed mouse coordinates before clicking the host window.
//  2022-10-06: Fixed mouse inputs on flipped views.
//  2022-09-26: Inputs: Renamed RblxUIKey_ModXXX introduced in 1.87 to RblxUIMod_XXX (old names still supported).
//  2022-05-03: Inputs: Removed RblxUI_ImplOSX_HandleEvent() from backend API in favor of backend automatically handling event capture.
//  2022-04-27: Misc: Store backend data in a per-context struct, allowing to use this backend with multiple contexts.
//  2022-03-22: Inputs: Monitor NSKeyUp events to catch missing keyUp for key when user press Cmd + key
//  2022-02-07: Inputs: Forward keyDown/keyUp events to OS when unused by dear rblxui.
//  2022-01-31: Fixed building with old Xcode versions that are missing gamepad features.
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using RblxUIKey_ModXXX flags. Sorry for the confusion.
//  2021-01-20: Inputs: calling new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly to io.NavInputs[].
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-12: Inputs: Added basic Platform IME support, hooking the io.SetPlatformImeDataFn() function.
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full RblxUIKey range.
//  2021-12-13: *BREAKING CHANGE* Add NSView parameter to RblxUI_ImplOSX_Init(). Generally fix keyboard support. Using kVK_* codes for keyboard keys.
//  2021-12-13: Add game controller support.
//  2021-09-21: Use mach_absolute_time as CFAbsoluteTimeGetCurrent can jump backwards.
//  2021-08-17: Calling io.AddFocusEvent() on NSApplicationDidBecomeActiveNotification/NSApplicationDidResignActiveNotification events.
//  2021-06-23: Inputs: Added a fix for shortcuts using CTRL key instead of CMD key.
//  2021-04-19: Inputs: Added a fix for keys remaining stuck in pressed state when CMD-tabbing into different application.
//  2021-01-27: Inputs: Added a fix for mouse position not being reported when mouse buttons other than left one are down.
//  2020-10-28: Inputs: Added a fix for handling keypad-enter key.
//  2020-05-25: Inputs: Added a fix for missing trackpad clicks when done with "soft tap".
//  2019-12-05: Inputs: Added support for RblxUIMouseCursor_NotAllowed mouse cursor.
//  2019-10-11: Inputs:  Fix using Backspace key.
//  2019-07-21: Re-added clipboard handlers as they are not enabled by default in core rblxui.cpp (reverted 2019-05-18 change).
//  2019-05-28: Inputs: Added mouse cursor shape and visibility support.
//  2019-05-18: Misc: Removed clipboard handlers as they are now supported by core rblxui.cpp.
//  2019-05-11: Inputs: Don't filter character values before calling AddInputCharacter() apart from 0xF700..0xFFFF range.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-07-07: Initial version.

#define APPLE_HAS_BUTTON_OPTIONS (__IPHONE_OS_VERSION_MIN_REQUIRED >= 130000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500 || __TV_OS_VERSION_MIN_REQUIRED >= 130000)
#define APPLE_HAS_CONTROLLER     (__IPHONE_OS_VERSION_MIN_REQUIRED >= 140000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 110000 || __TV_OS_VERSION_MIN_REQUIRED >= 140000)
#define APPLE_HAS_THUMBSTICKS    (__IPHONE_OS_VERSION_MIN_REQUIRED >= 120100 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101401 || __TV_OS_VERSION_MIN_REQUIRED >= 120100)

@class RblxUIObserver;
@class KeyEventResponder;

// Data
struct RblxUI_ImplOSX_Data
{
    CFTimeInterval              Time;
    NSCursor*                   MouseCursors[RblxUIMouseCursor_COUNT];
    bool                        MouseCursorHidden;
    RblxUIObserver*              Observer;
    KeyEventResponder*          KeyEventResponder;
    NSTextInputContext*         InputContext;
    id                          Monitor;
    NSWindow*                   Window;

    RblxUI_ImplOSX_Data()        { memset((void*)this, 0, sizeof(*this)); }
};

static RblxUI_ImplOSX_Data*      RblxUI_ImplOSX_GetBackendData()      { return (RblxUI_ImplOSX_Data*)RblxUI::GetIO().BackendPlatformUserData; }
static void                     RblxUI_ImplOSX_DestroyBackendData()  { RBLXUI_DELETE(RblxUI_ImplOSX_GetBackendData()); }

static inline CFTimeInterval    GetMachAbsoluteTimeInSeconds()      { return (CFTimeInterval)(double)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1e9); }

// Forward Declarations
static void RblxUI_ImplOSX_InitMultiViewportSupport();
static void RblxUI_ImplOSX_ShutdownMultiViewportSupport();
static void RblxUI_ImplOSX_UpdateMonitors();
static void RblxUI_ImplOSX_AddTrackingArea(NSView* _Nonnull view);
static bool RblxUI_ImplOSX_HandleEvent(NSEvent* event, NSView* view);

// Undocumented methods for creating cursors.
@interface NSCursor()
+ (id)_windowResizeNorthWestSouthEastCursor;
+ (id)_windowResizeNorthEastSouthWestCursor;
+ (id)_windowResizeNorthSouthCursor;
+ (id)_windowResizeEastWestCursor;
+ (id)busyButClickableCursor;
@end

/**
 KeyEventResponder implements the NSTextInputClient protocol as is required by the macOS text input manager.

 The macOS text input manager is invoked by calling the interpretKeyEvents method from the keyDown method.
 Keyboard events are then evaluated by the macOS input manager and valid text input is passed back via the
 insertText:replacementRange method.

 This is the same approach employed by other cross-platform libraries such as SDL2:
  https://github.com/spurious/SDL-mirror/blob/e17aacbd09e65a4fd1e166621e011e581fb017a8/src/video/cocoa/SDL_cocoakeyboard.m#L53
 and RBLXWIN:
  https://github.com/rblxwin/rblxwin/blob/b55a517ae0c7b5127dffa79a64f5406021bf9076/src/cocoa_window.m#L722-L723
 */
@interface KeyEventResponder: NSView<NSTextInputClient>
@end

@implementation KeyEventResponder
{
    float _posX;
    float _posY;
    NSRect _imeRect;
}

#pragma mark - Public

- (void)setImePosX:(float)posX imePosY:(float)posY
{
    _posX = posX;
    _posY = posY;
}

- (void)updateImePosWithView:(NSView *)view
{
    NSWindow* window = view.window;
    if (!window)
        return;

    RblxUIIO& io = RblxUI::GetIO();
    if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
    {
        NSRect frame = window.frame;
        NSRect contentRect = window.contentLayoutRect;
        if (window.styleMask & NSWindowStyleMaskFullSizeContentView) // No title bar windows should be considered.
            contentRect = frame;

        NSRect firstScreenFrame = NSScreen.screens[0].frame;
        _imeRect = NSMakeRect(_posX, _posY, 0, 0);
        _imeRect.origin.y = firstScreenFrame.size.height - _imeRect.size.height - _imeRect.origin.y; // Opposite of ConvertNSRect()
    }
    else
    {
        NSRect contentRect = [window contentRectForFrameRect:window.frame];
        NSRect rect = NSMakeRect(_posX, contentRect.size.height - _posY, 0, 0);
        _imeRect = [window convertRectToScreen:rect];
    }
}

- (void)viewDidMoveToWindow
{
    // Ensure self is a first responder to receive the input events.
    [self.window makeFirstResponder:self];
}

- (void)keyDown:(NSEvent*)event
{
    if (!RblxUI_ImplOSX_HandleEvent(event, self))
        [super keyDown:event];

    // Call to the macOS input manager system.
    [self interpretKeyEvents:@[event]];
}

- (void)keyUp:(NSEvent*)event
{
    if (!RblxUI_ImplOSX_HandleEvent(event, self))
        [super keyUp:event];
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
    RblxUIIO& io = RblxUI::GetIO();

    NSString* characters;
    if ([aString isKindOfClass:[NSAttributedString class]])
        characters = [aString string];
    else
        characters = (NSString*)aString;

    io.AddInputCharactersUTF8(characters.UTF8String);
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (void)doCommandBySelector:(SEL)myselector
{
}

- (nullable NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range actualRange:(nullable NSRangePointer)actualRange
{
    return _imeRect;
}

- (BOOL)hasMarkedText
{
    return NO;
}

- (NSRange)markedRange
{
    return NSMakeRange(NSNotFound, 0);
}

- (NSRange)selectedRange
{
    return NSMakeRange(NSNotFound, 0);
}

- (void)setMarkedText:(nonnull id)string selectedRange:(NSRange)selectedRange replacementRange:(NSRange)replacementRange
{
}

- (void)unmarkText
{
}

- (nonnull NSArray<NSAttributedStringKey>*)validAttributesForMarkedText
{
    return @[];
}

@end

@interface RblxUIObserver : NSObject

- (void)onApplicationBecomeActive:(NSNotification*)aNotification;
- (void)onApplicationBecomeInactive:(NSNotification*)aNotification;
- (void)displaysDidChange:(NSNotification*)aNotification;

@end

@implementation RblxUIObserver

- (void)onApplicationBecomeActive:(NSNotification*)aNotification
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddFocusEvent(true);
}

- (void)onApplicationBecomeInactive:(NSNotification*)aNotification
{
    RblxUIIO& io = RblxUI::GetIO();
    io.AddFocusEvent(false);
}

- (void)displaysDidChange:(NSNotification*)aNotification
{
    RblxUI_ImplOSX_UpdateMonitors();
}

@end

// Functions

// Not static to allow third-party code to use that if they want to (but undocumented)
RblxUIKey RblxUI_ImplOSX_KeyCodeToRblxUIKey(int key_code);
RblxUIKey RblxUI_ImplOSX_KeyCodeToRblxUIKey(int key_code)
{
    switch (key_code)
    {
        case kVK_ANSI_A: return RblxUIKey_A;
        case kVK_ANSI_S: return RblxUIKey_S;
        case kVK_ANSI_D: return RblxUIKey_D;
        case kVK_ANSI_F: return RblxUIKey_F;
        case kVK_ANSI_H: return RblxUIKey_H;
        case kVK_ANSI_G: return RblxUIKey_G;
        case kVK_ANSI_Z: return RblxUIKey_Z;
        case kVK_ANSI_X: return RblxUIKey_X;
        case kVK_ANSI_C: return RblxUIKey_C;
        case kVK_ANSI_V: return RblxUIKey_V;
        case kVK_ANSI_B: return RblxUIKey_B;
        case kVK_ANSI_Q: return RblxUIKey_Q;
        case kVK_ANSI_W: return RblxUIKey_W;
        case kVK_ANSI_E: return RblxUIKey_E;
        case kVK_ANSI_R: return RblxUIKey_R;
        case kVK_ANSI_Y: return RblxUIKey_Y;
        case kVK_ANSI_T: return RblxUIKey_T;
        case kVK_ANSI_1: return RblxUIKey_1;
        case kVK_ANSI_2: return RblxUIKey_2;
        case kVK_ANSI_3: return RblxUIKey_3;
        case kVK_ANSI_4: return RblxUIKey_4;
        case kVK_ANSI_6: return RblxUIKey_6;
        case kVK_ANSI_5: return RblxUIKey_5;
        case kVK_ANSI_Equal: return RblxUIKey_Equal;
        case kVK_ANSI_9: return RblxUIKey_9;
        case kVK_ANSI_7: return RblxUIKey_7;
        case kVK_ANSI_Minus: return RblxUIKey_Minus;
        case kVK_ANSI_8: return RblxUIKey_8;
        case kVK_ANSI_0: return RblxUIKey_0;
        case kVK_ANSI_RightBracket: return RblxUIKey_RightBracket;
        case kVK_ANSI_O: return RblxUIKey_O;
        case kVK_ANSI_U: return RblxUIKey_U;
        case kVK_ANSI_LeftBracket: return RblxUIKey_LeftBracket;
        case kVK_ANSI_I: return RblxUIKey_I;
        case kVK_ANSI_P: return RblxUIKey_P;
        case kVK_ANSI_L: return RblxUIKey_L;
        case kVK_ANSI_J: return RblxUIKey_J;
        case kVK_ANSI_Quote: return RblxUIKey_Apostrophe;
        case kVK_ANSI_K: return RblxUIKey_K;
        case kVK_ANSI_Semicolon: return RblxUIKey_Semicolon;
        case kVK_ANSI_Backslash: return RblxUIKey_Backslash;
        case kVK_ANSI_Comma: return RblxUIKey_Comma;
        case kVK_ANSI_Slash: return RblxUIKey_Slash;
        case kVK_ANSI_N: return RblxUIKey_N;
        case kVK_ANSI_M: return RblxUIKey_M;
        case kVK_ANSI_Period: return RblxUIKey_Period;
        case kVK_ANSI_Grave: return RblxUIKey_GraveAccent;
        case kVK_ANSI_KeypadDecimal: return RblxUIKey_KeypadDecimal;
        case kVK_ANSI_KeypadMultiply: return RblxUIKey_KeypadMultiply;
        case kVK_ANSI_KeypadPlus: return RblxUIKey_KeypadAdd;
        case kVK_ANSI_KeypadClear: return RblxUIKey_NumLock;
        case kVK_ANSI_KeypadDivide: return RblxUIKey_KeypadDivide;
        case kVK_ANSI_KeypadEnter: return RblxUIKey_KeypadEnter;
        case kVK_ANSI_KeypadMinus: return RblxUIKey_KeypadSubtract;
        case kVK_ANSI_KeypadEquals: return RblxUIKey_KeypadEqual;
        case kVK_ANSI_Keypad0: return RblxUIKey_Keypad0;
        case kVK_ANSI_Keypad1: return RblxUIKey_Keypad1;
        case kVK_ANSI_Keypad2: return RblxUIKey_Keypad2;
        case kVK_ANSI_Keypad3: return RblxUIKey_Keypad3;
        case kVK_ANSI_Keypad4: return RblxUIKey_Keypad4;
        case kVK_ANSI_Keypad5: return RblxUIKey_Keypad5;
        case kVK_ANSI_Keypad6: return RblxUIKey_Keypad6;
        case kVK_ANSI_Keypad7: return RblxUIKey_Keypad7;
        case kVK_ANSI_Keypad8: return RblxUIKey_Keypad8;
        case kVK_ANSI_Keypad9: return RblxUIKey_Keypad9;
        case kVK_Return: return RblxUIKey_Enter;
        case kVK_Tab: return RblxUIKey_Tab;
        case kVK_Space: return RblxUIKey_Space;
        case kVK_Delete: return RblxUIKey_Backspace;
        case kVK_Escape: return RblxUIKey_Escape;
        case kVK_CapsLock: return RblxUIKey_CapsLock;
        case kVK_Control: return RblxUIKey_LeftCtrl;
        case kVK_Shift: return RblxUIKey_LeftShift;
        case kVK_Option: return RblxUIKey_LeftAlt;
        case kVK_Command: return RblxUIKey_LeftSuper;
        case kVK_RightControl: return RblxUIKey_RightCtrl;
        case kVK_RightShift: return RblxUIKey_RightShift;
        case kVK_RightOption: return RblxUIKey_RightAlt;
        case kVK_RightCommand: return RblxUIKey_RightSuper;
//      case kVK_Function: return RblxUIKey_;
//      case kVK_VolumeUp: return RblxUIKey_;
//      case kVK_VolumeDown: return RblxUIKey_;
//      case kVK_Mute: return RblxUIKey_;
        case kVK_F1: return RblxUIKey_F1;
        case kVK_F2: return RblxUIKey_F2;
        case kVK_F3: return RblxUIKey_F3;
        case kVK_F4: return RblxUIKey_F4;
        case kVK_F5: return RblxUIKey_F5;
        case kVK_F6: return RblxUIKey_F6;
        case kVK_F7: return RblxUIKey_F7;
        case kVK_F8: return RblxUIKey_F8;
        case kVK_F9: return RblxUIKey_F9;
        case kVK_F10: return RblxUIKey_F10;
        case kVK_F11: return RblxUIKey_F11;
        case kVK_F12: return RblxUIKey_F12;
        case kVK_F13: return RblxUIKey_F13;
        case kVK_F14: return RblxUIKey_F14;
        case kVK_F15: return RblxUIKey_F15;
        case kVK_F16: return RblxUIKey_F16;
        case kVK_F17: return RblxUIKey_F17;
        case kVK_F18: return RblxUIKey_F18;
        case kVK_F19: return RblxUIKey_F19;
        case kVK_F20: return RblxUIKey_F20;
        case 0x6E: return RblxUIKey_Menu;
        case kVK_Help: return RblxUIKey_Insert;
        case kVK_Home: return RblxUIKey_Home;
        case kVK_PageUp: return RblxUIKey_PageUp;
        case kVK_ForwardDelete: return RblxUIKey_Delete;
        case kVK_End: return RblxUIKey_End;
        case kVK_PageDown: return RblxUIKey_PageDown;
        case kVK_LeftArrow: return RblxUIKey_LeftArrow;
        case kVK_RightArrow: return RblxUIKey_RightArrow;
        case kVK_DownArrow: return RblxUIKey_DownArrow;
        case kVK_UpArrow: return RblxUIKey_UpArrow;
        default: return RblxUIKey_None;
    }
}

#ifdef RBLXUI_IMPL_METAL_CPP_EXTENSIONS

RBLXUI_IMPL_API bool RblxUI_ImplOSX_Init(void* _Nonnull view) {
    return RblxUI_ImplOSX_Init((__bridge NSView*)(view));
}

RBLXUI_IMPL_API void RblxUI_ImplOSX_NewFrame(void* _Nullable view) {
    return RblxUI_ImplOSX_NewFrame((__bridge NSView*)(view));
}

#endif


bool RblxUI_ImplOSX_Init(NSView* view)
{
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    RBLXUI_CHECKVERSION();
    RBLXUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    RblxUI_ImplOSX_Data* bd = RBLXUI_NEW(RblxUI_ImplOSX_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "rblxui_impl_osx";
    io.BackendFlags |= RblxUIBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= RblxUIBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= RblxUIBackendFlags_PlatformHasViewports;      // We can create multi-viewports on the Platform side (optional)
    //io.BackendFlags |= RblxUIBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)

    bd->Observer = [RblxUIObserver new];
    bd->Window = view.window ?: NSApp.orderedWindows.firstObject;
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw = (__bridge_retained void*)bd->Window;
    RblxUI_ImplOSX_UpdateMonitors();
    RblxUI_ImplOSX_InitMultiViewportSupport();

    // Load cursors. Some of them are undocumented.
    bd->MouseCursorHidden = false;
    bd->MouseCursors[RblxUIMouseCursor_Arrow] = [NSCursor arrowCursor];
    bd->MouseCursors[RblxUIMouseCursor_TextInput] = [NSCursor IBeamCursor];
    bd->MouseCursors[RblxUIMouseCursor_ResizeAll] = [NSCursor closedHandCursor];
    bd->MouseCursors[RblxUIMouseCursor_ResizeNS] = [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)] ? [NSCursor _windowResizeNorthSouthCursor] : [NSCursor resizeUpDownCursor];
    bd->MouseCursors[RblxUIMouseCursor_ResizeEW] = [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)] ? [NSCursor _windowResizeEastWestCursor] : [NSCursor resizeLeftRightCursor];
    bd->MouseCursors[RblxUIMouseCursor_ResizeNESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)] ? [NSCursor _windowResizeNorthEastSouthWestCursor] : [NSCursor closedHandCursor];
    bd->MouseCursors[RblxUIMouseCursor_ResizeNWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)] ? [NSCursor _windowResizeNorthWestSouthEastCursor] : [NSCursor closedHandCursor];
    bd->MouseCursors[RblxUIMouseCursor_Hand] = [NSCursor pointingHandCursor];
    bd->MouseCursors[RblxUIMouseCursor_Wait] = bd->MouseCursors[RblxUIMouseCursor_Progress] = [NSCursor respondsToSelector:@selector(busyButClickableCursor)] ? [NSCursor busyButClickableCursor] : [NSCursor arrowCursor];
    bd->MouseCursors[RblxUIMouseCursor_NotAllowed] = [NSCursor operationNotAllowedCursor];

    // Note that rblxui.cpp also include default OSX clipboard handlers which can be enabled
    // by adding '#define RBLXUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS' in rblxuiconfig.h and adding '-framework ApplicationServices' to your linker command-line.
    // Since we are already in ObjC land here, it is easy for us to add a clipboard handler using the NSPasteboard api.
    platform_io.Platform_SetClipboardTextFn = [](RblxUIContext*, const char* str) -> void
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pasteboard setString:[NSString stringWithUTF8String:str] forType:NSPasteboardTypeString];
    };

    platform_io.Platform_GetClipboardTextFn = [](RblxUIContext*) -> const char*
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        NSString* available = [pasteboard availableTypeFromArray: [NSArray arrayWithObject:NSPasteboardTypeString]];
        if (![available isEqualToString:NSPasteboardTypeString])
            return nullptr;

        NSString* string = [pasteboard stringForType:NSPasteboardTypeString];
        if (string == nil)
            return nullptr;

        const char* string_c = (const char*)[string UTF8String];
        size_t string_len = strlen(string_c);
        static RblxVector<char> s_clipboard;
        s_clipboard.resize((int)string_len + 1);
        strcpy(s_clipboard.Data, string_c);
        return s_clipboard.Data;
    };

    [[NSNotificationCenter defaultCenter] addObserver:bd->Observer
                                             selector:@selector(onApplicationBecomeActive:)
                                                 name:NSApplicationDidBecomeActiveNotification
                                               object:nil];
    [[NSNotificationCenter defaultCenter] addObserver:bd->Observer
                                             selector:@selector(onApplicationBecomeInactive:)
                                                 name:NSApplicationDidResignActiveNotification
                                               object:nil];

    // Add the NSTextInputClient to the view hierarchy,
    // to receive keyboard events and translate them to input text.
    bd->KeyEventResponder = [[KeyEventResponder alloc] initWithFrame:NSZeroRect];
    bd->InputContext = [[NSTextInputContext alloc] initWithClient:bd->KeyEventResponder];
    [view addSubview:bd->KeyEventResponder];
    RblxUI_ImplOSX_AddTrackingArea(view);

    platform_io.Platform_SetImeDataFn = [](RblxUIContext*, RblxUIViewport*, RblxUIPlatformImeData* data) -> void
    {
        RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
        if (data->WantVisible)
        {
            [bd->InputContext activate];
        }
        else
        {
            [bd->InputContext discardMarkedText];
            [bd->InputContext invalidateCharacterCoordinates];
            [bd->InputContext deactivate];
        }
        [bd->KeyEventResponder setImePosX:data->InputPos.x imePosY:data->InputPos.y + data->InputLineHeight];
    };

    return true;
}

void RblxUI_ImplOSX_Shutdown()
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");

    [[NSNotificationCenter defaultCenter] removeObserver:bd->Observer];
    bd->Observer = nullptr;
    if (bd->Monitor != nullptr)
    {
        [NSEvent removeMonitor:bd->Monitor];
        bd->Monitor = nullptr;
    }

    RblxUI_ImplOSX_ShutdownMultiViewportSupport();
    RblxUI_ImplOSX_DestroyBackendData();
    RblxUIIO& io = RblxUI::GetIO();
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(RblxUIBackendFlags_HasMouseCursors | RblxUIBackendFlags_HasGamepad | RblxUIBackendFlags_PlatformHasViewports);
    platform_io.ClearPlatformHandlers();
}

static void RblxUI_ImplOSX_UpdateMouseCursor()
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    RblxUIIO& io = RblxUI::GetIO();
    if (io.ConfigFlags & RblxUIConfigFlags_NoMouseCursorChange)
        return;

    RblxUIMouseCursor rblxui_cursor = RblxUI::GetMouseCursor();
    if (io.MouseDrawCursor || rblxui_cursor == RblxUIMouseCursor_None)
    {
        // Hide OS mouse cursor if rblxui is drawing it or if it wants no cursor
        if (!bd->MouseCursorHidden)
        {
            bd->MouseCursorHidden = true;
            [NSCursor hide];
        }
    }
    else
    {
        NSCursor* desired = bd->MouseCursors[rblxui_cursor] ?: bd->MouseCursors[RblxUIMouseCursor_Arrow];
        // -[NSCursor set] generates measurable overhead if called unconditionally.
        if (desired != NSCursor.currentCursor)
        {
            [desired set];
        }
        if (bd->MouseCursorHidden)
        {
            bd->MouseCursorHidden = false;
            [NSCursor unhide];
        }
    }
}

static void RblxUI_ImplOSX_UpdateGamepads()
{
    RblxUIIO& io = RblxUI::GetIO();

#if APPLE_HAS_CONTROLLER
    GCController* controller = GCController.current;
#else
    GCController* controller = GCController.controllers.firstObject;
#endif
    if (controller == nil || controller.extendedGamepad == nil)
    {
        io.BackendFlags &= ~RblxUIBackendFlags_HasGamepad;
        return;
    }

    GCExtendedGamepad* gp = controller.extendedGamepad;

    // Update gamepad inputs
    #define RBLXUI_SATURATE(V)                        (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
    #define MAP_BUTTON(KEY_NO, BUTTON_NAME)       { io.AddKeyEvent(KEY_NO, gp.BUTTON_NAME.isPressed); }
    #define MAP_ANALOG(KEY_NO, AXIS_NAME, V0, V1) { float vn = (float)(gp.AXIS_NAME.value - V0) / (float)(V1 - V0); vn = RBLXUI_SATURATE(vn); io.AddKeyAnalogEvent(KEY_NO, vn > 0.1f, vn); }
    const float thumb_dead_zone = 0.0f;

#if APPLE_HAS_BUTTON_OPTIONS
    MAP_BUTTON(RblxUIKey_GamepadBack,            buttonOptions);
#endif
    MAP_BUTTON(RblxUIKey_GamepadFaceLeft,        buttonX);              // Xbox X, PS Square
    MAP_BUTTON(RblxUIKey_GamepadFaceRight,       buttonB);              // Xbox B, PS Circle
    MAP_BUTTON(RblxUIKey_GamepadFaceUp,          buttonY);              // Xbox Y, PS Triangle
    MAP_BUTTON(RblxUIKey_GamepadFaceDown,        buttonA);              // Xbox A, PS Cross
    MAP_BUTTON(RblxUIKey_GamepadDpadLeft,        dpad.left);
    MAP_BUTTON(RblxUIKey_GamepadDpadRight,       dpad.right);
    MAP_BUTTON(RblxUIKey_GamepadDpadUp,          dpad.up);
    MAP_BUTTON(RblxUIKey_GamepadDpadDown,        dpad.down);
    MAP_ANALOG(RblxUIKey_GamepadL1,              leftShoulder, 0.0f, 1.0f);
    MAP_ANALOG(RblxUIKey_GamepadR1,              rightShoulder, 0.0f, 1.0f);
    MAP_ANALOG(RblxUIKey_GamepadL2,              leftTrigger,  0.0f, 1.0f);
    MAP_ANALOG(RblxUIKey_GamepadR2,              rightTrigger, 0.0f, 1.0f);
#if APPLE_HAS_THUMBSTICKS
    MAP_BUTTON(RblxUIKey_GamepadL3,              leftThumbstickButton);
    MAP_BUTTON(RblxUIKey_GamepadR3,              rightThumbstickButton);
#endif
    MAP_ANALOG(RblxUIKey_GamepadLStickLeft,      leftThumbstick.xAxis,  -thumb_dead_zone, -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadLStickRight,     leftThumbstick.xAxis,  +thumb_dead_zone, +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadLStickUp,        leftThumbstick.yAxis,  +thumb_dead_zone, +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadLStickDown,      leftThumbstick.yAxis,  -thumb_dead_zone, -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickLeft,      rightThumbstick.xAxis, -thumb_dead_zone, -1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickRight,     rightThumbstick.xAxis, +thumb_dead_zone, +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickUp,        rightThumbstick.yAxis, +thumb_dead_zone, +1.0f);
    MAP_ANALOG(RblxUIKey_GamepadRStickDown,      rightThumbstick.yAxis, -thumb_dead_zone, -1.0f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG

    io.BackendFlags |= RblxUIBackendFlags_HasGamepad;
}

static void RblxUI_ImplOSX_UpdateImePosWithView(NSView* view)
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    RblxUIIO& io = RblxUI::GetIO();
    if (io.WantTextInput)
        [bd->KeyEventResponder updateImePosWithView:view];
}

void RblxUI_ImplOSX_NewFrame(NSView* view)
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    RBLXUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call RblxUI_ImplOSX_Init()?");
    RblxUIIO& io = RblxUI::GetIO();

    // Setup display size
    if (view)
    {
        const float fb_scale = (float)[view.window backingScaleFactor];
        io.DisplaySize = RblxVec2((float)view.bounds.size.width, (float)view.bounds.size.height);
        io.DisplayFramebufferScale = RblxVec2(fb_scale, fb_scale);
    }

    // Setup time step
    if (bd->Time == 0.0)
        bd->Time = GetMachAbsoluteTimeInSeconds();

    double current_time = GetMachAbsoluteTimeInSeconds();
    io.DeltaTime = (float)(current_time - bd->Time);
    bd->Time = current_time;

    RblxUI_ImplOSX_UpdateMouseCursor();
    RblxUI_ImplOSX_UpdateGamepads();
    RblxUI_ImplOSX_UpdateImePosWithView(view);
}

// Must only be called for a mouse event, otherwise an exception occurs
// (Note that NSEventTypeScrollWheel is considered "other input". Oddly enough an exception does not occur with it, but the value will sometimes be wrong!)
static RblxUIMouseSource GetMouseSource(NSEvent* event)
{
    switch (event.subtype)
    {
        case NSEventSubtypeTabletPoint:
            return RblxUIMouseSource_Pen;
        // macOS considers input from relative touch devices (like the trackpad or Apple Magic Mouse) to be touch input.
        // This doesn't really make sense for Dear RblxUI, which expects absolute touch devices only.
        // There does not seem to be a simple way to disambiguate things here so we consider NSEventSubtypeTouch events to always come from mice.
        // See https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/HandlingTouchEvents/HandlingTouchEvents.html#//apple_ref/doc/uid/10000060i-CH13-SW24
        //case NSEventSubtypeTouch:
        //    return RblxUIMouseSource_TouchScreen;
        case NSEventSubtypeMouseEvent:
        default:
            return RblxUIMouseSource_Mouse;
    }
}

static bool RblxUI_ImplOSX_HandleEvent(NSEvent* event, NSView* view)
{
    // Only process events from the window containing RblxUI view
    if (!RblxUI::FindViewportByPlatformHandle((__bridge void*)event.window))
        return false;

    RblxUIIO& io = RblxUI::GetIO();
    if (event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeOtherMouseDown)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < RblxUIMouseButton_COUNT)
        {
            io.AddMouseSourceEvent(GetMouseSource(event));
            io.AddMouseButtonEvent(button, true);
        }
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp || event.type == NSEventTypeOtherMouseUp)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < RblxUIMouseButton_COUNT)
        {
            io.AddMouseSourceEvent(GetMouseSource(event));
            io.AddMouseButtonEvent(button, false);
        }
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged || event.type == NSEventTypeRightMouseDragged || event.type == NSEventTypeOtherMouseDragged)
    {
        NSPoint mousePoint;
        if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
        {
            mousePoint = NSEvent.mouseLocation;
            mousePoint.y = CGDisplayPixelsHigh(kCGDirectMainDisplay) - mousePoint.y; // Normalize y coordinate to top-left of main display.
        }
        else
        {
            mousePoint = event.locationInWindow;
            if (event.window == nil)
                mousePoint = [[view window] convertPointFromScreen:mousePoint];
            mousePoint = [view convertPoint:mousePoint fromView:nil]; // Convert to local coordinates of view
            if ([view isFlipped])
                mousePoint = NSMakePoint(mousePoint.x, mousePoint.y);
            else
                mousePoint = NSMakePoint(mousePoint.x, view.bounds.size.height - mousePoint.y);
        }
        io.AddMouseSourceEvent(GetMouseSource(event));
        io.AddMousePosEvent((float)mousePoint.x, (float)mousePoint.y);
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeScrollWheel)
    {
        // Ignore canceled events.
        //
        // From macOS 12.1, scrolling with two fingers and then decelerating
        // by tapping two fingers results in two events appearing:
        //
        // 1. A scroll wheel NSEvent, with a phase == NSEventPhaseMayBegin, when the user taps
        // two fingers to decelerate or stop the scroll events.
        //
        // 2. A scroll wheel NSEvent, with a phase == NSEventPhaseCancelled, when the user releases the
        // two-finger tap. It is this event that sometimes contains large values for scrollingDeltaX and
        // scrollingDeltaY. When these are added to the current x and y positions of the scrolling view,
        // it appears to jump up or down. It can be observed in Preview, various JetBrains IDEs and here.
        if (event.phase == NSEventPhaseCancelled)
            return false;

        double wheel_dx = 0.0;
        double wheel_dy = 0.0;

        #if MAC_OS_X_VERSION_MAX_ALLOWED >= 1070
        if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6)
        {
            wheel_dx = [event scrollingDeltaX];
            wheel_dy = [event scrollingDeltaY];
            if ([event hasPreciseScrollingDeltas])
            {
                wheel_dx *= 0.01;
                wheel_dy *= 0.01;
            }
        }
        else
        #endif // MAC_OS_X_VERSION_MAX_ALLOWED
        {
            wheel_dx = [event deltaX] * 0.1;
            wheel_dy = [event deltaY] * 0.1;
        }
        if (wheel_dx != 0.0 || wheel_dy != 0.0)
            io.AddMouseWheelEvent((float)wheel_dx, (float)wheel_dy);

        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeKeyDown || event.type == NSEventTypeKeyUp)
    {
        if ([event isARepeat])
            return io.WantCaptureKeyboard;

        int key_code = (int)[event keyCode];
        RblxUIKey key = RblxUI_ImplOSX_KeyCodeToRblxUIKey(key_code);
        io.AddKeyEvent(key, event.type == NSEventTypeKeyDown);
        io.SetKeyEventNativeData(key, key_code, -1); // To support legacy indexing (<1.87 user code)

        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeFlagsChanged)
    {
        unsigned short key_code = [event keyCode];
        NSEventModifierFlags modifier_flags = [event modifierFlags];

        io.AddKeyEvent(RblxUIMod_Shift, (modifier_flags & NSEventModifierFlagShift)   != 0);
        io.AddKeyEvent(RblxUIMod_Ctrl,  (modifier_flags & NSEventModifierFlagControl) != 0);
        io.AddKeyEvent(RblxUIMod_Alt,   (modifier_flags & NSEventModifierFlagOption)  != 0);
        io.AddKeyEvent(RblxUIMod_Super, (modifier_flags & NSEventModifierFlagCommand) != 0);

        RblxUIKey key = RblxUI_ImplOSX_KeyCodeToRblxUIKey(key_code);
        if (key != RblxUIKey_None)
        {
            // macOS does not generate down/up event for modifiers. We're trying
            // to use hardware dependent masks to extract that information.
            // 'rblxui_mask' is left as a fallback.
            NSEventModifierFlags mask = 0;
            switch (key)
            {
                case RblxUIKey_LeftCtrl:   mask = 0x0001; break;
                case RblxUIKey_RightCtrl:  mask = 0x2000; break;
                case RblxUIKey_LeftShift:  mask = 0x0002; break;
                case RblxUIKey_RightShift: mask = 0x0004; break;
                case RblxUIKey_LeftSuper:  mask = 0x0008; break;
                case RblxUIKey_RightSuper: mask = 0x0010; break;
                case RblxUIKey_LeftAlt:    mask = 0x0020; break;
                case RblxUIKey_RightAlt:   mask = 0x0040; break;
                default:
                    return io.WantCaptureKeyboard;
            }
            io.AddKeyEvent(key, (modifier_flags & mask) != 0);
            io.SetKeyEventNativeData(key, key_code, -1); // To support legacy indexing (<1.87 user code)
        }

        return io.WantCaptureKeyboard;
    }

    return false;
}

static void RblxUI_ImplOSX_AddTrackingArea(NSView* _Nonnull view)
{
    // If we want to receive key events, we either need to be in the responder chain of the key view,
    // or else we can install a local monitor. The consequence of this heavy-handed approach is that
    // we receive events for all controls, not just Dear RblxUI widgets. If we had native controls in our
    // window, we'd want to be much more careful than just ingesting the complete event stream.
    // To match the behavior of other backends, we pass every event down to the OS.
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    if (bd->Monitor)
        return;
    NSEventMask eventMask = 0;
    eventMask |= NSEventMaskMouseMoved | NSEventMaskScrollWheel;
    eventMask |= NSEventMaskLeftMouseDown | NSEventMaskLeftMouseUp | NSEventMaskLeftMouseDragged;
    eventMask |= NSEventMaskRightMouseDown | NSEventMaskRightMouseUp | NSEventMaskRightMouseDragged;
    eventMask |= NSEventMaskOtherMouseDown | NSEventMaskOtherMouseUp | NSEventMaskOtherMouseDragged;
    eventMask |= NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;
    bd->Monitor = [NSEvent addLocalMonitorForEventsMatchingMask:eventMask
                                                        handler:^NSEvent* _Nullable(NSEvent* event)
    {
        RblxUI_ImplOSX_HandleEvent(event, view);
        return event;
    }];
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear rblxui or creating a new binding for dear rblxui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct RblxUI_ImplOSX_ViewportData
{
    NSWindow*        Window;
    bool             WindowOwned;

    RblxUI_ImplOSX_ViewportData()  { WindowOwned = false; }
    ~RblxUI_ImplOSX_ViewportData() { RBLXUI_ASSERT(Window == nil); }
};

@interface RblxUI_ImplOSX_Window: NSWindow
@end

@implementation RblxUI_ImplOSX_Window

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

@end

static void ConvertNSRect(NSRect* r)
{
    NSRect firstScreenFrame = NSScreen.screens[0].frame;
    RBLXUI_ASSERT(firstScreenFrame.origin.x == 0 && firstScreenFrame.origin.y == 0);
    r->origin.y = firstScreenFrame.size.height - r->origin.y - r->size.height;
}

static void RblxUI_ImplOSX_CreateWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    RblxUI_ImplOSX_ViewportData* vd = RBLXUI_NEW(RblxUI_ImplOSX_ViewportData)();
    viewport->PlatformUserData = vd;

    NSScreen* screen = bd->Window.screen;
    NSRect rect = NSMakeRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
    ConvertNSRect(&rect);

    NSWindowStyleMask styleMask = 0;
    if (viewport->Flags & RblxUIViewportFlags_NoDecoration)
        styleMask |= NSWindowStyleMaskBorderless;
    else
        styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

    NSWindow* window = [[RblxUI_ImplOSX_Window alloc] initWithContentRect:rect
                                                               styleMask:styleMask
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:YES
                                                                  screen:screen];
    if (viewport->Flags & RblxUIViewportFlags_TopMost)
        [window setLevel:NSFloatingWindowLevel];

    window.title = @"Untitled";
    window.opaque = YES;

    KeyEventResponder* view = [[KeyEventResponder alloc] initWithFrame:rect];
    if (floor(NSAppKitVersionNumber) > NSAppKitVersionNumber10_6 && ceil(NSAppKitVersionNumber) < NSAppKitVersionNumber10_15)
        [view setWantsBestResolutionOpenGLSurface:YES];

    window.contentView = view;

    vd->Window = window;
    vd->WindowOwned = true;
    viewport->PlatformRequestResize = false;
    viewport->PlatformHandle = viewport->PlatformHandleRaw = (__bridge_retained void*)window;
}

static void RblxUI_ImplOSX_DestroyWindow(RblxUIViewport* viewport)
{
    if (RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData)
    {
        NSWindow* window = vd->Window;
        if (window != nil && vd->WindowOwned)
        {
            window.contentView = nil;
            window.contentViewController = nil;
            [window orderOut:nil];
        }
        vd->Window = nil;
        RBLXUI_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = viewport->PlatformHandleRaw = nullptr;
}

static void RblxUI_ImplOSX_ShowWindow(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    if (viewport->Flags & RblxUIViewportFlags_NoFocusOnAppearing)
        [vd->Window orderFront:nil];
    else
        [vd->Window makeKeyAndOrderFront:nil];

    [vd->Window setIsVisible:YES];
}

static RblxVec2 RblxUI_ImplOSX_GetWindowPos(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSRect frame = window.frame;
    NSRect contentRect = window.contentLayoutRect;
    if (window.styleMask & NSWindowStyleMaskFullSizeContentView) // No title bar windows should be considered.
        contentRect = frame;

    NSRect firstScreenFrame = NSScreen.screens[0].frame;
    return RblxVec2(frame.origin.x, firstScreenFrame.size.height - frame.origin.y - contentRect.size.height);
}

static void RblxUI_ImplOSX_SetWindowPos(RblxUIViewport* viewport, RblxVec2 pos)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSSize size = window.frame.size;

    NSRect r = NSMakeRect(pos.x, pos.y, size.width, size.height);
    ConvertNSRect(&r);
    [window setFrameOrigin:r.origin];
}

static RblxVec2 RblxUI_ImplOSX_GetWindowSize(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSSize size = window.contentLayoutRect.size;
    return RblxVec2(size.width, size.height);
}

static void RblxUI_ImplOSX_SetWindowSize(RblxUIViewport* viewport, RblxVec2 size)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSRect rect = window.frame;
    rect.origin.y -= (size.y - rect.size.height);
    rect.size.width = size.x;
    rect.size.height = size.y;
    [window setFrame:rect display:YES];
}

static RblxVec2 RblxUI_ImplOSX_GetWindowFramebufferScale(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    NSWindow* window = vd->Window;
    const float fb_scale = (float)[window backingScaleFactor];
    return RblxVec2(fb_scale, fb_scale);
}

static void RblxUI_ImplOSX_SetWindowFocus(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);
    [vd->Window makeKeyAndOrderFront:bd->Window];
}

static bool RblxUI_ImplOSX_GetWindowFocus(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    return vd->Window.isKeyWindow;
}

static bool RblxUI_ImplOSX_GetWindowMinimized(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    return vd->Window.isMiniaturized;
}

static void RblxUI_ImplOSX_SetWindowTitle(RblxUIViewport* viewport, const char* title)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    vd->Window.title = [NSString stringWithUTF8String:title];
}

static void RblxUI_ImplOSX_SetWindowAlpha(RblxUIViewport* viewport, float alpha)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);
    RBLXUI_ASSERT(alpha >= 0.0f && alpha <= 1.0f);

    vd->Window.alphaValue = alpha;
}

static float RblxUI_ImplOSX_GetWindowDpiScale(RblxUIViewport* viewport)
{
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    RBLXUI_ASSERT(vd->Window != 0);

    return vd->Window.backingScaleFactor;
}

static void RblxUI_ImplOSX_UpdateMonitors()
{
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.Monitors.resize(0);

    NSRect firstScreenFrame = NSScreen.screens[0].frame;
    RBLXUI_ASSERT(firstScreenFrame.origin.x == 0 && firstScreenFrame.origin.y == 0);

    for (NSScreen* screen in NSScreen.screens)
    {
        NSRect frame = screen.frame;
        NSRect visibleFrame = screen.visibleFrame;
        ConvertNSRect(&frame);
        ConvertNSRect(&visibleFrame);

        RblxUIPlatformMonitor rblxui_monitor;
        rblxui_monitor.MainPos = RblxVec2(frame.origin.x, frame.origin.y);
        rblxui_monitor.MainSize = RblxVec2(frame.size.width, frame.size.height);
        rblxui_monitor.WorkPos = RblxVec2(visibleFrame.origin.x, visibleFrame.origin.y);
        rblxui_monitor.WorkSize = RblxVec2(visibleFrame.size.width, visibleFrame.size.height);
        rblxui_monitor.DpiScale = screen.backingScaleFactor;
        rblxui_monitor.PlatformHandle = (__bridge_retained void*)screen;

        platform_io.Monitors.push_back(rblxui_monitor);
    }
}

static void RblxUI_ImplOSX_InitMultiViewportSupport()
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();

    // Register platform interface (will be coupled with a renderer interface)
    RblxUIPlatformIO& platform_io = RblxUI::GetPlatformIO();
    platform_io.Platform_CreateWindow = RblxUI_ImplOSX_CreateWindow;
    platform_io.Platform_DestroyWindow = RblxUI_ImplOSX_DestroyWindow;
    platform_io.Platform_ShowWindow = RblxUI_ImplOSX_ShowWindow;
    platform_io.Platform_SetWindowPos = RblxUI_ImplOSX_SetWindowPos;
    platform_io.Platform_GetWindowPos = RblxUI_ImplOSX_GetWindowPos;
    platform_io.Platform_SetWindowSize = RblxUI_ImplOSX_SetWindowSize;
    platform_io.Platform_GetWindowSize = RblxUI_ImplOSX_GetWindowSize;
    platform_io.Platform_GetWindowFramebufferScale = RblxUI_ImplOSX_GetWindowFramebufferScale;
    platform_io.Platform_SetWindowFocus = RblxUI_ImplOSX_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = RblxUI_ImplOSX_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = RblxUI_ImplOSX_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = RblxUI_ImplOSX_SetWindowTitle;
    platform_io.Platform_SetWindowAlpha = RblxUI_ImplOSX_SetWindowAlpha;
    platform_io.Platform_GetWindowDpiScale = RblxUI_ImplOSX_GetWindowDpiScale; // FIXME-DPI

    // Register main window handle (which is owned by the main application, not by us)
    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI_ImplOSX_ViewportData* vd = RBLXUI_NEW(RblxUI_ImplOSX_ViewportData)();
    vd->Window = bd->Window;
    vd->WindowOwned = false;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (__bridge void*)bd->Window;

    [NSNotificationCenter.defaultCenter addObserver:bd->Observer
                                           selector:@selector(displaysDidChange:)
                                               name:NSApplicationDidChangeScreenParametersNotification
                                             object:nil];
}

static void RblxUI_ImplOSX_ShutdownMultiViewportSupport()
{
    RblxUI_ImplOSX_Data* bd = RblxUI_ImplOSX_GetBackendData();
    [NSNotificationCenter.defaultCenter removeObserver:bd->Observer
                                                  name:NSApplicationDidChangeScreenParametersNotification
                                                object:nil];
    bd->Observer = nullptr;
    bd->Window = nullptr;
    if (bd->Monitor != nullptr)
    {
        [NSEvent removeMonitor:bd->Monitor];
        bd->Monitor = nullptr;
    }

    RblxUIViewport* main_viewport = RblxUI::GetMainViewport();
    RblxUI_ImplOSX_ViewportData* vd = (RblxUI_ImplOSX_ViewportData*)main_viewport->PlatformUserData;
    RBLXUI_DELETE(vd);
    main_viewport->PlatformUserData = nullptr;
    RblxUI::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

#endif // #ifndef RBLXUI_DISABLE
