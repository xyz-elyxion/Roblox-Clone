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

// Libraries
#include "HingeJointScene.h"
#include <cmath>

// Namespaces
using namespace openglframework;
using namespace hingejointscene;

// Constructor
HingeJointScene::HingeJointScene(const std::string& name, EngineSettings& settings, rblxphysics::PhysicsCommon& physicsCommon)
      : SceneDemo(name, settings, physicsCommon, true, SCENE_RADIUS) {

    // Compute the radius and the center of the scene
    openglframework::Vector3 center(0, 5, 0);

    // Set the center of the scene
    setScenePosition(center, SCENE_RADIUS);
    setInitZoom(1.1);
    resetCameraToViewAll();

    mWorldSettings.worldName = name;
}

// Destructor
HingeJointScene::~HingeJointScene() {

    destroyPhysicsWorld();
}

// Create the physics world
void HingeJointScene::createPhysicsWorld() {

    // Gravity vector in the physics world
    mWorldSettings.gravity = rblxphysics::Vector3(mEngineSettings.gravity.x, mEngineSettings.gravity.y, mEngineSettings.gravity.z);

    // Create the physics world for the physics simulation
    mPhysicsWorld = mPhysicsCommon.createPhysicsWorld(mWorldSettings);
    mPhysicsWorld->setEventListener(this);

    // Create the Hinge joint
    createHingeJoint();
}

// Initialize the bodies positions
void HingeJointScene::initBodiesPositions() {

    mBox1->setTransform(rblxphysics::Transform(rblxphysics::Vector3(0, 4, 0), rblxphysics::Quaternion::identity()));
    mBox2->setTransform(rblxphysics::Transform(rblxphysics::Vector3(4, 4, 0), rblxphysics::Quaternion::identity()));
}

// Destroy the physics world
void HingeJointScene::destroyPhysicsWorld() {

    if (mPhysicsWorld != nullptr) {

        delete mBox1;
        delete mBox2;

        mPhysicsObjects.clear();

        mPhysicsCommon.destroyPhysicsWorld(mPhysicsWorld);
        mPhysicsWorld = nullptr;
    }
}
// Reset the scene
void HingeJointScene::reset() {

    SceneDemo::reset();

    destroyPhysicsWorld();
    createPhysicsWorld();
    initBodiesPositions();
}

/// Create the hinge joint
void HingeJointScene::createHingeJoint() {

    // --------------- Create the boxes --------------- //

    // Create a box and a corresponding rigid in the physics world
    mBox1 = new Box(true, Vector3(4, 4, 4), mPhysicsCommon, mPhysicsWorld, mMeshFolderPath);
    mBox1->setTransform(rblxphysics::Transform(rblxphysics::Vector3(0, 4, 0), rblxphysics::Quaternion::identity()));
    mBox1->getRigidBody()->setType(rblxphysics::BodyType::STATIC);

    // Set the box color
    mBox1->setColor(mObjectColorDemo);
    mBox1->setSleepingColor(mSleepingColorDemo);

    // Change the material properties of the rigid body
    mPhysicsObjects.push_back(mBox1);

    // Create a box and a corresponding rigid in the physics world
    mBox2 = new Box(true, Vector3(4, 4, 4), mPhysicsCommon, mPhysicsWorld, mMeshFolderPath);
    mBox2->setTransform(rblxphysics::Transform(rblxphysics::Vector3(4, 4, 0), rblxphysics::Quaternion::identity()));

    // Set the box color
    mBox2->setColor(mObjectColorDemo);
    mBox2->setSleepingColor(mSleepingColorDemo);

    // Change the material properties of the rigid body
    mPhysicsObjects.push_back(mBox2);

    // --------------- Create the Hinge joint --------------- //

    // Create the joint info object
    rblxphysics::RigidBody* body1 = mBox1->getRigidBody();
    rblxphysics::RigidBody* body2 = mBox2->getRigidBody();
    const rblxphysics::Vector3& body1Position = body1->getTransform().getPosition();
    const rblxphysics::Vector3& body2Position = body2->getTransform().getPosition();
    const rblxphysics::Vector3 anchorPointWorldSpace = 0.5 * (body2Position + body1Position) - rblxphysics::Vector3(0, 2, 0);
    const rblxphysics::Vector3 hingeAxisWorldSpace(0, 0, 1);
    rblxphysics::HingeJointInfo jointInfo(body1, body2, anchorPointWorldSpace, hingeAxisWorldSpace);
    jointInfo.isCollisionEnabled = false;

    // Create the joint in the physics world
    mJoint = dynamic_cast<rblxphysics::HingeJoint*>(mPhysicsWorld->createJoint(jointInfo));
}
