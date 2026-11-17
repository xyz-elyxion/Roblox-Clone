#pragma once

#include <lineargl/lineargl.h>
#include <vector>
#include <string>
#include <iostream>
#include "Shader.h"
#include <linearstb_image.h> // We need linearstb_image for loading textures

class Skybox {
public:
    Skybox();
    ~Skybox();
    void init();
    void draw(const linearmath::mat4& view, const linearmath::mat4& projection);
    unsigned int getTextureID() const { return cubemapTexture; }

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    Shader* shader;
    
    unsigned int loadCubemap(std::vector<std::string> faces);
};

