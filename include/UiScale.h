#pragma once

#include "rblxui.h"
#include <RBLXWIN/rblxwin.h>

#include <algorithm>
#include <cmath>

namespace UiScale {
    inline float Compute(RBLXWINwindow* window) {
        int framebufferWidth = 1280;
        int framebufferHeight = 720;
        rblxwinGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        float contentScaleX = 1.0f;
        float contentScaleY = 1.0f;
        rblxwinGetWindowContentScale(window, &contentScaleX, &contentScaleY);

        const float resolutionScale = std::sqrt(std::max(
            static_cast<float>(std::max(framebufferWidth, 1)) / 1280.0f,
            static_cast<float>(std::max(framebufferHeight, 1)) / 720.0f
        ));
        const float dpiScale = std::max(contentScaleX, contentScaleY);
        return std::clamp(std::max(resolutionScale, dpiScale), 1.0f, 1.85f);
    }

    inline float Apply(RBLXWINwindow* window) {
        const float scale = Compute(window);
        RblxUIIO& io = RblxUI::GetIO();
        RblxUIStyle& style = RblxUI::GetStyle();
        style.ScaleAllSizes(scale);
        io.FontGlobalScale = scale;
        return scale;
    }

    inline float Px(float value, float scale) {
        return value * scale;
    }

    inline RblxVec2 Size(float width, float height, float scale) {
        return RblxVec2(Px(width, scale), Px(height, scale));
    }

    inline RblxVec2 CenteredWindowPos(float width, float height, float scale, float yOffset = 0.0f) {
        const RblxVec2 displaySize = RblxUI::GetIO().DisplaySize;
        const float scaledWidth = Px(width, scale);
        const float scaledHeight = Px(height, scale);
        return RblxVec2(
            std::max(0.0f, (displaySize.x - scaledWidth) * 0.5f),
            std::max(0.0f, (displaySize.y - scaledHeight) * 0.5f + Px(yOffset, scale))
        );
    }
}
