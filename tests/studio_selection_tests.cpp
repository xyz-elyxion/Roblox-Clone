// Studio's selection: what counts as picked, what a folder takes with it when it
// is deleted, and what survives a part being deleted under it.

#include "StudioSelection.h"

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

// A folder holding a wall, the wall holding a chair, and a loose part beside them.
std::deque<Part> makeWorld() {
    std::deque<Part> parts;
    parts.emplace_back(rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(1.0f), ShapeType::Cube,
                       "Buildings");
    parts.back().isFolder = true;

    parts.emplace_back(rblxmath::vec3(1.0f, 0.0f, 0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(1.0f),
                       ShapeType::Cube, "Wall");
    parts.back().parentIndex = 0;

    parts.emplace_back(rblxmath::vec3(2.0f, 0.0f, 0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(1.0f),
                       ShapeType::Cube, "Chair");
    parts.back().parentIndex = 1;

    parts.emplace_back(rblxmath::vec3(4.0f, 0.0f, 0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(1.0f),
                       ShapeType::Cube, "Loose");
    return parts;
}

void testSelectOnlyAndToggle() {
    std::vector<int> selection;

    rblx::studio::selectOnly(selection, 2);
    check(selection.size() == 1 && selection[0] == 2, "a plain pick is the whole selection");
    check(rblx::studio::isSelected(selection, 2), "the picked part is selected");

    // Ctrl+click adds, and the part stays where it was in the order.
    rblx::studio::toggleSelected(selection, 0);
    check(selection.size() == 2, "a second pick joins the first");
    check(selection[0] == 2 && selection[1] == 0, "picks keep the order they were made in");

    // Ctrl+click again lets it go.
    rblx::studio::toggleSelected(selection, 2);
    check(selection.size() == 1 && selection[0] == 0, "picking a selected part removes it");

    // A plain pick replaces rather than adds.
    rblx::studio::selectOnly(selection, 3);
    check(selection.size() == 1 && selection[0] == 3, "a plain pick replaces the selection");
}

void testPrimaryIsTheLastPick() {
    std::vector<int> selection;
    check(rblx::studio::primarySelection(selection) == -1, "an empty selection has no primary");

    rblx::studio::selectOnly(selection, 1);
    check(rblx::studio::primarySelection(selection) == 1, "the only pick is the primary");

    rblx::studio::toggleSelected(selection, 3);
    check(rblx::studio::primarySelection(selection) == 3, "the last pick is the primary");

    rblx::studio::toggleSelected(selection, 3);
    check(rblx::studio::primarySelection(selection) == 1, "removing the primary falls back");
}

void testPruneDropsWhatTheWorldNoLongerHas() {
    std::deque<Part> parts = makeWorld();
    std::vector<int> selection{0, 1, 2, 3, 99};
    parts[1].deleted = true;

    rblx::studio::pruneSelection(selection, parts);
    check(selection.size() == 3, "a deleted part and an index past the end are dropped");
    check(rblx::studio::isSelected(selection, 0) && rblx::studio::isSelected(selection, 2) &&
              rblx::studio::isSelected(selection, 3),
          "the parts that are still there are kept");
    check(!rblx::studio::isSelected(selection, 1), "the deleted part is gone from the selection");

    std::vector<int> empty{-4};
    rblx::studio::pruneSelection(empty, parts);
    check(empty.empty(), "a negative index never survives");
}

void testFolderTakesItsContents() {
    std::deque<Part> parts = makeWorld();
    const std::vector<int> expanded = rblx::studio::withFolderContents(parts, {0});

    // The chair hangs off the wall, not off the folder, and a part can be a parent
    // without being a folder: the walk follows the chain either way.
    check(expanded.size() == 3, "a folder takes its child and its child's child");
    check(rblx::studio::isSelected(expanded, 0) && rblx::studio::isSelected(expanded, 1) &&
              rblx::studio::isSelected(expanded, 2),
          "the folder and everything inside it is included");
    check(!rblx::studio::isSelected(expanded, 3), "a part outside the folder is left alone");

    // Selecting a leaf part takes nothing else with it.
    const std::vector<int> leaf = rblx::studio::withFolderContents(parts, {3});
    check(leaf.size() == 1 && leaf[0] == 3, "a part that is not a folder stands alone");
}

void testParentLoopTerminates() {
    std::deque<Part> parts;
    parts.emplace_back(rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(1.0f), ShapeType::Cube, "A");
    parts.back().isFolder = true;
    parts.back().parentIndex = 1;
    parts.emplace_back(rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(1.0f), ShapeType::Cube, "B");
    parts.back().isFolder = true;
    parts.back().parentIndex = 0;

    const std::vector<int> expanded = rblx::studio::withFolderContents(parts, {1});
    check(expanded.size() == 2, "a parent loop is walked once and finishes");
}
}  // namespace

int main() {
    testSelectOnlyAndToggle();
    testPrimaryIsTheLastPick();
    testPruneDropsWhatTheWorldNoLongerHas();
    testFolderTakesItsContents();
    testParentLoopTerminates();

    if (failures != 0) {
        std::printf("studio_selection_tests: %d check(s) failed\n", failures);
        return 1;
    }
    std::printf("studio_selection_tests: all checks passed\n");
    return 0;
}
