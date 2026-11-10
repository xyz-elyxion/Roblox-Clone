#pragma once

#include <rblxmath/rblxmath.hpp>
#include <string>
#include <vector>
#include <deque>
#include "Part.h"
#include "PhysicsWorld.h"
#include "Camera.h"

class Character {
public:
    Character(rblxmath::vec3 startPos, PhysicsWorld* physicsWorld, std::deque<Part>* partsList, 
              const rblxmath::vec3& headColor = rblxmath::vec3(0.8f, 0.6f, 0.4f),
              const rblxmath::vec3& torsoColor = rblxmath::vec3(0.2f, 0.4f, 0.8f),
              const rblxmath::vec3& leftArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f),
              const rblxmath::vec3& rightArmColor = rblxmath::vec3(0.8f, 0.6f, 0.4f),
              const rblxmath::vec3& leftLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f),
              const rblxmath::vec3& rightLegColor = rblxmath::vec3(0.2f, 0.6f, 0.2f));
    ~Character();

    void update(float deltaTime, Camera* camera, bool isSprinting); // Camera pointer, optional
    
    // Remote Control
    void setRemoteState(rblxmath::vec3 pos, float yaw, bool walking, bool jumping);
    void setRemoteControlled(bool remote);
    bool isRemote;
    bool isJumping; // Added member

    // Shift lock: the character faces the camera and the camera sits over the
    // shoulder instead of directly behind the character.
    bool faceCameraYaw = false;
    float cameraShoulderOffset = 0.0f;

    void processInput(bool forward, bool backward, bool left, bool right, bool jump, const Camera& camera, float zoomInput, float deltaTime);

    rblxmath::vec3 getPosition() const;

    // Debug
    float debugTargetYaw;
    float debugCurrentYaw;
    rblxmath::vec3 debugMoveDir;

    // Health
    float health;
    float maxHealth;
    // Online, the game server owns health and respawns: the client must not run
    // its own death plane, or the two disagree about where the player is.
    bool serverControlsHealth = false;
    void takeDamage(float amount);
    void respawn();
    void setFaceTexture(unsigned int textureId);
    // Wears the hat with this item id, or takes the hat off for an empty id. The
    // hat is a posed part like the head, not a simulated body.
    void setHat(const std::string& hatId);

    // Moves the character to a world position and clears its velocity. Used by
    // the server when a script teleports a player (hazards, checkpoints).
    void teleportTo(const rblxmath::vec3& position);
    
    // Get the folder index for this character (for cleanup)
    int getFolderIndex() const;
    
private:
    PhysicsWorld* physicsWorld;
    std::deque<Part>* partsList;
    rblxmath::vec3 spawnPoint; 

    int headIndex;
    int torsoIndex;
    int leftArmIndex;
    int rightArmIndex;
    int leftLegIndex;
    int rightLegIndex;
    int hatIndex = -1;
    std::string hatId;

    // Animation State
    float walkTime;   // Walk cycle phase, kept while the walk blends out
    float walkBlend;  // 0 idle, 1 full walk cycle
    float airBlend;   // 0 grounded, 1 airborne pose
    float idleTime;
    bool isWalking;
    float zoomDistance;

    // Rotation State
    float internalYaw;
    bool hasReceivedFirstRotation; // Track if we've received first rotation update for remote characters

    // Physics
    int rootPartIndex;
};
