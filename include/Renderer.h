#pragma once

#include "Shader.h"
#include "Part.h"
#include <map>
#include <string>
#include <vector>
#include <linearmath/linearmath.hpp>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void drawPart(const Part& part, const linearmath::mat4& view, const linearmath::mat4& projection, const linearmath::mat4& lightSpaceMatrix, unsigned int shadowMap, unsigned int faceTexture = 0);
    void drawPartShadow(const Part& part, const linearmath::mat4& lightSpaceMatrix);
    
    Shader* shader; // Make public for main access to set lightPos
    Shader* depthShader;

private:
    unsigned int cubeVAO, cubeVBO;
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    unsigned int wedgeVAO, wedgeVBO;
    unsigned int sphereIndexCount;
    
    void initCubeMesh();
    void initSphereMesh();
    void initWedgeMesh();

    // Part textures are loaded on first use and kept for the life of the
    // renderer. A failed load is cached as 0 so a typo in a world file is logged
    // once instead of being retried every frame for every part using it.
    std::map<std::string, unsigned int> partTextures;
    unsigned int textureForPart(const std::string& path);

    // The four material surfaces, one image each, held once for every part that
    // uses that material. Keyed by the Material value rather than the file name
    // so a material with no image is cached too and costs one lookup per part.
    std::map<int, unsigned int> materialTextures;
    unsigned int textureForMaterial(Material material);

    // Reads an image from beside the program, textures/ or the two folders a
    // build/ checkout sits under, and uploads it as a repeating RGBA texture. 0
    // when it is not there.
    unsigned int loadTextureFile(const std::string& path, const char* kind);
};
