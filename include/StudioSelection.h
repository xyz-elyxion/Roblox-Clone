#pragma once

#include <algorithm>
#include <cstddef>
#include <deque>
#include <vector>

#include "Part.h"

namespace rblx {
namespace studio {

// The parts Studio has selected, as indices into the world.
//
// Indices rather than pointers for the same reason the undo history uses them:
// the world is a deque that every other part of Studio already walks by index.
// A part is never erased - deleting it raises `deleted` - so an index stays
// meaningful for the life of the world, and the only thing that can invalidate
// one is loading a different world, which clears the selection anyway.
//
// The order is the order the parts were picked in: the last one is what the
// Properties panel edits and where the gizmo draws, the rest follow it.
inline bool isSelected(const std::vector<int>& selection, int index) {
    return std::find(selection.begin(), selection.end(), index) != selection.end();
}

inline void selectOnly(std::vector<int>& selection, int index) {
    selection.clear();
    selection.push_back(index);
}

inline void toggleSelected(std::vector<int>& selection, int index) {
    const auto found = std::find(selection.begin(), selection.end(), index);
    if (found != selection.end()) {
        selection.erase(found);
        return;
    }
    selection.push_back(index);
}

inline int primarySelection(const std::vector<int>& selection) {
    return selection.empty() ? -1 : selection.back();
}

// Drops what the world no longer holds: an index past the end after a load, or a
// part that has just been deleted.
inline void pruneSelection(std::vector<int>& selection, const std::deque<Part>& parts) {
    selection.erase(
        std::remove_if(selection.begin(), selection.end(),
                       [&parts](int index) {
                           return index < 0 || index >= static_cast<int>(parts.size()) ||
                               parts[static_cast<std::size_t>(index)].deleted;
                       }),
        selection.end());
}

// The selection plus everything nested inside it, so deleting a folder takes its
// contents with it instead of leaving them parented to a part that is gone. A
// world file can parent a part to anything, not only to a folder, so the walk
// follows the chain rather than stopping at the first part that is not one.
//
// A world file can also name a parent chain that loops, so the walk marks what it
// has already taken instead of trusting the chain to end.
inline std::vector<int> withFolderContents(const std::deque<Part>& parts,
                                           const std::vector<int>& selection) {
    std::vector<int> expanded = selection;
    std::vector<int> pending = selection;

    while (!pending.empty()) {
        const int index = pending.back();
        pending.pop_back();
        if (index < 0 || index >= static_cast<int>(parts.size())) {
            continue;
        }

        for (std::size_t candidate = 0; candidate < parts.size(); ++candidate) {
            if (parts[candidate].parentIndex != index) {
                continue;
            }
            if (isSelected(expanded, static_cast<int>(candidate))) {
                continue;
            }
            expanded.push_back(static_cast<int>(candidate));
            pending.push_back(static_cast<int>(candidate));
        }
    }

    return expanded;
}

}  // namespace studio
}  // namespace rblx
