#include "Window.h"
#include <iostream>

Window::Window(int width, int height, const std::string& title) 
    : width(width), height(height), title(title), window(nullptr) {
    init();
}

Window::~Window() {
    linearwinDestroyWindow(window);
    linearwinTerminate();
}

void Window::init() {
    if (!linearwinInit()) {
        throw std::runtime_error("Failed to initialize LINEARWIN");
    }

    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MAJOR, 3);
    linearwinWindowHint(LINEARWIN_CONTEXT_VERSION_MINOR, 3);
    linearwinWindowHint(LINEARWIN_OPENGL_PROFILE, LINEARWIN_OPENGL_CORE_PROFILE);

    window = linearwinCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!window) {
        linearwinTerminate();
        throw std::runtime_error("Failed to create LINEARWIN window");
    }

    linearwinMakeContextCurrent(window);

    if (!linearglLoadGLLoader((LINEARGLloadproc)linearwinGetProcAddress)) {
        throw std::runtime_error("Failed to initialize LINEARGL");
    }

    glViewport(0, 0, width, height);
    
    linearwinSetFramebufferSizeCallback(window, [](LINEARWINwindow*, int w, int h) {
        glViewport(0, 0, w, h);
    });
}

bool Window::shouldClose() const {
    return linearwinWindowShouldClose(window);
}

void Window::swapBuffers() {
    linearwinSwapBuffers(window);
}

void Window::pollEvents() {
    linearwinPollEvents();
}



