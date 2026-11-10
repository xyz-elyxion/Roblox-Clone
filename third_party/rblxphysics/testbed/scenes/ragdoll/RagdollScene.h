/********************************************************************************
* RblxPhysics physics library, http://www.rblxphysics.com                 *
* Copyright (c) 2010-2016 Daniel Chappuis                                       *
*********************************************************************************
*                                                                               *
* This software is provided 'as-is', without any express or implied warranty.   *
* In no event will the authors be held liable for any damages arising from the  *
* use of this software.                                                         *
*                                                                               *
* Permission is granted to anyone to use this software for any purpose,         *
* including commercial applications, and to alter it and redistribute it        *
* freely, subject to the following restrictions:                                *
*                                                                               *
* 1. The origin of this software must not be misrepresented; you must not claim *
*    that you wrote the original software. If you use this software in a        *
*    product, an acknowledgment in the product documentation would be           *
*    appreciated but is not required.                                           *
*                                                                               *
* 2. Altered source versions must be plainly marked as such, and must not be    *
*    misrepresented as being the original software.                             *
*                                                                               *
* 3. This notice may not be removed or altered from any source distribution.    *
*                                                                               *
********************************************************************************/

#ifndef RAGDOLL_SCENE_H
#define RAGDOLL_SCENE_H

// Libraries
#include "openglframework.h"
#include <rblxphysics/rblxphysics.h>
#include "Box.h"
#include "Capsule.h"
#include "Sphere.h"
#include "SceneDemo.h"

namespace ragdollscene {

// Constants
const float SCENE_RADIUS = 45.0f;
const int NB_RAGDOLLS_ROWS = 3;
const int NB_RAGDOLLS_COLS = 2;
const int NB_RAGDOLLS = NB_RAGDOLLS_ROWS * NB_RAGDOLLS_COLS;
const openglframework::Vector3 BOX_SIZE(2, 2, 2);           // Box dimensions in meters
const openglframework::Vector3 FLOOR_1_SIZE(52, 0.5f, 52);  // Floor dimensions in meters
const openglframework::Vector3 FLOOR_2_SIZE(60, 0.5f, 82);  // Floor dimensions in meters
const int NB_BALLSOCKETJOINT_BOXES = 7;                     // Number of Ball-And-Socket chain boxes
const int NB_HINGE_BOXES = 7;                               // Number of Hinge chain boxes

// Class RagdollScene
class RagdollScene : public SceneDemo {

    protected :

        // -------------------- Attributes -------------------- //

        /// Head sphere
        Sphere* mHeadBox[NB_RAGDOLLS];

        /// Chest
        Capsule* mChestCapsule[NB_RAGDOLLS];

        /// Waist capsule
        Capsule* mWaistCapsule[NB_RAGDOLLS];

        /// Hip capsule
        Capsule* mHipCapsule[NB_RAGDOLLS];

        /// Left upper arm capsule
        Capsule* mLeftUpperArmCapsule[NB_RAGDOLLS];

        /// Left lower arm capsule
        Capsule* mLeftLowerArmCapsule[NB_RAGDOLLS];

        /// Left upper leg capsule
        Capsule* mLeftUpperLegCapsule[NB_RAGDOLLS];

        /// Left lower leg capsule
        Capsule* mLeftLowerLegCapsule[NB_RAGDOLLS];

        /// Right upper arm capsule
        Capsule* mRightUpperArmCapsule[NB_RAGDOLLS];

        /// Right lower arm capsule
        Capsule* mRightLowerArmCapsule[NB_RAGDOLLS];

        /// Right upper leg capsule
        Capsule* mRightUpperLegCapsule[NB_RAGDOLLS];

        /// Right lower leg capsule
        Capsule* mRightLowerLegCapsule[NB_RAGDOLLS];

        /// Box for the floor 1
        Box* mFloor1;

        /// Box for the floor 2
        Box* mFloor2;

        /// Large box
        Box* mLargeBox;

        /// Inclined plane box
        Box* mInclinedPlaneBox;

        /// Ball-And-Socket joint between head and torso
        rblxphysics::BallAndSocketJoint* mHeadChestJoint[NB_RAGDOLLS];

        /// Ball-And-Socket joint between torso and left upper arm
        rblxphysics::BallAndSocketJoint* mChestLeftUpperArmJoint[NB_RAGDOLLS];

        /// Hinge joint between left upper and left lower arm
        rblxphysics::HingeJoint* mLeftUpperLeftLowerArmJoint[NB_RAGDOLLS];

        /// Fixed joint between chest and waist
        rblxphysics::FixedJoint* mChestWaistJoint[NB_RAGDOLLS];

        /// Fixed joint between waist and hips
        rblxphysics::FixedJoint* mWaistHipsJoint[NB_RAGDOLLS];

        /// Ball-And-Socket joint between torso and left upper leg
        rblxphysics::BallAndSocketJoint* mHipLeftUpperLegJoint[NB_RAGDOLLS];

        /// Hinge joint between left upper and left lower leg
        rblxphysics::HingeJoint* mLeftUpperLeftLowerLegJoint[NB_RAGDOLLS];

        /// Ball-And-Socket joint between torso and right upper arm
        rblxphysics::BallAndSocketJoint* mChestRightUpperArmJoint[NB_RAGDOLLS];

        /// Hinge joint between left upper and right lower arm
        rblxphysics::HingeJoint* mRightUpperRightLowerArmJoint[NB_RAGDOLLS];

        /// Ball-And-Socket joint between torso and right upper leg
        rblxphysics::BallAndSocketJoint* mHipRightUpperLegJoint[NB_RAGDOLLS];

        /// Hinge joint between left upper and left lower leg
        rblxphysics::HingeJoint* mRightUpperRightLowerLegJoint[NB_RAGDOLLS];

        rblxphysics::Vector3 mChestPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mWaistPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mHipPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mHeadPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mLeftUpperArmPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mLeftLowerArmPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mLeftUpperLegPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mLeftLowerLegPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mRightUpperArmPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mRightLowerArmPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mRightUpperLegPos[NB_RAGDOLLS];
        rblxphysics::Vector3 mRightLowerLegPos[NB_RAGDOLLS];

        /// World settings
        rblxphysics::PhysicsWorld::WorldSettings mWorldSettings;

        // -------------------- Methods -------------------- //

        /// Create the bodies and joints for the ragdoll
        void createRagdolls();

    public:

        // -------------------- Methods -------------------- //

        /// Constructor
        RagdollScene(const std::string& name, EngineSettings& settings, rblxphysics::PhysicsCommon& physicsCommon);

        /// Destructor
        virtual ~RagdollScene() override ;

        /// Reset the scene
        virtual void reset() override;

        /// Create the physics world
        void createPhysicsWorld();

        /// Destroy the physics world
        void destroyPhysicsWorld();

        /// Initialize the bodies positions
        void initBodiesPositions();
};

}

#endif
