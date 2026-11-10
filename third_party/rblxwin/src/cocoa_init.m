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
#include <sys/param.h> // For MAXPATHLEN

// Needed for _NSGetProgname
#include <crt_externs.h>

// Change to our application bundle's resources directory, if present
//
static void changeToResourcesDirectory(void)
{
    char resourcesPath[MAXPATHLEN];

    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return;

    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(bundle);

    CFStringRef last = CFURLCopyLastPathComponent(resourcesURL);
    if (CFStringCompare(CFSTR("Resources"), last, 0) != kCFCompareEqualTo)
    {
        CFRelease(last);
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(last);

    if (!CFURLGetFileSystemRepresentation(resourcesURL,
                                          true,
                                          (UInt8*) resourcesPath,
                                          MAXPATHLEN))
    {
        CFRelease(resourcesURL);
        return;
    }

    CFRelease(resourcesURL);

    chdir(resourcesPath);
}

// Set up the menu bar (manually)
// This is nasty, nasty stuff -- calls to undocumented semi-private APIs that
// could go away at any moment, lots of stuff that really should be
// localize(d|able), etc.  Add a nib to save us this horror.
//
static void createMenuBar(void)
{
    size_t i;
    NSString* appName = nil;
    NSDictionary* bundleInfo = [[NSBundle mainBundle] infoDictionary];
    NSString* nameKeys[] =
    {
        @"CFBundleDisplayName",
        @"CFBundleName",
        @"CFBundleExecutable",
    };

    // Try to figure out what the calling application is called

    for (i = 0;  i < sizeof(nameKeys) / sizeof(nameKeys[0]);  i++)
    {
        id name = bundleInfo[nameKeys[i]];
        if (name &&
            [name isKindOfClass:[NSString class]] &&
            ![name isEqualToString:@""])
        {
            appName = name;
            break;
        }
    }

    if (!appName)
    {
        char** progname = _NSGetProgname();
        if (progname && *progname)
            appName = @(*progname);
        else
            appName = @"RBLXWIN Application";
    }

    NSMenu* bar = [[NSMenu alloc] init];
    [NSApp setMainMenu:bar];

    NSMenuItem* appMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    NSMenu* appMenu = [[NSMenu alloc] init];
    [appMenuItem setSubmenu:appMenu];

    [appMenu addItemWithTitle:[NSString stringWithFormat:@"About %@", appName]
                       action:@selector(orderFrontStandardAboutPanel:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    NSMenu* servicesMenu = [[NSMenu alloc] init];
    [NSApp setServicesMenu:servicesMenu];
    [[appMenu addItemWithTitle:@"Services"
                       action:NULL
                keyEquivalent:@""] setSubmenu:servicesMenu];
    [servicesMenu release];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Hide %@", appName]
                       action:@selector(hide:)
                keyEquivalent:@"h"];
    [[appMenu addItemWithTitle:@"Hide Others"
                       action:@selector(hideOtherApplications:)
                keyEquivalent:@"h"]
        setKeyEquivalentModifierMask:NSEventModifierFlagOption | NSEventModifierFlagCommand];
    [appMenu addItemWithTitle:@"Show All"
                       action:@selector(unhideAllApplications:)
                keyEquivalent:@""];
    [appMenu addItem:[NSMenuItem separatorItem]];
    [appMenu addItemWithTitle:[NSString stringWithFormat:@"Quit %@", appName]
                       action:@selector(terminate:)
                keyEquivalent:@"q"];

    NSMenuItem* windowMenuItem =
        [bar addItemWithTitle:@"" action:NULL keyEquivalent:@""];
    [bar release];
    NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
    [NSApp setWindowsMenu:windowMenu];
    [windowMenuItem setSubmenu:windowMenu];

    [windowMenu addItemWithTitle:@"Minimize"
                          action:@selector(performMiniaturize:)
                   keyEquivalent:@"m"];
    [windowMenu addItemWithTitle:@"Zoom"
                          action:@selector(performZoom:)
                   keyEquivalent:@""];
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [windowMenu addItemWithTitle:@"Bring All to Front"
                          action:@selector(arrangeInFront:)
                   keyEquivalent:@""];

    // TODO: Make this appear at the bottom of the menu (for consistency)
    [windowMenu addItem:[NSMenuItem separatorItem]];
    [[windowMenu addItemWithTitle:@"Enter Full Screen"
                           action:@selector(toggleFullScreen:)
                    keyEquivalent:@"f"]
     setKeyEquivalentModifierMask:NSEventModifierFlagControl | NSEventModifierFlagCommand];

    // Prior to Snow Leopard, we need to use this oddly-named semi-private API
    // to get the application menu working properly.
    SEL setAppleMenuSelector = NSSelectorFromString(@"setAppleMenu:");
    [NSApp performSelector:setAppleMenuSelector withObject:appMenu];
}

// Create key code translation tables
//
static void createKeyTables(void)
{
    int scancode;

    memset(_rblxwin.ns.keycodes, -1, sizeof(_rblxwin.ns.keycodes));
    memset(_rblxwin.ns.scancodes, -1, sizeof(_rblxwin.ns.scancodes));

    _rblxwin.ns.keycodes[0x1D] = RBLXWIN_KEY_0;
    _rblxwin.ns.keycodes[0x12] = RBLXWIN_KEY_1;
    _rblxwin.ns.keycodes[0x13] = RBLXWIN_KEY_2;
    _rblxwin.ns.keycodes[0x14] = RBLXWIN_KEY_3;
    _rblxwin.ns.keycodes[0x15] = RBLXWIN_KEY_4;
    _rblxwin.ns.keycodes[0x17] = RBLXWIN_KEY_5;
    _rblxwin.ns.keycodes[0x16] = RBLXWIN_KEY_6;
    _rblxwin.ns.keycodes[0x1A] = RBLXWIN_KEY_7;
    _rblxwin.ns.keycodes[0x1C] = RBLXWIN_KEY_8;
    _rblxwin.ns.keycodes[0x19] = RBLXWIN_KEY_9;
    _rblxwin.ns.keycodes[0x00] = RBLXWIN_KEY_A;
    _rblxwin.ns.keycodes[0x0B] = RBLXWIN_KEY_B;
    _rblxwin.ns.keycodes[0x08] = RBLXWIN_KEY_C;
    _rblxwin.ns.keycodes[0x02] = RBLXWIN_KEY_D;
    _rblxwin.ns.keycodes[0x0E] = RBLXWIN_KEY_E;
    _rblxwin.ns.keycodes[0x03] = RBLXWIN_KEY_F;
    _rblxwin.ns.keycodes[0x05] = RBLXWIN_KEY_G;
    _rblxwin.ns.keycodes[0x04] = RBLXWIN_KEY_H;
    _rblxwin.ns.keycodes[0x22] = RBLXWIN_KEY_I;
    _rblxwin.ns.keycodes[0x26] = RBLXWIN_KEY_J;
    _rblxwin.ns.keycodes[0x28] = RBLXWIN_KEY_K;
    _rblxwin.ns.keycodes[0x25] = RBLXWIN_KEY_L;
    _rblxwin.ns.keycodes[0x2E] = RBLXWIN_KEY_M;
    _rblxwin.ns.keycodes[0x2D] = RBLXWIN_KEY_N;
    _rblxwin.ns.keycodes[0x1F] = RBLXWIN_KEY_O;
    _rblxwin.ns.keycodes[0x23] = RBLXWIN_KEY_P;
    _rblxwin.ns.keycodes[0x0C] = RBLXWIN_KEY_Q;
    _rblxwin.ns.keycodes[0x0F] = RBLXWIN_KEY_R;
    _rblxwin.ns.keycodes[0x01] = RBLXWIN_KEY_S;
    _rblxwin.ns.keycodes[0x11] = RBLXWIN_KEY_T;
    _rblxwin.ns.keycodes[0x20] = RBLXWIN_KEY_U;
    _rblxwin.ns.keycodes[0x09] = RBLXWIN_KEY_V;
    _rblxwin.ns.keycodes[0x0D] = RBLXWIN_KEY_W;
    _rblxwin.ns.keycodes[0x07] = RBLXWIN_KEY_X;
    _rblxwin.ns.keycodes[0x10] = RBLXWIN_KEY_Y;
    _rblxwin.ns.keycodes[0x06] = RBLXWIN_KEY_Z;

    _rblxwin.ns.keycodes[0x27] = RBLXWIN_KEY_APOSTROPHE;
    _rblxwin.ns.keycodes[0x2A] = RBLXWIN_KEY_BACKSLASH;
    _rblxwin.ns.keycodes[0x2B] = RBLXWIN_KEY_COMMA;
    _rblxwin.ns.keycodes[0x18] = RBLXWIN_KEY_EQUAL;
    _rblxwin.ns.keycodes[0x32] = RBLXWIN_KEY_GRAVE_ACCENT;
    _rblxwin.ns.keycodes[0x21] = RBLXWIN_KEY_LEFT_BRACKET;
    _rblxwin.ns.keycodes[0x1B] = RBLXWIN_KEY_MINUS;
    _rblxwin.ns.keycodes[0x2F] = RBLXWIN_KEY_PERIOD;
    _rblxwin.ns.keycodes[0x1E] = RBLXWIN_KEY_RIGHT_BRACKET;
    _rblxwin.ns.keycodes[0x29] = RBLXWIN_KEY_SEMICOLON;
    _rblxwin.ns.keycodes[0x2C] = RBLXWIN_KEY_SLASH;
    _rblxwin.ns.keycodes[0x0A] = RBLXWIN_KEY_WORLD_1;

    _rblxwin.ns.keycodes[0x33] = RBLXWIN_KEY_BACKSPACE;
    _rblxwin.ns.keycodes[0x39] = RBLXWIN_KEY_CAPS_LOCK;
    _rblxwin.ns.keycodes[0x75] = RBLXWIN_KEY_DELETE;
    _rblxwin.ns.keycodes[0x7D] = RBLXWIN_KEY_DOWN;
    _rblxwin.ns.keycodes[0x77] = RBLXWIN_KEY_END;
    _rblxwin.ns.keycodes[0x24] = RBLXWIN_KEY_ENTER;
    _rblxwin.ns.keycodes[0x35] = RBLXWIN_KEY_ESCAPE;
    _rblxwin.ns.keycodes[0x7A] = RBLXWIN_KEY_F1;
    _rblxwin.ns.keycodes[0x78] = RBLXWIN_KEY_F2;
    _rblxwin.ns.keycodes[0x63] = RBLXWIN_KEY_F3;
    _rblxwin.ns.keycodes[0x76] = RBLXWIN_KEY_F4;
    _rblxwin.ns.keycodes[0x60] = RBLXWIN_KEY_F5;
    _rblxwin.ns.keycodes[0x61] = RBLXWIN_KEY_F6;
    _rblxwin.ns.keycodes[0x62] = RBLXWIN_KEY_F7;
    _rblxwin.ns.keycodes[0x64] = RBLXWIN_KEY_F8;
    _rblxwin.ns.keycodes[0x65] = RBLXWIN_KEY_F9;
    _rblxwin.ns.keycodes[0x6D] = RBLXWIN_KEY_F10;
    _rblxwin.ns.keycodes[0x67] = RBLXWIN_KEY_F11;
    _rblxwin.ns.keycodes[0x6F] = RBLXWIN_KEY_F12;
    _rblxwin.ns.keycodes[0x69] = RBLXWIN_KEY_F13;
    _rblxwin.ns.keycodes[0x6B] = RBLXWIN_KEY_F14;
    _rblxwin.ns.keycodes[0x71] = RBLXWIN_KEY_F15;
    _rblxwin.ns.keycodes[0x6A] = RBLXWIN_KEY_F16;
    _rblxwin.ns.keycodes[0x40] = RBLXWIN_KEY_F17;
    _rblxwin.ns.keycodes[0x4F] = RBLXWIN_KEY_F18;
    _rblxwin.ns.keycodes[0x50] = RBLXWIN_KEY_F19;
    _rblxwin.ns.keycodes[0x5A] = RBLXWIN_KEY_F20;
    _rblxwin.ns.keycodes[0x73] = RBLXWIN_KEY_HOME;
    _rblxwin.ns.keycodes[0x72] = RBLXWIN_KEY_INSERT;
    _rblxwin.ns.keycodes[0x7B] = RBLXWIN_KEY_LEFT;
    _rblxwin.ns.keycodes[0x3A] = RBLXWIN_KEY_LEFT_ALT;
    _rblxwin.ns.keycodes[0x3B] = RBLXWIN_KEY_LEFT_CONTROL;
    _rblxwin.ns.keycodes[0x38] = RBLXWIN_KEY_LEFT_SHIFT;
    _rblxwin.ns.keycodes[0x37] = RBLXWIN_KEY_LEFT_SUPER;
    _rblxwin.ns.keycodes[0x6E] = RBLXWIN_KEY_MENU;
    _rblxwin.ns.keycodes[0x47] = RBLXWIN_KEY_NUM_LOCK;
    _rblxwin.ns.keycodes[0x79] = RBLXWIN_KEY_PAGE_DOWN;
    _rblxwin.ns.keycodes[0x74] = RBLXWIN_KEY_PAGE_UP;
    _rblxwin.ns.keycodes[0x7C] = RBLXWIN_KEY_RIGHT;
    _rblxwin.ns.keycodes[0x3D] = RBLXWIN_KEY_RIGHT_ALT;
    _rblxwin.ns.keycodes[0x3E] = RBLXWIN_KEY_RIGHT_CONTROL;
    _rblxwin.ns.keycodes[0x3C] = RBLXWIN_KEY_RIGHT_SHIFT;
    _rblxwin.ns.keycodes[0x36] = RBLXWIN_KEY_RIGHT_SUPER;
    _rblxwin.ns.keycodes[0x31] = RBLXWIN_KEY_SPACE;
    _rblxwin.ns.keycodes[0x30] = RBLXWIN_KEY_TAB;
    _rblxwin.ns.keycodes[0x7E] = RBLXWIN_KEY_UP;

    _rblxwin.ns.keycodes[0x52] = RBLXWIN_KEY_KP_0;
    _rblxwin.ns.keycodes[0x53] = RBLXWIN_KEY_KP_1;
    _rblxwin.ns.keycodes[0x54] = RBLXWIN_KEY_KP_2;
    _rblxwin.ns.keycodes[0x55] = RBLXWIN_KEY_KP_3;
    _rblxwin.ns.keycodes[0x56] = RBLXWIN_KEY_KP_4;
    _rblxwin.ns.keycodes[0x57] = RBLXWIN_KEY_KP_5;
    _rblxwin.ns.keycodes[0x58] = RBLXWIN_KEY_KP_6;
    _rblxwin.ns.keycodes[0x59] = RBLXWIN_KEY_KP_7;
    _rblxwin.ns.keycodes[0x5B] = RBLXWIN_KEY_KP_8;
    _rblxwin.ns.keycodes[0x5C] = RBLXWIN_KEY_KP_9;
    _rblxwin.ns.keycodes[0x45] = RBLXWIN_KEY_KP_ADD;
    _rblxwin.ns.keycodes[0x41] = RBLXWIN_KEY_KP_DECIMAL;
    _rblxwin.ns.keycodes[0x4B] = RBLXWIN_KEY_KP_DIVIDE;
    _rblxwin.ns.keycodes[0x4C] = RBLXWIN_KEY_KP_ENTER;
    _rblxwin.ns.keycodes[0x51] = RBLXWIN_KEY_KP_EQUAL;
    _rblxwin.ns.keycodes[0x43] = RBLXWIN_KEY_KP_MULTIPLY;
    _rblxwin.ns.keycodes[0x4E] = RBLXWIN_KEY_KP_SUBTRACT;

    for (scancode = 0;  scancode < 256;  scancode++)
    {
        // Store the reverse translation for faster key name lookup
        if (_rblxwin.ns.keycodes[scancode] >= 0)
            _rblxwin.ns.scancodes[_rblxwin.ns.keycodes[scancode]] = scancode;
    }
}

// Retrieve Unicode data for the current keyboard layout
//
static RBLXWINbool updateUnicodeDataNS(void)
{
    if (_rblxwin.ns.inputSource)
    {
        CFRelease(_rblxwin.ns.inputSource);
        _rblxwin.ns.inputSource = NULL;
        _rblxwin.ns.unicodeData = nil;
    }

    _rblxwin.ns.inputSource = TISCopyCurrentKeyboardLayoutInputSource();
    if (!_rblxwin.ns.inputSource)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve keyboard layout input source");
        return RBLXWIN_FALSE;
    }

    _rblxwin.ns.unicodeData =
        TISGetInputSourceProperty(_rblxwin.ns.inputSource,
                                  kTISPropertyUnicodeKeyLayoutData);
    if (!_rblxwin.ns.unicodeData)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve keyboard layout Unicode data");
        return RBLXWIN_FALSE;
    }

    return RBLXWIN_TRUE;
}

// Load HIToolbox.framework and the TIS symbols we need from it
//
static RBLXWINbool initializeTIS(void)
{
    // This works only because Cocoa has already loaded it properly
    _rblxwin.ns.tis.bundle =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.HIToolbox"));
    if (!_rblxwin.ns.tis.bundle)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to load HIToolbox.framework");
        return RBLXWIN_FALSE;
    }

    CFStringRef* kPropertyUnicodeKeyLayoutData =
        CFBundleGetDataPointerForName(_rblxwin.ns.tis.bundle,
                                      CFSTR("kTISPropertyUnicodeKeyLayoutData"));
    _rblxwin.ns.tis.CopyCurrentKeyboardLayoutInputSource =
        CFBundleGetFunctionPointerForName(_rblxwin.ns.tis.bundle,
                                          CFSTR("TISCopyCurrentKeyboardLayoutInputSource"));
    _rblxwin.ns.tis.GetInputSourceProperty =
        CFBundleGetFunctionPointerForName(_rblxwin.ns.tis.bundle,
                                          CFSTR("TISGetInputSourceProperty"));
    _rblxwin.ns.tis.GetKbdType =
        CFBundleGetFunctionPointerForName(_rblxwin.ns.tis.bundle,
                                          CFSTR("LMGetKbdType"));

    if (!kPropertyUnicodeKeyLayoutData ||
        !TISCopyCurrentKeyboardLayoutInputSource ||
        !TISGetInputSourceProperty ||
        !LMGetKbdType)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to load TIS API symbols");
        return RBLXWIN_FALSE;
    }

    _rblxwin.ns.tis.kPropertyUnicodeKeyLayoutData =
        *kPropertyUnicodeKeyLayoutData;

    return updateUnicodeDataNS();
}

@interface RBLXWINHelper : NSObject
@end

@implementation RBLXWINHelper

- (void)selectedKeyboardInputSourceChanged:(NSObject* )object
{
    updateUnicodeDataNS();
}

- (void)doNothing:(id)object
{
}

@end // RBLXWINHelper

@interface RBLXWINApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation RBLXWINApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    _RBLXWINwindow* window;

    for (window = _rblxwin.windowListHead;  window;  window = window->next)
        _rblxwinInputWindowCloseRequest(window);

    return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
    _RBLXWINwindow* window;

    for (window = _rblxwin.windowListHead;  window;  window = window->next)
    {
        if (window->context.client != RBLXWIN_NO_API)
            [window->context.nsgl.object update];
    }

    _rblxwinPollMonitorsNS();
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    if (_rblxwin.hints.init.ns.menubar)
    {
        // Menu bar setup must go between sharedApplication and finishLaunching
        // in order to properly emulate the behavior of NSApplicationMain

        if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"])
        {
            [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
                                          owner:NSApp
                                topLevelObjects:&_rblxwin.ns.nibObjects];
        }
        else
            createMenuBar();
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    _rblxwin.ns.finishedLaunching = RBLXWIN_TRUE;
    _rblxwinPlatformPostEmptyEvent();

    // In case we are unbundled, make us a proper UI application
    if (_rblxwin.hints.init.ns.menubar)
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    [NSApp stop:nil];
}

- (void)applicationDidHide:(NSNotification *)notification
{
    int i;

    for (i = 0;  i < _rblxwin.monitorCount;  i++)
        _rblxwinRestoreVideoModeNS(_rblxwin.monitors[i]);
}

@end // RBLXWINApplicationDelegate


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

void* _rblxwinLoadLocalVulkanLoaderNS(void)
{
    CFBundleRef bundle = CFBundleGetMainBundle();
    if (!bundle)
        return NULL;

    CFURLRef frameworksUrl = CFBundleCopyPrivateFrameworksURL(bundle);
    if (!frameworksUrl)
        return NULL;

    CFURLRef loaderUrl = CFURLCreateCopyAppendingPathComponent(
        kCFAllocatorDefault, frameworksUrl, CFSTR("libvulkan.1.dylib"), false);
    if (!loaderUrl)
    {
        CFRelease(frameworksUrl);
        return NULL;
    }

    char path[PATH_MAX];
    void* handle = NULL;

    if (CFURLGetFileSystemRepresentation(loaderUrl, true, (UInt8*) path, sizeof(path) - 1))
        handle = _rblxwin_dlopen(path);

    CFRelease(loaderUrl);
    CFRelease(frameworksUrl);
    return handle;
}


//////////////////////////////////////////////////////////////////////////
//////                       RBLXWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _rblxwinPlatformInit(void)
{
    @autoreleasepool {

    _rblxwin.ns.helper = [[RBLXWINHelper alloc] init];

    [NSThread detachNewThreadSelector:@selector(doNothing:)
                             toTarget:_rblxwin.ns.helper
                           withObject:nil];

    if (NSApp)
        _rblxwin.ns.finishedLaunching = RBLXWIN_TRUE;

    [NSApplication sharedApplication];

    _rblxwin.ns.delegate = [[RBLXWINApplicationDelegate alloc] init];
    if (_rblxwin.ns.delegate == nil)
    {
        _rblxwinInputError(RBLXWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to create application delegate");
        return RBLXWIN_FALSE;
    }

    [NSApp setDelegate:_rblxwin.ns.delegate];

    NSEvent* (^block)(NSEvent*) = ^ NSEvent* (NSEvent* event)
    {
        if ([event modifierFlags] & NSEventModifierFlagCommand)
            [[NSApp keyWindow] sendEvent:event];

        return event;
    };

    _rblxwin.ns.keyUpMonitor =
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp
                                              handler:block];

    if (_rblxwin.hints.init.ns.chdir)
        changeToResourcesDirectory();

    // Press and Hold prevents some keys from emitting repeated characters
    NSDictionary* defaults = @{@"ApplePressAndHoldEnabled":@NO};
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];

    [[NSNotificationCenter defaultCenter]
        addObserver:_rblxwin.ns.helper
           selector:@selector(selectedKeyboardInputSourceChanged:)
               name:NSTextInputContextKeyboardSelectionDidChangeNotification
             object:nil];

    createKeyTables();

    _rblxwin.ns.eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (!_rblxwin.ns.eventSource)
        return RBLXWIN_FALSE;

    CGEventSourceSetLocalEventsSuppressionInterval(_rblxwin.ns.eventSource, 0.0);

    if (!initializeTIS())
        return RBLXWIN_FALSE;

    _rblxwinInitTimerNS();
    _rblxwinInitJoysticksNS();

    _rblxwinPollMonitorsNS();
    return RBLXWIN_TRUE;

    } // autoreleasepool
}

void _rblxwinPlatformTerminate(void)
{
    @autoreleasepool {

    if (_rblxwin.ns.inputSource)
    {
        CFRelease(_rblxwin.ns.inputSource);
        _rblxwin.ns.inputSource = NULL;
        _rblxwin.ns.unicodeData = nil;
    }

    if (_rblxwin.ns.eventSource)
    {
        CFRelease(_rblxwin.ns.eventSource);
        _rblxwin.ns.eventSource = NULL;
    }

    if (_rblxwin.ns.delegate)
    {
        [NSApp setDelegate:nil];
        [_rblxwin.ns.delegate release];
        _rblxwin.ns.delegate = nil;
    }

    if (_rblxwin.ns.helper)
    {
        [[NSNotificationCenter defaultCenter]
            removeObserver:_rblxwin.ns.helper
                      name:NSTextInputContextKeyboardSelectionDidChangeNotification
                    object:nil];
        [[NSNotificationCenter defaultCenter]
            removeObserver:_rblxwin.ns.helper];
        [_rblxwin.ns.helper release];
        _rblxwin.ns.helper = nil;
    }

    if (_rblxwin.ns.keyUpMonitor)
        [NSEvent removeMonitor:_rblxwin.ns.keyUpMonitor];

    free(_rblxwin.ns.clipboardString);

    _rblxwinTerminateNSGL();
    _rblxwinTerminateEGL();
    _rblxwinTerminateOSMesa();
    _rblxwinTerminateJoysticksNS();

    } // autoreleasepool
}

const char* _rblxwinPlatformGetVersionString(void)
{
    return _RBLXWIN_VERSION_NUMBER " Cocoa NSGL EGL OSMesa"
#if defined(_RBLXWIN_BUILD_DLL)
        " dynamic"
#endif
        ;
}

