// Dear RblxUI: standalone example application for RBLXWIN + OpenGL2, using legacy fixed pipeline
// (RBLXWIN is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_rblxwin_opengl2/ folder**
// See rblxui_impl_rblxwin.cpp for details.

#include "rblxui.h"
#include "rblxui_impl_rblxwin.h"
#include "rblxui_impl_opengl2.h"
#include <stdio.h>
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#endif
#include <RBLXWIN/rblxwin.h>

// [Win32] Our example includes a copy of rblxwin.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of RBLXWIN that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(RBLXUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void rblxwin_error_callback(int error, const char* description)
{
    fprintf(stderr, "RBLXWIN Error %d: %s\n", error, description);
}

// Main code
int main(int, char**)
{
    rblxwinSetErrorCallback(rblxwin_error_callback);
    if (!rblxwinInit())
        return 1;

    // Create window with graphics context
    float main_scale = RblxUI_ImplRblxWin_GetContentScaleForMonitor(rblxwinGetPrimaryMonitor()); // Valid on RBLXWIN 3.3+ only
    RBLXWINwindow* window = rblxwinCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear RblxUI RBLXWIN+OpenGL2 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    rblxwinMakeContextCurrent(window);
    rblxwinSwapInterval(1); // Enable vsync

    // Setup Dear RblxUI context
    RBLXUI_CHECKVERSION();
    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO(); (void)io;
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= RblxUIConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows
    //io.ConfigViewportsNoAutoMerge = true;
    //io.ConfigViewportsNoTaskBarIcon = true;

    // Setup Dear RblxUI style
    RblxUI::StyleColorsDark();
    //RblxUI::StyleColorsLight();

    // Setup scaling
    RblxUIStyle& style = RblxUI::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (in docking branch: using io.ConfigDpiScaleFonts=true automatically overrides this for every window depending on the current monitor)
    io.ConfigDpiScaleFonts = true;          // [Experimental] Automatically overwrite style.FontScaleDpi in Begin() when Monitor DPI changes. This will scale fonts but _NOT_ scale sizes/padding for now.
    io.ConfigDpiScaleViewports = true;      // [Experimental] Scale Dear RblxUI and Platform Windows when Monitor DPI changes.

    // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
    if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
    {
        style.WindowRounding = 0.0f;
        style.Colors[RblxUICol_WindowBg].w = 1.0f;
    }

    // Setup Platform/Renderer backends
    RblxUI_ImplRblxWin_InitForOpenGL(window, true);
    RblxUI_ImplOpenGL2_Init();

    // Load Fonts
    // - If fonts are not explicitly loaded, Dear RblxUI will select an embedded font: either AddFontDefaultVector() or AddFontDefaultBitmap().
    //   This selection is based on (style.FontSizeBase * style.FontScaleMain * style.FontScaleDpi) reaching a small threshold.
    // - You can load multiple fonts and use RblxUI::PushFont()/PopFont() to select them.
    // - If a file cannot be loaded, AddFont functions will return a nullptr. Please handle those errors in your code (e.g. use an assertion, display an error and quit).
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Use '#define RBLXUI_ENABLE_FREETYPE' in your rblxuiconfig file to use FreeType for higher quality font rendering.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefaultVector();
    //io.Fonts->AddFontDefaultBitmap();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //RblxFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //RBLXUI_ASSERT(font != nullptr);

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    RblxVec4 clear_color = RblxVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!rblxwinWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear rblxui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear rblxui, and hide them from your application based on those two flags.
        rblxwinPollEvents();
        if (rblxwinGetWindowAttrib(window, RBLXWIN_ICONIFIED) != 0)
        {
            RblxUI_ImplRblxWin_Sleep(10);
            continue;
        }

        // Start the Dear RblxUI frame
        RblxUI_ImplOpenGL2_NewFrame();
        RblxUI_ImplRblxWin_NewFrame();
        RblxUI::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in RblxUI::ShowDemoWindow()! You can browse its code to learn more about Dear RblxUI!).
        if (show_demo_window)
            RblxUI::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            RblxUI::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            RblxUI::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            RblxUI::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            RblxUI::Checkbox("Another Window", &show_another_window);

            RblxUI::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            RblxUI::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (RblxUI::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            RblxUI::SameLine();
            RblxUI::Text("counter = %d", counter);

            RblxUI::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            RblxUI::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            RblxUI::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            RblxUI::Text("Hello from another window!");
            if (RblxUI::Button("Close Me"))
                show_another_window = false;
            RblxUI::End();
        }

        // Rendering
        RblxUI::Render();
        int display_w, display_h;
        rblxwinGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // If you are using this code with non-legacy OpenGL header/contexts (which you should not, prefer using rblxui_impl_opengl3.cpp!!),
        // you may need to backup/reset/restore other state, e.g. for current shader using the commented lines below.
        //GLint last_program;
        //glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
        //glUseProgram(0);
        RblxUI_ImplOpenGL2_RenderDrawData(RblxUI::GetDrawData());
        //glUseProgram(last_program);

        // Update and Render additional Platform Windows
        // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
        //  For this specific demo app we could also call rblxwinMakeContextCurrent(window) directly)
        if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
        {
            RBLXWINwindow* backup_current_context = rblxwinGetCurrentContext();
            RblxUI::UpdatePlatformWindows();
            RblxUI::RenderPlatformWindowsDefault();
            rblxwinMakeContextCurrent(backup_current_context);
        }

        rblxwinMakeContextCurrent(window);
        rblxwinSwapBuffers(window);
    }

    // Cleanup
    RblxUI_ImplOpenGL2_Shutdown();
    RblxUI_ImplRblxWin_Shutdown();
    RblxUI::DestroyContext();

    rblxwinDestroyWindow(window);
    rblxwinTerminate();

    return 0;
}
