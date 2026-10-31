#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>

enum class ShapeType {
    Cube,
    Sphere,
    Wedge,
    Cylinder
};

// How a part's surface is shaded. Plastic keeps the studded brick look; the
// others are flat-surface materials. They are implemented in
// shaders/fragment.glsl from one uniform, so a world file can use them without
// shipping any extra assets.
enum class Material : int {
    Plastic = 0,
    SmoothPlastic,
    Neon,
    Metal,
    Wood,
    Grass,
    Concrete,
    Brick
};

inline constexpr int kMaterialCount = 8;

inline const char* materialName(Material material) {
    switch (material) {
        case Material::SmoothPlastic: return "SmoothPlastic";
        case Material::Neon: return "Neon";
        case Material::Metal: return "Metal";
        case Material::Wood: return "Wood";
        case Material::Grass: return "Grass";
        case Material::Concrete: return "Concrete";
        case Material::Brick: return "Brick";
        case Material::Plastic:
        default: return "Plastic";
    }
}

inline bool materialFromName(const std::string& name, Material& out) {
    for (int i = 0; i < kMaterialCount; ++i) {
        const Material candidate = static_cast<Material>(i);
        if (name == materialName(candidate)) {
            out = candidate;
            return true;
        }
    }
    return false;
}

class Part;

struct Weld {
    Part* part1; // Target part (part0 is the parent)
};

class Part {
public:
    std::string name; // Added name
    int parentIndex; // -1 if root (Workspace), otherwise index in parts list (simple hierarchy)
    bool isFolder;   // Is this a Folder?
    bool isCamera;   // Is this a Camera Part?
    bool isSpawn;    // Is this a SpawnLocation?
    bool deleted;    // Mark as deleted instead of removing (prevents index shifts)

    glm::vec3 position;
    glm::vec3 size;
    glm::vec3 color;
    glm::vec3 rotation; // Euler angles in degrees
    float transparency;
    float reflectance;
    bool anchored;
    bool canCollide;
    float mass;
    glm::vec3 velocity;
    void* physicsBody; // Pointer to Bullet RigidBody
    ShapeType shape;
    std::vector<Weld> welds;
    unsigned int textureId; // Added for face texture
    Material material;      // Surface shading used by the part shader
    std::string texture;    // Optional image path, tiled across every face

    Part(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), glm::vec3 color = glm::vec3(0.6f), ShapeType shape = ShapeType::Cube, std::string n = "Part")
        : position(pos), size(size), color(color), rotation(glm::vec3(0.0f)), 
          transparency(0.0f), reflectance(0.0f), anchored(true), canCollide(true), mass(1.0f), 
          velocity(glm::vec3(0.0f)), physicsBody(nullptr), shape(shape), name(n), parentIndex(-1), isFolder(false), isCamera(false), isSpawn(false), textureId(0), material(Material::Plastic), texture(), deleted(false) {}
};
