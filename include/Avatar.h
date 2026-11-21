#pragma once

#include <linearmath/linearmath.hpp>
#include <string>

#include "LinearGameApi.h"

LINEAR_GAME_API_BEGIN
struct LINEAR_GAME_API AvatarConfig {
    linearmath::vec3 headColor = linearmath::vec3(0.8f, 0.6f, 0.4f);      // Skin color
    linearmath::vec3 torsoColor = linearmath::vec3(0.2f, 0.4f, 0.8f);    // Blue shirt
    linearmath::vec3 leftArmColor = linearmath::vec3(0.8f, 0.6f, 0.4f);  // Skin color
    linearmath::vec3 rightArmColor = linearmath::vec3(0.8f, 0.6f, 0.4f); // Skin color
    linearmath::vec3 leftLegColor = linearmath::vec3(0.2f, 0.6f, 0.2f);  // Green pants
    linearmath::vec3 rightLegColor = linearmath::vec3(0.2f, 0.6f, 0.2f); // Green pants
    std::string faceId = "classic";
    // Equipped hat item id; empty when the character wears none.
    std::string hatId;
    
    void saveToFile(const std::string& filename = "avatar.txt");
    void loadFromFile(const std::string& filename = "avatar.txt");
};
LINEAR_GAME_API_END


