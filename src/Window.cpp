#include "Window.h"
#include <iostream>

Window::Window(int width, int height, const std::string& title) 
    : width(width), height(height), title(title), window(nullptr) {
    init();
}

Window::~Window() {
    rblxwinDestroyWindow(window);
    rblxwinTerminate();
}

void Window::init() {
    if (!rblxwinInit()) {
        throw std::runtime_error("Failed to initialize RBLXWIN");
    }

    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MAJOR, 3);
    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MINOR, 3);
    rblxwinWindowHint(RBLXWIN_OPENGL_PROFILE, RBLXWIN_OPENGL_CORE_PROFILE);

    window = rblxwinCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        rblxwinTerminate();
        throw std::runtime_error("Failed to create RBLXWIN window");
    }

    rblxwinMakeContextCurrent(window);

    if (!rblxglLoadGLLoader((RBLXGLloadproc)rblxwinGetProcAddress)) {
        throw std::runtime_error("Failed to initialize RBLXGL");
    }

    glViewport(0, 0, width, height);
    
    rblxwinSetFramebufferSizeCallback(window, [](RBLXWINwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });
}

bool Window::shouldClose() const {
    return rblxwinWindowShouldClose(window);
}

void Window::swapBuffers() {
    rblxwinSwapBuffers(window);
}

void Window::pollEvents() {
    rblxwinPollEvents();
}



