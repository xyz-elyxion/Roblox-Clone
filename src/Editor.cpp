#include "Editor.h"
#include "UiScale.h"
#include "UiTheme.h"
#include <iostream>
#include "linearui_internal.h" // Needed for DockBuilder

Editor::Editor(LINEARWINwindow* window) : window(window) {}

Editor::~Editor() {
    cleanup();
}

void Editor::init() {
    // Setup Dear LinearUI context
    LINEARUI_CHECKVERSION();
    LinearUI::CreateContext();
    LinearUIIO& io = LinearUI::GetIO(); (void)io;
    io.ConfigFlags |= LinearUIConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= LinearUIConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= LinearUIConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // The same palette as the rest of the project, then the window's own
    // resolution scaling on top.
    linear::ui::theme::apply();
    UiScale::Apply(window);

    // Setup Platform/Renderer backends
    LinearUI_ImplLinearWin_InitForOpenGL(window, true);
    LinearUI_ImplOpenGL3_Init("#version 330");
}

void Editor::newFrame() {
    LinearUI_ImplOpenGL3_NewFrame();
    LinearUI_ImplLinearWin_NewFrame();
    LinearUI::NewFrame();
    LinearGizmo::BeginFrame();
    
    setupDockSpace();
}

void Editor::render() {
    LinearUI::Render();
    LinearUI_ImplOpenGL3_RenderDrawData(LinearUI::GetDrawData());

    LinearUIIO& io = LinearUI::GetIO();
    if (io.ConfigFlags & LinearUIConfigFlags_ViewportsEnable)
    {
        LINEARWINwindow* backup_current_context = linearwinGetCurrentContext();
        LinearUI::UpdatePlatformWindows();
        LinearUI::RenderPlatformWindowsDefault();
        linearwinMakeContextCurrent(backup_current_context);
    }
}

void Editor::cleanup() {
    LinearUI_ImplOpenGL3_Shutdown();
    LinearUI_ImplLinearWin_Shutdown();
    LinearUI::DestroyContext();
}

void Editor::setupDockSpace() {
    static bool opt_fullscreen = true;
    static bool opt_padding = false;
    static LinearUIDockNodeFlags dockspace_flags = LinearUIDockNodeFlags_None;

    // We are using the LinearUIWindowFlags_NoDocking flag to make the parent window not dockable into,
    // because it would be confusing to have two docking targets within each others.
    LinearUIWindowFlags window_flags = LinearUIWindowFlags_MenuBar | LinearUIWindowFlags_NoDocking;
    if (opt_fullscreen)
    {
        const LinearUIViewport* viewport = LinearUI::GetMainViewport();
        LinearUI::SetNextWindowPos(viewport->WorkPos);
        LinearUI::SetNextWindowSize(viewport->WorkSize);
        LinearUI::SetNextWindowViewport(viewport->ID);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowRounding, 0.0f);
        LinearUI::PushStyleVar(LinearUIStyleVar_WindowBorderSize, 0.0f);
        window_flags |= LinearUIWindowFlags_NoTitleBar | LinearUIWindowFlags_NoCollapse | LinearUIWindowFlags_NoResize | LinearUIWindowFlags_NoMove;
        window_flags |= LinearUIWindowFlags_NoBringToFrontOnFocus | LinearUIWindowFlags_NoNavFocus;
    }

    if (dockspace_flags & LinearUIDockNodeFlags_PassthruCentralNode)
        window_flags |= LinearUIWindowFlags_NoBackground;
    else
        // Important: If we want to see the 3D scene behind the central node (which is transparent usually)
        // we need to make sure PassthruCentralNode is set!
        dockspace_flags |= LinearUIDockNodeFlags_PassthruCentralNode;

    LinearUI::PushStyleVar(LinearUIStyleVar_WindowPadding, LinearVec2(0.0f, 0.0f));
    LinearUI::Begin("DockSpace Demo", nullptr, window_flags);
    LinearUI::PopStyleVar();

    if (opt_fullscreen)
        LinearUI::PopStyleVar(2);

    // Submit the DockSpace
    LinearUIIO& io = LinearUI::GetIO();
    if (io.ConfigFlags & LinearUIConfigFlags_DockingEnable)
    {
        LinearUIID dockspace_id = LinearUI::GetID("MyDockSpace");
        LinearUI::DockSpace(dockspace_id, LinearVec2(0.0f, 0.0f), dockspace_flags);

        static bool first_time = true;
        if (first_time) {
            first_time = false;

            LinearUI::DockBuilderRemoveNode(dockspace_id);
            LinearUI::DockBuilderAddNode(dockspace_id, dockspace_flags | LinearUIDockNodeFlags_DockSpace);
            LinearUI::DockBuilderSetNodeSize(dockspace_id, LinearUI::GetMainViewport()->Size);

            LinearUIID dock_main_id = dockspace_id;
            LinearUIID dock_id_right = LinearUI::DockBuilderSplitNode(dock_main_id, LinearUIDir_Right, 0.25f, nullptr, &dock_main_id);
            LinearUIID dock_id_left = LinearUI::DockBuilderSplitNode(dock_main_id, LinearUIDir_Left, 0.20f, nullptr, &dock_main_id);
            
            LinearUI::DockBuilderDockWindow("Explorer", dock_id_right);
            LinearUI::DockBuilderDockWindow("Properties", dock_id_right); // We will split this below
            
            // Split the right dock node into Up/Down for Explorer/Properties
            LinearUIID dock_id_right_down = LinearUI::DockBuilderSplitNode(dock_id_right, LinearUIDir_Down, 0.50f, nullptr, &dock_id_right);
            LinearUI::DockBuilderDockWindow("Explorer", dock_id_right);
            LinearUI::DockBuilderDockWindow("Properties", dock_id_right_down);
            
            LinearUI::DockBuilderDockWindow("Toolbox", dock_id_left);
            LinearUI::DockBuilderFinish(dockspace_id);
        }
    }

    // Removed duplicated MenuBar here since we have one in main.cpp or we should move it here.
    // The user mentioned "two file button", so we likely have one here and one in main loop.
    // Let's disable this one or keep only this one. 
    // Since main.cpp logic depends on the loop (for Play/Stop), let's REMOVE this one.
    
    /* 
    if (LinearUI::BeginMenuBar())
    {
        if (LinearUI::BeginMenu("File"))
        {
            if (LinearUI::MenuItem("Exit")) linearwinSetWindowShouldClose(window, true);
            LinearUI::EndMenu();
        }
        LinearUI::EndMenuBar();
    }
    */

    LinearUI::End();
}

