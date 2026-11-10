// dear rblxui: "null" example application
// (compile and link rblxui, create context, run headless with NO INPUTS, NO GRAPHICS OUTPUT)
// This is useful to test building, but you cannot interact with anything here!
#include "rblxui.h"
#include <stdio.h>

// For rblxui_impl_null: use relative filename + embed implementation directly by including the .cpp file.
// This is to simplify casual building of this example from all sorts of test scripts.
#include "../../backends/rblxui_impl_null.h"
#include "../../backends/rblxui_impl_null.cpp"

int main(int, char**)
{
    RBLXUI_CHECKVERSION();

    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO();

    RblxUI_ImplNullPlatform_Init();
    RblxUI_ImplNullRender_Init();

    for (int n = 0; n < 20; n++)
    {
        printf("NewFrame() %d\n", n);
        RblxUI_ImplNullPlatform_NewFrame();
        RblxUI_ImplNullRender_NewFrame();
        RblxUI::NewFrame();

        static float f = 0.0f;
        RblxUI::Text("Hello, world!");
        RblxUI::SliderFloat("float", &f, 0.0f, 1.0f);
        RblxUI::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        RblxUI::ShowDemoWindow(nullptr);

        RblxUI::Render();
    }

    printf("DestroyContext()\n");
    RblxUI_ImplNullRender_Shutdown();
    RblxUI_ImplNullPlatform_Shutdown();
    RblxUI::DestroyContext();
    return 0;
}
