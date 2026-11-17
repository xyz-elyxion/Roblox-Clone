
HELPER FILES FOR POPULAR DEBUGGERS

linearui.gdb
    GDB: disable stepping into trivial functions.
    (read comments inside file for details)

linearui.natstepfilter
    Visual Studio Debugger: disable stepping into trivial functions.
    (read comments inside file for details)

linearui.natvis
    Visual Studio Debugger: describe Dear LinearUI types for better display.
    With this, types like LinearVector<> will be displayed nicely in the debugger.
    (read comments inside file for details)

linearui_lldb.py
    LLDB-based debuggers (*): synthetic children provider and summaries for Dear LinearUI types.
    With this, types like LinearVector<> will be displayed nicely in the debugger.
    (read comments inside file for details)
    (*) Xcode, Android Studio, may be used from VS Code, C++Builder, CLion, Eclipse etc.
