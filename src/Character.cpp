#include "Character.h"
#include <algorithm>
#include <cmath>
#include <iostream>

namespace {
    float normalizeDegrees(float angle) {
        if (!std::isfinite(angle)) {
            return 0.0f;
        }

        angle = std::fmod(angle, 360.0f);
        if (angle > 180.0f) angle -= 360.0f;
        if (angle < -180.0f) angle += 360.0f;
        return angle;
    }
}

Character::Character(glm::vec3 startPos, PhysicsWorld* physicsWorld, std::deque<Part>* partsList,
                     const glm::vec3& headColor, const glm::vec3& torsoColor,
                     const glm::vec3& leftArmColor, const glm::vec3& rightArmColor,
                     const glm::vec3& leftLegColor, const glm::vec3& rightLegColor)
    : isRemote(false), isJumping(false), debugTargetYaw(0.0f), debugCurrentYaw(0.0f),
      debugMoveDir(0.0f), health(100.0f), maxHealth(100.0f),
      physicsWorld(physicsWorld), partsList(partsList), spawnPoint(startPos),
      headIndex(-1), torsoIndex(-1), leftArmIndex(-1), rightArmIndex(-1),
      leftLegIndex(-1), rightLegIndex(-1), walkTime(0.0f), walkBlend(0.0f),
      airBlend(0.0f), idleTime(0.0f), isWalking(false),
      zoomDistance(15.0f), internalYaw(0.0f), hasReceivedFirstRotation(false),
      rootPartIndex(-1) {
    
    // Create Folder "Player"
    Part playerFolder;
    playerFolder.name = "Player";
    playerFolder.isFolder = true;
    playerFolder.anchored = true;
    playerFolder.canCollide = false;
    playerFolder.transparency = 1.0f;
    partsList->push_back(playerFolder);
    int folderIndex = partsList->size() - 1;

    // 1. RootPart / Torso (Main Physics Body)
    glm::vec3 rootSize(2.0f, 5.0f, 1.0f);
    Part rootPart(startPos + glm::vec3(0, 2.5f, 0), rootSize, glm::vec3(1,0,0)); 
    rootPart.name = "HumanoidRootPart";
    rootPart.parentIndex = folderIndex; 
    rootPart.anchored = false;
    rootPart.canCollide = true;
    rootPart.mass = 10.0f;
    rootPart.transparency = 1.0f; // Invisible
    rootPart.material = Material::SmoothPlastic;
    
    partsList->push_back(rootPart);
    rootPartIndex = partsList->size() - 1;
    physicsWorld->addPart(&partsList->back());
    
    if (partsList->back().physicsBody) {
        rp3d::RigidBody* body = (rp3d::RigidBody*)partsList->back().physicsBody;
        body->setAngularLockAxisFactor(rp3d::Vector3(0, 1, 0));
        if (body->getNbColliders() > 0) {
            body->getCollider(0)->getMaterial().setFrictionCoefficient(0.0f);
        }
    }
    
    // Torso (Visual)
    Part torso(startPos, glm::vec3(2.0f, 2.0f, 1.0f), torsoColor);
    torso.name = "Torso";
    torso.parentIndex = folderIndex; 
    torso.anchored = true;
    torso.canCollide = false;
    // Avatar limbs are smooth: studs on an arm read as a stack of bricks, not a
    // character.
    torso.material = Material::SmoothPlastic;
    partsList->push_back(torso);
    torsoIndex = partsList->size() - 1;

    auto createLimb = [&](std::string name, glm::vec3 size, glm::vec3 col) -> int {
        Part p(startPos, size, col); 
        p.name = name;
        p.parentIndex = folderIndex; 
        p.anchored = true; 
        p.canCollide = false; 
        p.material = Material::SmoothPlastic;
        partsList->push_back(p);
        return partsList->size() - 1;
    };

    headIndex = createLimb("Head", glm::vec3(1.2f, 1.2f, 1.2f), headColor);
    leftArmIndex = createLimb("LeftArm", glm::vec3(1.0f, 2.0f, 1.0f), leftArmColor);
    rightArmIndex = createLimb("RightArm", glm::vec3(1.0f, 2.0f, 1.0f), rightArmColor);
    leftLegIndex = createLimb("LeftLeg", glm::vec3(1.0f, 2.0f, 1.0f), leftLegColor);
    rightLegIndex = createLimb("RightLeg", glm::vec3(1.0f, 2.0f, 1.0f), rightLegColor);
}

Character::~Character() {
}

int Character::getFolderIndex() const {
    if (!partsList || rootPartIndex < 0 || rootPartIndex >= partsList->size()) {
        return -1;
    }
    const Part& root = (*partsList)[rootPartIndex];
    return root.parentIndex; // The folder is the parent of the root part
}

void Character::teleportTo(const glm::vec3& position) {
    if (!partsList || rootPartIndex < 0 || rootPartIndex >= static_cast<int>(partsList->size())) {
        return;
    }

    Part& root = (*partsList)[rootPartIndex];
    if (root.deleted) return;
    root.position = position;

    if (root.physicsBody == nullptr) return;
    auto* body = static_cast<rp3d::RigidBody*>(root.physicsBody);
    rp3d::Transform transform = body->getTransform();
    transform.setPosition(rp3d::Vector3(position.x, position.y, position.z));
    body->setTransform(transform);
    body->setLinearVelocity(rp3d::Vector3(0.0f, 0.0f, 0.0f));
    body->setAngularVelocity(rp3d::Vector3(0.0f, 0.0f, 0.0f));
}

void Character::setFaceTexture(unsigned int textureId) {
    if (!partsList || headIndex < 0 || headIndex >= static_cast<int>(partsList->size())) {
        return;
    }
    (*partsList)[headIndex].textureId = textureId;
}

void Character::setRemoteControlled(bool remote) {
    isRemote = remote;
    if (!partsList || rootPartIndex < 0 || rootPartIndex >= static_cast<int>(partsList->size())) {
        return;
    }

    Part& root = (*partsList)[rootPartIndex];
    if (root.deleted || !root.physicsBody) {
        return;
    }

    rp3d::RigidBody* body = static_cast<rp3d::RigidBody*>(root.physicsBody);
    if (remote) {
        root.canCollide = false;
        body->setType(rp3d::BodyType::KINEMATIC);
        body->setLinearVelocity(rp3d::Vector3(0, 0, 0));
        body->setAngularVelocity(rp3d::Vector3(0, 0, 0));
        if (body->getNbColliders() > 0) {
            body->getCollider(0)->setCollideWithMaskBits(0);
        }
        return;
    }

    root.canCollide = true;
    body->setType(root.anchored ? rp3d::BodyType::STATIC : rp3d::BodyType::DYNAMIC);
    if (body->getNbColliders() > 0) {
        body->getCollider(0)->setCollideWithMaskBits(0xFFFF);
    }
}

void Character::setRemoteState(glm::vec3 pos, float yaw, bool walking, bool jumping) {
    if (!isRemote) return;
    if (partsList && rootPartIndex >= 0 && rootPartIndex < partsList->size()) {
        Part& root = (*partsList)[rootPartIndex];
        if (root.deleted) return; // Character is deleted, don't update
        
        // Very fast interpolation for local testing (low latency)
        float lerpFactor = 0.8f; // 80% per update for smooth, responsive movement
        root.position = glm::mix(root.position, pos, lerpFactor);
        
        float targetYaw = normalizeDegrees(yaw);
        
        // On first rotation update, snap immediately to avoid wrong angle
        if (!hasReceivedFirstRotation) {
            internalYaw = targetYaw;
            root.rotation.y = targetYaw;
            hasReceivedFirstRotation = true;
        } else {
            // Use internalYaw for smooth interpolation (matches local character behavior)
            float currentYaw = normalizeDegrees(internalYaw);
            float diff = normalizeDegrees(targetYaw - currentYaw);
            
            // Update internalYaw (matches local character)
            internalYaw = normalizeDegrees(currentYaw + diff * lerpFactor);
            
            // Set root rotation to match internalYaw (matches local character)
            root.rotation.y = internalYaw;
        }
        
        // Update debug values for remote characters
        debugCurrentYaw = internalYaw;
        debugTargetYaw = targetYaw;
        debugMoveDir = glm::vec3(0); // Remote characters don't have move dir
        
        isWalking = walking;
        isJumping = jumping;
        
        // Update Physics Body Transform smoothly
        rp3d::RigidBody* body = (rp3d::RigidBody*)root.physicsBody;
        if (body) {
            rp3d::Transform t = body->getTransform();
            rp3d::Vector3 currentPos = t.getPosition();
            rp3d::Vector3 targetPos(pos.x, pos.y, pos.z);
            rp3d::Vector3 lerpedPos = currentPos + (targetPos - currentPos) * lerpFactor;
            t.setPosition(lerpedPos);
            
            // Smooth rotation with quaternion slerp
            rp3d::Quaternion currentQ = t.getOrientation();
            rp3d::Quaternion targetQ = rp3d::Quaternion::fromEulerAngles(0, glm::radians(root.rotation.y), 0);
            rp3d::Quaternion lerpedQ = rp3d::Quaternion::slerp(currentQ, targetQ, lerpFactor);
            t.setOrientation(lerpedQ);
            
            body->setTransform(t);
            // Lightly dampen velocity to allow some physics feel
            rp3d::Vector3 vel = body->getLinearVelocity();
            body->setLinearVelocity(vel * 0.5f);
            body->setAngularVelocity(body->getAngularVelocity() * 0.5f);
        }
    }
}

void Character::update(float deltaTime, Camera* camera, bool isSprinting) {
    if (!partsList || rootPartIndex < 0 || rootPartIndex >= partsList->size()) return;
    Part& root = (*partsList)[rootPartIndex];
    if (root.deleted) return; // Character is deleted
    rp3d::RigidBody* body = (rp3d::RigidBody*)root.physicsBody;
    if (!body) return; // Safety check: body might be invalid

    // For remote characters, ensure root.rotation.y matches internalYaw (like local characters)
    if (isRemote) {
        root.rotation.y = internalYaw;
    }

    // 1. Local Physics Logic
    if (!isRemote && body) {
        rp3d::Vector3 vel = body->getLinearVelocity();
        if (vel.y < -0.1f) {
            body->applyWorldForceAtCenterOfMass(rp3d::Vector3(0, -2000.0f * deltaTime, 0)); 
        }
        
        if (root.position.y < -185.0f) {
            // Offline this character is the only authority there is. Online the
            // server applies the same death plane itself and decides when to
            // respawn, so respawning here as well would fight its position.
            if (!serverControlsHealth) {
                takeDamage(100.0f);
            }
            return;
        }
        
        // Determine Animation State from Physics
        float speed = glm::length(glm::vec2(vel.x, vel.z));
        isWalking = speed > 0.1f;
        isJumping = std::abs(vel.y) > 0.5f;
    }

    // 2. Animation
    // The avatar is a root plus five boxes, so the pose is sold with blending
    // rather than a joint chain: one walk phase drives the swing, and separate
    // weights ease the walk and the airborne pose in and out. Snapping the swing
    // to zero the moment a key was released is what made movement look like a
    // sliding toy.
    glm::vec3 rootPos = root.position;
    glm::vec3 rootRot = root.rotation;

    float horizontalSpeed = 0.0f;
    if (body) {
        rp3d::Vector3 velocity = body->getLinearVelocity();
        horizontalSpeed = glm::length(glm::vec2(velocity.x, velocity.z));
    }

    const float walkTarget = isWalking ? 1.0f : 0.0f;
    const float airTarget = isJumping ? 1.0f : 0.0f;
    walkBlend += (walkTarget - walkBlend) * std::min(1.0f, deltaTime * 9.0f);
    airBlend += (airTarget - airBlend) * std::min(1.0f, deltaTime * 7.0f);
    if (walkBlend < 0.002f) walkBlend = walkTarget;
    if (airBlend < 0.002f) airBlend = airTarget;
    idleTime += deltaTime;

    // Faster legs turn faster, but the phase keeps its value while the walk
    // blends out, so the character eases to a stop instead of resetting mid-stride.
    const float strideRate = std::clamp(horizontalSpeed * 0.62f, 6.0f, 13.0f);
    if (walkBlend > 0.002f) {
        walkTime += deltaTime * strideRate * walkBlend;
    }

    const float swing = std::sin(walkTime) * walkBlend;
    const float idleSway = std::sin(idleTime * 1.6f);
    const float breathe = std::sin(idleTime * 1.9f) * 0.035f;

    // Airborne pose: arms up and out, legs tucked, instead of the old 180° that
    // put both arms straight over the head.
    const float airArmPitch = -135.0f;
    const float airLegPitch = -26.0f;
    float leftArmPitch = swing * 34.0f;
    float rightArmPitch = -swing * 34.0f;
    float leftLegPitch = -swing * 40.0f;
    float rightLegPitch = swing * 40.0f;
    leftArmPitch = glm::mix(leftArmPitch, airArmPitch, airBlend);
    rightArmPitch = glm::mix(rightArmPitch, airArmPitch, airBlend);
    leftLegPitch = glm::mix(leftLegPitch, airLegPitch, airBlend);
    rightLegPitch = glm::mix(rightLegPitch, airLegPitch, airBlend);

    // A slow sway at rest, small enough that it never fights the walk cycle.
    const float idleArm = idleSway * 2.2f * (1.0f - walkBlend) * (1.0f - airBlend);
    leftArmPitch += idleArm;
    rightArmPitch -= idleArm;

    auto yawed = [&](const glm::vec3& local) {
        const float yaw = glm::radians(rootRot.y);
        const float c = std::cos(yaw);
        const float s = std::sin(yaw);
        return glm::vec3(local.x * c + local.z * s, local.y, -local.x * s + local.z * c);
    };

    auto placeDirect = [&](int index, const glm::vec3& localCenter, float pitchDeg,
                           float twistDeg, float rollDeg) {
        if (index < 0 || index >= static_cast<int>(partsList->size())) return;
        Part& p = (*partsList)[index];
        if (p.deleted) return;
        p.position = rootPos + yawed(localCenter);
        p.rotation = glm::vec3(pitchDeg, rootRot.y + twistDeg, rollDeg);
    };

    // Swings a limb about its shoulder or hip rather than its centre, so the far
    // end travels like an arm and the top stays joined to the torso.
    auto placeSwinging = [&](int index, const glm::vec3& pivot, const glm::vec3& limbSize,
                             float pitchDeg, float rollDeg) {
        const float pitch = glm::radians(pitchDeg);
        const float roll = glm::radians(rollDeg);
        const glm::vec3 offset(0.0f, -limbSize.y * 0.5f, 0.0f);

        // Roll first (splay), then pitch (forward and back), both about the pivot.
        const glm::vec3 rolled(offset.x * std::cos(roll) - offset.y * std::sin(roll),
                               offset.x * std::sin(roll) + offset.y * std::cos(roll),
                               offset.z);
        const glm::vec3 swung(rolled.x,
                              rolled.y * std::cos(pitch) - rolled.z * std::sin(pitch),
                              rolled.y * std::sin(pitch) + rolled.z * std::cos(pitch));
        placeDirect(index, pivot + swung, pitchDeg, 0.0f, rollDeg);
    };

    // Torso and head: a small counter-twist to the arm swing, plus breathing.
    const float counterTwist = -swing * 5.0f;
    placeDirect(torsoIndex, glm::vec3(0.0f, 0.5f + breathe, 0.0f), 0.0f, counterTwist, 0.0f);
    placeDirect(headIndex, glm::vec3(0.0f, 2.1f + breathe * 0.6f, 0.0f),
                airBlend * -6.0f + swing * 2.5f, counterTwist * 0.4f, 0.0f);

    placeSwinging(leftArmIndex, glm::vec3(-1.5f, 1.5f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f),
                  leftArmPitch, -airBlend * 12.0f);
    placeSwinging(rightArmIndex, glm::vec3(1.5f, 1.5f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f),
                  rightArmPitch, airBlend * 12.0f);
    placeSwinging(leftLegIndex, glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f),
                  leftLegPitch, -airBlend * 6.0f);
    placeSwinging(rightLegIndex, glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(1.0f, 2.0f, 1.0f),
                  rightLegPitch, airBlend * 6.0f);
    
    // Update Camera (Local only)
    if (!isRemote && camera) {
        float distance = zoomDistance;
        float height = 3.0f; 
        // Calculate camera position behind character
        float yawRad = glm::radians(camera->Yaw);
        float pitchRad = glm::radians(camera->Pitch);
        
        glm::vec3 offset;
        offset.x = cos(yawRad) * cos(pitchRad);
        offset.y = sin(pitchRad);
        offset.z = sin(yawRad) * cos(pitchRad);
        offset = glm::normalize(offset) * distance; // Direction from character to camera if reversed, or camera to character?
        // Camera Front points TO character usually? No, Front points AWAY from viewer.
        // Let's stick to standard orbital logic:
        // Position = Target - (Front * dist)
        
        // Ensure Camera Front is updated by Mouse Input (it is)
        // So just set position relative to Root
        
        camera->Position = rootPos - (camera->Front * distance);
        camera->Position.y += height; 

        // Shift lock pulls the camera over the shoulder so the character does not
        // block the middle of the screen.
        if (cameraShoulderOffset != 0.0f) {
            camera->Position += camera->Right * cameraShoulderOffset;
        }
    }
}

void Character::processInput(bool forward, bool backward, bool left, bool right, bool jump, const Camera& camera, float zoomInput, float deltaTime) {
    if (isRemote) return; // Remote characters don't process input
    if (!partsList || rootPartIndex < 0 || rootPartIndex >= partsList->size()) return;
    Part& root = (*partsList)[rootPartIndex];
    if (root.deleted) return; // Character is deleted
    rp3d::RigidBody* body = (rp3d::RigidBody*)root.physicsBody;
    if (!body) return;
    
    zoomDistance -= zoomInput * 2.0f;
    if (zoomDistance < 2.0f) zoomDistance = 2.0f;
    if (zoomDistance > 50.0f) zoomDistance = 50.0f;

    float moveSpeed = 20.0f;
    
    float yawRad = glm::radians(camera.Yaw);
    glm::vec3 camFront;
    camFront.x = cos(yawRad) * cos(glm::radians(0.0f)); 
    camFront.y = 0.0f;
    camFront.z = sin(yawRad) * cos(glm::radians(0.0f));
    camFront = glm::normalize(camFront);
    
    glm::vec3 camRight = glm::normalize(glm::cross(camFront, glm::vec3(0, 1, 0)));

    glm::vec3 moveDir(0.0f);
    if (forward) moveDir += camFront;
    if (backward) moveDir -= camFront;
    if (right) moveDir += camRight;
    if (left) moveDir -= camRight;
    
    if (glm::length(moveDir) > 0.1f) {
        moveDir = glm::normalize(moveDir);
        body->setIsActive(true);

        rp3d::Vector3 currentVel = body->getLinearVelocity();
        rp3d::Vector3 targetVel(moveDir.x * moveSpeed, currentVel.y, moveDir.z * moveSpeed);
        body->setLinearVelocity(targetVel);
        
        // Calculate target yaw from movement direction
        // atan2(x, z) gives angle in XZ plane, convert to degrees
        float angleRad = atan2(moveDir.x, moveDir.z);
        float targetYaw = normalizeDegrees(glm::degrees(angleRad));
        float currentYaw = normalizeDegrees(internalYaw);
        
        debugTargetYaw = targetYaw;
        debugCurrentYaw = currentYaw;
        debugMoveDir = moveDir;

        // Fast rotation towards movement direction
        float diff = normalizeDegrees(targetYaw - currentYaw);
        
        // Faster rotation for more responsive feel
        float lerpFactor = 15.0f; 
        float step = diff * lerpFactor * deltaTime; 
        
        internalYaw = normalizeDegrees(internalYaw + step);

        body->setAngularVelocity(rp3d::Vector3(0, 0, 0));

    } else {
        rp3d::Vector3 currentVel = body->getLinearVelocity();
        body->setLinearVelocity(rp3d::Vector3(0, currentVel.y, 0));
        body->setAngularVelocity(rp3d::Vector3(0, 0, 0));
        debugCurrentYaw = internalYaw;
        debugTargetYaw = internalYaw;
        debugMoveDir = glm::vec3(0.0f); // Clear move dir to stop animation state
    }
    
    // Shift lock keeps the character facing where the camera looks, even while
    // strafing, instead of turning towards the movement direction.
    if (faceCameraYaw) {
        const float cameraYaw = normalizeDegrees(glm::degrees(atan2(camFront.x, camFront.z)));
        const float currentYaw = normalizeDegrees(internalYaw);
        const float diff = normalizeDegrees(cameraYaw - currentYaw);
        internalYaw = normalizeDegrees(internalYaw + diff * 15.0f * deltaTime);
        debugTargetYaw = cameraYaw;
        debugCurrentYaw = internalYaw;
    }

    // Always use internalYaw (which smoothly interpolates to target when moving)
    root.rotation.y = internalYaw;
    
    rp3d::Transform t = body->getTransform();
    rp3d::Quaternion q = rp3d::Quaternion::fromEulerAngles(0, glm::radians(internalYaw), 0); 
    t.setOrientation(q);
    body->setTransform(t);
    
    if (jump) {
        rp3d::Vector3 vel = body->getLinearVelocity();
        if (std::abs(vel.y) < 0.1f) {
            body->setIsActive(true);
            body->setLinearVelocity(rp3d::Vector3(vel.x, 12.0f, vel.z)); 
        }
    }
}

glm::vec3 Character::getPosition() const {
    if (partsList && rootPartIndex >= 0 && rootPartIndex < partsList->size()) {
        const Part& root = (*partsList)[rootPartIndex];
        if (!root.deleted) {
            return root.position;
        }
    }
    return glm::vec3(0);
}

void Character::takeDamage(float amount) {
    health -= amount;
    if (health <= 0.0f) {
        respawn();
    }
}

void Character::respawn() {
    health = maxHealth;
    if (partsList && rootPartIndex >= 0 && rootPartIndex < partsList->size()) {
        Part& root = (*partsList)[rootPartIndex];
        if (root.deleted) return; // Character is deleted
        
        // Reset Remote State as well
        if (isRemote) {
            root.position = spawnPoint;
            return;
        }
        
        rp3d::RigidBody* body = (rp3d::RigidBody*)root.physicsBody;
        if (body) {
            rp3d::Transform t = body->getTransform();
            t.setPosition(rp3d::Vector3(spawnPoint.x, spawnPoint.y + 2.5f, spawnPoint.z)); 
            body->setTransform(t);
            body->setLinearVelocity(rp3d::Vector3(0, 0, 0));
            body->setAngularVelocity(rp3d::Vector3(0, 0, 0));
        }
    }
}
