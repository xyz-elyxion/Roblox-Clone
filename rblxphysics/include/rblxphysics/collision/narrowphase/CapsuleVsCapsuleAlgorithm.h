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

#ifndef RBLXPHYSICS_CAPSULE_VS_CAPSULE_ALGORITHM_H
#define	RBLXPHYSICS_CAPSULE_VS_CAPSULE_ALGORITHM_H

// Libraries
#include <rblxphysics/collision/narrowphase/NarrowPhaseAlgorithm.h>
#include <rblxphysics/configuration.h>

/// Namespace RblxPhysics
namespace rblxphysics {

// Declarations
struct NarrowPhaseInfoBatch;
class ContactPoint;

// Class CapsuleVsCapsuleAlgorithm
/**
 * This class is used to compute the narrow-phase collision detection
 * between two capsules collision shapes. We do not use the GJK or SAT
 * algorithm here. We directly compute the contact points and contact normal.
 * This is based on the "Robust Contact Creation for Physics Simulation"
 * presentation by Dirk Gregorius.
 */
class CapsuleVsCapsuleAlgorithm : public NarrowPhaseAlgorithm {

    protected :

    public :

        // -------------------- Methods -------------------- //

        /// Constructor
		CapsuleVsCapsuleAlgorithm() = default;

        /// Destructor
        virtual ~CapsuleVsCapsuleAlgorithm() override = default;

        /// Deleted copy-constructor
		CapsuleVsCapsuleAlgorithm(const CapsuleVsCapsuleAlgorithm& algorithm) = delete;

        /// Deleted assignment operator
		CapsuleVsCapsuleAlgorithm& operator=(const CapsuleVsCapsuleAlgorithm& algorithm) = delete;

        /// Compute the narrow-phase collision detection between two capsules
        bool testCollision(NarrowPhaseInfoBatch& narrowPhaseInfoBatch, uint32 batchStartIndex,
                           uint32 batchNbItems, MemoryAllocator& memoryAllocator);
};

}

#endif

