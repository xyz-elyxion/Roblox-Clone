#pragma once

#include "linearui.h"
#include <LINEARWIN/linearwin.h>

#include <algorithm>
#include <cmath>

namespace UiScale {
    inline float Compute(LINEARWINwindow* window) {
        int framebufferWidth = 1280;
        int framebufferHeight = 720;
        linearwinGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);

        float contentScaleX = 1.0f;
        float contentScaleY = 1.0f;
        linearwinGetWindowContentScale(window, &contentScaleX, &contentScaleY);

        const float resolutionScale = std::sqrt(std::max(
            static_cast<float>(std::max(framebufferWidth, 1)) / 1280.0f,
            static_cast<float>(std::max(framebufferHeight, 1)) / 720.0f
        ));
        const float dpiScale = std::max(contentScaleX, contentScaleY);
        return std::clamp(std::max(resolutionScale, dpiScale), 1.0f, 1.85f);
    }

    inline float Apply(LINEARWINwindow* window) {
        const float scale = Compute(window);
        LinearUIIO& io = LinearUI::GetIO();
        LinearUIStyle& style = LinearUI::GetStyle();
        style.ScaleAllSizes(scale);
        io.FontGlobalScale = scale;
        return scale;
    }

    inline float Px(float value, float scale) {
        return value * scale;
    }

    inline LinearVec2 Size(float width, float height, float scale) {
        return LinearVec2(Px(width, scale), Px(height, scale));
    }

    inline LinearVec2 CenteredWindowPos(float width, float height, float scale, float yOffset = 0.0f) {
        const LinearVec2 displaySize = LinearUI::GetIO().DisplaySize;
        const float scaledWidth = Px(width, scale);
        const float scaledHeight = Px(height, scale);
        return LinearVec2(
            std::max(0.0f, (displaySize.x - scaledWidth) * 0.5f),
            std::max(0.0f, (displaySize.y - scaledHeight) * 0.5f + Px(yOffset, scale))
        );
    }
}
