#pragma once

#include <rblxmath/rblxmath.hpp>
#include <string>

struct AvatarConfig {
    rblxmath::vec3 headColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);      // Skin color
    rblxmath::vec3 torsoColor = rblxmath::vec3(0.2f, 0.4f, 0.8f);    // Blue shirt
    rblxmath::vec3 leftArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f);  // Skin color
    rblxmath::vec3 rightArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f); // Skin color
    rblxmath::vec3 leftLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f);  // Green pants
    rblxmath::vec3 rightLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f); // Green pants
    std::string faceId = "classic";
    // Equipped hat item id; empty when the character wears none.
    std::string hatId;
    
    void saveToFile(const std::string& filename = "avatar.txt");
    void loadFromFile(const std::string& filename = "avatar.txt");
};


