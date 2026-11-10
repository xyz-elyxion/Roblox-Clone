#pragma once

#include "Shader.h"
#include "Part.h"
#include <map>
#include <string>
#include <vector>
#include <rblxmath/rblxmath.hpp>

class Renderer {
public:
    Renderer();
    ~Renderer();

    void init();
    void drawPart(const Part& part, const rblxmath::mat4& view, const rblxmath::mat4& projection, const rblxmath::mat4& lightSpaceMatrix, unsigned int shadowMap, unsigned int faceTexture = 0);
    void drawPartShadow(const Part& part, const rblxmath::mat4& lightSpaceMatrix);
    
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
};
