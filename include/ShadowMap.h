#pragma once

#include <lineargl/lineargl.h>
#include <linearmath/linearmath.hpp>

class ShadowMap {
public:
    unsigned int depthMapFBO;
    unsigned int depthMap;
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;

    ShadowMap();
    ~ShadowMap();
    void init();
    void bindForWriting();
    void unbind();
    void bindForReading(unsigned int textureUnit);
    
    linearmath::mat4 getLightSpaceMatrix(linearmath::vec3 lightPos);
};
