#pragma once

#include <rblxgl/rblxgl.h>
#include <RBLXWIN/rblxwin.h>
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
            rblxwinGetFramebufferSize(window, &w, &h);
            return w;
        }
        return width; 
    }
    int getHeight() const { 
        if (window) {
            int w, h;
            rblxwinGetFramebufferSize(window, &w, &h);
            return h;
        }
        return height; 
    }
    RBLXWINwindow* getNativeWindow() const { return window; }

private:
    RBLXWINwindow* window;
    int width;
    int height;
    std::string title;

    void init();
};



