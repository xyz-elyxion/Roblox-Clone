
HELPER FILES FOR POPULAR DEBUGGERS

rblxui.gdb
    GDB: disable stepping into trivial functions.
    (read comments inside file for details)

rblxui.natstepfilter
    Visual Studio Debugger: disable stepping into trivial functions.
    (read comments inside file for details)

rblxui.natvis
    Visual Studio Debugger: describe Dear RblxUI types for better display.
    With this, types like RblxVector<> will be displayed nicely in the debugger.
    (read comments inside file for details)

rblxui_lldb.py
    LLDB-based debuggers (*): synthetic children provider and summaries for Dear RblxUI types.
    With this, types like RblxVector<> will be displayed nicely in the debugger.
    (read comments inside file for details)
    (*) Xcode, Android Studio, may be used from VS Code, C++Builder, CLion, Eclipse etc.
