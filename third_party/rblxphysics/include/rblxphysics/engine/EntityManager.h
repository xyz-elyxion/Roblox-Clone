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

#ifndef RBLXPHYSICS_ENTITY_MANAGER_H
#define RBLXPHYSICS_ENTITY_MANAGER_H

// Libraries
#include <rblxphysics/configuration.h>
#include <rblxphysics/containers/Array.h>
#include <rblxphysics/containers/Deque.h>
#include <rblxphysics/engine/Entity.h>

/// Namespace rblxphysics
namespace rblxphysics {

// Class EntityManager
/**
 * This class is responsible to manage the entities of the ECS.
 */
class EntityManager {

    private:

        // -------------------- Attributes -------------------- //

        /// Array storing the generations of the created entities
        Array<uint8> mGenerations;

        /// Deque with the indices of destroyed entities that can be reused
        Deque<uint32> mFreeIndices;

        // -------------------- Methods -------------------- //

    public:

        // -------------------- Methods -------------------- //

        /// Constructor
        EntityManager(MemoryAllocator& allocator);

        /// Create a new entity
        Entity createEntity();

        /// Destroy an entity
        void destroyEntity(Entity entity);

        /// Return true if the entity is still valid (not destroyed)
        bool isValid(Entity entity) const;
};

// Return true if the entity is still valid (not destroyed)
RBLXPHYSICS_FORCE_INLINE bool EntityManager::isValid(Entity entity) const {
    return mGenerations[entity.getIndex()] == entity.getGeneration();
}

}

#endif
