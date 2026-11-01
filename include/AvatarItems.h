#pragma once

#include <string>
#include <vector>

namespace rblx {
namespace items {

// A wearable hat. Sizes and offsets are in studs relative to the 1.2-stud head
// cube, so the web preview and the 3D client draw the same shape from these
// numbers instead of each keeping its own copy of the catalog.
struct AvatarItem {
    std::string id;
    std::string name;
    int price = 0;
    float color[3] = {1.0f, 1.0f, 1.0f};
    float size[3] = {1.2f, 0.4f, 1.2f};
    float offsetY = 0.7f;  // centre above the centre of the head
    float offsetZ = 0.0f;
};

// Cheapest first, so the first thing a new account sees is something its coins
// can actually buy.
inline const std::vector<AvatarItem>& avatarItems() {
    static const std::vector<AvatarItem> items = {
        {"red-cap", "Red Cap", 20, {0.75f, 0.16f, 0.18f}, {1.34f, 0.44f, 1.34f}, 0.62f, 0.0f},
        {"headphones", "Headphones", 40, {0.16f, 0.16f, 0.18f}, {1.50f, 0.50f, 1.24f}, 0.20f, 0.0f},
        {"top-hat", "Top Hat", 60, {0.10f, 0.10f, 0.12f}, {0.86f, 1.10f, 0.86f}, 1.00f, 0.0f},
        {"propeller-cap", "Propeller Cap", 80, {0.20f, 0.42f, 0.85f}, {1.34f, 0.40f, 1.34f}, 0.60f, 0.0f},
        {"cowboy-hat", "Cowboy Hat", 100, {0.45f, 0.28f, 0.14f}, {1.62f, 0.50f, 1.62f}, 0.60f, 0.0f},
        {"halo", "Halo", 150, {0.95f, 0.82f, 0.25f}, {1.50f, 0.12f, 1.50f}, 0.95f, 0.0f}
    };
    return items;
}

inline const AvatarItem* findAvatarItem(const std::string& id) {
    if (id.empty()) {
        return nullptr;
    }
    for (const AvatarItem& item : avatarItems()) {
        if (item.id == id) {
            return &item;
        }
    }
    return nullptr;
}

}  // namespace items
}  // namespace rblx
