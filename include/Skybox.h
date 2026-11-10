#pragma once

#include <rblxgl/rblxgl.h>
#include <vector>
#include <string>
#include <iostream>
#include "Shader.h"
#include <rblxstb_image.h> // We need rblxstb_image for loading textures

class Skybox {
public:
    Skybox();
    ~Skybox();
    void init();
    void draw(const rblxmath::mat4& view, const rblxmath::mat4& projection);
    unsigned int getTextureID() const { return cubemapTexture; }

private:
    unsigned int skyboxVAO, skyboxVBO;
    unsigned int cubemapTexture;
    Shader* shader;
    
    unsigned int loadCubemap(std::vector<std::string> faces);
};

