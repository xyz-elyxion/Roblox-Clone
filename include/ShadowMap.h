#pragma once

#include <rblxgl/rblxgl.h>
#include <rblxmath/rblxmath.hpp>

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
    
    rblxmath::mat4 getLightSpaceMatrix(rblxmath::vec3 lightPos);
};
