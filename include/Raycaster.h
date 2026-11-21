#pragma once

#include <linearmath/linearmath.hpp>
#include <deque>
#include "Part.h"
#include "Camera.h"
#include "LinearGameApi.h"

LINEAR_GAME_API_BEGIN
class LINEAR_GAME_API Raycaster {
public:
    // Returns index of selected part, or -1 if none
    static int GetPartFromMouse(const std::deque<Part>& parts, const Camera& camera, float mouseX, float mouseY, float screenWidth, float screenHeight);

private:
    static bool RayCubeIntersection(const linearmath::vec3& rayOrigin, const linearmath::vec3& rayDir, const Part& part, float& distance);
};
LINEAR_GAME_API_END

