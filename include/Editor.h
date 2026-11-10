#pragma once

#include <rblxgl/rblxgl.h>
#include <RBLXWIN/rblxwin.h>
#include "rblxui.h"
#include "rblxui_impl_rblxwin.h"
#include "rblxui_impl_opengl3.h"
#include "RblxGizmo.h"

class Editor {
public:
    Editor(RBLXWINwindow* window);
    ~Editor();

    void init();
    void newFrame();
    void render();
    void cleanup();

    void setupDockSpace();

private:
    RBLXWINwindow* window;
};

