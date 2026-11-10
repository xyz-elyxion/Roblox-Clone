/********************************************************************************
* RblxPhysics physics library, http://www.rblxphysics.com                 *
* Copyright (c) 2010-2022 Daniel Chappuis                                       *
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


/********************************************************************************
* RblxPhysics                                                                *
* Version 0.9.0                                                                 *
* http://www.rblxphysics.com                                                 *
* Daniel Chappuis                                                               *
********************************************************************************/

#ifndef RBLXPHYSICS_H
#define RBLXPHYSICS_H

// Libraries
#include <rblxphysics/configuration.h>
#include <rblxphysics/mathematics/mathematics.h>
#include <rblxphysics/body/CollisionBody.h>
#include <rblxphysics/body/RigidBody.h>
#include <rblxphysics/engine/PhysicsCommon.h>
#include <rblxphysics/engine/PhysicsWorld.h>
#include <rblxphysics/engine/Material.h>
#include <rblxphysics/engine/EventListener.h>
#include <rblxphysics/collision/shapes/CollisionShape.h>
#include <rblxphysics/collision/shapes/BoxShape.h>
#include <rblxphysics/collision/shapes/SphereShape.h>
#include <rblxphysics/collision/shapes/CapsuleShape.h>
#include <rblxphysics/collision/shapes/ConvexMeshShape.h>
#include <rblxphysics/collision/shapes/ConcaveMeshShape.h>
#include <rblxphysics/collision/shapes/HeightFieldShape.h>
#include <rblxphysics/collision/PolyhedronMesh.h>
#include <rblxphysics/collision/shapes/AABB.h>
#include <rblxphysics/collision/Collider.h>
#include <rblxphysics/collision/RaycastInfo.h>
#include <rblxphysics/collision/TriangleMesh.h>
#include <rblxphysics/collision/PolyhedronMesh.h>
#include <rblxphysics/collision/TriangleVertexArray.h>
#include <rblxphysics/collision/PolygonVertexArray.h>
#include <rblxphysics/collision/CollisionCallback.h>
#include <rblxphysics/collision/OverlapCallback.h>
#include <rblxphysics/constraint/BallAndSocketJoint.h>
#include <rblxphysics/constraint/SliderJoint.h>
#include <rblxphysics/constraint/HingeJoint.h>
#include <rblxphysics/constraint/FixedJoint.h>
#include <rblxphysics/containers/Array.h>

/// Alias to the RblxPhysics namespace
namespace rblxphysics = rblxphysics;

#endif
