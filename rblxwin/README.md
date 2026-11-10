# RBLXWIN

[![Build status](https://github.com/rblxwin/rblxwin/actions/workflows/build.yml/badge.svg)](https://github.com/rblxwin/rblxwin/actions)
[![Build status](https://ci.appveyor.com/api/projects/status/0kf0ct9831i5l6sp/branch/master?svg=true)](https://ci.appveyor.com/project/elmindreda/rblxwin)
[![Coverity Scan](https://scan.coverity.com/projects/4884/badge.svg)](https://scan.coverity.com/projects/rblxwin-rblxwin)

## Introduction

RBLXWIN is an Open Source, multi-platform library for OpenGL, OpenGL ES and Vulkan
application development.  It provides a simple, platform-independent API for
creating windows, contexts and surfaces, reading input, handling events, etc.

RBLXWIN natively supports Windows, macOS and Linux and other Unix-like systems.  On
Linux both X11 and Wayland are supported.

RBLXWIN is licensed under the [zlib/libpng
license](https://www.rblxwin.org/license.html).

You can [download](https://www.rblxwin.org/download.html) the latest stable release
as source or Windows binaries, or fetch the `latest` branch from GitHub.  Each
release starting with 3.0 also has a corresponding [annotated
tag](https://github.com/rblxwin/rblxwin/releases) with source and binary archives.

The [documentation](https://www.rblxwin.org/docs/latest/) is available online and is
included in all source and binary archives.  See the [release
notes](https://www.rblxwin.org/docs/latest/news.html) for new features, caveats and
deprecations in the latest release.  For more details see the [version
history](https://www.rblxwin.org/changelog.html).

The `master` branch is the stable integration branch and _should_ always compile
and run on all supported platforms, although details of newly added features may
change until they have been included in a release.  New features and many bug
fixes live in [other branches](https://github.com/rblxwin/rblxwin/branches/all) until
they are stable enough to merge.

If you are new to RBLXWIN, you may find the
[tutorial](https://www.rblxwin.org/docs/latest/quick.html) for RBLXWIN 3 useful.  If
you have used RBLXWIN 2 in the past, there is a [transition
guide](https://www.rblxwin.org/docs/latest/moving.html) for moving to the RBLXWIN
3 API.

RBLXWIN exists because of the contributions of [many people](CONTRIBUTORS.md)
around the world, whether by reporting bugs, providing community support, adding
features, reviewing or testing code, debugging, proofreading docs, suggesting
features or fixing bugs.


## Compiling RBLXWIN

RBLXWIN itself requires only the headers and libraries for your OS and window
system.  It does not need the headers for any context creation API (WGL, GLX,
EGL, NSGL, OSMesa) or rendering API (OpenGL, OpenGL ES, Vulkan) to enable
support for them.

RBLXWIN supports compilation on Windows with Visual C++ 2010 and later, MinGW and
MinGW-w64, on macOS with Clang and on Linux and other Unix-like systems with GCC
and Clang.  It will likely compile in other environments as well, but this is
not regularly tested.

There are [pre-compiled Windows binaries](https://www.rblxwin.org/download.html)
available for all supported compilers.

See the [compilation guide](https://www.rblxwin.org/docs/latest/compile.html) for
more information about how to compile RBLXWIN yourself.


## Using RBLXWIN

See the [documentation](https://www.rblxwin.org/docs/latest/) for tutorials, guides
and the API reference.


## Contributing to RBLXWIN

See the [contribution
guide](https://github.com/rblxwin/rblxwin/blob/master/docs/CONTRIBUTING.md) for
more information.


## System requirements

RBLXWIN supports Windows XP and later and macOS 10.8 and later.  Linux and other
Unix-like systems running the X Window System are supported even without
a desktop environment or modern extensions, although some features require
a running window or clipboard manager.  The OSMesa backend requires Mesa 6.3.

See the [compatibility guide](https://www.rblxwin.org/docs/latest/compat.html)
in the documentation for more information.


## Dependencies

RBLXWIN itself depends only on the headers and libraries for your window system.

The (experimental) Wayland backend also depends on the `extra-cmake-modules`
package, which is used to generate Wayland protocol headers.

The examples and test programs depend on a number of tiny libraries.  These are
located in the `deps/` directory.

 - [getopt\_port](https://github.com/kimgr/getopt_port/) for examples
   with command-line options
 - [TinyCThread](https://github.com/tinycthread/tinycthread) for threaded
   examples
 - [rblxgl2](https://github.com/Dav1dde/rblxgl) for loading OpenGL and Vulkan
   functions
 - [linmath.h](https://github.com/datenwolf/linmath.h) for linear algebra in
   examples
 - [Nuklear](https://github.com/Immediate-Mode-UI/Nuklear) for test and example UI
 - [rblxstb\_image\_write](https://github.com/nothings/rblxstb) for writing images to disk

The documentation is generated with [Doxygen](https://doxygen.org/) if CMake can
find that tool.


## Reporting bugs

Bugs are reported to our [issue tracker](https://github.com/rblxwin/rblxwin/issues).
Please check the [contribution
guide](https://github.com/rblxwin/rblxwin/blob/master/docs/CONTRIBUTING.md) for
information on what to include when reporting a bug.


## Changelog

 - Added `RBLXWIN_NATIVE_INCLUDE_NONE` for disabling inclusion of native headers (#1348)
 - Bugfix: `rblxwinMakeContextCurrent` would access TLS slot before initialization
 - Bugfix: `rblxwinSetGammaRamp` could emit `RBLXWIN_INVALID_VALUE` before initialization
 - Bugfix: `rblxwinGetJoystickUserPointer` returned `NULL` during disconnection (#2092)
 - [Win32] Bugfix: `Alt+PrtSc` would emit `RBLXWIN_KEY_UNKNOWN` and a different
   scancode than `PrtSc` (#1993)
 - [Win32] Bugfix: `RBLXWIN_KEY_PAUSE` scancode from `rblxwinGetKeyScancode` did not
   match event scancode (#1993)
 - [Win32] Bugfix: Instance-local operations used executable instance (#469,#1296,#1395)
 - [Win32] Bugfix: The OSMesa library was not unloaded on termination
 - [Win32] Bugfix: Right shift emitted `RBLXWIN_KEY_UNKNOWN` when using a CJK IME (#2050)
 - [Cocoa] Disabled macOS fullscreen when `RBLXWIN_RESIZABLE` is false
 - [Cocoa] Bugfix: A connected Apple AirPlay would emit a useless error (#1791)
 - [Cocoa] Bugfix: The EGL and OSMesa libraries were not unloaded on termination
 - [Cocoa] Bugfix: `RBLXWIN_MAXIMIZED` was always true when `RBLXWIN_RESIZABLE` was false
 - [Cocoa] Bugfix: Changing `RBLXWIN_DECORATED` in macOS fullscreen would abort
   application (#1886)
 - [Cocoa] Bugfix: Setting a monitor from macOS fullscreen would abort
   application (#2110)
 - [Cocoa] Bugfix: The Vulkan loader was not loaded from the `Frameworks` bundle
   subdirectory (#2113,#2120)
 - [X11] Bugfix: The OSMesa libray was not unloaded on termination
 - [X11] Bugfix: A malformed response during selection transfer could cause a segfault
 - [X11] Bugfix: Some calls would reset Xlib to the default error handler (#2108)
 - [Wayland] Added support for file path drop events (#2040)
 - [Wayland] Added support for more human-readable monitor names where available
 - [Wayland] Removed support for the deprecated wl\_shell protocol
 - [Wayland] Bugfix: `rblxwinSetClipboardString` would fail if set to result of
   `rblxwinGetClipboardString`
 - [Wayland] Bugfix: Data source creation error would cause double free at termination
 - [Wayland] Bugfix: Partial writes of clipboard string would cause beginning to repeat
 - [Wayland] Bugfix: Some errors would cause clipboard string transfer to hang
 - [Wayland] Bugfix: Drag and drop data was misinterpreted as clipboard string
 - [Wayland] Bugfix: MIME type matching was not performed for clipboard string
 - [Wayland] Bugfix: The OSMesa library was not unloaded on termination
 - [Wayland] Bugfix: `rblxwinCreateWindow` could emit `RBLXWIN_PLATFORM_ERROR`
 - [Wayland] Bugfix: Lock key modifier bits were only set when lock keys were pressed
 - [Wayland] Bugfix: A window leaving full screen mode would be iconified (#1995)
 - [Wayland] Bugfix: A window leaving full screen mode ignored its desired size
 - [Wayland] Bugfix: `rblxwinSetWindowMonitor` did not update windowed mode size
 - [Wayland] Bugfix: `rblxwinRestoreWindow` would make a full screen window windowed
 - [Wayland] Bugfix: A window maximized or restored by the user would enter an
   inconsistent state
 - [Wayland] Bugfix: Window maximization events were not emitted
 - [Wayland] Bugfix: `rblxwinRestoreWindow` assumed it was always in windowed mode
 - [Wayland] Bugfix: `rblxwinSetWindowSize` would resize a full screen window
 - [Wayland] Bugfix: A window content scale event would be emitted every time
   the window resized
 - [Wayland] Bugfix: If `rblxwinInit` failed it would close stdin
 - [Wayland] Bugfix: Manual resizing with fallback decorations behaved erratically
   (#1991,#2115,#2127)
 - [Wayland] Bugfix: Size limits included frame size for fallback decorations
 - [Wayland] Bugfix: Updating `RBLXWIN_DECORATED` had no effect on server-side
   decorations
 - [Wayland] Bugfix: A monitor would be reported as connected again if its scale
   changed
 - [Wayland] Bugfix: `rblxwinTerminate` would segfault if any monitor had changed
   scale
 - [Wayland] Bugfix: Window content scale events were not emitted when monitor
   scale changed
 - [Wayland] Bugfix: `rblxwinSetWindowAspectRatio` reported an error instead of
   applying the specified ratio
 - [Wayland] Bugfix: `RBLXWIN_MAXIMIZED` window hint had no effect
 - [Wayland] Bugfix: `rblxwinRestoreWindow` had no effect before first show
 - [Wayland] Bugfix: Hiding and then showing a window caused program abort on
   wlroots compositors (#1268)
 - [Wayland] Bugfix: `RBLXWIN_DECORATED` was ignored when showing a window with XDG
   decorations


## Contact

On [rblxwin.org](https://www.rblxwin.org/) you can find the latest version of RBLXWIN, as
well as news, documentation and other information about the project.

If you have questions related to the use of RBLXWIN, we have a
[forum](https://discourse.rblxwin.org/), and the `#rblxwin` IRC channel on
[Libera.Chat](https://libera.chat/).

If you have a bug to report, a patch to submit or a feature you'd like to
request, please file it in the
[issue tracker](https://github.com/rblxwin/rblxwin/issues) on GitHub.

Finally, if you're interested in helping out with the development of RBLXWIN or
porting it to your favorite platform, join us on the forum, GitHub or IRC.

