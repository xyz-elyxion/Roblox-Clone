#include "PhysicsWorld.h"
#include <algorithm>
#include <iostream>
#include <rblxmath/gtc/quaternion.hpp>
#include <rblxmath/gtx/quaternion.hpp>

PhysicsWorld::PhysicsWorld() : world(nullptr) {}

PhysicsWorld::~PhysicsWorld() {
    if (world) {
        physicsCommon.destroyPhysicsWorld(world);
    }
}

void PhysicsWorld::init() {
    rblxphysics::PhysicsWorld::WorldSettings settings;
    settings.gravity = rblxphysics::Vector3(0, -9.81f * 2.0f, 0); 
    settings.defaultVelocitySolverNbIterations = 10;
    settings.defaultPositionSolverNbIterations = 5;
    
    world = physicsCommon.createPhysicsWorld(settings);
}

void PhysicsWorld::addPart(Part* part) {
    if (part->isFolder) return; // Do not create physics body for folders

    if (bodyMap.find(part) != bodyMap.end()) return;

    rblxphysics::Vector3 pos(part->position.x, part->position.y, part->position.z);
    
    rblxmath::quat q = rblxmath::quat(rblxmath::radians(part->rotation)); 
    rblxphysics::Quaternion orientation(q.x, q.y, q.z, q.w);
    
    rblxphysics::Transform transform(pos, orientation);
    
    rblxphysics::RigidBody* body = world->createRigidBody(transform);
    
    if (part->anchored) {
        body->setType(rblxphysics::BodyType::STATIC);
    } else {
        body->setType(rblxphysics::BodyType::DYNAMIC);
    }
    
    rblxphysics::CollisionShape* shape = nullptr;

    if (part->shape == ShapeType::Cube) {
        rblxphysics::Vector3 halfExtents(part->size.x * 0.5f, part->size.y * 0.5f, part->size.z * 0.5f);
        shape = physicsCommon.createBoxShape(halfExtents);
    } 
    else if (part->shape == ShapeType::Sphere) {
        float radius = std::max(std::max(part->size.x, part->size.y), part->size.z) * 0.5f;
        shape = physicsCommon.createSphereShape(radius);
    }
    else if (part->shape == ShapeType::Wedge) {
        float hx = part->size.x * 0.5f;
        float hy = part->size.y * 0.5f;
        float hz = part->size.z * 0.5f;

        int nbVertices = 6;
        float* vertices = new float[nbVertices * 3] {
            -hx, -hy,  hz, // 0
             hx, -hy,  hz, // 1
             hx, -hy, -hz, // 2
            -hx, -hy, -hz, // 3
            -hx,  hy, -hz, // 4
             hx,  hy, -hz  // 5
        };

        // Indices (Flat array of vertex indices for all faces)
        int* indices = new int[18] {
            3, 2, 1, 0, // Bottom (4)
            3, 4, 5, 2, // Back (4) - Corrected
            0, 4, 3,    // Left (3) - Corrected
            1, 2, 5,    // Right (3)
            0, 1, 5, 4  // Slope (4)
        };
        
        // Polygon Faces
        int nbFaces = 5;
        rblxphysics::PolygonVertexArray::PolygonFace* polygonFaces = new rblxphysics::PolygonVertexArray::PolygonFace[nbFaces];
        
        // Face 0: Bottom
        polygonFaces[0].nbVertices = 4;
        polygonFaces[0].indexBase = 0;
        
        // Face 1: Back
        polygonFaces[1].nbVertices = 4;
        polygonFaces[1].indexBase = 4;
        
        // Face 2: Left
        polygonFaces[2].nbVertices = 3;
        polygonFaces[2].indexBase = 8;
        
        // Face 3: Right
        polygonFaces[3].nbVertices = 3;
        polygonFaces[3].indexBase = 11;
        
        // Face 4: Slope
        polygonFaces[4].nbVertices = 4;
        polygonFaces[4].indexBase = 14;

        rblxphysics::PolygonVertexArray::VertexDataType vertexType = rblxphysics::PolygonVertexArray::VertexDataType::VERTEX_FLOAT_TYPE;
        rblxphysics::PolygonVertexArray::IndexDataType indexType = rblxphysics::PolygonVertexArray::IndexDataType::INDEX_INTEGER_TYPE;
        
        rblxphysics::PolygonVertexArray* vertexArray = new rblxphysics::PolygonVertexArray(
            (uint32_t)nbVertices, (const void*)vertices, (uint32_t)(3 * sizeof(float)), 
            (const void*)indices, (uint32_t)sizeof(int), 
            (uint32_t)nbFaces, polygonFaces, 
            vertexType, indexType
        );
        
        rblxphysics::PolyhedronMesh* mesh = physicsCommon.createPolyhedronMesh(vertexArray);
        
        if (mesh) {
             shape = physicsCommon.createConvexMeshShape(mesh);
             meshMap[part] = mesh;
             vertexArrayMap[part] = vertexArray;
             vertexDataMap[part] = vertices;
             indicesDataMap[part] = indices;
             facesDataMap[part] = polygonFaces;
        }
    }
    
    if (shape) {
        rblxphysics::Collider* collider = body->addCollider(shape, rblxphysics::Transform::identity());
        
        rblxphysics::Material& material = collider->getMaterial();
        material.setBounciness(0.1f); 
        material.setFrictionCoefficient(0.5f); 
        
        // Set collision filtering based on canCollide
        if (!part->canCollide) {
            // Set collideWithMaskBits to 0 to disable all collisions
            collider->setCollideWithMaskBits(0);
        } else {
            // Default: collide with everything (0xFFFF = all bits set)
            collider->setCollideWithMaskBits(0xFFFF);
        }
        
        bodyMap[part] = body;
        shapeMap[part] = shape;
        part->physicsBody = body; // Update Part's pointer
    }
}

void PhysicsWorld::removePart(Part* part) {
    if (bodyMap.find(part) != bodyMap.end()) {
        part->physicsBody = nullptr; // Clear Part's pointer
        world->destroyRigidBody(bodyMap[part]);
        
        rblxphysics::CollisionShape* shape = shapeMap[part];
        if (part->shape == ShapeType::Cube) {
             physicsCommon.destroyBoxShape(dynamic_cast<rblxphysics::BoxShape*>(shape));
        }
        else if (part->shape == ShapeType::Sphere) {
             physicsCommon.destroySphereShape(dynamic_cast<rblxphysics::SphereShape*>(shape));
        }
        else if (part->shape == ShapeType::Wedge) {
             physicsCommon.destroyConvexMeshShape(dynamic_cast<rblxphysics::ConvexMeshShape*>(shape));
             physicsCommon.destroyPolyhedronMesh(meshMap[part]);
             delete vertexArrayMap[part]; 
             delete[] vertexDataMap[part]; 
             delete[] indicesDataMap[part];
             delete[] facesDataMap[part];
             
             meshMap.erase(part);
             vertexArrayMap.erase(part);
             vertexDataMap.erase(part);
             indicesDataMap.erase(part);
             facesDataMap.erase(part);
        }

        bodyMap.erase(part);
        shapeMap.erase(part);
    }
}

void PhysicsWorld::updatePartBody(Part* part) {
    if (bodyMap.find(part) != bodyMap.end()) {
        rblxphysics::RigidBody* body = bodyMap[part];
        
        // CRITICAL: Don't change KINEMATIC bodies - they're controlled by server
        if (body->getType() == rblxphysics::BodyType::KINEMATIC) {
            return; // Server controls this, don't touch it
        }
        
        rblxphysics::Vector3 pos(part->position.x, part->position.y, part->position.z);
        rblxmath::quat q = rblxmath::quat(rblxmath::radians(part->rotation));
        rblxphysics::Quaternion orientation(q.x, q.y, q.z, q.w);
        
        rblxphysics::Transform transform(pos, orientation);
        
        body->setTransform(transform);
        
        if (part->anchored) {
            if (body->getType() != rblxphysics::BodyType::STATIC) {
                body->setType(rblxphysics::BodyType::STATIC);
                body->setLinearVelocity(rblxphysics::Vector3(0,0,0));
                body->setAngularVelocity(rblxphysics::Vector3(0,0,0));
            }
        } else {
            if (body->getType() != rblxphysics::BodyType::DYNAMIC) body->setType(rblxphysics::BodyType::DYNAMIC);
        }
        
        // Update collision filtering based on canCollide
        // Get the first (and only) collider from the body
        if (body->getNbColliders() > 0) {
            rblxphysics::Collider* collider = body->getCollider(0);
            if (!part->canCollide) {
                // Disable all collisions
                collider->setCollideWithMaskBits(0);
            } else {
                // Enable collisions with everything
                collider->setCollideWithMaskBits(0xFFFF);
            }
        }
    }
}

void PhysicsWorld::update(float deltaTime) {
    // A frame that took a long time - loading a world, a stalled shader compile,
    // a minimised window - used to arrive here as one huge step. Nothing sweeps
    // the bodies, so a character dropping onto a thin part passed straight
    // through it: the first frame after joining an online game put the player
    // below the map, where the camera followed it into empty sky. Step in fixed
    // slices instead, and treat anything past a quarter second as a hitch
    // rather than as gameplay that should be simulated.
    constexpr float kFixedStep = 1.0f / 60.0f;
    constexpr float kMaxFrameSeconds = 0.25f;

    float remaining = std::min(std::max(deltaTime, 0.0f), kMaxFrameSeconds);
    while (remaining > 1.0e-4f) {
        const float step = std::min(remaining, kFixedStep);
        world->update(step);
        remaining -= step;
    }
    
    for (auto const& [part, body] : bodyMap) {
        if (part->anchored) continue;
        
        // Skip KINEMATIC bodies - they are controlled by server, don't overwrite their positions
        if (body->getType() == rblxphysics::BodyType::KINEMATIC) {
            continue; // Server controls these, don't update from physics
        }
        
        const rblxphysics::Transform& transform = body->getTransform();
        rblxphysics::Vector3 pos = transform.getPosition();
        rblxphysics::Quaternion rot = transform.getOrientation();
        rblxphysics::Vector3 vel = body->getLinearVelocity(); // Read Velocity
        
        part->position = rblxmath::vec3(pos.x, pos.y, pos.z);
        
        rblxmath::quat q(rot.w, rot.x, rot.y, rot.z);
        part->rotation = rblxmath::degrees(rblxmath::eulerAngles(q));
        
        part->velocity = rblxmath::vec3(vel.x, vel.y, vel.z); // Update Part Velocity
    }
}

void PhysicsWorld::reset() {
    for (auto const& [part, body] : bodyMap) {
        part->physicsBody = nullptr; // Clear pointer
        world->destroyRigidBody(body);
        
        rblxphysics::CollisionShape* shape = shapeMap[part];
        if (part->shape == ShapeType::Cube) {
             physicsCommon.destroyBoxShape(dynamic_cast<rblxphysics::BoxShape*>(shape));
        }
        else if (part->shape == ShapeType::Sphere) {
             physicsCommon.destroySphereShape(dynamic_cast<rblxphysics::SphereShape*>(shape));
        }
        else if (part->shape == ShapeType::Wedge) {
             physicsCommon.destroyConvexMeshShape(dynamic_cast<rblxphysics::ConvexMeshShape*>(shape));
             physicsCommon.destroyPolyhedronMesh(meshMap[part]);
             delete vertexArrayMap[part];
             delete[] vertexDataMap[part];
             delete[] indicesDataMap[part];
             delete[] facesDataMap[part];
        }
    }
    bodyMap.clear();
    shapeMap.clear();
    meshMap.clear();
    vertexArrayMap.clear();
    vertexDataMap.clear();
    indicesDataMap.clear();
    facesDataMap.clear();
}

// --- Raycast Callback ---
class RaycastCallback : public rblxphysics::RaycastCallback {
public:
    PhysicsWorld::RaycastResult* result;
    std::map<Part*, rblxphysics::RigidBody*>& bodyMap;
    Part* ignorePart;
    bool hitFound;

    RaycastCallback(PhysicsWorld::RaycastResult* res, std::map<Part*, rblxphysics::RigidBody*>& map, Part* ignore)
        : result(res), bodyMap(map), ignorePart(ignore), hitFound(false) {}

    virtual rblxphysics::decimal notifyRaycastHit(const rblxphysics::RaycastInfo& info) override {
        // Check if we hit the ignored part
        for (auto const& [part, body] : bodyMap) {
            if (body == info.body) {
                if (part == ignorePart) return -1.0; // Continue raycast, ignore this hit
                
                // Valid hit
                result->part = part;
                result->hitPoint = rblxmath::vec3(info.worldPoint.x, info.worldPoint.y, info.worldPoint.z);
                result->hitNormal = rblxmath::vec3(info.worldNormal.x, info.worldNormal.y, info.worldNormal.z);
                result->hitFraction = info.hitFraction;
                hitFound = true;
                
                return info.hitFraction; // Return fraction to clip ray for closer hits
            }
        }
        return -1.0; // Should not happen if body map is sync
    }
};

bool PhysicsWorld::raycast(const rblxmath::vec3& start, const rblxmath::vec3& end, RaycastResult& result, Part* ignorePart) {
    if (!world) return false;

    rblxphysics::Vector3 startPoint(start.x, start.y, start.z);
    rblxphysics::Vector3 endPoint(end.x, end.y, end.z);
    rblxphysics::Ray ray(startPoint, endPoint);
    
    RaycastCallback callback(&result, bodyMap, ignorePart);
    world->raycast(ray, &callback);
    
    return callback.hitFound;
}

void PhysicsWorld::createWeld(Part* partA, Part* partB) {
    if (!partA || !partB || partA == partB) return;
    
    if (bodyMap.find(partA) == bodyMap.end() || bodyMap.find(partB) == bodyMap.end()) return;
    
    rblxphysics::RigidBody* bodyA = bodyMap[partA];
    rblxphysics::RigidBody* bodyB = bodyMap[partB];
    
    // Create Fixed Joint
    rblxphysics::FixedJointInfo jointInfo(bodyA, bodyB, (bodyA->getTransform().getPosition() + bodyB->getTransform().getPosition()) * 0.5f);
    world->createJoint(jointInfo);
}
