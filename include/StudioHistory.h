#pragma once

#include <cstddef>
#include <deque>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Part.h"

namespace rblx {
namespace studio {

// One part as an undo step remembers it. The values are copied out of the Part
// instead of pointing at it: the world is a deque that reallocates, so a weld
// that named a Part would name a dead object as soon as anything was added.
struct PartSnapshot {
    std::string name;
    int parentIndex = -1;
    bool isFolder = false;
    bool isCamera = false;
    bool isSpawn = false;
    bool deleted = false;
    glm::vec3 position{0.0f};
    glm::vec3 size{1.0f};
    glm::vec3 color{0.6f};
    glm::vec3 rotation{0.0f};
    float transparency = 0.0f;
    float reflectance = 0.0f;
    bool anchored = true;
    bool canCollide = true;
    ShapeType shape = ShapeType::Cube;
    Material material = Material::Plastic;
    std::string texture;
    // Weld targets by index, for the same reason the parts themselves are copied.
    std::vector<int> weldTargets;
};

using WorldSnapshot = std::vector<PartSnapshot>;

WorldSnapshot captureWorld(const std::deque<Part>& parts);
bool sameWorld(const WorldSnapshot& first, const WorldSnapshot& second);

// Undo and redo for Studio, held as whole-world snapshots.
//
// A step is recorded when an interaction finishes rather than when a value
// changes: ImGui keeps a widget active for the whole of a drag, so dragging a part
// across the map is one step instead of one per frame, and typing a name is one
// step instead of one per keystroke.
class History {
public:
    // Deep enough for an editing session, small enough that a world is not held
    // in memory fifty times over.
    static const std::size_t kMaxDepth = 50;

    // Called once a frame with the world as it now stands. `interacting` says a
    // widget or the gizmo still owns the change.
    void note(const WorldSnapshot& current, bool interacting);

    // Steps back and hands over the world to restore; false when there is
    // nothing to step back to. `current` becomes the redo point.
    bool undo(const WorldSnapshot& current, WorldSnapshot* restored);
    bool redo(const WorldSnapshot& current, WorldSnapshot* restored);

    bool canUndo() const { return !undo_.empty(); }
    bool canRedo() const { return !redo_.empty(); }
    std::size_t undoDepth() const { return undo_.size(); }
    std::size_t redoDepth() const { return redo_.size(); }
    // Forgets everything: a world that was just loaded has no steps behind it.
    void clear();

private:
    void push(const WorldSnapshot& snapshot);

    std::vector<WorldSnapshot> undo_;
    std::vector<WorldSnapshot> redo_;
    // The world as it stood when the last step was recorded, which is what the
    // next change is measured against.
    WorldSnapshot settled_;
    bool started_ = false;
};

}  // namespace studio
}  // namespace rblx
