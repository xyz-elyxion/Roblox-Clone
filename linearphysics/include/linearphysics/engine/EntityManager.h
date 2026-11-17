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

#ifndef LINEARPHYSICS_ENTITY_MANAGER_H
#define LINEARPHYSICS_ENTITY_MANAGER_H

// Libraries
#include <linearphysics/configuration.h>
#include <linearphysics/containers/Array.h>
#include <linearphysics/containers/Deque.h>
#include <linearphysics/engine/Entity.h>

/// Namespace linearphysics
namespace linearphysics {

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
LINEARPHYSICS_FORCE_INLINE bool EntityManager::isValid(Entity entity) const {
    return mGenerations[entity.getIndex()] == entity.getGeneration();
}

}

#endif
