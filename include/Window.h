#pragma once

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>
#include <string>

class Window {
public:
    Window(int width, int height, const std::string& title);
    ~Window();

    bool shouldClose() const;
    void swapBuffers();
    void pollEvents();
    
    int getWidth() const { 
        if (window) {
            int w, h;
            linearwinGetFramebufferSize(window, &w, &h);
            return w;
        }
        return width; 
    }
    int getHeight() const { 
        if (window) {
            int w, h;
            linearwinGetFramebufferSize(window, &w, &h);
            return h;
        }
        return height; 
    }
    LINEARWINwindow* getNativeWindow() const { return window; }

private:
    LINEARWINwindow* window;
    int width;
    int height;
    std::string title;

    void init();
};



