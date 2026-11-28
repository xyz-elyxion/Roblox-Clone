#pragma once

#include <linearmath/linearmath.hpp>
#include <string>
#include <vector>

enum class ShapeType {
    Cube,
    Sphere,
    Wedge,
    Cylinder
};

// How a part's surface is shaded. Plastic keeps the studded brick look; the
// others are flat-surface materials. Wood, grass, concrete and brick are drawn
// from an image in textures/ (see textureFile below); plastic, smooth plastic,
// neon and metal are a colour and a reflectance rather than a pattern. A world
// file names a material and needs no assets of its own either way.
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

namespace linear {
namespace material {

// The image a material's surface is drawn from: textures/material-<name>.png, the
// counterpart of a part's own texture. It is data the way a sound is: replacing
// the file re-skins every wall of that material with no rebuild.
//
// The four patterned materials have one; the rest return an empty name, because a
// smooth plastic, a neon part and a metal sheet are a colour and a reflectance
// rather than a pattern, and an image for them would be a file with nothing in it.
//
// The renderer multiplies the part's colour by the image, so it tints rather than
// replaces the colour a world picked.
inline const char* textureFile(Material material) {
    switch (material) {
        case Material::Wood: return "material-wood.png";
        case Material::Grass: return "material-grass.png";
        case Material::Concrete: return "material-concrete.png";
        case Material::Brick: return "material-brick.png";
        default: return "";
    }
}

}  // namespace material
}  // namespace linear

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

    linearmath::vec3 position;
    linearmath::vec3 size;
    linearmath::vec3 color;
    linearmath::vec3 rotation; // Euler angles in degrees
    float transparency;
    float reflectance;
    bool anchored;
    bool canCollide;
    float mass;
    linearmath::vec3 velocity;
    void* physicsBody; // Pointer to Bullet RigidBody
    ShapeType shape;
    std::vector<Weld> welds;
    unsigned int textureId; // Added for face texture
    Material material;      // Surface shading used by the part shader
    std::string texture;    // Optional image path, tiled across every face

    Part(linearmath::vec3 pos = linearmath::vec3(0.0f), linearmath::vec3 size = linearmath::vec3(1.0f), linearmath::vec3 color = linearmath::vec3(0.6f), ShapeType shape = ShapeType::Cube, std::string n = "Part")
        : position(pos), size(size), color(color), rotation(linearmath::vec3(0.0f)), 
          transparency(0.0f), reflectance(0.0f), anchored(true), canCollide(true), mass(1.0f), 
          velocity(linearmath::vec3(0.0f)), physicsBody(nullptr), shape(shape), name(n), parentIndex(-1), isFolder(false), isCamera(false), isSpawn(false), textureId(0), material(Material::Plastic), texture(), deleted(false) {}
};
