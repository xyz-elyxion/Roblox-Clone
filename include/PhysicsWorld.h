#pragma once

#include <linearphysics/linearphysics.h>
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
        linearmath::vec3 hitPoint;
        linearmath::vec3 hitNormal;
        float hitFraction;
    };
    
    bool raycast(const linearmath::vec3& start, const linearmath::vec3& end, RaycastResult& result, Part* ignorePart = nullptr);

    // Welding
    void createWeld(Part* partA, Part* partB);

// Make these public or provide accessors, but for now public for Server usage
    linearphysics::PhysicsCommon physicsCommon;
    linearphysics::PhysicsWorld* world;

private:
    std::map<Part*, linearphysics::RigidBody*> bodyMap;
    std::map<Part*, linearphysics::CollisionShape*> shapeMap;
    std::map<Part*, linearphysics::PolyhedronMesh*> meshMap; // For Polyhedron shapes (Wedge)
    std::map<Part*, linearphysics::PolygonVertexArray*> vertexArrayMap; // For Polyhedron shapes
    std::map<Part*, float*> vertexDataMap; // For Polyhedron shapes
    std::map<Part*, int*> indicesDataMap; // For Polyhedron shapes
    std::map<Part*, linearphysics::PolygonVertexArray::PolygonFace*> facesDataMap; // For Polyhedron shapes
};
