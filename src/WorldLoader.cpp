#include "WorldLoader.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <sstream>

namespace {
constexpr std::size_t kMaxParts = 4096;
// A part record is a few hundred bytes; anything far beyond that is not a world
// file we want to build geometry from.
constexpr std::size_t kMaxLineLength = 8192;

// A texture name comes from a world file anybody can publish, so it is held to
// plain relative image paths: no absolute paths and no parent traversal.
bool isSafeTexturePath(const std::string& path) {
    if (path.empty() || path.size() > 96) return false;
    if (path.find("..") != std::string::npos) return false;
    if (path.front() == '/' || path.front() == '\\') return false;
    for (const char c : path) {
        const bool allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
                             (c >= '0' && c <= '9') || c == '_' || c == '-' || c == '.' || c == '/';
        if (!allowed) return false;
    }
    return true;
}

bool isBlankLine(const std::string& line) {
    return line.find_first_not_of(" \t\r") == std::string::npos;
}
} // namespace

void WorldLoader::loadWorld(const std::string& filename, std::deque<Part>& parts, PhysicsWorld& physicsWorld,
                            WorldEnvironment* environment) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Failed to open world file: " << filename << std::endl;
        return;
    }
    
    physicsWorld.reset();
    parts.clear();
    
    // One part per line, so the optional trailing fields can be read without
    // guessing where a record ends.
    std::string headerLine;
    if (!std::getline(in, headerLine)) {
        std::cerr << "Malformed world file header: " << filename << std::endl;
        return;
    }

    std::size_t count = 0;
    glm::vec3 headerFogColor(0.0f);
    float headerFogDensity = 0.0f;
    {
        std::istringstream headerStream(headerLine);
        if (!(headerStream >> count)) {
            std::cerr << "Malformed world file header: " << filename << std::endl;
            return;
        }

        // Optional trailing fields on the header line: the fog colour and how
        // thick it is per stud. A world with no fog stops after the count, so
        // the old header keeps its old meaning.
        if (headerStream >> headerFogColor.x >> headerFogColor.y >> headerFogColor.z >>
                headerFogDensity) {
            if (std::isfinite(headerFogColor.x) && std::isfinite(headerFogColor.y) &&
                std::isfinite(headerFogColor.z) && std::isfinite(headerFogDensity)) {
                headerFogColor = glm::clamp(headerFogColor, glm::vec3(0.0f), glm::vec3(1.0f));
                headerFogDensity = std::clamp(headerFogDensity, 0.0f, 0.2f);
            } else {
                headerFogColor = glm::vec3(0.0f);
                headerFogDensity = 0.0f;
            }
        }
    }

    if (environment) {
        environment->fogColor = headerFogColor;
        environment->fogDensity = headerFogDensity;
    }

    if (count > kMaxParts) {
        std::cerr << "World file has too many parts: " << count << std::endl;
        return;
    }
    
    struct TempWeld {
        int partIndex;
        std::vector<int> targets;
    };
    std::vector<TempWeld> tempWelds;
    
    std::string line;
    for (std::size_t i = 0; i < count; i++) {
        do {
            if (!std::getline(in, line)) {
                std::cerr << "World file ended before part " << i << " of " << count << std::endl;
                parts.clear();
                physicsWorld.reset();
                return;
            }
        } while (isBlankLine(line));

        if (line.size() > kMaxLineLength) {
            std::cerr << "World part " << i << " has an implausibly long line" << std::endl;
            parts.clear();
            physicsWorld.reset();
            return;
        }

        std::istringstream stream(line);

        int shapeInt = 0;
        if (!(stream >> shapeInt) || shapeInt < 0 || shapeInt > static_cast<int>(ShapeType::Cylinder)) {
            std::cerr << "Malformed or unsupported part shape at index " << i << std::endl;
            parts.clear();
            physicsWorld.reset();
            return;
        }
        
        Part p;
        p.shape = (ShapeType)shapeInt;
        
        stream >> std::ws;
        if (stream.peek() == '"') {
            stream >> std::quoted(p.name);
        } else {
            stream >> p.name; 
        }
        
        if (!(stream >> p.parentIndex >> p.isFolder >> p.isCamera >> p.isSpawn)) {
            std::cerr << "Malformed hierarchy flags at part " << i << std::endl;
            parts.clear();
            physicsWorld.reset();
            return;
        }
        
        if (!(stream >> p.position.x >> p.position.y >> p.position.z
                     >> p.size.x >> p.size.y >> p.size.z
                     >> p.color.x >> p.color.y >> p.color.z
                     >> p.rotation.x >> p.rotation.y >> p.rotation.z
                     >> p.transparency >> p.reflectance
                     >> p.anchored >> p.canCollide)) {
            std::cerr << "Malformed transform/material data at part " << i << std::endl;
            parts.clear();
            physicsWorld.reset();
            return;
        }

        auto finiteVec3 = [](const glm::vec3& value) {
            return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
        };

        if (!finiteVec3(p.position) || !finiteVec3(p.size) || !finiteVec3(p.color) || !finiteVec3(p.rotation)) {
            std::cerr << "Non-finite values in world part " << i << std::endl;
            parts.clear();
            physicsWorld.reset();
            return;
        }

        p.size.x = std::clamp(p.size.x, 0.05f, 512.0f);
        p.size.y = std::clamp(p.size.y, 0.05f, 512.0f);
        p.size.z = std::clamp(p.size.z, 0.05f, 512.0f);
        p.color.x = std::clamp(p.color.x, 0.0f, 1.0f);
        p.color.y = std::clamp(p.color.y, 0.0f, 1.0f);
        p.color.z = std::clamp(p.color.z, 0.0f, 1.0f);
        p.transparency = std::clamp(p.transparency, 0.0f, 1.0f);
        p.reflectance = std::clamp(p.reflectance, 0.0f, 1.0f);
           
        std::size_t weldCount = 0;
        if (!(stream >> weldCount) || weldCount > kMaxParts) {
            std::cerr << "Malformed weld data at part " << i << std::endl;
            parts.clear();
            physicsWorld.reset();
            return;
        }
        
        if (weldCount > 0) {
            TempWeld tw;
            tw.partIndex = (int)i;
            for (std::size_t j = 0; j < weldCount; j++) {
                int targetIdx = -1;
                if (!(stream >> targetIdx)) {
                    std::cerr << "Malformed weld data at part " << i << std::endl;
                    parts.clear();
                    physicsWorld.reset();
                    return;
                }
                tw.targets.push_back(targetIdx);
            }
            tempWelds.push_back(tw);
        }

        // Optional trailing fields, written by newer Studio builds: a material
        // id and a quoted texture path. Older files end after the welds and older
        // readers stop there, so worlds keep loading in both directions.
        int materialId = 0;
        if (stream >> materialId) {
            if (materialId >= 0 && materialId < kMaterialCount) {
                p.material = static_cast<Material>(materialId);
            }
            stream >> std::ws;
            if (stream.peek() == '"') {
                std::string textureName;
                stream >> std::quoted(textureName);
                if (isSafeTexturePath(textureName)) {
                    p.texture = textureName;
                } else {
                    std::cerr << "Ignoring unsafe texture path on part " << i << std::endl;
                }
            }
        }
        
        parts.push_back(p);
        if (!p.isFolder) physicsWorld.addPart(&parts.back());
    }
    
    for (const auto& tw : tempWelds) {
        for (int target : tw.targets) {
            if (target >= 0 && target < static_cast<int>(parts.size())) {
                Weld w;
                w.part1 = &parts[target];
                parts[tw.partIndex].welds.push_back(w);
                if (parts[tw.partIndex].physicsBody && w.part1->physicsBody) {
                     physicsWorld.createWeld(&parts[tw.partIndex], w.part1);
                }
            }
        }
    }
    
    in.close();
}
