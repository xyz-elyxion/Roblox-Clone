#include "Editor.h"
#include "UiScale.h"
#include "UiTheme.h"
#include <iostream>
#include "rblxui_internal.h" // Needed for DockBuilder

Editor::Editor(RBLXWINwindow* window) : window(window) {}

Editor::~Editor() {
    cleanup();
}

void Editor::init() {
    // Setup Dear RblxUI context
    RBLXUI_CHECKVERSION();
    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO(); (void)io;
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= RblxUIConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= RblxUIConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // The same palette as the rest of the project, then the window's own
    // resolution scaling on top.
    rblx::ui::theme::apply();
    UiScale::Apply(window);

    // Setup Platform/Renderer backends
    RblxUI_ImplRblxWin_InitForOpenGL(window, true);
    RblxUI_ImplOpenGL3_Init("#version 330");
}

void Editor::newFrame() {
    RblxUI_ImplOpenGL3_NewFrame();
    RblxUI_ImplRblxWin_NewFrame();
    RblxUI::NewFrame();
    RblxGizmo::BeginFrame();
    
    setupDockSpace();
}

void Editor::render() {
    RblxUI::Render();
    RblxUI_ImplOpenGL3_RenderDrawData(RblxUI::GetDrawData());

    RblxUIIO& io = RblxUI::GetIO();
    if (io.ConfigFlags & RblxUIConfigFlags_ViewportsEnable)
    {
        RBLXWINwindow* backup_current_context = rblxwinGetCurrentContext();
        RblxUI::UpdatePlatformWindows();
        RblxUI::RenderPlatformWindowsDefault();
        rblxwinMakeContextCurrent(backup_current_context);
    }
}

void Editor::cleanup() {
    RblxUI_ImplOpenGL3_Shutdown();
    RblxUI_ImplRblxWin_Shutdown();
    RblxUI::DestroyContext();
}

void Editor::setupDockSpace() {
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static RblxUIDockNodeFlags dockspace_flags = RblxUIDockNodeFlags_None;

    // We are using the RblxUIWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    RblxUIWindowFlags window_flags = RblxUIWindowFlags_MenuBar | RblxUIWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const RblxUIViewport* viewport = RblxUI::GetMainViewport();
        RblxUI::SetNextWindowPos(viewport->WorkPos);
        RblxUI::SetNextWindowSize(viewport->WorkSize);
        RblxUI::SetNextWindowViewport(viewport->ID);
        RblxUI::PushStyleVar(RblxUIStyleVar_WindowRounding, 0.0f);
        RblxUI::PushStyleVar(RblxUIStyleVar_WindowBorderSize, 0.0f);
        window_flags |= RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoCollapse | RblxUIWindowFlags_NoResize | RblxUIWindowFlags_NoMove;
        window_flags |= RblxUIWindowFlags_NoBringToFrontOnFocus | RblxUIWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & RblxUIDockNodeFlags_PassthruCentralNode)
        window_flags |= RblxUIWindowFlags_NoBackground;
    else
        // Important: If we want to see the 3D scene behind the central node (which is transparent usually)
        // we need to make sure PassthruCentralNode is set!
        dockspace_flags |= RblxUIDockNodeFlags_PassthruCentralNode;

    RblxUI::PushStyleVar(RblxUIStyleVar_WindowPadding, RblxVec2(0.0f, 0.0f));
    RblxUI::Begin("DockSpace Demo", nullptr, window_flags);
    RblxUI::PopStyleVar();

    if (opt_fullscreen)
        RblxUI::PopStyleVar(2);

    // Submit the DockSpace
    RblxUIIO& io = RblxUI::GetIO();
    if (io.ConfigFlags & RblxUIConfigFlags_DockingEnable)
    {
        RblxUIID dockspace_id = RblxUI::GetID("MyDockSpace");
        RblxUI::DockSpace(dockspace_id, RblxVec2(0.0f, 0.0f), dockspace_flags);

        static bool first_time = true;
        if (first_time) {
            first_time = false;

            RblxUI::DockBuilderRemoveNode(dockspace_id);
            RblxUI::DockBuilderAddNode(dockspace_id, dockspace_flags | RblxUIDockNodeFlags_DockSpace);
            RblxUI::DockBuilderSetNodeSize(dockspace_id, RblxUI::GetMainViewport()->Size);

            RblxUIID dock_main_id = dockspace_id;
            RblxUIID dock_id_right = RblxUI::DockBuilderSplitNode(dock_main_id, RblxUIDir_Right, 0.25f, nullptr, &dock_main_id);
            RblxUIID dock_id_left = RblxUI::DockBuilderSplitNode(dock_main_id, RblxUIDir_Left, 0.20f, nullptr, &dock_main_id);
            
            RblxUI::DockBuilderDockWindow("Explorer", dock_id_right);
            RblxUI::DockBuilderDockWindow("Properties", dock_id_right); // We will split this below
            
            // Split the right dock node into Up/Down for Explorer/Properties
            RblxUIID dock_id_right_down = RblxUI::DockBuilderSplitNode(dock_id_right, RblxUIDir_Down, 0.50f, nullptr, &dock_id_right);
            RblxUI::DockBuilderDockWindow("Explorer", dock_id_right);
            RblxUI::DockBuilderDockWindow("Properties", dock_id_right_down);
            
            RblxUI::DockBuilderDockWindow("Toolbox", dock_id_left);
            RblxUI::DockBuilderFinish(dockspace_id);
        }
    }

    // Removed duplicated MenuBar here since we have one in main.cpp or we should move it here.
    // The user mentioned "two file button", so we likely have one here and one in main loop.
    // Let's disable this one or keep only this one. 
    // Since main.cpp logic depends on the loop (for Play/Stop), let's REMOVE this one.
    
    /* 
    if (RblxUI::BeginMenuBar())
    {
        if (RblxUI::BeginMenu("File"))
        {
            if (RblxUI::MenuItem("Exit")) rblxwinSetWindowShouldClose(window, true);
            RblxUI::EndMenu();
        }
        RblxUI::EndMenuBar();
    }
    */

    RblxUI::End();
}

