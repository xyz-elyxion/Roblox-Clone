# RBLXAntiCheat - vendored from https://github.com/AlSch092/UltimateAntiCheat
# and renamed. Licensed under the GNU AGPL v3; see LICENSE in this folder.
#
# Upstream ships two ways to build the same code: a standalone console program
# (main.cpp + SplashScreen.cpp) and a library flavour guarded by _LIB_ that
# exposes the PIMPL class in Core/AntiCheatLib.hpp. The game uses the library
# flavour and links it into Client, so the checks inspect the process they are
# meant to protect instead of running beside it - which is also what upstream
# recommends over shipping the anti-cheat as its own DLL.
#
# LibTest/ is a manual harness with its own main(), so it is not built here.

if(NOT WIN32)
    message(STATUS "RBLXAntiCheat is Windows-only; skipping the anti-cheat library.")
    return()
endif()

set(RBLX_ANTICHEAT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

# Core/AntiCheatLib.cpp is the _LIB_ flavour of Core/AntiCheat.cpp: only one of
# the two is ever compiled, and the _LIB_ guard in each picks the matching one.

add_library(RBLXAntiCheat STATIC
    AntiDebug/AntiDebugger.cpp
    AntiDebug/DebuggerDetections.cpp
    AntiTamper/Integrity.cpp
    AntiTamper/NAuthenticode.cpp
    AntiTamper/remap.cpp
    Common/EvidenceLocker.cpp
    Common/Logger.cpp
    Common/SHA256.cpp
    Common/Utility.cpp
    Core/AntiCheatLib.cpp
    Core/Detections.cpp
    Core/Preventions.cpp
    Environment/Services.cpp
    GameEvents/EventLogger.cpp
    Network/HttpClient.cpp
    Network/NetClient.cpp
    Network/Packets/Packets.cpp
    Process/Exports.cpp
    Process/Handles.cpp
    Process/PEB.cpp
    Process/Process.cpp
    Process/Thread.cpp
)

# Upstream targets C++14 while the game builds as C++17, so pin this target's
# standard rather than letting the project default change its meaning.
set_target_properties(RBLXAntiCheat PROPERTIES
    CXX_STANDARD 14
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_include_directories(RBLXAntiCheat PUBLIC "${RBLX_ANTICHEAT_DIR}")

target_compile_definitions(RBLXAntiCheat PRIVATE
    # Selects the library flavour of the core class (see Core/AntiCheat.hpp).
    _LIB_
    _WIN32_WINNT=0x0A00
    NOMINMAX
    _CRT_SECURE_NO_WARNINGS
    _WINSOCK_DEPRECATED_NO_WARNINGS
)

if(MSVC)
    target_compile_options(RBLXAntiCheat PRIVATE /W3)
endif()

# Prebuilt dependencies that ship with the vendored source: curl (used by the
# optional anti-cheat server link), brotli and zlib. They are checked in under
# Libs/ because the project has no package manager for them, and they must
# stay with the source for the link to succeed.
target_link_libraries(RBLXAntiCheat PUBLIC
    ntdll
    imagehlp
    psapi
    ws2_32
    "${RBLX_ANTICHEAT_DIR}/Libs/brotlicommon.lib"
    "${RBLX_ANTICHEAT_DIR}/Libs/brotlidec.lib"
    "${RBLX_ANTICHEAT_DIR}/Libs/brotlienc.lib"
    "$<$<CONFIG:Debug>:${RBLX_ANTICHEAT_DIR}/Libs/libcurl-d.lib>"
    "$<$<NOT:$<CONFIG:Debug>>:${RBLX_ANTICHEAT_DIR}/Libs/libcurl.lib>"
    "$<$<CONFIG:Debug>:${RBLX_ANTICHEAT_DIR}/Libs/zlibd.lib>"
    "$<$<NOT:$<CONFIG:Debug>>:${RBLX_ANTICHEAT_DIR}/Libs/zlib.lib>"
)

# The libraries above are built for x64; a 32-bit build would fail at link time
# with confusing unresolved externals.
if(CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(WARNING "RBLXAntiCheat ships x64-only prebuilt libraries; build the game for x64.")
endif()
