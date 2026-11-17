/********************************************************************************
* LinearPhysics physics library, http://www.linearphysics.com                 *
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
* LinearPhysics                                                                *
* Version 0.9.0                                                                 *
* http://www.linearphysics.com                                                 *
* Daniel Chappuis                                                               *
********************************************************************************/

#ifndef LINEARPHYSICS_H
#define LINEARPHYSICS_H

// Libraries
#include <linearphysics/configuration.h>
#include <linearphysics/mathematics/mathematics.h>
#include <linearphysics/body/CollisionBody.h>
#include <linearphysics/body/RigidBody.h>
#include <linearphysics/engine/PhysicsCommon.h>
#include <linearphysics/engine/PhysicsWorld.h>
#include <linearphysics/engine/Material.h>
#include <linearphysics/engine/EventListener.h>
#include <linearphysics/collision/shapes/CollisionShape.h>
#include <linearphysics/collision/shapes/BoxShape.h>
#include <linearphysics/collision/shapes/SphereShape.h>
#include <linearphysics/collision/shapes/CapsuleShape.h>
#include <linearphysics/collision/shapes/ConvexMeshShape.h>
#include <linearphysics/collision/shapes/ConcaveMeshShape.h>
#include <linearphysics/collision/shapes/HeightFieldShape.h>
#include <linearphysics/collision/PolyhedronMesh.h>
#include <linearphysics/collision/shapes/AABB.h>
#include <linearphysics/collision/Collider.h>
#include <linearphysics/collision/RaycastInfo.h>
#include <linearphysics/collision/TriangleMesh.h>
#include <linearphysics/collision/PolyhedronMesh.h>
#include <linearphysics/collision/TriangleVertexArray.h>
#include <linearphysics/collision/PolygonVertexArray.h>
#include <linearphysics/collision/CollisionCallback.h>
#include <linearphysics/collision/OverlapCallback.h>
#include <linearphysics/constraint/BallAndSocketJoint.h>
#include <linearphysics/constraint/SliderJoint.h>
#include <linearphysics/constraint/HingeJoint.h>
#include <linearphysics/constraint/FixedJoint.h>
#include <linearphysics/containers/Array.h>

// Altered source: upstream declared a short alias for this namespace on these two lines. The
// rename rewrote the alias target as well, which left `namespace linearphysics = linearphysics` — a
// namespace that aliases itself. MSVC rejects that outright (C2386) while GCC accepts it silently,
// so it only ever failed on Windows. Nothing in the tree used the short name, so the alias is
// dropped rather than given a fresh name that nothing references.

#endif
