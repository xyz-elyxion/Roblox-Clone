#include "StudioHistory.h"

#include <utility>

namespace rblx {
namespace studio {
namespace {

bool sameVec3(const rblxmath::vec3& first, const rblxmath::vec3& second) {
    return first.x == second.x && first.y == second.y && first.z == second.z;
}

bool samePart(const PartSnapshot& first, const PartSnapshot& second) {
    return first.name == second.name && first.parentIndex == second.parentIndex &&
        first.isFolder == second.isFolder && first.isCamera == second.isCamera &&
        first.isSpawn == second.isSpawn && first.deleted == second.deleted &&
        sameVec3(first.position, second.position) && sameVec3(first.size, second.size) &&
        sameVec3(first.color, second.color) && sameVec3(first.rotation, second.rotation) &&
        first.transparency == second.transparency &&
        first.reflectance == second.reflectance && first.anchored == second.anchored &&
        first.canCollide == second.canCollide && first.shape == second.shape &&
        first.material == second.material && first.texture == second.texture &&
        first.weldTargets == second.weldTargets;
}

}  // namespace

WorldSnapshot captureWorld(const std::deque<Part>& parts) {
    WorldSnapshot snapshot;
    snapshot.reserve(parts.size());

    for (const Part& part : parts) {
        PartSnapshot copy;
        copy.name = part.name;
        copy.parentIndex = part.parentIndex;
        copy.isFolder = part.isFolder;
        copy.isCamera = part.isCamera;
        copy.isSpawn = part.isSpawn;
        copy.deleted = part.deleted;
        copy.position = part.position;
        copy.size = part.size;
        copy.color = part.color;
        copy.rotation = part.rotation;
        copy.transparency = part.transparency;
        copy.reflectance = part.reflectance;
        copy.anchored = part.anchored;
        copy.canCollide = part.canCollide;
        copy.shape = part.shape;
        copy.material = part.material;
        copy.texture = part.texture;

        for (const Weld& weld : part.welds) {
            int target = -1;
            for (std::size_t index = 0; index < parts.size(); ++index) {
                if (&parts[index] == weld.part1) {
                    target = static_cast<int>(index);
                    break;
                }
            }
            copy.weldTargets.push_back(target);
        }

        snapshot.push_back(std::move(copy));
    }

    return snapshot;
}

bool sameWorld(const WorldSnapshot& first, const WorldSnapshot& second) {
    if (first.size() != second.size()) {
        return false;
    }
    for (std::size_t index = 0; index < first.size(); ++index) {
        if (!samePart(first[index], second[index])) {
            return false;
        }
    }
    return true;
}

void History::push(const WorldSnapshot& snapshot) {
    undo_.push_back(snapshot);
    if (undo_.size() > kMaxDepth) {
        undo_.erase(undo_.begin());
    }
}

void History::note(const WorldSnapshot& current, bool interacting) {
    // Mid-drag the world is only part-way to where it will end up, so nothing is
    // recorded until the interaction is over.
    if (interacting) {
        return;
    }
    if (!started_) {
        settled_ = current;
        started_ = true;
        return;
    }
    if (sameWorld(settled_, current)) {
        return;
    }

    push(settled_);
    // A new change is a new branch: whatever was undone is no longer reachable.
    redo_.clear();
    settled_ = current;
}

bool History::undo(const WorldSnapshot& current, WorldSnapshot* restored) {
    if (undo_.empty() || restored == nullptr) {
        return false;
    }

    redo_.push_back(current);
    *restored = undo_.back();
    undo_.pop_back();
    settled_ = *restored;
    started_ = true;
    return true;
}

bool History::redo(const WorldSnapshot& current, WorldSnapshot* restored) {
    if (redo_.empty() || restored == nullptr) {
        return false;
    }

    push(current);
    *restored = redo_.back();
    redo_.pop_back();
    settled_ = *restored;
    started_ = true;
    return true;
}

void History::clear() {
    undo_.clear();
    redo_.clear();
    settled_.clear();
    started_ = false;
}

}  // namespace studio
}  // namespace rblx
