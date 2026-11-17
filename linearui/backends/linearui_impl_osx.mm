// dear linearui: Platform Backend for OSX / Cocoa
// This needs to be used along with a Renderer (e.g. OpenGL2, OpenGL3, Vulkan, Metal..)
// - Not well tested. If you want a portable application, prefer using the LINEARWIN or SDL platform Backends on Mac.
// - Requires linking with the GameController framework ("-framework GameController").

// Implemented features:
//  [X] Platform: Clipboard support is part of core Dear LinearUI (no specific code in this backend).
//  [X] Platform: Mouse support. Can discriminate Mouse/Pen.
//  [X] Platform: Keyboard support. Since 1.87 we are using the io.AddKeyEvent() function. Pass LinearUIKey values to all key functions e.g. LinearUI::IsKeyPressed(LinearUIKey_Space). [Legacy kVK_* values are obsolete since 1.87 and not supported since 1.91.5]
//  [X] Platform: Gamepad support.
//  [X] Platform: Mouse cursor shape and visibility (LinearUIBackendFlags_HasMouseCursors). Disable with 'io.ConfigFlags |= LinearUIConfigFlags_NoMouseCursorChange'.
//  [X] Platform: IME support.
//  [x] Platform: Multi-viewport / platform windows.
// Missing features or Issues:
//  [ ] Platform: Multi-viewport: Window size not correctly reported when enabling io.ConfigViewportsNoDecoration
//  [ ] Platform: Multi-viewport: Missing LinearUIBackendFlags_HasParentViewport support. The viewport->ParentViewportID field is ignored, and therefore io.ConfigViewportsNoDefaultParent has no effect either.

// You can use unmodified linearui_impl_* files in your project. See examples/ folder for examples of using this.
// Prefer including the entire linearui/ repository into your project (either as a copy or as a submodule), and only build the backends you need.
// Learn about Dear LinearUI:
// - FAQ                  https://dearlinearui.com/faq
// - Getting Started      https://dearlinearui.com/getting-started
// - Documentation        https://dearlinearui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of linearui.cpp

#import "linearui.h"
#ifndef LINEARUI_DISABLE
#import "linearui_impl_osx.h"
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>
#import <GameController/GameController.h>
#import <time.h>

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2026-XX-XX: Added support for multiple windows via the LinearUIPlatformIO interface.
//  2025-09-18: Call platform_io.ClearPlatformHandlers() on shutdown.
//  2025-07-08: [Docking] Fixed multi-viewport handling broken on 2025-06-02. (#8644, #8777)
//  2025-06-27: Added LinearUIMouseCursor_Wait and LinearUIMouseCursor_Progress mouse cursor support.
//  2025-06-12: LinearUI_ImplOSX_HandleEvent() only process event for window containing our view. (#8644)
//  2025-05-15: [Docking] Add Platform_GetWindowFramebufferScale() handler, to allow varying Retina display density on multiple monitors.
//  2025-03-21: Fill gamepad inputs and set LinearUIBackendFlags_HasGamepad regardless of LinearUIConfigFlags_NavEnableGamepad being set.
//  2025-01-20: Removed notification observer when shutting down. (#8331)
//  2024-08-22: moved some OS/backend related function pointers from LinearUIIO to LinearUIPlatformIO:
//               - io.GetClipboardTextFn    -> platform_io.Platform_GetClipboardTextFn
//               - io.SetClipboardTextFn    -> platform_io.Platform_SetClipboardTextFn
//               - io.PlatformSetImeDataFn  -> platform_io.Platform_SetImeDataFn
//  2024-07-02: Update for io.SetPlatformImeDataFn() -> io.PlatformSetImeDataFn() renaming in main library.
//  2023-10-05: Inputs: Added support for extra LinearUIKey values: F13 to F20 function keys. Stopped mapping F13 into PrintScreen.
//  2023-04-09: Inputs: Added support for io.AddMouseSourceEvent() to discriminate LinearUIMouseSource_Mouse/LinearUIMouseSource_Pen.
//  2023-02-01: Fixed scroll wheel scaling for devices emitting events with hasPreciseScrollingDeltas==false (e.g. non-Apple mice).
//  2022-11-02: Fixed mouse coordinates before clicking the host window.
//  2022-10-06: Fixed mouse inputs on flipped views.
//  2022-09-26: Inputs: Renamed LinearUIKey_ModXXX introduced in 1.87 to LinearUIMod_XXX (old names still supported).
//  2022-05-03: Inputs: Removed LinearUI_ImplOSX_HandleEvent() from backend API in favor of backend automatically handling event capture.
//  2022-04-27: Misc: Store backend data in a per-context struct, allowing to use this backend with multiple contexts.
//  2022-03-22: Inputs: Monitor NSKeyUp events to catch missing keyUp for key when user press Cmd + key
//  2022-02-07: Inputs: Forward keyDown/keyUp events to OS when unused by dear linearui.
//  2022-01-31: Fixed building with old Xcode versions that are missing gamepad features.
//  2022-01-26: Inputs: replaced short-lived io.AddKeyModsEvent() (added two weeks ago) with io.AddKeyEvent() using LinearUIKey_ModXXX flags. Sorry for the confusion.
//  2021-01-20: Inputs: calling new io.AddKeyAnalogEvent() for gamepad support, instead of writing directly to io.NavInputs[].
//  2022-01-17: Inputs: calling new io.AddMousePosEvent(), io.AddMouseButtonEvent(), io.AddMouseWheelEvent() API (1.87+).
//  2022-01-12: Inputs: Added basic Platform IME support, hooking the io.SetPlatformImeDataFn() function.
//  2022-01-10: Inputs: calling new io.AddKeyEvent(), io.AddKeyModsEvent() + io.SetKeyEventNativeData() API (1.87+). Support for full LinearUIKey range.
//  2021-12-13: *BREAKING CHANGE* Add NSView parameter to LinearUI_ImplOSX_Init(). Generally fix keyboard support. Using kVK_* codes for keyboard keys.
//  2021-12-13: Add game controller support.
//  2021-09-21: Use mach_absolute_time as CFAbsoluteTimeGetCurrent can jump backwards.
//  2021-08-17: Calling io.AddFocusEvent() on NSApplicationDidBecomeActiveNotification/NSApplicationDidResignActiveNotification events.
//  2021-06-23: Inputs: Added a fix for shortcuts using CTRL key instead of CMD key.
//  2021-04-19: Inputs: Added a fix for keys remaining stuck in pressed state when CMD-tabbing into different application.
//  2021-01-27: Inputs: Added a fix for mouse position not being reported when mouse buttons other than left one are down.
//  2020-10-28: Inputs: Added a fix for handling keypad-enter key.
//  2020-05-25: Inputs: Added a fix for missing trackpad clicks when done with "soft tap".
//  2019-12-05: Inputs: Added support for LinearUIMouseCursor_NotAllowed mouse cursor.
//  2019-10-11: Inputs:  Fix using Backspace key.
//  2019-07-21: Re-added clipboard handlers as they are not enabled by default in core linearui.cpp (reverted 2019-05-18 change).
//  2019-05-28: Inputs: Added mouse cursor shape and visibility support.
//  2019-05-18: Misc: Removed clipboard handlers as they are now supported by core linearui.cpp.
//  2019-05-11: Inputs: Don't filter character values before calling AddInputCharacter() apart from 0xF700..0xFFFF range.
//  2018-11-30: Misc: Setting up io.BackendPlatformName so it can be displayed in the About Window.
//  2018-07-07: Initial version.

#define APPLE_HAS_BUTTON_OPTIONS (__IPHONE_OS_VERSION_MIN_REQUIRED >= 130000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101500 || __TV_OS_VERSION_MIN_REQUIRED >= 130000)
#define APPLE_HAS_CONTROLLER     (__IPHONE_OS_VERSION_MIN_REQUIRED >= 140000 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 110000 || __TV_OS_VERSION_MIN_REQUIRED >= 140000)
#define APPLE_HAS_THUMBSTICKS    (__IPHONE_OS_VERSION_MIN_REQUIRED >= 120100 || __MAC_OS_X_VERSION_MIN_REQUIRED >= 101401 || __TV_OS_VERSION_MIN_REQUIRED >= 120100)

@class LinearUIObserver;
@class KeyEventResponder;

// Data
struct LinearUI_ImplOSX_Data
{
    CFTimeInterval              Time;
    NSCursor*                   MouseCursors[LinearUIMouseCursor_COUNT];
    bool                        MouseCursorHidden;
    LinearUIObserver*              Observer;
    KeyEventResponder*          KeyEventResponder;
    NSTextInputContext*         InputContext;
    id                          Monitor;
    NSWindow*                   Window;

    LinearUI_ImplOSX_Data()        { memset((void*)this, 0, sizeof(*this)); }
};

static LinearUI_ImplOSX_Data*      LinearUI_ImplOSX_GetBackendData()      { return (LinearUI_ImplOSX_Data*)LinearUI::GetIO().BackendPlatformUserData; }
static void                     LinearUI_ImplOSX_DestroyBackendData()  { LINEARUI_DELETE(LinearUI_ImplOSX_GetBackendData()); }

static inline CFTimeInterval    GetMachAbsoluteTimeInSeconds()      { return (CFTimeInterval)(double)(clock_gettime_nsec_np(CLOCK_UPTIME_RAW) / 1e9); }

// Forward Declarations
static void LinearUI_ImplOSX_InitMultiViewportSupport();
static void LinearUI_ImplOSX_ShutdownMultiViewportSupport();
static void LinearUI_ImplOSX_UpdateMonitors();
static void LinearUI_ImplOSX_AddTrackingArea(NSView* _Nonnull view);
static bool LinearUI_ImplOSX_HandleEvent(NSEvent* event, NSView* view);

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
 and LINEARWIN:
  https://github.com/linearwin/linearwin/blob/b55a517ae0c7b5127dffa79a64f5406021bf9076/src/cocoa_window.m#L722-L723
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

    LinearUIIO& io = LinearUI::GetIO();
    if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
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
    if (!LinearUI_ImplOSX_HandleEvent(event, self))
        [super keyDown:event];

    // Call to the macOS input manager system.
    [self interpretKeyEvents:@[event]];
}

- (void)keyUp:(NSEvent*)event
{
    if (!LinearUI_ImplOSX_HandleEvent(event, self))
        [super keyUp:event];
}

- (void)insertText:(id)aString replacementRange:(NSRange)replacementRange
{
    LinearUIIO& io = LinearUI::GetIO();

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

@interface LinearUIObserver : NSObject

- (void)onApplicationBecomeActive:(NSNotification*)aNotification;
- (void)onApplicationBecomeInactive:(NSNotification*)aNotification;
- (void)displaysDidChange:(NSNotification*)aNotification;

@end

@implementation LinearUIObserver

- (void)onApplicationBecomeActive:(NSNotification*)aNotification
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddFocusEvent(true);
}

- (void)onApplicationBecomeInactive:(NSNotification*)aNotification
{
    LinearUIIO& io = LinearUI::GetIO();
    io.AddFocusEvent(false);
}

- (void)displaysDidChange:(NSNotification*)aNotification
{
    LinearUI_ImplOSX_UpdateMonitors();
}

@end

// Functions

// Not static to allow third-party code to use that if they want to (but undocumented)
LinearUIKey LinearUI_ImplOSX_KeyCodeToLinearUIKey(int key_code);
LinearUIKey LinearUI_ImplOSX_KeyCodeToLinearUIKey(int key_code)
{
    switch (key_code)
    {
        case kVK_ANSI_A: return LinearUIKey_A;
        case kVK_ANSI_S: return LinearUIKey_S;
        case kVK_ANSI_D: return LinearUIKey_D;
        case kVK_ANSI_F: return LinearUIKey_F;
        case kVK_ANSI_H: return LinearUIKey_H;
        case kVK_ANSI_G: return LinearUIKey_G;
        case kVK_ANSI_Z: return LinearUIKey_Z;
        case kVK_ANSI_X: return LinearUIKey_X;
        case kVK_ANSI_C: return LinearUIKey_C;
        case kVK_ANSI_V: return LinearUIKey_V;
        case kVK_ANSI_B: return LinearUIKey_B;
        case kVK_ANSI_Q: return LinearUIKey_Q;
        case kVK_ANSI_W: return LinearUIKey_W;
        case kVK_ANSI_E: return LinearUIKey_E;
        case kVK_ANSI_R: return LinearUIKey_R;
        case kVK_ANSI_Y: return LinearUIKey_Y;
        case kVK_ANSI_T: return LinearUIKey_T;
        case kVK_ANSI_1: return LinearUIKey_1;
        case kVK_ANSI_2: return LinearUIKey_2;
        case kVK_ANSI_3: return LinearUIKey_3;
        case kVK_ANSI_4: return LinearUIKey_4;
        case kVK_ANSI_6: return LinearUIKey_6;
        case kVK_ANSI_5: return LinearUIKey_5;
        case kVK_ANSI_Equal: return LinearUIKey_Equal;
        case kVK_ANSI_9: return LinearUIKey_9;
        case kVK_ANSI_7: return LinearUIKey_7;
        case kVK_ANSI_Minus: return LinearUIKey_Minus;
        case kVK_ANSI_8: return LinearUIKey_8;
        case kVK_ANSI_0: return LinearUIKey_0;
        case kVK_ANSI_RightBracket: return LinearUIKey_RightBracket;
        case kVK_ANSI_O: return LinearUIKey_O;
        case kVK_ANSI_U: return LinearUIKey_U;
        case kVK_ANSI_LeftBracket: return LinearUIKey_LeftBracket;
        case kVK_ANSI_I: return LinearUIKey_I;
        case kVK_ANSI_P: return LinearUIKey_P;
        case kVK_ANSI_L: return LinearUIKey_L;
        case kVK_ANSI_J: return LinearUIKey_J;
        case kVK_ANSI_Quote: return LinearUIKey_Apostrophe;
        case kVK_ANSI_K: return LinearUIKey_K;
        case kVK_ANSI_Semicolon: return LinearUIKey_Semicolon;
        case kVK_ANSI_Backslash: return LinearUIKey_Backslash;
        case kVK_ANSI_Comma: return LinearUIKey_Comma;
        case kVK_ANSI_Slash: return LinearUIKey_Slash;
        case kVK_ANSI_N: return LinearUIKey_N;
        case kVK_ANSI_M: return LinearUIKey_M;
        case kVK_ANSI_Period: return LinearUIKey_Period;
        case kVK_ANSI_Grave: return LinearUIKey_GraveAccent;
        case kVK_ANSI_KeypadDecimal: return LinearUIKey_KeypadDecimal;
        case kVK_ANSI_KeypadMultiply: return LinearUIKey_KeypadMultiply;
        case kVK_ANSI_KeypadPlus: return LinearUIKey_KeypadAdd;
        case kVK_ANSI_KeypadClear: return LinearUIKey_NumLock;
        case kVK_ANSI_KeypadDivide: return LinearUIKey_KeypadDivide;
        case kVK_ANSI_KeypadEnter: return LinearUIKey_KeypadEnter;
        case kVK_ANSI_KeypadMinus: return LinearUIKey_KeypadSubtract;
        case kVK_ANSI_KeypadEquals: return LinearUIKey_KeypadEqual;
        case kVK_ANSI_Keypad0: return LinearUIKey_Keypad0;
        case kVK_ANSI_Keypad1: return LinearUIKey_Keypad1;
        case kVK_ANSI_Keypad2: return LinearUIKey_Keypad2;
        case kVK_ANSI_Keypad3: return LinearUIKey_Keypad3;
        case kVK_ANSI_Keypad4: return LinearUIKey_Keypad4;
        case kVK_ANSI_Keypad5: return LinearUIKey_Keypad5;
        case kVK_ANSI_Keypad6: return LinearUIKey_Keypad6;
        case kVK_ANSI_Keypad7: return LinearUIKey_Keypad7;
        case kVK_ANSI_Keypad8: return LinearUIKey_Keypad8;
        case kVK_ANSI_Keypad9: return LinearUIKey_Keypad9;
        case kVK_Return: return LinearUIKey_Enter;
        case kVK_Tab: return LinearUIKey_Tab;
        case kVK_Space: return LinearUIKey_Space;
        case kVK_Delete: return LinearUIKey_Backspace;
        case kVK_Escape: return LinearUIKey_Escape;
        case kVK_CapsLock: return LinearUIKey_CapsLock;
        case kVK_Control: return LinearUIKey_LeftCtrl;
        case kVK_Shift: return LinearUIKey_LeftShift;
        case kVK_Option: return LinearUIKey_LeftAlt;
        case kVK_Command: return LinearUIKey_LeftSuper;
        case kVK_RightControl: return LinearUIKey_RightCtrl;
        case kVK_RightShift: return LinearUIKey_RightShift;
        case kVK_RightOption: return LinearUIKey_RightAlt;
        case kVK_RightCommand: return LinearUIKey_RightSuper;
//      case kVK_Function: return LinearUIKey_;
//      case kVK_VolumeUp: return LinearUIKey_;
//      case kVK_VolumeDown: return LinearUIKey_;
//      case kVK_Mute: return LinearUIKey_;
        case kVK_F1: return LinearUIKey_F1;
        case kVK_F2: return LinearUIKey_F2;
        case kVK_F3: return LinearUIKey_F3;
        case kVK_F4: return LinearUIKey_F4;
        case kVK_F5: return LinearUIKey_F5;
        case kVK_F6: return LinearUIKey_F6;
        case kVK_F7: return LinearUIKey_F7;
        case kVK_F8: return LinearUIKey_F8;
        case kVK_F9: return LinearUIKey_F9;
        case kVK_F10: return LinearUIKey_F10;
        case kVK_F11: return LinearUIKey_F11;
        case kVK_F12: return LinearUIKey_F12;
        case kVK_F13: return LinearUIKey_F13;
        case kVK_F14: return LinearUIKey_F14;
        case kVK_F15: return LinearUIKey_F15;
        case kVK_F16: return LinearUIKey_F16;
        case kVK_F17: return LinearUIKey_F17;
        case kVK_F18: return LinearUIKey_F18;
        case kVK_F19: return LinearUIKey_F19;
        case kVK_F20: return LinearUIKey_F20;
        case 0x6E: return LinearUIKey_Menu;
        case kVK_Help: return LinearUIKey_Insert;
        case kVK_Home: return LinearUIKey_Home;
        case kVK_PageUp: return LinearUIKey_PageUp;
        case kVK_ForwardDelete: return LinearUIKey_Delete;
        case kVK_End: return LinearUIKey_End;
        case kVK_PageDown: return LinearUIKey_PageDown;
        case kVK_LeftArrow: return LinearUIKey_LeftArrow;
        case kVK_RightArrow: return LinearUIKey_RightArrow;
        case kVK_DownArrow: return LinearUIKey_DownArrow;
        case kVK_UpArrow: return LinearUIKey_UpArrow;
        default: return LinearUIKey_None;
    }
}

#ifdef LINEARUI_IMPL_METAL_CPP_EXTENSIONS

LINEARUI_IMPL_API bool LinearUI_ImplOSX_Init(void* _Nonnull view) {
    return LinearUI_ImplOSX_Init((__bridge NSView*)(view));
}

LINEARUI_IMPL_API void LinearUI_ImplOSX_NewFrame(void* _Nullable view) {
    return LinearUI_ImplOSX_NewFrame((__bridge NSView*)(view));
}

#endif


bool LinearUI_ImplOSX_Init(NSView* view)
{
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    LINEARUI_CHECKVERSION();
    LINEARUI_ASSERT(io.BackendPlatformUserData == nullptr && "Already initialized a platform backend!");

    // Setup backend capabilities flags
    LinearUI_ImplOSX_Data* bd = LINEARUI_NEW(LinearUI_ImplOSX_Data)();
    io.BackendPlatformUserData = (void*)bd;
    io.BackendPlatformName = "linearui_impl_osx";
    io.BackendFlags |= LinearUIBackendFlags_HasMouseCursors;           // We can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= LinearUIBackendFlags_HasSetMousePos;          // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendFlags |= LinearUIBackendFlags_PlatformHasViewports;      // We can create multi-viewports on the Platform side (optional)
    //io.BackendFlags |= LinearUIBackendFlags_HasMouseHoveredViewport; // We can call io.AddMouseViewportEvent() with correct data (optional)

    bd->Observer = [LinearUIObserver new];
    bd->Window = view.window ?: NSApp.orderedWindows.firstObject;
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    main_viewport->PlatformHandle = main_viewport->PlatformHandleRaw = (__bridge_retained void*)bd->Window;
    LinearUI_ImplOSX_UpdateMonitors();
    LinearUI_ImplOSX_InitMultiViewportSupport();

    // Load cursors. Some of them are undocumented.
    bd->MouseCursorHidden = false;
    bd->MouseCursors[LinearUIMouseCursor_Arrow] = [NSCursor arrowCursor];
    bd->MouseCursors[LinearUIMouseCursor_TextInput] = [NSCursor IBeamCursor];
    bd->MouseCursors[LinearUIMouseCursor_ResizeAll] = [NSCursor closedHandCursor];
    bd->MouseCursors[LinearUIMouseCursor_ResizeNS] = [NSCursor respondsToSelector:@selector(_windowResizeNorthSouthCursor)] ? [NSCursor _windowResizeNorthSouthCursor] : [NSCursor resizeUpDownCursor];
    bd->MouseCursors[LinearUIMouseCursor_ResizeEW] = [NSCursor respondsToSelector:@selector(_windowResizeEastWestCursor)] ? [NSCursor _windowResizeEastWestCursor] : [NSCursor resizeLeftRightCursor];
    bd->MouseCursors[LinearUIMouseCursor_ResizeNESW] = [NSCursor respondsToSelector:@selector(_windowResizeNorthEastSouthWestCursor)] ? [NSCursor _windowResizeNorthEastSouthWestCursor] : [NSCursor closedHandCursor];
    bd->MouseCursors[LinearUIMouseCursor_ResizeNWSE] = [NSCursor respondsToSelector:@selector(_windowResizeNorthWestSouthEastCursor)] ? [NSCursor _windowResizeNorthWestSouthEastCursor] : [NSCursor closedHandCursor];
    bd->MouseCursors[LinearUIMouseCursor_Hand] = [NSCursor pointingHandCursor];
    bd->MouseCursors[LinearUIMouseCursor_Wait] = bd->MouseCursors[LinearUIMouseCursor_Progress] = [NSCursor respondsToSelector:@selector(busyButClickableCursor)] ? [NSCursor busyButClickableCursor] : [NSCursor arrowCursor];
    bd->MouseCursors[LinearUIMouseCursor_NotAllowed] = [NSCursor operationNotAllowedCursor];

    // Note that linearui.cpp also include default OSX clipboard handlers which can be enabled
    // by adding '#define LINEARUI_ENABLE_OSX_DEFAULT_CLIPBOARD_FUNCTIONS' in linearuiconfig.h and adding '-framework ApplicationServices' to your linker command-line.
    // Since we are already in ObjC land here, it is easy for us to add a clipboard handler using the NSPasteboard api.
    platform_io.Platform_SetClipboardTextFn = [](LinearUIContext*, const char* str) -> void
    {
        NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
        [pasteboard declareTypes:[NSArray arrayWithObject:NSPasteboardTypeString] owner:nil];
        [pasteboard setString:[NSString stringWithUTF8String:str] forType:NSPasteboardTypeString];
    };

    platform_io.Platform_GetClipboardTextFn = [](LinearUIContext*) -> const char*
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
        static LinearVector<char> s_clipboard;
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
    LinearUI_ImplOSX_AddTrackingArea(view);

    platform_io.Platform_SetImeDataFn = [](LinearUIContext*, LinearUIViewport*, LinearUIPlatformImeData* data) -> void
    {
        LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
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

void LinearUI_ImplOSX_Shutdown()
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "No platform backend to shutdown, or already shutdown?");

    [[NSNotificationCenter defaultCenter] removeObserver:bd->Observer];
    bd->Observer = nullptr;
    if (bd->Monitor != nullptr)
    {
        [NSEvent removeMonitor:bd->Monitor];
        bd->Monitor = nullptr;
    }

    LinearUI_ImplOSX_ShutdownMultiViewportSupport();
    LinearUI_ImplOSX_DestroyBackendData();
    LinearUIIO& io = LinearUI::GetIO();
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();

    io.BackendPlatformName = nullptr;
    io.BackendPlatformUserData = nullptr;
    io.BackendFlags &= ~(LinearUIBackendFlags_HasMouseCursors | LinearUIBackendFlags_HasGamepad | LinearUIBackendFlags_PlatformHasViewports);
    platform_io.ClearPlatformHandlers();
}

static void LinearUI_ImplOSX_UpdateMouseCursor()
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
    LinearUIIO& io = LinearUI::GetIO();
    if (io.ConfigFlags & LinearUIConfigFlags_NoMouseCursorChange)
        return;

    LinearUIMouseCursor linearui_cursor = LinearUI::GetMouseCursor();
    if (io.MouseDrawCursor || linearui_cursor == LinearUIMouseCursor_None)
    {
        // Hide OS mouse cursor if linearui is drawing it or if it wants no cursor
        if (!bd->MouseCursorHidden)
        {
            bd->MouseCursorHidden = true;
            [NSCursor hide];
        }
    }
    else
    {
        NSCursor* desired = bd->MouseCursors[linearui_cursor] ?: bd->MouseCursors[LinearUIMouseCursor_Arrow];
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

static void LinearUI_ImplOSX_UpdateGamepads()
{
    LinearUIIO& io = LinearUI::GetIO();

#if APPLE_HAS_CONTROLLER
    GCController* controller = GCController.current;
#else
    GCController* controller = GCController.controllers.firstObject;
#endif
    if (controller == nil || controller.extendedGamepad == nil)
    {
        io.BackendFlags &= ~LinearUIBackendFlags_HasGamepad;
        return;
    }

    GCExtendedGamepad* gp = controller.extendedGamepad;

    // Update gamepad inputs
    #define LINEARUI_SATURATE(V)                        (V < 0.0f ? 0.0f : V > 1.0f ? 1.0f : V)
    #define MAP_BUTTON(KEY_NO, BUTTON_NAME)       { io.AddKeyEvent(KEY_NO, gp.BUTTON_NAME.isPressed); }
    #define MAP_ANALOG(KEY_NO, AXIS_NAME, V0, V1) { float vn = (float)(gp.AXIS_NAME.value - V0) / (float)(V1 - V0); vn = LINEARUI_SATURATE(vn); io.AddKeyAnalogEvent(KEY_NO, vn > 0.1f, vn); }
    const float thumb_dead_zone = 0.0f;

#if APPLE_HAS_BUTTON_OPTIONS
    MAP_BUTTON(LinearUIKey_GamepadBack,            buttonOptions);
#endif
    MAP_BUTTON(LinearUIKey_GamepadFaceLeft,        buttonX);              // Xbox X, PS Square
    MAP_BUTTON(LinearUIKey_GamepadFaceRight,       buttonB);              // Xbox B, PS Circle
    MAP_BUTTON(LinearUIKey_GamepadFaceUp,          buttonY);              // Xbox Y, PS Triangle
    MAP_BUTTON(LinearUIKey_GamepadFaceDown,        buttonA);              // Xbox A, PS Cross
    MAP_BUTTON(LinearUIKey_GamepadDpadLeft,        dpad.left);
    MAP_BUTTON(LinearUIKey_GamepadDpadRight,       dpad.right);
    MAP_BUTTON(LinearUIKey_GamepadDpadUp,          dpad.up);
    MAP_BUTTON(LinearUIKey_GamepadDpadDown,        dpad.down);
    MAP_ANALOG(LinearUIKey_GamepadL1,              leftShoulder, 0.0f, 1.0f);
    MAP_ANALOG(LinearUIKey_GamepadR1,              rightShoulder, 0.0f, 1.0f);
    MAP_ANALOG(LinearUIKey_GamepadL2,              leftTrigger,  0.0f, 1.0f);
    MAP_ANALOG(LinearUIKey_GamepadR2,              rightTrigger, 0.0f, 1.0f);
#if APPLE_HAS_THUMBSTICKS
    MAP_BUTTON(LinearUIKey_GamepadL3,              leftThumbstickButton);
    MAP_BUTTON(LinearUIKey_GamepadR3,              rightThumbstickButton);
#endif
    MAP_ANALOG(LinearUIKey_GamepadLStickLeft,      leftThumbstick.xAxis,  -thumb_dead_zone, -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadLStickRight,     leftThumbstick.xAxis,  +thumb_dead_zone, +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadLStickUp,        leftThumbstick.yAxis,  +thumb_dead_zone, +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadLStickDown,      leftThumbstick.yAxis,  -thumb_dead_zone, -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickLeft,      rightThumbstick.xAxis, -thumb_dead_zone, -1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickRight,     rightThumbstick.xAxis, +thumb_dead_zone, +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickUp,        rightThumbstick.yAxis, +thumb_dead_zone, +1.0f);
    MAP_ANALOG(LinearUIKey_GamepadRStickDown,      rightThumbstick.yAxis, -thumb_dead_zone, -1.0f);
    #undef MAP_BUTTON
    #undef MAP_ANALOG

    io.BackendFlags |= LinearUIBackendFlags_HasGamepad;
}

static void LinearUI_ImplOSX_UpdateImePosWithView(NSView* view)
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
    LinearUIIO& io = LinearUI::GetIO();
    if (io.WantTextInput)
        [bd->KeyEventResponder updateImePosWithView:view];
}

void LinearUI_ImplOSX_NewFrame(NSView* view)
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
    LINEARUI_ASSERT(bd != nullptr && "Context or backend not initialized! Did you call LinearUI_ImplOSX_Init()?");
    LinearUIIO& io = LinearUI::GetIO();

    // Setup display size
    if (view)
    {
        const float fb_scale = (float)[view.window backingScaleFactor];
        io.DisplaySize = LinearVec2((float)view.bounds.size.width, (float)view.bounds.size.height);
        io.DisplayFramebufferScale = LinearVec2(fb_scale, fb_scale);
    }

    // Setup time step
    if (bd->Time == 0.0)
        bd->Time = GetMachAbsoluteTimeInSeconds();

    double current_time = GetMachAbsoluteTimeInSeconds();
    io.DeltaTime = (float)(current_time - bd->Time);
    bd->Time = current_time;

    LinearUI_ImplOSX_UpdateMouseCursor();
    LinearUI_ImplOSX_UpdateGamepads();
    LinearUI_ImplOSX_UpdateImePosWithView(view);
}

// Must only be called for a mouse event, otherwise an exception occurs
// (Note that NSEventTypeScrollWheel is considered "other input". Oddly enough an exception does not occur with it, but the value will sometimes be wrong!)
static LinearUIMouseSource GetMouseSource(NSEvent* event)
{
    switch (event.subtype)
    {
        case NSEventSubtypeTabletPoint:
            return LinearUIMouseSource_Pen;
        // macOS considers input from relative touch devices (like the trackpad or Apple Magic Mouse) to be touch input.
        // This doesn't really make sense for Dear LinearUI, which expects absolute touch devices only.
        // There does not seem to be a simple way to disambiguate things here so we consider NSEventSubtypeTouch events to always come from mice.
        // See https://developer.apple.com/library/archive/documentation/Cocoa/Conceptual/EventOverview/HandlingTouchEvents/HandlingTouchEvents.html#//apple_ref/doc/uid/10000060i-CH13-SW24
        //case NSEventSubtypeTouch:
        //    return LinearUIMouseSource_TouchScreen;
        case NSEventSubtypeMouseEvent:
        default:
            return LinearUIMouseSource_Mouse;
    }
}

static bool LinearUI_ImplOSX_HandleEvent(NSEvent* event, NSView* view)
{
    // Only process events from the window containing LinearUI view
    if (!LinearUI::FindViewportByPlatformHandle((__bridge void*)event.window))
        return false;

    LinearUIIO& io = LinearUI::GetIO();
    if (event.type == NSEventTypeLeftMouseDown || event.type == NSEventTypeRightMouseDown || event.type == NSEventTypeOtherMouseDown)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < LinearUIMouseButton_COUNT)
        {
            io.AddMouseSourceEvent(GetMouseSource(event));
            io.AddMouseButtonEvent(button, true);
        }
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeLeftMouseUp || event.type == NSEventTypeRightMouseUp || event.type == NSEventTypeOtherMouseUp)
    {
        int button = (int)[event buttonNumber];
        if (button >= 0 && button < LinearUIMouseButton_COUNT)
        {
            io.AddMouseSourceEvent(GetMouseSource(event));
            io.AddMouseButtonEvent(button, false);
        }
        return io.WantCaptureMouse;
    }

    if (event.type == NSEventTypeMouseMoved || event.type == NSEventTypeLeftMouseDragged || event.type == NSEventTypeRightMouseDragged || event.type == NSEventTypeOtherMouseDragged)
    {
        NSPoint mousePoint;
        if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
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
        LinearUIKey key = LinearUI_ImplOSX_KeyCodeToLinearUIKey(key_code);
        io.AddKeyEvent(key, event.type == NSEventTypeKeyDown);
        io.SetKeyEventNativeData(key, key_code, -1); // To support legacy indexing (<1.87 user code)

        return io.WantCaptureKeyboard;
    }

    if (event.type == NSEventTypeFlagsChanged)
    {
        unsigned short key_code = [event keyCode];
        NSEventModifierFlags modifier_flags = [event modifierFlags];

        io.AddKeyEvent(LinearUIMod_Shift, (modifier_flags & NSEventModifierFlagShift)   != 0);
        io.AddKeyEvent(LinearUIMod_Ctrl,  (modifier_flags & NSEventModifierFlagControl) != 0);
        io.AddKeyEvent(LinearUIMod_Alt,   (modifier_flags & NSEventModifierFlagOption)  != 0);
        io.AddKeyEvent(LinearUIMod_Super, (modifier_flags & NSEventModifierFlagCommand) != 0);

        LinearUIKey key = LinearUI_ImplOSX_KeyCodeToLinearUIKey(key_code);
        if (key != LinearUIKey_None)
        {
            // macOS does not generate down/up event for modifiers. We're trying
            // to use hardware dependent masks to extract that information.
            // 'linearui_mask' is left as a fallback.
            NSEventModifierFlags mask = 0;
            switch (key)
            {
                case LinearUIKey_LeftCtrl:   mask = 0x0001; break;
                case LinearUIKey_RightCtrl:  mask = 0x2000; break;
                case LinearUIKey_LeftShift:  mask = 0x0002; break;
                case LinearUIKey_RightShift: mask = 0x0004; break;
                case LinearUIKey_LeftSuper:  mask = 0x0008; break;
                case LinearUIKey_RightSuper: mask = 0x0010; break;
                case LinearUIKey_LeftAlt:    mask = 0x0020; break;
                case LinearUIKey_RightAlt:   mask = 0x0040; break;
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

static void LinearUI_ImplOSX_AddTrackingArea(NSView* _Nonnull view)
{
    // If we want to receive key events, we either need to be in the responder chain of the key view,
    // or else we can install a local monitor. The consequence of this heavy-handed approach is that
    // we receive events for all controls, not just Dear LinearUI widgets. If we had native controls in our
    // window, we'd want to be much more careful than just ingesting the complete event stream.
    // To match the behavior of other backends, we pass every event down to the OS.
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
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
        LinearUI_ImplOSX_HandleEvent(event, view);
        return event;
    }];
}

//--------------------------------------------------------------------------------------------------------
// MULTI-VIEWPORT / PLATFORM INTERFACE SUPPORT
// This is an _advanced_ and _optional_ feature, allowing the back-end to create and handle multiple viewports simultaneously.
// If you are new to dear linearui or creating a new binding for dear linearui, it is recommended that you completely ignore this section first..
//--------------------------------------------------------------------------------------------------------

struct LinearUI_ImplOSX_ViewportData
{
    NSWindow*        Window;
    bool             WindowOwned;

    LinearUI_ImplOSX_ViewportData()  { WindowOwned = false; }
    ~LinearUI_ImplOSX_ViewportData() { LINEARUI_ASSERT(Window == nil); }
};

@interface LinearUI_ImplOSX_Window: NSWindow
@end

@implementation LinearUI_ImplOSX_Window

- (BOOL)canBecomeKeyWindow
{
    return YES;
}

@end

static void ConvertNSRect(NSRect* r)
{
    NSRect firstScreenFrame = NSScreen.screens[0].frame;
    LINEARUI_ASSERT(firstScreenFrame.origin.x == 0 && firstScreenFrame.origin.y == 0);
    r->origin.y = firstScreenFrame.size.height - r->origin.y - r->size.height;
}

static void LinearUI_ImplOSX_CreateWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
    LinearUI_ImplOSX_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplOSX_ViewportData)();
    viewport->PlatformUserData = vd;

    NSScreen* screen = bd->Window.screen;
    NSRect rect = NSMakeRect(viewport->Pos.x, viewport->Pos.y, viewport->Size.x, viewport->Size.y);
    ConvertNSRect(&rect);

    NSWindowStyleMask styleMask = 0;
    if (viewport->Flags & LinearUIViewportFlags_NoDecoration)
        styleMask |= NSWindowStyleMaskBorderless;
    else
        styleMask |= NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;

    NSWindow* window = [[LinearUI_ImplOSX_Window alloc] initWithContentRect:rect
                                                               styleMask:styleMask
                                                                 backing:NSBackingStoreBuffered
                                                                   defer:YES
                                                                  screen:screen];
    if (viewport->Flags & LinearUIViewportFlags_TopMost)
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

static void LinearUI_ImplOSX_DestroyWindow(LinearUIViewport* viewport)
{
    if (LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData)
    {
        NSWindow* window = vd->Window;
        if (window != nil && vd->WindowOwned)
        {
            window.contentView = nil;
            window.contentViewController = nil;
            [window orderOut:nil];
        }
        vd->Window = nil;
        LINEARUI_DELETE(vd);
    }
    viewport->PlatformUserData = viewport->PlatformHandle = viewport->PlatformHandleRaw = nullptr;
}

static void LinearUI_ImplOSX_ShowWindow(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    if (viewport->Flags & LinearUIViewportFlags_NoFocusOnAppearing)
        [vd->Window orderFront:nil];
    else
        [vd->Window makeKeyAndOrderFront:nil];

    [vd->Window setIsVisible:YES];
}

static LinearVec2 LinearUI_ImplOSX_GetWindowPos(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSRect frame = window.frame;
    NSRect contentRect = window.contentLayoutRect;
    if (window.styleMask & NSWindowStyleMaskFullSizeContentView) // No title bar windows should be considered.
        contentRect = frame;

    NSRect firstScreenFrame = NSScreen.screens[0].frame;
    return LinearVec2(frame.origin.x, firstScreenFrame.size.height - frame.origin.y - contentRect.size.height);
}

static void LinearUI_ImplOSX_SetWindowPos(LinearUIViewport* viewport, LinearVec2 pos)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSSize size = window.frame.size;

    NSRect r = NSMakeRect(pos.x, pos.y, size.width, size.height);
    ConvertNSRect(&r);
    [window setFrameOrigin:r.origin];
}

static LinearVec2 LinearUI_ImplOSX_GetWindowSize(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSSize size = window.contentLayoutRect.size;
    return LinearVec2(size.width, size.height);
}

static void LinearUI_ImplOSX_SetWindowSize(LinearUIViewport* viewport, LinearVec2 size)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    NSWindow* window = vd->Window;
    NSRect rect = window.frame;
    rect.origin.y -= (size.y - rect.size.height);
    rect.size.width = size.x;
    rect.size.height = size.y;
    [window setFrame:rect display:YES];
}

static LinearVec2 LinearUI_ImplOSX_GetWindowFramebufferScale(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    NSWindow* window = vd->Window;
    const float fb_scale = (float)[window backingScaleFactor];
    return LinearVec2(fb_scale, fb_scale);
}

static void LinearUI_ImplOSX_SetWindowFocus(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);
    [vd->Window makeKeyAndOrderFront:bd->Window];
}

static bool LinearUI_ImplOSX_GetWindowFocus(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    return vd->Window.isKeyWindow;
}

static bool LinearUI_ImplOSX_GetWindowMinimized(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    return vd->Window.isMiniaturized;
}

static void LinearUI_ImplOSX_SetWindowTitle(LinearUIViewport* viewport, const char* title)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    vd->Window.title = [NSString stringWithUTF8String:title];
}

static void LinearUI_ImplOSX_SetWindowAlpha(LinearUIViewport* viewport, float alpha)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);
    LINEARUI_ASSERT(alpha >= 0.0f && alpha <= 1.0f);

    vd->Window.alphaValue = alpha;
}

static float LinearUI_ImplOSX_GetWindowDpiScale(LinearUIViewport* viewport)
{
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)viewport->PlatformUserData;
    LINEARUI_ASSERT(vd->Window != 0);

    return vd->Window.backingScaleFactor;
}

static void LinearUI_ImplOSX_UpdateMonitors()
{
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Monitors.resize(0);

    NSRect firstScreenFrame = NSScreen.screens[0].frame;
    LINEARUI_ASSERT(firstScreenFrame.origin.x == 0 && firstScreenFrame.origin.y == 0);

    for (NSScreen* screen in NSScreen.screens)
    {
        NSRect frame = screen.frame;
        NSRect visibleFrame = screen.visibleFrame;
        ConvertNSRect(&frame);
        ConvertNSRect(&visibleFrame);

        LinearUIPlatformMonitor linearui_monitor;
        linearui_monitor.MainPos = LinearVec2(frame.origin.x, frame.origin.y);
        linearui_monitor.MainSize = LinearVec2(frame.size.width, frame.size.height);
        linearui_monitor.WorkPos = LinearVec2(visibleFrame.origin.x, visibleFrame.origin.y);
        linearui_monitor.WorkSize = LinearVec2(visibleFrame.size.width, visibleFrame.size.height);
        linearui_monitor.DpiScale = screen.backingScaleFactor;
        linearui_monitor.PlatformHandle = (__bridge_retained void*)screen;

        platform_io.Monitors.push_back(linearui_monitor);
    }
}

static void LinearUI_ImplOSX_InitMultiViewportSupport()
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();

    // Register platform interface (will be coupled with a renderer interface)
    LinearUIPlatformIO& platform_io = LinearUI::GetPlatformIO();
    platform_io.Platform_CreateWindow = LinearUI_ImplOSX_CreateWindow;
    platform_io.Platform_DestroyWindow = LinearUI_ImplOSX_DestroyWindow;
    platform_io.Platform_ShowWindow = LinearUI_ImplOSX_ShowWindow;
    platform_io.Platform_SetWindowPos = LinearUI_ImplOSX_SetWindowPos;
    platform_io.Platform_GetWindowPos = LinearUI_ImplOSX_GetWindowPos;
    platform_io.Platform_SetWindowSize = LinearUI_ImplOSX_SetWindowSize;
    platform_io.Platform_GetWindowSize = LinearUI_ImplOSX_GetWindowSize;
    platform_io.Platform_GetWindowFramebufferScale = LinearUI_ImplOSX_GetWindowFramebufferScale;
    platform_io.Platform_SetWindowFocus = LinearUI_ImplOSX_SetWindowFocus;
    platform_io.Platform_GetWindowFocus = LinearUI_ImplOSX_GetWindowFocus;
    platform_io.Platform_GetWindowMinimized = LinearUI_ImplOSX_GetWindowMinimized;
    platform_io.Platform_SetWindowTitle = LinearUI_ImplOSX_SetWindowTitle;
    platform_io.Platform_SetWindowAlpha = LinearUI_ImplOSX_SetWindowAlpha;
    platform_io.Platform_GetWindowDpiScale = LinearUI_ImplOSX_GetWindowDpiScale; // FIXME-DPI

    // Register main window handle (which is owned by the main application, not by us)
    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    LinearUI_ImplOSX_ViewportData* vd = LINEARUI_NEW(LinearUI_ImplOSX_ViewportData)();
    vd->Window = bd->Window;
    vd->WindowOwned = false;
    main_viewport->PlatformUserData = vd;
    main_viewport->PlatformHandle = (__bridge void*)bd->Window;

    [NSNotificationCenter.defaultCenter addObserver:bd->Observer
                                           selector:@selector(displaysDidChange:)
                                               name:NSApplicationDidChangeScreenParametersNotification
                                             object:nil];
}

static void LinearUI_ImplOSX_ShutdownMultiViewportSupport()
{
    LinearUI_ImplOSX_Data* bd = LinearUI_ImplOSX_GetBackendData();
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

    LinearUIViewport* main_viewport = LinearUI::GetMainViewport();
    LinearUI_ImplOSX_ViewportData* vd = (LinearUI_ImplOSX_ViewportData*)main_viewport->PlatformUserData;
    LINEARUI_DELETE(vd);
    main_viewport->PlatformUserData = nullptr;
    LinearUI::DestroyPlatformWindows();
}

//-----------------------------------------------------------------------------

#endif // #ifndef LINEARUI_DISABLE
