#pragma once

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>
#include "linearui.h"
#include "linearui_impl_linearwin.h"
#include "linearui_impl_opengl3.h"
#include "LinearGizmo.h"

class Editor {
public:
    Editor(LINEARWINwindow* window);
    ~Editor();

    void init();
    void newFrame();
    void render();
    void cleanup();

    void setupDockSpace();

private:
    LINEARWINwindow* window;
};

