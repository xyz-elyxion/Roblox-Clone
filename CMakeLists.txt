cmake_minimum_required(VERSION 3.14)
project(RBLXClone LANGUAGES CXX)

# Link the MSVC runtime statically so the executables in the packaged build run
# on a machine without Visual Studio (Debug) or the VC++ redistributable
# (Release). CMP0091 only exists from CMake 3.15, so opt into it by policy; the
# 3.14 minimum is kept for toolchains that ignore the setting.
if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
endif()
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(RBLX_BUILD_STUDIO "Build the Studio editor target" ON)
option(RBLX_BUILD_CODEEDITOR "Build the RBLXCode source editor target" ON)
option(RBLX_BUILD_CLIENT "Build the game client target" ON)
option(RBLX_BUILD_SERVER "Build the game server target" ON)
option(RBLX_BUILD_TESTS "Build C++ test targets" ON)
option(RBLX_BUILD_WEBAPP "Build the native website backend target" ON)
option(RBLX_BUILD_TRAINER "Build the headless AI trainer target" ON)
option(RBLX_BUILD_CC "Build the RBLXCC C++ compiler target" ON)
option(RBLX_BUILD_ANTICHEAT "Embed the vendored RBLXAntiCheat library in the game client (Windows only)" ON)
option(RBLX_ENABLE_WARNINGS "Enable stricter compiler warnings" ON)

add_compile_definitions(NOMINMAX GLM_ENABLE_EXPERIMENTAL)

include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build GLFW examples" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build GLFW tests" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build GLFW documentation" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw
    GIT_TAG 3.3.8
)
FetchContent_MakeAvailable(glfw)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm
    GIT_TAG 0af55ccecd98d4e5a8d1fad7de25ba429d60e863 # 1.0.1
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui
    GIT_TAG 2af6dd9694288e6befe1edb7ce25510911693c22 # docking branch snapshot
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    imguizmo
    GIT_REPOSITORY https://github.com/CedricGuillemet/ImGuizmo
    GIT_TAG be8aa4aeab86b402701c8c1df011bd8cd776760b
)
FetchContent_GetProperties(imguizmo)
if(NOT imguizmo_POPULATED)
    FetchContent_Populate(imguizmo)
endif()

# Studio's script panel is a code editor with line numbers, syntax highlighting,
# selection and undo. ImGui has no widget for that, so it comes from the editor
# everybody embeds in ImGui apps; it is two files and builds against the pinned
# ImGui above.
FetchContent_Declare(
    imguicolortextedit
    GIT_REPOSITORY https://github.com/BalazsJako/ImGuiColorTextEdit
    GIT_TAG ca2f9f1462e3b60e56351bc466acda448c5ea50d
)
FetchContent_GetProperties(imguicolortextedit)
if(NOT imguicolortextedit_POPULATED)
    FetchContent_Populate(imguicolortextedit)
endif()

FetchContent_Declare(
    rp3d
    GIT_REPOSITORY https://github.com/DanielChappuis/reactphysics3d
    GIT_TAG v0.9.0
)
FetchContent_MakeAvailable(rp3d)
if(TARGET reactphysics3d)
    if(MSVC)
        # rp3d 0.9.0's DefaultLogger.h uses std::chrono::system_clock but never
        # includes <chrono>. Older MSVC only saw it through <mutex> transitively
        # and newer MSVC (19.4x) no longer does, which breaks the build.
        target_compile_options(reactphysics3d PUBLIC
            $<$<COMPILE_LANGUAGE:CXX>:/FIchrono>
        )
    else()
        target_compile_options(reactphysics3d PUBLIC
            $<$<COMPILE_LANGUAGE:CXX>:-include>
            $<$<COMPILE_LANGUAGE:CXX>:cstdint>
        )
    endif()
endif()

FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb
    GIT_TAG 31c1ad37456438565541f4919958214b6e762fb4
)
FetchContent_MakeAvailable(stb)

FetchContent_Declare(
    glad
    GIT_REPOSITORY https://github.com/libigl/libigl-glad
    GIT_TAG 651a425101365aa6e8504988ef9bb363d066c5ee
)
FetchContent_MakeAvailable(glad)

set(SHARED_SOURCES
    src/Renderer.cpp
    src/Camera.cpp
    src/Shader.cpp
    src/Window.cpp
    src/Raycaster.cpp
    src/Skybox.cpp
    src/PhysicsWorld.cpp
    src/ShadowMap.cpp
    src/Character.cpp
    src/WorldLoader.cpp
    src/Avatar.cpp
)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

set(IMGUIZMO_SOURCES
    ${imguizmo_SOURCE_DIR}/src/ImGuizmo.cpp
)

function(configure_rblx_target target_name)
    target_include_directories(${target_name} PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${stb_SOURCE_DIR}
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${imguizmo_SOURCE_DIR}
        ${imguizmo_SOURCE_DIR}/src
        ${glad_SOURCE_DIR}/include
    )

    if(RBLX_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(${target_name} PRIVATE /W4 /permissive-)
        else()
            target_compile_options(${target_name} PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
endfunction()

if(RBLX_BUILD_STUDIO)
    add_executable(Studio
        src/main.cpp
        src/HttpClient.cpp
        # The publish login solves the machine captcha, which needs sha256.
        src/WebJson.cpp
        src/WebCrypto.cpp
        src/Editor.cpp
        ${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp
        src/RblxScript.cpp
        src/WorldScript.cpp
        ${SHARED_SOURCES}
        ${IMGUI_SOURCES}
        ${IMGUIZMO_SOURCES}
    )
    configure_rblx_target(Studio)
    target_include_directories(Studio PRIVATE ${imguicolortextedit_SOURCE_DIR})
    target_link_libraries(Studio PRIVATE glfw glm reactphysics3d glad)
    if(WIN32)
        # HttpClient is the only transport left (the website calls used to go
        # through WinHTTP), and it is plain sockets, hence ws2_32.
        target_link_libraries(Studio PRIVATE ws2_32 shell32)
    endif()
endif()

if(RBLX_BUILD_CODEEDITOR)
    # The source editor for this project: a window, a project tree, tabs and the
    # same text widget Studio's script panel uses, with CodeProject doing the
    # scanning, reading, saving and searching. It has no renderer and no physics,
    # so it builds on a machine that cannot run the game, and it is a separate
    # executable because editing C++ and playing a level are different jobs.
    add_executable(RBLXCode
        src/CodeMain.cpp
        src/CodeProject.cpp
        src/Window.cpp
        ${imguicolortextedit_SOURCE_DIR}/TextEditor.cpp
        ${IMGUI_SOURCES}
    )
    configure_rblx_target(RBLXCode)
    target_include_directories(RBLXCode PRIVATE ${imguicolortextedit_SOURCE_DIR})
    target_link_libraries(RBLXCode PRIVATE glfw glad)
    if(NOT WIN32)
        # glad reaches GL through GLFW at runtime, but the loader still needs
        # dlopen.
        find_package(Threads REQUIRED)
        target_link_libraries(RBLXCode PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
    endif()
endif()

# RBLXAntiCheat is compiled straight into the client (see its CMakeLists): the
# checks have to run inside the process they watch, so there is no separate
# executable to launch or ship.
if(RBLX_BUILD_ANTICHEAT AND WIN32)
    add_subdirectory(RBLXAntiCheat)
endif()

if(RBLX_BUILD_CLIENT)
    add_executable(Client
        src/Client.cpp
        src/ClientNet.cpp
        src/ClientUi.cpp
        src/WebJson.cpp
        src/WebCrypto.cpp
        src/HttpClient.cpp
        ${SHARED_SOURCES}
        ${IMGUI_SOURCES}
    )
    configure_rblx_target(Client)
    target_link_libraries(Client PRIVATE glfw glm reactphysics3d glad)
    if(WIN32)
        target_link_libraries(Client PRIVATE ws2_32 shell32 comdlg32)
    else()
        # glad reaches GL through GLFW at runtime, but the loader still needs
        # dlopen, and the client's worker threads are std::thread.
        find_package(Threads REQUIRED)
        target_link_libraries(Client PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
    endif()
    if(TARGET RBLXAntiCheat)
        target_compile_definitions(Client PRIVATE RBLX_ANTICHEAT)
        target_link_libraries(Client PRIVATE RBLXAntiCheat)
    endif()
    if(MSVC)
        # Hardening for the executable players actually run. Control-flow guard
        # rejects indirect calls that do not land on a valid target, so a patched
        # call site in the anti-cheat path cannot simply be redirected, and full
        # ASLR keeps the image from loading at a predictable base. Identical-code
        # folding makes the individual checks harder to tell apart when reading
        # the disassembly.
        target_compile_options(Client PRIVATE
            $<$<CONFIG:Release>:/guard:cf>
            $<$<CONFIG:Release>:/Gy>
            $<$<CONFIG:Release>:/GS>
        )
        target_link_options(Client PRIVATE
            $<$<CONFIG:Release>:/guard:cf>
            $<$<CONFIG:Release>:/DYNAMICBASE>
            $<$<CONFIG:Release>:/HIGHENTROPYVA>
            $<$<CONFIG:Release>:/OPT:REF>
            $<$<CONFIG:Release>:/OPT:ICF>
        )
    endif()
endif()

if(RBLX_BUILD_SERVER)
    add_executable(Server
        src/Server.cpp
        src/ServerAuth.cpp
        src/HttpClient.cpp
        src/WorldLoader.cpp
        src/PhysicsWorld.cpp
        src/RblxScript.cpp
        src/WorldScript.cpp
    )
    configure_rblx_target(Server)
    target_link_libraries(Server PRIVATE glm reactphysics3d)
    if(WIN32)
        target_link_libraries(Server PRIVATE ws2_32)
    else()
        find_package(Threads REQUIRED)
        target_link_libraries(Server PRIVATE Threads::Threads)
    endif()
endif()

if(RBLX_BUILD_TRAINER)
    # Headless learner: loads a .world, drives the same Character and PhysicsWorld
    # the game uses, and writes a policy file. No window and no GL, so it runs on
    # a build machine or in CI.
    add_executable(AiTrainer
        src/AiTrainer.cpp
        src/PhysicsWorld.cpp
        src/Character.cpp
        src/Camera.cpp
        src/WorldLoader.cpp
    )
    configure_rblx_target(AiTrainer)
    target_link_libraries(AiTrainer PRIVATE reactphysics3d glm)
    if(NOT WIN32)
        find_package(Threads REQUIRED)
        target_link_libraries(AiTrainer PRIVATE Threads::Threads)
    endif()
endif()

if(RBLX_BUILD_CC)
    # RBLXCC is the project's own C++ compiler: a lexer and preprocessor, a
    # recursive-descent parser that type-checks as it parses, and an x86-64 back
    # end that writes assembly. It has no dependencies beyond the standard
    # library, so nothing here is fetched or linked.
    add_executable(RBLXCC
        src/CxxMain.cpp
        src/CxxLexer.cpp
        src/CxxParser.cpp
        src/CxxCodegen.cpp
    )
    configure_rblx_target(RBLXCC)

    # CMake builds the compiler once, and the compiler builds this program: two
    # translation units, each compiled on its own and linked into one executable
    # by RBLXCC instead of add_executable. MSVC is skipped because the back end
    # writes AT&T assembly, which only an assembler like cc's understands.
    #
    # The real targets are not here because of the language, not the wiring: they
    # use std::string, std::vector, threads, floating point, classes and templates,
    # plus GLFW, GLM, ImGui and reactphysics3d headers. RBLXCC implements the C++
    # that plain programs need; until it grows those features, CMake keeps
    # compiling them.
    if(NOT MSVC)
        set(RBLXCC_EXAMPLE_SOURCES
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/rblxcc/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/examples/rblxcc/totals.cpp
        )
        set(RBLXCC_EXAMPLE_PROGRAM
            ${CMAKE_CURRENT_BINARY_DIR}/RBLXCCExample${CMAKE_EXECUTABLE_SUFFIX}
        )
        add_custom_command(
            OUTPUT ${RBLXCC_EXAMPLE_PROGRAM}
            COMMAND $<TARGET_FILE:RBLXCC> ${RBLXCC_EXAMPLE_SOURCES} -o ${RBLXCC_EXAMPLE_PROGRAM}
            DEPENDS RBLXCC ${RBLXCC_EXAMPLE_SOURCES}
            COMMENT "Compiling RBLXCCExample with RBLXCC"
            VERBATIM
        )
        # The target and the file it produces cannot share a name, or make reports a
        # circular dependency between them.
        add_custom_target(BuildRBLXCCExample ALL DEPENDS ${RBLXCC_EXAMPLE_PROGRAM})
        if(RBLX_BUILD_TESTS)
            add_test(NAME RBLXCCExample COMMAND ${RBLXCC_EXAMPLE_PROGRAM})
            set_tests_properties(RBLXCCExample PROPERTIES PASS_REGULAR_EXPRESSION "total=10")
        endif()
    endif()
endif()

set(WEB_SOURCES
    src/WebApp.cpp
    src/WebHttp.cpp
    src/WebJson.cpp
    src/WebCrypto.cpp
    src/WebStore.cpp
)

if(RBLX_BUILD_WEBAPP)
    # The website server has a desktop console window: it shows the process log,
    # every running game server and that server's output instead of a bare
    # terminal. --nogui / RBLX_NO_GUI keeps the plain console for CI, tests and
    # headless hosts, and a machine without a usable display falls back to it.
    add_executable(WebApp
        src/WebMain.cpp
        src/ConsoleGui.cpp
        ${WEB_SOURCES}
        ${IMGUI_SOURCES}
    )
    configure_rblx_target(WebApp)
    target_link_libraries(WebApp PRIVATE glfw glad)
    if(WIN32)
        target_link_libraries(WebApp PRIVATE ws2_32 shell32)
    else()
        find_package(Threads REQUIRED)
        target_link_libraries(WebApp PRIVATE Threads::Threads)
    endif()
endif()

if(RBLX_BUILD_TESTS)
    enable_testing()
    add_executable(ScriptTests tests/rblx_script_tests.cpp src/RblxScript.cpp)
    target_include_directories(ScriptTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME ScriptTests COMMAND ScriptTests)

    # The source editor's file handling is headless, so its checks run without a
    # display or the graphics dependencies.
    add_executable(CodeProjectTests tests/code_project_tests.cpp src/CodeProject.cpp)
    target_include_directories(CodeProjectTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME CodeProjectTests COMMAND CodeProjectTests)

    add_executable(NetworkProtocolTests tests/network_protocol_tests.cpp)
    target_include_directories(NetworkProtocolTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(NetworkProtocolTests PRIVATE glm)
    add_test(NAME NetworkProtocolTests COMMAND NetworkProtocolTests)

    add_executable(MovementGuardTests tests/movement_guard_tests.cpp)
    target_include_directories(MovementGuardTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(MovementGuardTests PRIVATE glm)
    add_test(NAME MovementGuardTests COMMAND MovementGuardTests)

    add_executable(ModerationTests tests/moderation_tests.cpp)
    target_include_directories(ModerationTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME ModerationTests COMMAND ModerationTests)

    add_executable(ClientCheatReportTests tests/client_cheat_report_tests.cpp)
    target_include_directories(ClientCheatReportTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME ClientCheatReportTests COMMAND ClientCheatReportTests)

    add_executable(ObfuscatedTests tests/obfuscated_tests.cpp)
    target_include_directories(ObfuscatedTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME ObfuscatedTests COMMAND ObfuscatedTests)

    add_executable(HealthStateTests tests/health_state_tests.cpp)
    target_include_directories(HealthStateTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME HealthStateTests COMMAND HealthStateTests)

    add_executable(WebCaptchaTests
        tests/web_captcha_tests.cpp
        src/WebJson.cpp
        src/WebCrypto.cpp
    )
    target_include_directories(WebCaptchaTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    add_test(NAME WebCaptchaTests COMMAND WebCaptchaTests)

    add_executable(VehicleTests tests/vehicle_tests.cpp)
    target_include_directories(VehicleTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(VehicleTests PRIVATE glm)
    add_test(NAME VehicleTests COMMAND VehicleTests)

    add_executable(HttpClientTests tests/http_client_tests.cpp src/HttpClient.cpp)
    target_include_directories(HttpClientTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(WIN32)
        target_link_libraries(HttpClientTests PRIVATE ws2_32)
    else()
        find_package(Threads REQUIRED)
        target_link_libraries(HttpClientTests PRIVATE Threads::Threads)
    endif()
    add_test(NAME HttpClientTests COMMAND HttpClientTests)

    add_executable(WebAppTests
        tests/web_app_tests.cpp
        ${WEB_SOURCES}
    )
    target_include_directories(WebAppTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(RBLX_ENABLE_WARNINGS)
        if(MSVC)
            target_compile_options(WebAppTests PRIVATE /W4 /permissive-)
        else()
            target_compile_options(WebAppTests PRIVATE -Wall -Wextra -Wpedantic)
        endif()
    endif()
    if(WIN32)
        target_link_libraries(WebAppTests PRIVATE ws2_32)
    else()
        find_package(Threads REQUIRED)
        target_link_libraries(WebAppTests PRIVATE Threads::Threads)
    endif()
    add_test(NAME WebAppTests COMMAND WebAppTests)

    # The compiler is driven through its public API: source in, diagnostics and
    # assembly out. Half of these tests go further and assemble, link and run the
    # programs they compile with the host C compiler.
    add_executable(CxxCompilerTests
        tests/cxx_compiler_tests.cpp
        src/CxxLexer.cpp
        src/CxxParser.cpp
        src/CxxCodegen.cpp
    )
    target_include_directories(CxxCompilerTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
    if(TARGET RBLXCC)
        # The suite also drives the compiler the way a user does: several source
        # files in, one linked program out.
        add_test(NAME CxxCompilerTests COMMAND CxxCompilerTests $<TARGET_FILE:RBLXCC>)
    else()
        add_test(NAME CxxCompilerTests COMMAND CxxCompilerTests)
    endif()
endif()
