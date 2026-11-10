#pragma once

#include <rblxphysics/rblxphysics.h>
#include <vector>
#include <map>
#include "Part.h"

class PhysicsWorld {
public:
    PhysicsWorld();
    ~PhysicsWorld();

    void init();
    void update(float deltaTime);
    
    void addPart(Part* part);
    void removePart(Part* part);
    void updatePartBody(Part* part);
    void reset();

    // Raycasting
    struct RaycastResult {
        Part* part;
        rblxmath::vec3 hitPoint;
        rblxmath::vec3 hitNormal;
        float hitFraction;
    };
    
    bool raycast(const rblxmath::vec3& start, const rblxmath::vec3& end, RaycastResult& result, Part* ignorePart = nullptr);

    // Welding
    void createWeld(Part* partA, Part* partB);

// Make these public or provide accessors, but for now public for Server usage
    rblxphysics::PhysicsCommon physicsCommon;
    rblxphysics::PhysicsWorld* world;

private:
    std::map<Part*, rblxphysics::RigidBody*> bodyMap;
    std::map<Part*, rblxphysics::CollisionShape*> shapeMap;
    std::map<Part*, rblxphysics::PolyhedronMesh*> meshMap; // For Polyhedron shapes (Wedge)
    std::map<Part*, rblxphysics::PolygonVertexArray*> vertexArrayMap; // For Polyhedron shapes
    std::map<Part*, float*> vertexDataMap; // For Polyhedron shapes
    std::map<Part*, int*> indicesDataMap; // For Polyhedron shapes
    std::map<Part*, rblxphysics::PolygonVertexArray::PolygonFace*> facesDataMap; // For Polyhedron shapes
};
