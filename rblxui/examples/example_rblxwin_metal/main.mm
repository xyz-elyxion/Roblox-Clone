// Dear RblxUI: standalone example application for RBLXWIN + Metal, using programmable pipeline
// (RBLXWIN is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear RblxUI:
// - FAQ                  https://dearrblxui.com/faq
// - Getting Started      https://dearrblxui.com/getting-started
// - Documentation        https://dearrblxui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of rblxui.cpp

#include "rblxui.h"
#include "rblxui_impl_rblxwin.h"
#include "rblxui_impl_metal.h"
#include <stdio.h>

#define RBLXWIN_INCLUDE_NONE
#define RBLXWIN_EXPOSE_NATIVE_COCOA
#include <RBLXWIN/rblxwin.h>
#include <RBLXWIN/rblxwinnative.h>

#import <Metal/Metal.h>
#import <QuartzCore/QuartzCore.h>

static void rblxwin_error_callback(int error, const char* description)
{
    fprintf(stderr, "RblxWin Error %d: %s\n", error, description);
}

int main(int, char**)
{
    rblxwinSetErrorCallback(rblxwin_error_callback);
    if (!rblxwinInit())
        return 1;

    // Create window with graphics context
    float main_scale = RblxUI_ImplRblxWin_GetContentScaleForMonitor(rblxwinGetPrimaryMonitor()); // Valid on RBLXWIN 3.3+ only
    rblxwinWindowHint(RBLXWIN_CLIENT_API, RBLXWIN_NO_API);
    RBLXWINwindow* window = rblxwinCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear RblxUI RBLXWIN+Metal example", nullptr, nullptr);
    if (window == nullptr)
        return 1;

    // Setup Dear RblxUI context
    RBLXUI_CHECKVERSION();
    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO(); (void)io;
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    io.ConfigFlags |= RblxUIConfigFlags_DockingEnable;         // Enable Docking
    io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

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

    id <MTLDevice> device = MTLCreateSystemDefaultDevice();
    id <MTLCommandQueue> commandQueue = [device newCommandQueue];

    // Setup Platform/Renderer backends
    RblxUI_ImplRblxWin_InitForOpenGL(window, true);
    RblxUI_ImplMetal_Init(device);

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

    NSWindow *nswin = rblxwinGetCocoaWindow(window);
    CAMetalLayer *layer = [CAMetalLayer layer];
    layer.device = device;
    layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    nswin.contentView.layer = layer;
    nswin.contentView.wantsLayer = YES;

    MTLRenderPassDescriptor *renderPassDescriptor = [MTLRenderPassDescriptor new];

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    float clear_color[4] = {0.45f, 0.55f, 0.60f, 1.00f};

    // Main loop
    while (!rblxwinWindowShouldClose(window))
    {
        @autoreleasepool
        {
            // Poll and handle events (inputs, window resize, etc.)
            // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear rblxui wants to use your inputs.
            // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
            // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
            // Generally you may always pass all inputs to dear rblxui, and hide them from your application based on those two flags.
            rblxwinPollEvents();

            int width, height;
            rblxwinGetFramebufferSize(window, &width, &height);
            layer.drawableSize = CGSizeMake(width, height);
            id<CAMetalDrawable> drawable = [layer nextDrawable];

            id<MTLCommandBuffer> commandBuffer = [commandQueue commandBuffer];
            renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color[0] * clear_color[3], clear_color[1] * clear_color[3], clear_color[2] * clear_color[3], clear_color[3]);
            renderPassDescriptor.colorAttachments[0].texture = drawable.texture;
            renderPassDescriptor.colorAttachments[0].loadAction = MTLLoadActionClear;
            renderPassDescriptor.colorAttachments[0].storeAction = MTLStoreActionStore;
            id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
            [renderEncoder pushDebugGroup:@"RblxUI demo"];

            // Start the Dear RblxUI frame
            RblxUI_ImplMetal_NewFrame(renderPassDescriptor);
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
            RblxUI_ImplMetal_RenderDrawData(RblxUI::GetDrawData(), commandBuffer, renderEncoder);

            // Update and Render additional Platform Windows
            if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
            {
                RblxUI::UpdatePlatformWindows();
                RblxUI::RenderPlatformWindowsDefault();
            }

            [renderEncoder popDebugGroup];
            [renderEncoder endEncoding];

            [commandBuffer presentDrawable:drawable];
            [commandBuffer commit];
        }
    }

    // Cleanup
    RblxUI_ImplMetal_Shutdown();
    RblxUI_ImplRblxWin_Shutdown();
    RblxUI::DestroyContext();

    rblxwinDestroyWindow(window);
    rblxwinTerminate();

    return 0;
}
