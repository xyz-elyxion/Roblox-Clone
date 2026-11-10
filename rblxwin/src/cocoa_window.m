//========================================================================
// RBLXWIN 3.3 macOS - www.rblxwin.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@rblxwin.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
// It is fine to use C99 in this file because it will not be built with VS
//========================================================================

#include "internal.h"

#include <float.h>
#include <string.h>

// HACK: This enum value is missing from framework headers on OS X 10.11 despite
//       having been (according to documentation) added in Mac OS X 10.7
#define NSWindowCollectionBehaviorFullScreenNone (1 << 9)

// Returns whether the cursor is in the content area of the specified window
//
static RBLXWINbool cursorInContentArea(_RBLXWINwindow* window)
{
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];
    return [window->ns.view mouse:pos inRect:[window->ns.view frame]];
}

// Hides the cursor if not already hidden
//
static void hideCursor(_RBLXWINwindow* window)
{
    if (!_rblxwin.ns.cursorHidden)
    {
        [NSCursor hide];
        _rblxwin.ns.cursorHidden = RBLXWIN_TRUE;
    }
}

// Shows the cursor if not already shown
//
static void showCursor(_RBLXWINwindow* window)
{
    if (_rblxwin.ns.cursorHidden)
    {
        [NSCursor unhide];
        _rblxwin.ns.cursorHidden = RBLXWIN_FALSE;
    }
}

// Updates the cursor image according to its cursor mode
//
static void updateCursorImage(_RBLXWINwindow* window)
{
    if (window->cursorMode == RBLXWIN_CURSOR_NORMAL)
    {
        showCursor(window);

        if (window->cursor)
            [(NSCursor*) window->cursor->ns.object set];
        else
            [[NSCursor arrowCursor] set];
    }
    else
        hideCursor(window);
}

// Apply chosen cursor mode to a focused window
//
static void updateCursorMode(_RBLXWINwindow* window)
{
    if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
    {
        _rblxwin.ns.disabledCursorWindow = window;
        _rblxwinPlatformGetCursorPos(window,
                                  &_rblxwin.ns.restoreCursorPosX,
                                  &_rblxwin.ns.restoreCursorPosY);
        _rblxwinCenterCursorInContentArea(window);
        CGAssociateMouseAndMouseCursorPosition(false);
    }
    else if (_rblxwin.ns.disabledCursorWindow == window)
    {
        _rblxwin.ns.disabledCursorWindow = NULL;
        _rblxwinPlatformSetCursorPos(window,
                                  _rblxwin.ns.restoreCursorPosX,
                                  _rblxwin.ns.restoreCursorPosY);
        // NOTE: The matching CGAssociateMouseAndMouseCursorPosition call is
        //       made in _rblxwinPlatformSetCursorPos as part of a workaround
    }

    if (cursorInContentArea(window))
        updateCursorImage(window);
}

// Make the specified window and its video mode active on its monitor
//
static void acquireMonitor(_RBLXWINwindow* window)
{
    _rblxwinSetVideoModeNS(window->monitor, &window->videoMode);
    const CGRect bounds = CGDisplayBounds(window->monitor->ns.displayID);
    const NSRect frame = NSMakeRect(bounds.origin.x,
                                    _rblxwinTransformYNS(bounds.origin.y + bounds.size.height - 1),
                                    bounds.size.width,
                                    bounds.size.height);

    [window->ns.object setFrame:frame display:YES];

    _rblxwinInputMonitorWindow(window->monitor, window);
}

// Remove the window and restore the original video mode
//
static void releaseMonitor(_RBLXWINwindow* window)
{
    if (window->monitor->window != window)
        return;

    _rblxwinInputMonitorWindow(window->monitor, NULL);
    _rblxwinRestoreVideoModeNS(window->monitor);
}

// Translates macOS key modifiers into RBLXWIN ones
//
static int translateFlags(NSUInteger flags)
{
    int mods = 0;

    if (flags & NSEventModifierFlagShift)
        mods |= RBLXWIN_MOD_SHIFT;
    if (flags & NSEventModifierFlagControl)
        mods |= RBLXWIN_MOD_CONTROL;
    if (flags & NSEventModifierFlagOption)
        mods |= RBLXWIN_MOD_ALT;
    if (flags & NSEventModifierFlagCommand)
        mods |= RBLXWIN_MOD_SUPER;
    if (flags & NSEventModifierFlagCapsLock)
        mods |= RBLXWIN_MOD_CAPS_LOCK;

    return mods;
}

// Translates a macOS keycode to a RBLXWIN keycode
//
static int translateKey(unsigned int key)
{
    if (key >= sizeof(_rblxwin.ns.keycodes) / sizeof(_rblxwin.ns.keycodes[0]))
        return RBLXWIN_KEY_UNKNOWN;

    return _rblxwin.ns.keycodes[key];
}

// Translate a RBLXWIN keycode to a Cocoa modifier flag
//
static NSUInteger translateKeyToModifierFlag(int key)
{
    switch (key)
    {
        case RBLXWIN_KEY_LEFT_SHIFT:
        case RBLXWIN_KEY_RIGHT_SHIFT:
            return NSEventModifierFlagShift;
        case RBLXWIN_KEY_LEFT_CONTROL:
        case RBLXWIN_KEY_RIGHT_CONTROL:
            return NSEventModifierFlagControl;
        case RBLXWIN_KEY_LEFT_ALT:
        case RBLXWIN_KEY_RIGHT_ALT:
            return NSEventModifierFlagOption;
        case RBLXWIN_KEY_LEFT_SUPER:
        case RBLXWIN_KEY_RIGHT_SUPER:
            return NSEventModifierFlagCommand;
        case RBLXWIN_KEY_CAPS_LOCK:
            return NSEventModifierFlagCapsLock;
    }

    return 0;
}

// Defines a constant for empty ranges in NSTextInputClient
//
static const NSRange kEmptyRange = { NSNotFound, 0 };


//------------------------------------------------------------------------
// Delegate for window related notifications
//------------------------------------------------------------------------

@interface RBLXWINWindowDelegate : NSObject
{
    _RBLXWINwindow* window;
}

- (instancetype)initWithRblxWinWindow:(_RBLXWINwindow *)initWindow;

@end

@implementation RBLXWINWindowDelegate

- (instancetype)initWithRblxWinWindow:(_RBLXWINwindow *)initWindow
{
    self = [super init];
    if (self != nil)
        window = initWindow;

    return self;
}

- (BOOL)windowShouldClose:(id)sender
{
    _rblxwinInputWindowCloseRequest(window);
    return NO;
}

- (void)windowDidResize:(NSNotification *)notification
{
    if (window->context.source == RBLXWIN_NATIVE_CONTEXT_API)
        [window->context.nsgl.object update];

    if (_rblxwin.ns.disabledCursorWindow == window)
        _rblxwinCenterCursorInContentArea(window);

    const int maximized = [window->ns.object isZoomed];
    if (window->ns.maximized != maximized)
    {
        window->ns.maximized = maximized;
        _rblxwinInputWindowMaximize(window, maximized);
    }

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (fbRect.size.width != window->ns.fbWidth ||
        fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth  = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _rblxwinInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }

    if (contentRect.size.width != window->ns.width ||
        contentRect.size.height != window->ns.height)
    {
        window->ns.width  = contentRect.size.width;
        window->ns.height = contentRect.size.height;
        _rblxwinInputWindowSize(window, contentRect.size.width, contentRect.size.height);
    }
}

- (void)windowDidMove:(NSNotification *)notification
{
    if (window->context.source == RBLXWIN_NATIVE_CONTEXT_API)
        [window->context.nsgl.object update];

    if (_rblxwin.ns.disabledCursorWindow == window)
        _rblxwinCenterCursorInContentArea(window);

    int x, y;
    _rblxwinPlatformGetWindowPos(window, &x, &y);
    _rblxwinInputWindowPos(window, x, y);
}

- (void)windowDidMiniaturize:(NSNotification *)notification
{
    if (window->monitor)
        releaseMonitor(window);

    _rblxwinInputWindowIconify(window, RBLXWIN_TRUE);
}

- (void)windowDidDeminiaturize:(NSNotification *)notification
{
    if (window->monitor)
        acquireMonitor(window);

    _rblxwinInputWindowIconify(window, RBLXWIN_FALSE);
}

- (void)windowDidBecomeKey:(NSNotification *)notification
{
    if (_rblxwin.ns.disabledCursorWindow == window)
        _rblxwinCenterCursorInContentArea(window);

    _rblxwinInputWindowFocus(window, RBLXWIN_TRUE);
    updateCursorMode(window);
}

- (void)windowDidResignKey:(NSNotification *)notification
{
    if (window->monitor && window->autoIconify)
        _rblxwinPlatformIconifyWindow(window);

    _rblxwinInputWindowFocus(window, RBLXWIN_FALSE);
}

- (void)windowDidChangeOcclusionState:(NSNotification* )notification
{
    if ([window->ns.object occlusionState] & NSWindowOcclusionStateVisible)
        window->ns.occluded = RBLXWIN_FALSE;
    else
        window->ns.occluded = RBLXWIN_TRUE;
}

@end


//------------------------------------------------------------------------
// Content view class for the RBLXWIN window
//------------------------------------------------------------------------

@interface RBLXWINContentView : NSView <NSTextInputClient>
{
    _RBLXWINwindow* window;
    NSTrackingArea* trackingArea;
    NSMutableAttributedString* markedText;
}

- (instancetype)initWithRblxWinWindow:(_RBLXWINwindow *)initWindow;

@end

@implementation RBLXWINContentView

- (instancetype)initWithRblxWinWindow:(_RBLXWINwindow *)initWindow
{
    self = [super init];
    if (self != nil)
    {
        window = initWindow;
        trackingArea = nil;
        markedText = [[NSMutableAttributedString alloc] init];

        [self updateTrackingAreas];
        [self registerForDraggedTypes:@[NSPasteboardTypeURL]];
    }

    return self;
}

- (void)dealloc
{
    [trackingArea release];
    [markedText release];
    [super dealloc];
}

- (BOOL)isOpaque
{
    return [window->ns.object isOpaque];
}

- (BOOL)canBecomeKeyView
{
    return YES;
}

- (BOOL)acceptsFirstResponder
{
    return YES;
}

- (BOOL)wantsUpdateLayer
{
    return YES;
}

- (void)updateLayer
{
    if (window->context.source == RBLXWIN_NATIVE_CONTEXT_API)
        [window->context.nsgl.object update];

    _rblxwinInputWindowDamage(window);
}

- (void)cursorUpdate:(NSEvent *)event
{
    updateCursorImage(window);
}

- (BOOL)acceptsFirstMouse:(NSEvent *)event
{
    return YES;
}

- (void)mouseDown:(NSEvent *)event
{
    _rblxwinInputMouseClick(window,
                         RBLXWIN_MOUSE_BUTTON_LEFT,
                         RBLXWIN_PRESS,
                         translateFlags([event modifierFlags]));
}

- (void)mouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)mouseUp:(NSEvent *)event
{
    _rblxwinInputMouseClick(window,
                         RBLXWIN_MOUSE_BUTTON_LEFT,
                         RBLXWIN_RELEASE,
                         translateFlags([event modifierFlags]));
}

- (void)mouseMoved:(NSEvent *)event
{
    if (window->cursorMode == RBLXWIN_CURSOR_DISABLED)
    {
        const double dx = [event deltaX] - window->ns.cursorWarpDeltaX;
        const double dy = [event deltaY] - window->ns.cursorWarpDeltaY;

        _rblxwinInputCursorPos(window,
                            window->virtualCursorPosX + dx,
                            window->virtualCursorPosY + dy);
    }
    else
    {
        const NSRect contentRect = [window->ns.view frame];
        // NOTE: The returned location uses base 0,1 not 0,0
        const NSPoint pos = [event locationInWindow];

        _rblxwinInputCursorPos(window, pos.x, contentRect.size.height - pos.y);
    }

    window->ns.cursorWarpDeltaX = 0;
    window->ns.cursorWarpDeltaY = 0;
}

- (void)rightMouseDown:(NSEvent *)event
{
    _rblxwinInputMouseClick(window,
                         RBLXWIN_MOUSE_BUTTON_RIGHT,
                         RBLXWIN_PRESS,
                         translateFlags([event modifierFlags]));
}

- (void)rightMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)rightMouseUp:(NSEvent *)event
{
    _rblxwinInputMouseClick(window,
                         RBLXWIN_MOUSE_BUTTON_RIGHT,
                         RBLXWIN_RELEASE,
                         translateFlags([event modifierFlags]));
}

- (void)otherMouseDown:(NSEvent *)event
{
    _rblxwinInputMouseClick(window,
                         (int) [event buttonNumber],
                         RBLXWIN_PRESS,
                         translateFlags([event modifierFlags]));
}

- (void)otherMouseDragged:(NSEvent *)event
{
    [self mouseMoved:event];
}

- (void)otherMouseUp:(NSEvent *)event
{
    _rblxwinInputMouseClick(window,
                         (int) [event buttonNumber],
                         RBLXWIN_RELEASE,
                         translateFlags([event modifierFlags]));
}

- (void)mouseExited:(NSEvent *)event
{
    if (window->cursorMode == RBLXWIN_CURSOR_HIDDEN)
        showCursor(window);

    _rblxwinInputCursorEnter(window, RBLXWIN_FALSE);
}

- (void)mouseEntered:(NSEvent *)event
{
    if (window->cursorMode == RBLXWIN_CURSOR_HIDDEN)
        hideCursor(window);

    _rblxwinInputCursorEnter(window, RBLXWIN_TRUE);
}

- (void)viewDidChangeBackingProperties
{
    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];
    const float xscale = fbRect.size.width / contentRect.size.width;
    const float yscale = fbRect.size.height / contentRect.size.height;

    if (xscale != window->ns.xscale || yscale != window->ns.yscale)
    {
        if (window->ns.retina && window->ns.layer)
            [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];

        window->ns.xscale = xscale;
        window->ns.yscale = yscale;
        _rblxwinInputWindowContentScale(window, xscale, yscale);
    }

    if (fbRect.size.width != window->ns.fbWidth ||
        fbRect.size.height != window->ns.fbHeight)
    {
        window->ns.fbWidth  = fbRect.size.width;
        window->ns.fbHeight = fbRect.size.height;
        _rblxwinInputFramebufferSize(window, fbRect.size.width, fbRect.size.height);
    }
}

- (void)drawRect:(NSRect)rect
{
    _rblxwinInputWindowDamage(window);
}

- (void)updateTrackingAreas
{
    if (trackingArea != nil)
    {
        [self removeTrackingArea:trackingArea];
        [trackingArea release];
    }

    const NSTrackingAreaOptions options = NSTrackingMouseEnteredAndExited |
                                          NSTrackingActiveInKeyWindow |
                                          NSTrackingEnabledDuringMouseDrag |
                                          NSTrackingCursorUpdate |
                                          NSTrackingInVisibleRect |
                                          NSTrackingAssumeInside;

    trackingArea = [[NSTrackingArea alloc] initWithRect:[self bounds]
                                                options:options
                                                  owner:self
                                               userInfo:nil];

    [self addTrackingArea:trackingArea];
    [super updateTrackingAreas];
}

- (void)keyDown:(NSEvent *)event
{
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);

    _rblxwinInputKey(window, key, [event keyCode], RBLXWIN_PRESS, mods);

    [self interpretKeyEvents:@[event]];
}

- (void)flagsChanged:(NSEvent *)event
{
    int action;
    const unsigned int modifierFlags =
        [event modifierFlags] & NSEventModifierFlagDeviceIndependentFlagsMask;
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags(modifierFlags);
    const NSUInteger keyFlag = translateKeyToModifierFlag(key);

    if (keyFlag & modifierFlags)
    {
        if (window->keys[key] == RBLXWIN_PRESS)
            action = RBLXWIN_RELEASE;
        else
            action = RBLXWIN_PRESS;
    }
    else
        action = RBLXWIN_RELEASE;

    _rblxwinInputKey(window, key, [event keyCode], action, mods);
}

- (void)keyUp:(NSEvent *)event
{
    const int key = translateKey([event keyCode]);
    const int mods = translateFlags([event modifierFlags]);
    _rblxwinInputKey(window, key, [event keyCode], RBLXWIN_RELEASE, mods);
}

- (void)scrollWheel:(NSEvent *)event
{
    double deltaX = [event scrollingDeltaX];
    double deltaY = [event scrollingDeltaY];

    if ([event hasPreciseScrollingDeltas])
    {
        deltaX *= 0.1;
        deltaY *= 0.1;
    }

    if (fabs(deltaX) > 0.0 || fabs(deltaY) > 0.0)
        _rblxwinInputScroll(window, deltaX, deltaY);
}

- (NSDragOperation)draggingEntered:(id <NSDraggingInfo>)sender
{
    // HACK: We don't know what to say here because we don't know what the
    //       application wants to do with the paths
    return NSDragOperationGeneric;
}

- (BOOL)performDragOperation:(id <NSDraggingInfo>)sender
{
    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [sender draggingLocation];
    _rblxwinInputCursorPos(window, pos.x, contentRect.size.height - pos.y);

    NSPasteboard* pasteboard = [sender draggingPasteboard];
    NSDictionary* options = @{NSPasteboardURLReadingFileURLsOnlyKey:@YES};
    NSArray* urls = [pasteboard readObjectsForClasses:@[[NSURL class]]
                                              options:options];
    const NSUInteger count = [urls count];
    if (count)
    {
        char** paths = calloc(count, sizeof(char*));

        for (NSUInteger i = 0;  i < count;  i++)
            paths[i] = _rblxwin_strdup([urls[i] fileSystemRepresentation]);

        _rblxwinInputDrop(window, (int) count, (const char**) paths);

        for (NSUInteger i = 0;  i < count;  i++)
            free(paths[i]);
        free(paths);
    }

    return YES;
}

- (BOOL)hasMarkedText
{
    return [markedText length] > 0;
}

- (NSRange)markedRange
{
    if ([markedText length] > 0)
        return NSMakeRange(0, [markedText length] - 1);
    else
        return kEmptyRange;
}

- (NSRange)selectedRange
{
    return kEmptyRange;
}

- (void)setMarkedText:(id)string
        selectedRange:(NSRange)selectedRange
     replacementRange:(NSRange)replacementRange
{
    [markedText release];
    if ([string isKindOfClass:[NSAttributedString class]])
        markedText = [[NSMutableAttributedString alloc] initWithAttributedString:string];
    else
        markedText = [[NSMutableAttributedString alloc] initWithString:string];
}

- (void)unmarkText
{
    [[markedText mutableString] setString:@""];
}

- (NSArray*)validAttributesForMarkedText
{
    return [NSArray array];
}

- (NSAttributedString*)attributedSubstringForProposedRange:(NSRange)range
                                               actualRange:(NSRangePointer)actualRange
{
    return nil;
}

- (NSUInteger)characterIndexForPoint:(NSPoint)point
{
    return 0;
}

- (NSRect)firstRectForCharacterRange:(NSRange)range
                         actualRange:(NSRangePointer)actualRange
{
    const NSRect frame = [window->ns.view frame];
    return NSMakeRect(frame.origin.x, frame.origin.y, 0.0, 0.0);
}

- (void)insertText:(id)string replacementRange:(NSRange)replacementRange
{
    NSString* characters;
    NSEvent* event = [NSApp currentEvent];
    const int mods = translateFlags([event modifierFlags]);
    const int plain = !(mods & RBLXWIN_MOD_SUPER);

    if ([string isKindOfClass:[NSAttributedString class]])
        characters = [string string];
    else
        characters = (NSString*) string;

    NSRange range = NSMakeRange(0, [characters length]);
    while (range.length)
    {
        uint32_t codepoint = 0;

        if ([characters getBytes:&codepoint
                       maxLength:sizeof(codepoint)
                      usedLength:NULL
                        encoding:NSUTF32StringEncoding
                         options:0
                           range:range
                  remainingRange:&range])
        {
            if (codepoint >= 0xf700 && codepoint <= 0xf7ff)
                continue;

            _rblxwinInputChar(window, codepoint, mods, plain);
        }
    }
}

- (void)doCommandBySelector:(SEL)selector
{
}

@end


//------------------------------------------------------------------------
// RBLXWIN window class
//------------------------------------------------------------------------

@interface RBLXWINWindow : NSWindow {}
@end

@implementation RBLXWINWindow

- (BOOL)canBecomeKeyWindow
{
    // Required for NSWindowStyleMaskBorderless windows
    return YES;
}

- (BOOL)canBecomeMainWindow
{
    return YES;
}

@end


// Create the Cocoa window
//
static RBLXWINbool createNativeWindow(_RBLXWINwindow* window,
                                   const _RBLXWINwndconfig* wndconfig,
                                   const _RBLXWINfbconfig* fbconfig)
{
    window->ns.delegate = [[RBLXWINWindowDelegate alloc] initWithRblxWinWindow:window];
    if (window->ns.delegate == nil)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to create window delegate");
        return RBLXWIN_FALSE;
    }

    NSRect contentRect;

    if (window->monitor)
    {
        RBLXWINvidmode mode;
        int xpos, ypos;

        _rblxwinPlatformGetVideoMode(window->monitor, &mode);
        _rblxwinPlatformGetMonitorPos(window->monitor, &xpos, &ypos);

        contentRect = NSMakeRect(xpos, ypos, mode.width, mode.height);
    }
    else
        contentRect = NSMakeRect(0, 0, wndconfig->width, wndconfig->height);

    NSUInteger styleMask = NSWindowStyleMaskMiniaturizable;

    if (window->monitor || !window->decorated)
        styleMask |= NSWindowStyleMaskBorderless;
    else
    {
        styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);

        if (window->resizable)
            styleMask |= NSWindowStyleMaskResizable;
    }

    window->ns.object = [[RBLXWINWindow alloc]
        initWithContentRect:contentRect
                  styleMask:styleMask
                    backing:NSBackingStoreBuffered
                      defer:NO];

    if (window->ns.object == nil)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR, "Cocoa: Failed to create window");
        return RBLXWIN_FALSE;
    }

    if (window->monitor)
        [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
    else
    {
        [(NSWindow*) window->ns.object center];
        _rblxwin.ns.cascadePoint =
            NSPointToCGPoint([window->ns.object cascadeTopLeftFromPoint:
                              NSPointFromCGPoint(_rblxwin.ns.cascadePoint)]);

        if (wndconfig->resizable)
        {
            const NSWindowCollectionBehavior behavior =
                NSWindowCollectionBehaviorFullScreenPrimary |
                NSWindowCollectionBehaviorManaged;
            [window->ns.object setCollectionBehavior:behavior];
        }
        else
        {
            const NSWindowCollectionBehavior behavior =
                NSWindowCollectionBehaviorFullScreenNone;
            [window->ns.object setCollectionBehavior:behavior];
        }

        if (wndconfig->floating)
            [window->ns.object setLevel:NSFloatingWindowLevel];

        if (wndconfig->maximized)
            [window->ns.object zoom:nil];
    }

    if (strlen(wndconfig->ns.frameName))
        [window->ns.object setFrameAutosaveName:@(wndconfig->ns.frameName)];

    window->ns.view = [[RBLXWINContentView alloc] initWithRblxWinWindow:window];
    window->ns.retina = wndconfig->ns.retina;

    if (fbconfig->transparent)
    {
        [window->ns.object setOpaque:NO];
        [window->ns.object setHasShadow:NO];
        [window->ns.object setBackgroundColor:[NSColor clearColor]];
    }

    [window->ns.object setContentView:window->ns.view];
    [window->ns.object makeFirstResponder:window->ns.view];
    [window->ns.object setTitle:@(wndconfig->title)];
    [window->ns.object setDelegate:window->ns.delegate];
    [window->ns.object setAcceptsMouseMovedEvents:YES];
    [window->ns.object setRestorable:NO];

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101200
    if ([window->ns.object respondsToSelector:@selector(setTabbingMode:)])
        [window->ns.object setTabbingMode:NSWindowTabbingModeDisallowed];
#endif

    _rblxwinPlatformGetWindowSize(window, &window->ns.width, &window->ns.height);
    _rblxwinPlatformGetFramebufferSize(window, &window->ns.fbWidth, &window->ns.fbHeight);

    return RBLXWIN_TRUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Transforms a y-coordinate between the CG display and NS screen spaces
//
float _rblxwinTransformYNS(float y)
{
    return CGDisplayBounds(CGMainDisplayID()).size.height - y - 1;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformCreateWindow(_RBLXWINwindow* window,
                              const _RBLXWINwndconfig* wndconfig,
                              const _RBLXWINctxconfig* ctxconfig,
                              const _RBLXWINfbconfig* fbconfig)
{
    @autoreleasepool {

    if (!_rblxwin.ns.finishedLaunching)
        [NSApp run];

    if (!createNativeWindow(window, wndconfig, fbconfig))
        return RBLXWIN_FALSE;

    if (ctxconfig->client != RBLXWIN_NO_API)
    {
        if (ctxconfig->source == RBLXWIN_NATIVE_CONTEXT_API)
        {
            if (!_rblxwinInitNSGL())
                return RBLXWIN_FALSE;
            if (!_rblxwinCreateContextNSGL(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_EGL_CONTEXT_API)
        {
            // EGL implementation on macOS use CALayer* EGLNativeWindowType so we
            // need to get the layer for EGL window surface creation.
            [window->ns.view setWantsLayer:YES];
            window->ns.layer = [window->ns.view layer];

            if (!_rblxwinInitEGL())
                return RBLXWIN_FALSE;
            if (!_rblxwinCreateContextEGL(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }
        else if (ctxconfig->source == RBLXWIN_OSMESA_CONTEXT_API)
        {
            if (!_rblxwinInitOSMesa())
                return RBLXWIN_FALSE;
            if (!_rblxwinCreateContextOSMesa(window, ctxconfig, fbconfig))
                return RBLXWIN_FALSE;
        }

        if (!_rblxwinRefreshContextAttribs(window, ctxconfig))
            return RBLXWIN_FALSE;
    }

    if (window->monitor)
    {
        _rblxwinPlatformShowWindow(window);
        _rblxwinPlatformFocusWindow(window);
        acquireMonitor(window);

        if (wndconfig->centerCursor)
            _rblxwinCenterCursorInContentArea(window);
    }
    else
    {
        if (wndconfig->visible)
        {
            _rblxwinPlatformShowWindow(window);
            if (wndconfig->focused)
                _rblxwinPlatformFocusWindow(window);
        }
    }

    return RBLXWIN_TRUE;

    } // autoreleasepool
}

void _rblxwinPlatformDestroyWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {

    if (_rblxwin.ns.disabledCursorWindow == window)
        _rblxwin.ns.disabledCursorWindow = NULL;

    [window->ns.object orderOut:nil];

    if (window->monitor)
        releaseMonitor(window);

    if (window->context.destroy)
        window->context.destroy(window);

    [window->ns.object setDelegate:nil];
    [window->ns.delegate release];
    window->ns.delegate = nil;

    [window->ns.view release];
    window->ns.view = nil;

    [window->ns.object close];
    window->ns.object = nil;

    // HACK: Allow Cocoa to catch up before returning
    _rblxwinPlatformPollEvents();

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowTitle(_RBLXWINwindow* window, const char* title)
{
    @autoreleasepool {
    NSString* string = @(title);
    [window->ns.object setTitle:string];
    // HACK: Set the miniwindow title explicitly as setTitle: doesn't update it
    //       if the window lacks NSWindowStyleMaskTitled
    [window->ns.object setMiniwindowTitle:string];
    } // autoreleasepool
}

void _rblxwinPlatformSetWindowIcon(_RBLXWINwindow* window,
                                int count, const RBLXWINimage* images)
{
    // Regular windows do not have icons
}

void _rblxwinPlatformGetWindowPos(_RBLXWINwindow* window, int* xpos, int* ypos)
{
    @autoreleasepool {

    const NSRect contentRect =
        [window->ns.object contentRectForFrameRect:[window->ns.object frame]];

    if (xpos)
        *xpos = contentRect.origin.x;
    if (ypos)
        *ypos = _rblxwinTransformYNS(contentRect.origin.y + contentRect.size.height - 1);

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowPos(_RBLXWINwindow* window, int x, int y)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect dummyRect = NSMakeRect(x, _rblxwinTransformYNS(y + contentRect.size.height - 1), 0, 0);
    const NSRect frameRect = [window->ns.object frameRectForContentRect:dummyRect];
    [window->ns.object setFrameOrigin:frameRect.origin];

    } // autoreleasepool
}

void _rblxwinPlatformGetWindowSize(_RBLXWINwindow* window, int* width, int* height)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];

    if (width)
        *width = contentRect.size.width;
    if (height)
        *height = contentRect.size.height;

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowSize(_RBLXWINwindow* window, int width, int height)
{
    @autoreleasepool {

    if (window->monitor)
    {
        if (window->monitor->window == window)
            acquireMonitor(window);
    }
    else
    {
        NSRect contentRect =
            [window->ns.object contentRectForFrameRect:[window->ns.object frame]];
        contentRect.origin.y += contentRect.size.height - height;
        contentRect.size = NSMakeSize(width, height);
        [window->ns.object setFrame:[window->ns.object frameRectForContentRect:contentRect]
                            display:YES];
    }

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowSizeLimits(_RBLXWINwindow* window,
                                      int minwidth, int minheight,
                                      int maxwidth, int maxheight)
{
    @autoreleasepool {

    if (minwidth == RBLXWIN_DONT_CARE || minheight == RBLXWIN_DONT_CARE)
        [window->ns.object setContentMinSize:NSMakeSize(0, 0)];
    else
        [window->ns.object setContentMinSize:NSMakeSize(minwidth, minheight)];

    if (maxwidth == RBLXWIN_DONT_CARE || maxheight == RBLXWIN_DONT_CARE)
        [window->ns.object setContentMaxSize:NSMakeSize(DBL_MAX, DBL_MAX)];
    else
        [window->ns.object setContentMaxSize:NSMakeSize(maxwidth, maxheight)];

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowAspectRatio(_RBLXWINwindow* window, int numer, int denom)
{
    @autoreleasepool {
    if (numer == RBLXWIN_DONT_CARE || denom == RBLXWIN_DONT_CARE)
        [window->ns.object setResizeIncrements:NSMakeSize(1.0, 1.0)];
    else
        [window->ns.object setContentAspectRatio:NSMakeSize(numer, denom)];
    } // autoreleasepool
}

void _rblxwinPlatformGetFramebufferSize(_RBLXWINwindow* window, int* width, int* height)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect fbRect = [window->ns.view convertRectToBacking:contentRect];

    if (width)
        *width = (int) fbRect.size.width;
    if (height)
        *height = (int) fbRect.size.height;

    } // autoreleasepool
}

void _rblxwinPlatformGetWindowFrameSize(_RBLXWINwindow* window,
                                     int* left, int* top,
                                     int* right, int* bottom)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    const NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect];

    if (left)
        *left = contentRect.origin.x - frameRect.origin.x;
    if (top)
        *top = frameRect.origin.y + frameRect.size.height -
               contentRect.origin.y - contentRect.size.height;
    if (right)
        *right = frameRect.origin.x + frameRect.size.width -
                 contentRect.origin.x - contentRect.size.width;
    if (bottom)
        *bottom = contentRect.origin.y - frameRect.origin.y;

    } // autoreleasepool
}

void _rblxwinPlatformGetWindowContentScale(_RBLXWINwindow* window,
                                        float* xscale, float* yscale)
{
    @autoreleasepool {

    const NSRect points = [window->ns.view frame];
    const NSRect pixels = [window->ns.view convertRectToBacking:points];

    if (xscale)
        *xscale = (float) (pixels.size.width / points.size.width);
    if (yscale)
        *yscale = (float) (pixels.size.height / points.size.height);

    } // autoreleasepool
}

void _rblxwinPlatformIconifyWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {
    [window->ns.object miniaturize:nil];
    } // autoreleasepool
}

void _rblxwinPlatformRestoreWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {
    if ([window->ns.object isMiniaturized])
        [window->ns.object deminiaturize:nil];
    else if ([window->ns.object isZoomed])
        [window->ns.object zoom:nil];
    } // autoreleasepool
}

void _rblxwinPlatformMaximizeWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {
    if (![window->ns.object isZoomed])
        [window->ns.object zoom:nil];
    } // autoreleasepool
}

void _rblxwinPlatformShowWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {
    [window->ns.object orderFront:nil];
    } // autoreleasepool
}

void _rblxwinPlatformHideWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {
    [window->ns.object orderOut:nil];
    } // autoreleasepool
}

void _rblxwinPlatformRequestWindowAttention(_RBLXWINwindow* window)
{
    @autoreleasepool {
    [NSApp requestUserAttention:NSInformationalRequest];
    } // autoreleasepool
}

void _rblxwinPlatformFocusWindow(_RBLXWINwindow* window)
{
    @autoreleasepool {
    // Make us the active application
    // HACK: This is here to prevent applications using only hidden windows from
    //       being activated, but should probably not be done every time any
    //       window is shown
    [NSApp activateIgnoringOtherApps:YES];
    [window->ns.object makeKeyAndOrderFront:nil];
    } // autoreleasepool
}

void _rblxwinPlatformSetWindowMonitor(_RBLXWINwindow* window,
                                   _RBLXWINmonitor* monitor,
                                   int xpos, int ypos,
                                   int width, int height,
                                   int refreshRate)
{
    @autoreleasepool {

    if (window->monitor == monitor)
    {
        if (monitor)
        {
            if (monitor->window == window)
                acquireMonitor(window);
        }
        else
        {
            const NSRect contentRect =
                NSMakeRect(xpos, _rblxwinTransformYNS(ypos + height - 1), width, height);
            const NSUInteger styleMask = [window->ns.object styleMask];
            const NSRect frameRect =
                [window->ns.object frameRectForContentRect:contentRect
                                                 styleMask:styleMask];

            [window->ns.object setFrame:frameRect display:YES];
        }

        return;
    }

    if (window->monitor)
        releaseMonitor(window);

    _rblxwinInputWindowMonitor(window, monitor);

    // HACK: Allow the state cached in Cocoa to catch up to reality
    // TODO: Solve this in a less terrible way
    _rblxwinPlatformPollEvents();

    NSUInteger styleMask = [window->ns.object styleMask];

    if (window->monitor)
    {
        styleMask &= ~(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
        styleMask |= NSWindowStyleMaskBorderless;
    }
    else
    {
        if (window->decorated)
        {
            styleMask &= ~NSWindowStyleMaskBorderless;
            styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
        }

        if (window->resizable)
            styleMask |= NSWindowStyleMaskResizable;
        else
            styleMask &= ~NSWindowStyleMaskResizable;
    }

    [window->ns.object setStyleMask:styleMask];
    // HACK: Changing the style mask can cause the first responder to be cleared
    [window->ns.object makeFirstResponder:window->ns.view];

    if (window->monitor)
    {
        [window->ns.object setLevel:NSMainMenuWindowLevel + 1];
        [window->ns.object setHasShadow:NO];

        acquireMonitor(window);
    }
    else
    {
        NSRect contentRect = NSMakeRect(xpos, _rblxwinTransformYNS(ypos + height - 1),
                                        width, height);
        NSRect frameRect = [window->ns.object frameRectForContentRect:contentRect
                                                            styleMask:styleMask];
        [window->ns.object setFrame:frameRect display:YES];

        if (window->numer != RBLXWIN_DONT_CARE &&
            window->denom != RBLXWIN_DONT_CARE)
        {
            [window->ns.object setContentAspectRatio:NSMakeSize(window->numer,
                                                                window->denom)];
        }

        if (window->minwidth != RBLXWIN_DONT_CARE &&
            window->minheight != RBLXWIN_DONT_CARE)
        {
            [window->ns.object setContentMinSize:NSMakeSize(window->minwidth,
                                                            window->minheight)];
        }

        if (window->maxwidth != RBLXWIN_DONT_CARE &&
            window->maxheight != RBLXWIN_DONT_CARE)
        {
            [window->ns.object setContentMaxSize:NSMakeSize(window->maxwidth,
                                                            window->maxheight)];
        }

        if (window->floating)
            [window->ns.object setLevel:NSFloatingWindowLevel];
        else
            [window->ns.object setLevel:NSNormalWindowLevel];

        if (window->resizable)
        {
            const NSWindowCollectionBehavior behavior =
                NSWindowCollectionBehaviorFullScreenPrimary |
                NSWindowCollectionBehaviorManaged;
            [window->ns.object setCollectionBehavior:behavior];
        }
        else
        {
            const NSWindowCollectionBehavior behavior =
                NSWindowCollectionBehaviorFullScreenNone;
            [window->ns.object setCollectionBehavior:behavior];
        }

        [window->ns.object setHasShadow:YES];
        // HACK: Clearing NSWindowStyleMaskTitled resets and disables the window
        //       title property but the miniwindow title property is unaffected
        [window->ns.object setTitle:[window->ns.object miniwindowTitle]];
    }

    } // autoreleasepool
}

int _rblxwinPlatformWindowFocused(_RBLXWINwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isKeyWindow];
    } // autoreleasepool
}

int _rblxwinPlatformWindowIconified(_RBLXWINwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isMiniaturized];
    } // autoreleasepool
}

int _rblxwinPlatformWindowVisible(_RBLXWINwindow* window)
{
    @autoreleasepool {
    return [window->ns.object isVisible];
    } // autoreleasepool
}

int _rblxwinPlatformWindowMaximized(_RBLXWINwindow* window)
{
    @autoreleasepool {

    if (window->resizable)
        return [window->ns.object isZoomed];
    else
        return RBLXWIN_FALSE;

    } // autoreleasepool
}

int _rblxwinPlatformWindowHovered(_RBLXWINwindow* window)
{
    @autoreleasepool {

    const NSPoint point = [NSEvent mouseLocation];

    if ([NSWindow windowNumberAtPoint:point belowWindowWithWindowNumber:0] !=
        [window->ns.object windowNumber])
    {
        return RBLXWIN_FALSE;
    }

    return NSMouseInRect(point,
        [window->ns.object convertRectToScreen:[window->ns.view frame]], NO);

    } // autoreleasepool
}

int _rblxwinPlatformFramebufferTransparent(_RBLXWINwindow* window)
{
    @autoreleasepool {
    return ![window->ns.object isOpaque] && ![window->ns.view isOpaque];
    } // autoreleasepool
}

void _rblxwinPlatformSetWindowResizable(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    @autoreleasepool {

    const NSUInteger styleMask = [window->ns.object styleMask];
    if (enabled)
    {
        [window->ns.object setStyleMask:(styleMask | NSWindowStyleMaskResizable)];
        const NSWindowCollectionBehavior behavior =
            NSWindowCollectionBehaviorFullScreenPrimary |
            NSWindowCollectionBehaviorManaged;
        [window->ns.object setCollectionBehavior:behavior];
    }
    else
    {
        [window->ns.object setStyleMask:(styleMask & ~NSWindowStyleMaskResizable)];
        const NSWindowCollectionBehavior behavior =
            NSWindowCollectionBehaviorFullScreenNone;
        [window->ns.object setCollectionBehavior:behavior];
    }

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowDecorated(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    @autoreleasepool {

    NSUInteger styleMask = [window->ns.object styleMask];
    if (enabled)
    {
        styleMask |= (NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
        styleMask &= ~NSWindowStyleMaskBorderless;
    }
    else
    {
        styleMask |= NSWindowStyleMaskBorderless;
        styleMask &= ~(NSWindowStyleMaskTitled | NSWindowStyleMaskClosable);
    }

    [window->ns.object setStyleMask:styleMask];
    [window->ns.object makeFirstResponder:window->ns.view];

    } // autoreleasepool
}

void _rblxwinPlatformSetWindowFloating(_RBLXWINwindow* window, RBLXWINbool enabled)
{
    @autoreleasepool {
    if (enabled)
        [window->ns.object setLevel:NSFloatingWindowLevel];
    else
        [window->ns.object setLevel:NSNormalWindowLevel];
    } // autoreleasepool
}

float _rblxwinPlatformGetWindowOpacity(_RBLXWINwindow* window)
{
    @autoreleasepool {
    return (float) [window->ns.object alphaValue];
    } // autoreleasepool
}

void _rblxwinPlatformSetWindowOpacity(_RBLXWINwindow* window, float opacity)
{
    @autoreleasepool {
    [window->ns.object setAlphaValue:opacity];
    } // autoreleasepool
}

void _rblxwinPlatformSetRawMouseMotion(_RBLXWINwindow *window, RBLXWINbool enabled)
{
}

RBLXWINbool _rblxwinPlatformRawMouseMotionSupported(void)
{
    return RBLXWIN_FALSE;
}

void _rblxwinPlatformPollEvents(void)
{
    @autoreleasepool {

    if (!_rblxwin.ns.finishedLaunching)
        [NSApp run];

    for (;;)
    {
        NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                            untilDate:[NSDate distantPast]
                                               inMode:NSDefaultRunLoopMode
                                              dequeue:YES];
        if (event == nil)
            break;

        [NSApp sendEvent:event];
    }

    } // autoreleasepool
}

void _rblxwinPlatformWaitEvents(void)
{
    @autoreleasepool {

    if (!_rblxwin.ns.finishedLaunching)
        [NSApp run];

    // I wanted to pass NO to dequeue:, and rely on PollEvents to
    // dequeue and send.  For reasons not at all clear to me, passing
    // NO to dequeue: causes this method never to return.
    NSEvent *event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:[NSDate distantFuture]
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    [NSApp sendEvent:event];

    _rblxwinPlatformPollEvents();

    } // autoreleasepool
}

void _rblxwinPlatformWaitEventsTimeout(double timeout)
{
    @autoreleasepool {

    if (!_rblxwin.ns.finishedLaunching)
        [NSApp run];

    NSDate* date = [NSDate dateWithTimeIntervalSinceNow:timeout];
    NSEvent* event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                        untilDate:date
                                           inMode:NSDefaultRunLoopMode
                                          dequeue:YES];
    if (event)
        [NSApp sendEvent:event];

    _rblxwinPlatformPollEvents();

    } // autoreleasepool
}

void _rblxwinPlatformPostEmptyEvent(void)
{
    @autoreleasepool {

    if (!_rblxwin.ns.finishedLaunching)
        [NSApp run];

    NSEvent* event = [NSEvent otherEventWithType:NSEventTypeApplicationDefined
                                        location:NSMakePoint(0, 0)
                                   modifierFlags:0
                                       timestamp:0
                                    windowNumber:0
                                         context:nil
                                         subtype:0
                                           data1:0
                                           data2:0];
    [NSApp postEvent:event atStart:YES];

    } // autoreleasepool
}

void _rblxwinPlatformGetCursorPos(_RBLXWINwindow* window, double* xpos, double* ypos)
{
    @autoreleasepool {

    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

    if (xpos)
        *xpos = pos.x;
    if (ypos)
        *ypos = contentRect.size.height - pos.y;

    } // autoreleasepool
}

void _rblxwinPlatformSetCursorPos(_RBLXWINwindow* window, double x, double y)
{
    @autoreleasepool {

    updateCursorImage(window);

    const NSRect contentRect = [window->ns.view frame];
    // NOTE: The returned location uses base 0,1 not 0,0
    const NSPoint pos = [window->ns.object mouseLocationOutsideOfEventStream];

    window->ns.cursorWarpDeltaX += x - pos.x;
    window->ns.cursorWarpDeltaY += y - contentRect.size.height + pos.y;

    if (window->monitor)
    {
        CGDisplayMoveCursorToPoint(window->monitor->ns.displayID,
                                   CGPointMake(x, y));
    }
    else
    {
        const NSRect localRect = NSMakeRect(x, contentRect.size.height - y - 1, 0, 0);
        const NSRect globalRect = [window->ns.object convertRectToScreen:localRect];
        const NSPoint globalPoint = globalRect.origin;

        CGWarpMouseCursorPosition(CGPointMake(globalPoint.x,
                                              _rblxwinTransformYNS(globalPoint.y)));
    }

    // HACK: Calling this right after setting the cursor position prevents macOS
    //       from freezing the cursor for a fraction of a second afterwards
    if (window->cursorMode != RBLXWIN_CURSOR_DISABLED)
        CGAssociateMouseAndMouseCursorPosition(true);

    } // autoreleasepool
}

void _rblxwinPlatformSetCursorMode(_RBLXWINwindow* window, int mode)
{
    @autoreleasepool {
    if (_rblxwinPlatformWindowFocused(window))
        updateCursorMode(window);
    } // autoreleasepool
}

const char* _rblxwinPlatformGetScancodeName(int scancode)
{
    @autoreleasepool {

    if (scancode < 0 || scancode > 0xff ||
        _rblxwin.ns.keycodes[scancode] == RBLXWIN_KEY_UNKNOWN)
    {
        _rblxwinInputError(RBLXWIN_INVALID_VALUE, "Invalid scancode %i", scancode);
        return NULL;
    }

    const int key = _rblxwin.ns.keycodes[scancode];

    UInt32 deadKeyState = 0;
    UniChar characters[4];
    UniCharCount characterCount = 0;

    if (UCKeyTranslate([(NSData*) _rblxwin.ns.unicodeData bytes],
                       scancode,
                       kUCKeyActionDisplay,
                       0,
                       LMGetKbdType(),
                       kUCKeyTranslateNoDeadKeysBit,
                       &deadKeyState,
                       sizeof(characters) / sizeof(characters[0]),
                       &characterCount,
                       characters) != noErr)
    {
        return NULL;
    }

    if (!characterCount)
        return NULL;

    CFStringRef string = CFStringCreateWithCharactersNoCopy(kCFAllocatorDefault,
                                                            characters,
                                                            characterCount,
                                                            kCFAllocatorNull);
    CFStringGetCString(string,
                       _rblxwin.ns.keynames[key],
                       sizeof(_rblxwin.ns.keynames[key]),
                       kCFStringEncodingUTF8);
    CFRelease(string);

    return _rblxwin.ns.keynames[key];

    } // autoreleasepool
}

int _rblxwinPlatformGetKeyScancode(int key)
{
    return _rblxwin.ns.scancodes[key];
}

int _rblxwinPlatformCreateCursor(_RBLXWINcursor* cursor,
                              const RBLXWINimage* image,
                              int xhot, int yhot)
{
    @autoreleasepool {

    NSImage* native;
    NSBitmapImageRep* rep;

    rep = [[NSBitmapImageRep alloc]
        initWithBitmapDataPlanes:NULL
                      pixelsWide:image->width
                      pixelsHigh:image->height
                   bitsPerSample:8
                 samplesPerPixel:4
                        hasAlpha:YES
                        isPlanar:NO
                  colorSpaceName:NSCalibratedRGBColorSpace
                    bitmapFormat:NSBitmapFormatAlphaNonpremultiplied
                     bytesPerRow:image->width * 4
                    bitsPerPixel:32];

    if (rep == nil)
        return RBLXWIN_FALSE;

    memcpy([rep bitmapData], image->pixels, image->width * image->height * 4);

    native = [[NSImage alloc] initWithSize:NSMakeSize(image->width, image->height)];
    [native addRepresentation:rep];

    cursor->ns.object = [[NSCursor alloc] initWithImage:native
                                                hotSpot:NSMakePoint(xhot, yhot)];

    [native release];
    [rep release];

    if (cursor->ns.object == nil)
        return RBLXWIN_FALSE;

    return RBLXWIN_TRUE;

    } // autoreleasepool
}

int _rblxwinPlatformCreateStandardCursor(_RBLXWINcursor* cursor, int shape)
{
    @autoreleasepool {

    if (shape == RBLXWIN_ARROW_CURSOR)
        cursor->ns.object = [NSCursor arrowCursor];
    else if (shape == RBLXWIN_IBEAM_CURSOR)
        cursor->ns.object = [NSCursor IBeamCursor];
    else if (shape == RBLXWIN_CROSSHAIR_CURSOR)
        cursor->ns.object = [NSCursor crosshairCursor];
    else if (shape == RBLXWIN_HAND_CURSOR)
        cursor->ns.object = [NSCursor pointingHandCursor];
    else if (shape == RBLXWIN_HRESIZE_CURSOR)
        cursor->ns.object = [NSCursor resizeLeftRightCursor];
    else if (shape == RBLXWIN_VRESIZE_CURSOR)
        cursor->ns.object = [NSCursor resizeUpDownCursor];

    if (!cursor->ns.object)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve standard cursor");
        return RBLXWIN_FALSE;
    }

    [cursor->ns.object retain];
    return RBLXWIN_TRUE;

    } // autoreleasepool
}

void _rblxwinPlatformDestroyCursor(_RBLXWINcursor* cursor)
{
    @autoreleasepool {
    if (cursor->ns.object)
        [(NSCursor*) cursor->ns.object release];
    } // autoreleasepool
}

void _rblxwinPlatformSetCursor(_RBLXWINwindow* window, _RBLXWINcursor* cursor)
{
    @autoreleasepool {
    if (cursorInContentArea(window))
        updateCursorImage(window);
    } // autoreleasepool
}

void _rblxwinPlatformSetClipboardString(const char* string)
{
    @autoreleasepool {
    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];
    [pasteboard declareTypes:@[NSPasteboardTypeString] owner:nil];
    [pasteboard setString:@(string) forType:NSPasteboardTypeString];
    } // autoreleasepool
}

const char* _rblxwinPlatformGetClipboardString(void)
{
    @autoreleasepool {

    NSPasteboard* pasteboard = [NSPasteboard generalPasteboard];

    if (![[pasteboard types] containsObject:NSPasteboardTypeString])
    {
        _rblxwinInputError(RBLXWIN_FORMAT_UNAVAILABLE,
                        "Cocoa: Failed to retrieve string from pasteboard");
        return NULL;
    }

    NSString* object = [pasteboard stringForType:NSPasteboardTypeString];
    if (!object)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve object from pasteboard");
        return NULL;
    }

    free(_rblxwin.ns.clipboardString);
    _rblxwin.ns.clipboardString = _rblxwin_strdup([object UTF8String]);

    return _rblxwin.ns.clipboardString;

    } // autoreleasepool
}

void _rblxwinPlatformGetRequiredInstanceExtensions(char** extensions)
{
    if (_rblxwin.vk.KHR_surface && _rblxwin.vk.EXT_metal_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_EXT_metal_surface";
    }
    else if (_rblxwin.vk.KHR_surface && _rblxwin.vk.MVK_macos_surface)
    {
        extensions[0] = "VK_KHR_surface";
        extensions[1] = "VK_MVK_macos_surface";
    }
}

int _rblxwinPlatformGetPhysicalDevicePresentationSupport(VkInstance instance,
                                                      VkPhysicalDevice device,
                                                      uint32_t queuefamily)
{
    return RBLXWIN_TRUE;
}

VkResult _rblxwinPlatformCreateWindowSurface(VkInstance instance,
                                          _RBLXWINwindow* window,
                                          const VkAllocationCallbacks* allocator,
                                          VkSurfaceKHR* surface)
{
    @autoreleasepool {

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
    // HACK: Dynamically load Core Animation to avoid adding an extra
    //       dependency for the majority who don't use MoltenVK
    NSBundle* bundle = [NSBundle bundleWithPath:@"/System/Library/Frameworks/QuartzCore.framework"];
    if (!bundle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to find QuartzCore.framework");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    // NOTE: Create the layer here as makeBackingLayer should not return nil
    window->ns.layer = [[bundle classNamed:@"CAMetalLayer"] layer];
    if (!window->ns.layer)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to create layer for view");
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    if (window->ns.retina)
        [window->ns.layer setContentsScale:[window->ns.object backingScaleFactor]];

    [window->ns.view setLayer:window->ns.layer];
    [window->ns.view setWantsLayer:YES];

    VkResult err;

    if (_rblxwin.vk.EXT_metal_surface)
    {
        VkMetalSurfaceCreateInfoEXT sci;

        PFN_vkCreateMetalSurfaceEXT vkCreateMetalSurfaceEXT;
        vkCreateMetalSurfaceEXT = (PFN_vkCreateMetalSurfaceEXT)
            vkGetInstanceProcAddr(instance, "vkCreateMetalSurfaceEXT");
        if (!vkCreateMetalSurfaceEXT)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "Cocoa: Vulkan instance missing VK_EXT_metal_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
        sci.pLayer = window->ns.layer;

        err = vkCreateMetalSurfaceEXT(instance, &sci, allocator, surface);
    }
    else
    {
        VkMacOSSurfaceCreateInfoMVK sci;

        PFN_vkCreateMacOSSurfaceMVK vkCreateMacOSSurfaceMVK;
        vkCreateMacOSSurfaceMVK = (PFN_vkCreateMacOSSurfaceMVK)
            vkGetInstanceProcAddr(instance, "vkCreateMacOSSurfaceMVK");
        if (!vkCreateMacOSSurfaceMVK)
        {
            _rblxwinInputError(RBLXWIN_API_UNAVAILABLE,
                            "Cocoa: Vulkan instance missing VK_MVK_macos_surface extension");
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }

        memset(&sci, 0, sizeof(sci));
        sci.sType = VK_STRUCTURE_TYPE_MACOS_SURFACE_CREATE_INFO_MVK;
        sci.pView = window->ns.view;

        err = vkCreateMacOSSurfaceMVK(instance, &sci, allocator, surface);
    }

    if (err)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to create Vulkan surface: %s",
                        _rblxwinGetVulkanResultString(err));
    }

    return err;
#else
    return VK_ERROR_EXTENSION_NOT_PRESENT;
#endif

    } // autoreleasepool
}


//////////////////////////////////////////////////////////////////////////
//////                        RBLXWIN native API                       //////
//////////////////////////////////////////////////////////////////////////

RBLXWINAPI id rblxwinGetCocoaWindow(RBLXWINwindow* handle)
{
    _RBLXWINwindow* window = (_RBLXWINwindow*) handle;
    _RBLXWIN_REQUIRE_INIT_OR_RETURN(nil);
    return window->ns.object;
}

