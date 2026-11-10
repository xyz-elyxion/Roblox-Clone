// Studio's undo history: one step per finished interaction, and a world that
// comes back exactly as it was.

#include "StudioHistory.h"

#include <cstdio>
#include <deque>
#include <string>

namespace {
int failures = 0;

void check(bool condition, const std::string& name) {
    if (condition) {
        return;
    }
    ++failures;
    std::printf("FAIL %s\n", name.c_str());
}

std::deque<Part> makeWorld() {
    std::deque<Part> parts;
    parts.emplace_back(rblxmath::vec3(0.0f, -1.0f, 0.0f), rblxmath::vec3(40.0f, 1.0f, 40.0f),
                       rblxmath::vec3(0.3f, 0.4f, 0.3f), ShapeType::Cube, "Baseplate");
    parts.emplace_back(rblxmath::vec3(0.0f, 3.0f, 0.0f), rblxmath::vec3(2.0f),
                       rblxmath::vec3(0.8f, 0.2f, 0.2f), ShapeType::Cube, "Part");
    return parts;
}

void testOneChangeIsOneStep() {
    std::deque<Part> parts = makeWorld();
    rblx::studio::History history;
    history.note(rblx::studio::captureWorld(parts), false);
    check(history.undoDepth() == 0, "a fresh history holds no steps");
    check(!history.canUndo(), "there is nothing to undo yet");

    parts[1].position = rblxmath::vec3(4.0f, 3.0f, 0.0f);
    history.note(rblx::studio::captureWorld(parts), false);
    check(history.undoDepth() == 1, "one change is one step");

    // The same world seen twice is not a second step.
    history.note(rblx::studio::captureWorld(parts), false);
    check(history.undoDepth() == 1, "an unchanged frame adds nothing");
}

void testDragIsOneStep() {
    std::deque<Part> parts = makeWorld();
    rblx::studio::History history;
    history.note(rblx::studio::captureWorld(parts), false);

    // A gizmo drag: the world moves every frame while the mouse is down.
    for (int frame = 1; frame <= 10; ++frame) {
        parts[1].position.x = static_cast<float>(frame);
        history.note(rblx::studio::captureWorld(parts), true);
    }
    check(history.undoDepth() == 0, "a drag in progress records nothing");

    history.note(rblx::studio::captureWorld(parts), false);
    check(history.undoDepth() == 1, "a finished drag is one step, not ten");
}

void testUndoAndRedo() {
    std::deque<Part> parts = makeWorld();
    rblx::studio::History history;
    history.note(rblx::studio::captureWorld(parts), false);

    parts[1].position = rblxmath::vec3(4.0f, 3.0f, 0.0f);
    parts[1].name = "Moved";
    history.note(rblx::studio::captureWorld(parts), false);

    rblx::studio::WorldSnapshot restored;
    check(history.undo(rblx::studio::captureWorld(parts), &restored), "undo hands back a world");
    check(restored.size() == 2, "the snapshot keeps every part");
    check(restored[1].position == rblxmath::vec3(0.0f, 3.0f, 0.0f), "undo brings the position back");
    check(restored[1].name == "Part", "undo brings the name back");
    check(history.undoDepth() == 0, "the step was spent");
    check(history.canRedo(), "the change can be put back");

    rblx::studio::WorldSnapshot forward;
    check(history.redo(restored, &forward), "redo hands back the world it left");
    check(forward[1].position == rblxmath::vec3(4.0f, 3.0f, 0.0f), "redo restores the move");
    check(forward[1].name == "Moved", "redo restores the rename");
    check(!history.canRedo(), "the redo was spent");
}

void testNewChangeDropsRedo() {
    std::deque<Part> parts = makeWorld();
    rblx::studio::History history;
    history.note(rblx::studio::captureWorld(parts), false);

    parts[0].color = rblxmath::vec3(1.0f, 0.0f, 0.0f);
    history.note(rblx::studio::captureWorld(parts), false);

    rblx::studio::WorldSnapshot restored;
    check(history.undo(rblx::studio::captureWorld(parts), &restored), "the repaint can be undone");
    check(history.canRedo(), "it can be redone until something else changes");

    parts[1].size = rblxmath::vec3(6.0f);
    history.note(restored, false);  // the world the undo produced
    history.note(rblx::studio::captureWorld(parts), false);
    check(!history.canRedo(), "a new change drops the redo branch");
}

void testDepthIsBounded() {
    std::deque<Part> parts = makeWorld();
    rblx::studio::History history;
    history.note(rblx::studio::captureWorld(parts), false);

    for (int step = 1; step <= 60; ++step) {
        parts[1].position.y = static_cast<float>(step);
        history.note(rblx::studio::captureWorld(parts), false);
    }
    check(history.undoDepth() == rblx::studio::History::kMaxDepth,
          "the history keeps its deepest steps and no more");
}

void testSnapshotKeepsTheWholePart() {
    std::deque<Part> parts = makeWorld();
    parts[1].material = Material::Neon;
    parts[1].texture = "brick.png";
    parts[1].transparency = 0.5f;
    parts[1].canCollide = false;
    parts[1].isSpawn = true;

    Weld weld;
    weld.part1 = &parts[0];
    parts[1].welds.push_back(weld);

    const rblx::studio::WorldSnapshot snapshot = rblx::studio::captureWorld(parts);
    check(snapshot.size() == 2, "both parts are captured");
    check(snapshot[1].material == Material::Neon, "the material is captured");
    check(snapshot[1].texture == "brick.png", "the texture is captured");
    check(snapshot[1].transparency == 0.5f, "the transparency is captured");
    check(!snapshot[1].canCollide, "the collision flag is captured");
    check(snapshot[1].isSpawn, "the spawn flag is captured");
    check(snapshot[1].weldTargets.size() == 1 && snapshot[1].weldTargets[0] == 0,
          "a weld is captured by index");

    // Any field a part carries is a reason to call two worlds different.
    rblx::studio::WorldSnapshot changed = snapshot;
    changed[0].isSpawn = true;
    check(!rblx::studio::sameWorld(snapshot, changed), "a changed flag makes the worlds differ");

    changed = snapshot;
    changed[1].weldTargets.clear();
    check(!rblx::studio::sameWorld(snapshot, changed), "a dropped weld makes the worlds differ");
}

void testClearForgetsEverything() {
    std::deque<Part> parts = makeWorld();
    rblx::studio::History history;
    history.note(rblx::studio::captureWorld(parts), false);
    parts[1].position.z = 9.0f;
    history.note(rblx::studio::captureWorld(parts), false);

    history.clear();
    check(!history.canUndo() && !history.canRedo(), "a loaded world has no history");
    check(history.undoDepth() == 0, "the steps are gone");
}
}  // namespace

int main() {
    testOneChangeIsOneStep();
    testDragIsOneStep();
    testUndoAndRedo();
    testNewChangeDropsRedo();
    testDepthIsBounded();
    testSnapshotKeepsTheWholePart();
    testClearForgetsEverything();

    if (failures != 0) {
        std::printf("studio_history_tests: %d check(s) failed\n", failures);
        return 1;
    }
    std::printf("studio_history_tests: all checks passed\n");
    return 0;
}
