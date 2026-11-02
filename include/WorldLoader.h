#pragma once

#include <string>
#include <deque>

#include <glm/glm.hpp>

#include "Part.h"
#include "PhysicsWorld.h"

// The air of a world, read from the optional fields after the part count on its
// header line. A world file written before fog existed has none of them, and
// then the world simply has no fog.
struct WorldEnvironment {
    glm::vec3 fogColor{0.0f};
    float fogDensity = 0.0f;
};

class WorldLoader {
public:
    static void loadWorld(const std::string& filename, std::deque<Part>& parts, PhysicsWorld& physicsWorld,
                          WorldEnvironment* environment = nullptr);
};

