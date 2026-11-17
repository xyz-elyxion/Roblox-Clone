//========================================================================
// LINEARWIN 3.3 macOS - www.linearwin.org
//------------------------------------------------------------------------
// Copyright (c) 2009-2019 Camilla Löwy <elmindreda@linearwin.org>
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
            appName = @"LINEARWIN Application";
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

    memset(_linearwin.ns.keycodes, -1, sizeof(_linearwin.ns.keycodes));
    memset(_linearwin.ns.scancodes, -1, sizeof(_linearwin.ns.scancodes));

    _linearwin.ns.keycodes[0x1D] = LINEARWIN_KEY_0;
    _linearwin.ns.keycodes[0x12] = LINEARWIN_KEY_1;
    _linearwin.ns.keycodes[0x13] = LINEARWIN_KEY_2;
    _linearwin.ns.keycodes[0x14] = LINEARWIN_KEY_3;
    _linearwin.ns.keycodes[0x15] = LINEARWIN_KEY_4;
    _linearwin.ns.keycodes[0x17] = LINEARWIN_KEY_5;
    _linearwin.ns.keycodes[0x16] = LINEARWIN_KEY_6;
    _linearwin.ns.keycodes[0x1A] = LINEARWIN_KEY_7;
    _linearwin.ns.keycodes[0x1C] = LINEARWIN_KEY_8;
    _linearwin.ns.keycodes[0x19] = LINEARWIN_KEY_9;
    _linearwin.ns.keycodes[0x00] = LINEARWIN_KEY_A;
    _linearwin.ns.keycodes[0x0B] = LINEARWIN_KEY_B;
    _linearwin.ns.keycodes[0x08] = LINEARWIN_KEY_C;
    _linearwin.ns.keycodes[0x02] = LINEARWIN_KEY_D;
    _linearwin.ns.keycodes[0x0E] = LINEARWIN_KEY_E;
    _linearwin.ns.keycodes[0x03] = LINEARWIN_KEY_F;
    _linearwin.ns.keycodes[0x05] = LINEARWIN_KEY_G;
    _linearwin.ns.keycodes[0x04] = LINEARWIN_KEY_H;
    _linearwin.ns.keycodes[0x22] = LINEARWIN_KEY_I;
    _linearwin.ns.keycodes[0x26] = LINEARWIN_KEY_J;
    _linearwin.ns.keycodes[0x28] = LINEARWIN_KEY_K;
    _linearwin.ns.keycodes[0x25] = LINEARWIN_KEY_L;
    _linearwin.ns.keycodes[0x2E] = LINEARWIN_KEY_M;
    _linearwin.ns.keycodes[0x2D] = LINEARWIN_KEY_N;
    _linearwin.ns.keycodes[0x1F] = LINEARWIN_KEY_O;
    _linearwin.ns.keycodes[0x23] = LINEARWIN_KEY_P;
    _linearwin.ns.keycodes[0x0C] = LINEARWIN_KEY_Q;
    _linearwin.ns.keycodes[0x0F] = LINEARWIN_KEY_R;
    _linearwin.ns.keycodes[0x01] = LINEARWIN_KEY_S;
    _linearwin.ns.keycodes[0x11] = LINEARWIN_KEY_T;
    _linearwin.ns.keycodes[0x20] = LINEARWIN_KEY_U;
    _linearwin.ns.keycodes[0x09] = LINEARWIN_KEY_V;
    _linearwin.ns.keycodes[0x0D] = LINEARWIN_KEY_W;
    _linearwin.ns.keycodes[0x07] = LINEARWIN_KEY_X;
    _linearwin.ns.keycodes[0x10] = LINEARWIN_KEY_Y;
    _linearwin.ns.keycodes[0x06] = LINEARWIN_KEY_Z;

    _linearwin.ns.keycodes[0x27] = LINEARWIN_KEY_APOSTROPHE;
    _linearwin.ns.keycodes[0x2A] = LINEARWIN_KEY_BACKSLASH;
    _linearwin.ns.keycodes[0x2B] = LINEARWIN_KEY_COMMA;
    _linearwin.ns.keycodes[0x18] = LINEARWIN_KEY_EQUAL;
    _linearwin.ns.keycodes[0x32] = LINEARWIN_KEY_GRAVE_ACCENT;
    _linearwin.ns.keycodes[0x21] = LINEARWIN_KEY_LEFT_BRACKET;
    _linearwin.ns.keycodes[0x1B] = LINEARWIN_KEY_MINUS;
    _linearwin.ns.keycodes[0x2F] = LINEARWIN_KEY_PERIOD;
    _linearwin.ns.keycodes[0x1E] = LINEARWIN_KEY_RIGHT_BRACKET;
    _linearwin.ns.keycodes[0x29] = LINEARWIN_KEY_SEMICOLON;
    _linearwin.ns.keycodes[0x2C] = LINEARWIN_KEY_SLASH;
    _linearwin.ns.keycodes[0x0A] = LINEARWIN_KEY_WORLD_1;

    _linearwin.ns.keycodes[0x33] = LINEARWIN_KEY_BACKSPACE;
    _linearwin.ns.keycodes[0x39] = LINEARWIN_KEY_CAPS_LOCK;
    _linearwin.ns.keycodes[0x75] = LINEARWIN_KEY_DELETE;
    _linearwin.ns.keycodes[0x7D] = LINEARWIN_KEY_DOWN;
    _linearwin.ns.keycodes[0x77] = LINEARWIN_KEY_END;
    _linearwin.ns.keycodes[0x24] = LINEARWIN_KEY_ENTER;
    _linearwin.ns.keycodes[0x35] = LINEARWIN_KEY_ESCAPE;
    _linearwin.ns.keycodes[0x7A] = LINEARWIN_KEY_F1;
    _linearwin.ns.keycodes[0x78] = LINEARWIN_KEY_F2;
    _linearwin.ns.keycodes[0x63] = LINEARWIN_KEY_F3;
    _linearwin.ns.keycodes[0x76] = LINEARWIN_KEY_F4;
    _linearwin.ns.keycodes[0x60] = LINEARWIN_KEY_F5;
    _linearwin.ns.keycodes[0x61] = LINEARWIN_KEY_F6;
    _linearwin.ns.keycodes[0x62] = LINEARWIN_KEY_F7;
    _linearwin.ns.keycodes[0x64] = LINEARWIN_KEY_F8;
    _linearwin.ns.keycodes[0x65] = LINEARWIN_KEY_F9;
    _linearwin.ns.keycodes[0x6D] = LINEARWIN_KEY_F10;
    _linearwin.ns.keycodes[0x67] = LINEARWIN_KEY_F11;
    _linearwin.ns.keycodes[0x6F] = LINEARWIN_KEY_F12;
    _linearwin.ns.keycodes[0x69] = LINEARWIN_KEY_F13;
    _linearwin.ns.keycodes[0x6B] = LINEARWIN_KEY_F14;
    _linearwin.ns.keycodes[0x71] = LINEARWIN_KEY_F15;
    _linearwin.ns.keycodes[0x6A] = LINEARWIN_KEY_F16;
    _linearwin.ns.keycodes[0x40] = LINEARWIN_KEY_F17;
    _linearwin.ns.keycodes[0x4F] = LINEARWIN_KEY_F18;
    _linearwin.ns.keycodes[0x50] = LINEARWIN_KEY_F19;
    _linearwin.ns.keycodes[0x5A] = LINEARWIN_KEY_F20;
    _linearwin.ns.keycodes[0x73] = LINEARWIN_KEY_HOME;
    _linearwin.ns.keycodes[0x72] = LINEARWIN_KEY_INSERT;
    _linearwin.ns.keycodes[0x7B] = LINEARWIN_KEY_LEFT;
    _linearwin.ns.keycodes[0x3A] = LINEARWIN_KEY_LEFT_ALT;
    _linearwin.ns.keycodes[0x3B] = LINEARWIN_KEY_LEFT_CONTROL;
    _linearwin.ns.keycodes[0x38] = LINEARWIN_KEY_LEFT_SHIFT;
    _linearwin.ns.keycodes[0x37] = LINEARWIN_KEY_LEFT_SUPER;
    _linearwin.ns.keycodes[0x6E] = LINEARWIN_KEY_MENU;
    _linearwin.ns.keycodes[0x47] = LINEARWIN_KEY_NUM_LOCK;
    _linearwin.ns.keycodes[0x79] = LINEARWIN_KEY_PAGE_DOWN;
    _linearwin.ns.keycodes[0x74] = LINEARWIN_KEY_PAGE_UP;
    _linearwin.ns.keycodes[0x7C] = LINEARWIN_KEY_RIGHT;
    _linearwin.ns.keycodes[0x3D] = LINEARWIN_KEY_RIGHT_ALT;
    _linearwin.ns.keycodes[0x3E] = LINEARWIN_KEY_RIGHT_CONTROL;
    _linearwin.ns.keycodes[0x3C] = LINEARWIN_KEY_RIGHT_SHIFT;
    _linearwin.ns.keycodes[0x36] = LINEARWIN_KEY_RIGHT_SUPER;
    _linearwin.ns.keycodes[0x31] = LINEARWIN_KEY_SPACE;
    _linearwin.ns.keycodes[0x30] = LINEARWIN_KEY_TAB;
    _linearwin.ns.keycodes[0x7E] = LINEARWIN_KEY_UP;

    _linearwin.ns.keycodes[0x52] = LINEARWIN_KEY_KP_0;
    _linearwin.ns.keycodes[0x53] = LINEARWIN_KEY_KP_1;
    _linearwin.ns.keycodes[0x54] = LINEARWIN_KEY_KP_2;
    _linearwin.ns.keycodes[0x55] = LINEARWIN_KEY_KP_3;
    _linearwin.ns.keycodes[0x56] = LINEARWIN_KEY_KP_4;
    _linearwin.ns.keycodes[0x57] = LINEARWIN_KEY_KP_5;
    _linearwin.ns.keycodes[0x58] = LINEARWIN_KEY_KP_6;
    _linearwin.ns.keycodes[0x59] = LINEARWIN_KEY_KP_7;
    _linearwin.ns.keycodes[0x5B] = LINEARWIN_KEY_KP_8;
    _linearwin.ns.keycodes[0x5C] = LINEARWIN_KEY_KP_9;
    _linearwin.ns.keycodes[0x45] = LINEARWIN_KEY_KP_ADD;
    _linearwin.ns.keycodes[0x41] = LINEARWIN_KEY_KP_DECIMAL;
    _linearwin.ns.keycodes[0x4B] = LINEARWIN_KEY_KP_DIVIDE;
    _linearwin.ns.keycodes[0x4C] = LINEARWIN_KEY_KP_ENTER;
    _linearwin.ns.keycodes[0x51] = LINEARWIN_KEY_KP_EQUAL;
    _linearwin.ns.keycodes[0x43] = LINEARWIN_KEY_KP_MULTIPLY;
    _linearwin.ns.keycodes[0x4E] = LINEARWIN_KEY_KP_SUBTRACT;

    for (scancode = 0;  scancode < 256;  scancode++)
    {
        // Store the reverse translation for faster key name lookup
        if (_linearwin.ns.keycodes[scancode] >= 0)
            _linearwin.ns.scancodes[_linearwin.ns.keycodes[scancode]] = scancode;
    }
}

// Retrieve Unicode data for the current keyboard layout
//
static LINEARWINbool updateUnicodeDataNS(void)
{
    if (_linearwin.ns.inputSource)
    {
        CFRelease(_linearwin.ns.inputSource);
        _linearwin.ns.inputSource = NULL;
        _linearwin.ns.unicodeData = nil;
    }

    _linearwin.ns.inputSource = TISCopyCurrentKeyboardLayoutInputSource();
    if (!_linearwin.ns.inputSource)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve keyboard layout input source");
        return LINEARWIN_FALSE;
    }

    _linearwin.ns.unicodeData =
        TISGetInputSourceProperty(_linearwin.ns.inputSource,
                                  kTISPropertyUnicodeKeyLayoutData);
    if (!_linearwin.ns.unicodeData)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to retrieve keyboard layout Unicode data");
        return LINEARWIN_FALSE;
    }

    return LINEARWIN_TRUE;
}

// Load HIToolbox.framework and the TIS symbols we need from it
//
static LINEARWINbool initializeTIS(void)
{
    // This works only because Cocoa has already loaded it properly
    _linearwin.ns.tis.bundle =
        CFBundleGetBundleWithIdentifier(CFSTR("com.apple.HIToolbox"));
    if (!_linearwin.ns.tis.bundle)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to load HIToolbox.framework");
        return LINEARWIN_FALSE;
    }

    CFStringRef* kPropertyUnicodeKeyLayoutData =
        CFBundleGetDataPointerForName(_linearwin.ns.tis.bundle,
                                      CFSTR("kTISPropertyUnicodeKeyLayoutData"));
    _linearwin.ns.tis.CopyCurrentKeyboardLayoutInputSource =
        CFBundleGetFunctionPointerForName(_linearwin.ns.tis.bundle,
                                          CFSTR("TISCopyCurrentKeyboardLayoutInputSource"));
    _linearwin.ns.tis.GetInputSourceProperty =
        CFBundleGetFunctionPointerForName(_linearwin.ns.tis.bundle,
                                          CFSTR("TISGetInputSourceProperty"));
    _linearwin.ns.tis.GetKbdType =
        CFBundleGetFunctionPointerForName(_linearwin.ns.tis.bundle,
                                          CFSTR("LMGetKbdType"));

    if (!kPropertyUnicodeKeyLayoutData ||
        !TISCopyCurrentKeyboardLayoutInputSource ||
        !TISGetInputSourceProperty ||
        !LMGetKbdType)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to load TIS API symbols");
        return LINEARWIN_FALSE;
    }

    _linearwin.ns.tis.kPropertyUnicodeKeyLayoutData =
        *kPropertyUnicodeKeyLayoutData;

    return updateUnicodeDataNS();
}

@interface LINEARWINHelper : NSObject
@end

@implementation LINEARWINHelper

- (void)selectedKeyboardInputSourceChanged:(NSObject* )object
{
    updateUnicodeDataNS();
}

- (void)doNothing:(id)object
{
}

@end // LINEARWINHelper

@interface LINEARWINApplicationDelegate : NSObject <NSApplicationDelegate>
@end

@implementation LINEARWINApplicationDelegate

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    _LINEARWINwindow* window;

    for (window = _linearwin.windowListHead;  window;  window = window->next)
        _linearwinInputWindowCloseRequest(window);

    return NSTerminateCancel;
}

- (void)applicationDidChangeScreenParameters:(NSNotification *) notification
{
    _LINEARWINwindow* window;

    for (window = _linearwin.windowListHead;  window;  window = window->next)
    {
        if (window->context.client != LINEARWIN_NO_API)
            [window->context.nsgl.object update];
    }

    _linearwinPollMonitorsNS();
}

- (void)applicationWillFinishLaunching:(NSNotification *)notification
{
    if (_linearwin.hints.init.ns.menubar)
    {
        // Menu bar setup must go between sharedApplication and finishLaunching
        // in order to properly emulate the behavior of NSApplicationMain

        if ([[NSBundle mainBundle] pathForResource:@"MainMenu" ofType:@"nib"])
        {
            [[NSBundle mainBundle] loadNibNamed:@"MainMenu"
                                          owner:NSApp
                                topLevelObjects:&_linearwin.ns.nibObjects];
        }
        else
            createMenuBar();
    }
}

- (void)applicationDidFinishLaunching:(NSNotification *)notification
{
    _linearwin.ns.finishedLaunching = LINEARWIN_TRUE;
    _linearwinPlatformPostEmptyEvent();

    // In case we are unbundled, make us a proper UI application
    if (_linearwin.hints.init.ns.menubar)
        [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];

    [NSApp stop:nil];
}

- (void)applicationDidHide:(NSNotification *)notification
{
    int i;

    for (i = 0;  i < _linearwin.monitorCount;  i++)
        _linearwinRestoreVideoModeNS(_linearwin.monitors[i]);
}

@end // LINEARWINApplicationDelegate


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN internal API                      //////
//////////////////////////////////////////////////////////////////////////

void* _linearwinLoadLocalVulkanLoaderNS(void)
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
        handle = _linearwin_dlopen(path);

    CFRelease(loaderUrl);
    CFRelease(frameworksUrl);
    return handle;
}


//////////////////////////////////////////////////////////////////////////
//////                       LINEARWIN platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _linearwinPlatformInit(void)
{
    @autoreleasepool {

    _linearwin.ns.helper = [[LINEARWINHelper alloc] init];

    [NSThread detachNewThreadSelector:@selector(doNothing:)
                             toTarget:_linearwin.ns.helper
                           withObject:nil];

    if (NSApp)
        _linearwin.ns.finishedLaunching = LINEARWIN_TRUE;

    [NSApplication sharedApplication];

    _linearwin.ns.delegate = [[LINEARWINApplicationDelegate alloc] init];
    if (_linearwin.ns.delegate == nil)
    {
        _linearwinInputError(LINEARWIN_PLATFORM_ERROR,
                        "Cocoa: Failed to create application delegate");
        return LINEARWIN_FALSE;
    }

    [NSApp setDelegate:_linearwin.ns.delegate];

    NSEvent* (^block)(NSEvent*) = ^ NSEvent* (NSEvent* event)
    {
        if ([event modifierFlags] & NSEventModifierFlagCommand)
            [[NSApp keyWindow] sendEvent:event];

        return event;
    };

    _linearwin.ns.keyUpMonitor =
        [NSEvent addLocalMonitorForEventsMatchingMask:NSEventMaskKeyUp
                                              handler:block];

    if (_linearwin.hints.init.ns.chdir)
        changeToResourcesDirectory();

    // Press and Hold prevents some keys from emitting repeated characters
    NSDictionary* defaults = @{@"ApplePressAndHoldEnabled":@NO};
    [[NSUserDefaults standardUserDefaults] registerDefaults:defaults];

    [[NSNotificationCenter defaultCenter]
        addObserver:_linearwin.ns.helper
           selector:@selector(selectedKeyboardInputSourceChanged:)
               name:NSTextInputContextKeyboardSelectionDidChangeNotification
             object:nil];

    createKeyTables();

    _linearwin.ns.eventSource = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    if (!_linearwin.ns.eventSource)
        return LINEARWIN_FALSE;

    CGEventSourceSetLocalEventsSuppressionInterval(_linearwin.ns.eventSource, 0.0);

    if (!initializeTIS())
        return LINEARWIN_FALSE;

    _linearwinInitTimerNS();
    _linearwinInitJoysticksNS();

    _linearwinPollMonitorsNS();
    return LINEARWIN_TRUE;

    } // autoreleasepool
}

void _linearwinPlatformTerminate(void)
{
    @autoreleasepool {

    if (_linearwin.ns.inputSource)
    {
        CFRelease(_linearwin.ns.inputSource);
        _linearwin.ns.inputSource = NULL;
        _linearwin.ns.unicodeData = nil;
    }

    if (_linearwin.ns.eventSource)
    {
        CFRelease(_linearwin.ns.eventSource);
        _linearwin.ns.eventSource = NULL;
    }

    if (_linearwin.ns.delegate)
    {
        [NSApp setDelegate:nil];
        [_linearwin.ns.delegate release];
        _linearwin.ns.delegate = nil;
    }

    if (_linearwin.ns.helper)
    {
        [[NSNotificationCenter defaultCenter]
            removeObserver:_linearwin.ns.helper
                      name:NSTextInputContextKeyboardSelectionDidChangeNotification
                    object:nil];
        [[NSNotificationCenter defaultCenter]
            removeObserver:_linearwin.ns.helper];
        [_linearwin.ns.helper release];
        _linearwin.ns.helper = nil;
    }

    if (_linearwin.ns.keyUpMonitor)
        [NSEvent removeMonitor:_linearwin.ns.keyUpMonitor];

    free(_linearwin.ns.clipboardString);

    _linearwinTerminateNSGL();
    _linearwinTerminateEGL();
    _linearwinTerminateOSMesa();
    _linearwinTerminateJoysticksNS();

    } // autoreleasepool
}

const char* _linearwinPlatformGetVersionString(void)
{
    return _LINEARWIN_VERSION_NUMBER " Cocoa NSGL EGL OSMesa"
#if defined(_LINEARWIN_BUILD_DLL)
        " dynamic"
#endif
        ;
}

