#include "WorldServices.h"

namespace linear {
namespace worldscript {
namespace {

// The order is the one every builder of a sandbox level knows. Five of these hold
// real state in this build; the rest are here because a script written against
// that tree should find a service that says what this build has instead of a nil
// that fails three lines later.
const std::vector<Service> kServices = {
    {"Workspace", "the world's parts"},
    {"Players", "who is in the game; the server has them, not the editor"},
    {"Lighting", "the fog colour and density the world file sets"},
    {"MaterialService", "no service: a part's material is a field on the part"},
    {"ReplicatedFirst", "no service: a client is handed the world as one snapshot"},
    {"ReplicatedStorage", "no service: a world has no assets of its own to replicate"},
    {"ServerScriptService", "no service: a world has one script, and the server runs it"},
    {"ServerStorage", "no service: a world travels as a .world file and its .rbx"},
    {"StarterGui", "no service: the client's menus and HUD are native code"},
    {"StarterPack", "no service: there is no inventory"},
    {"StarterPlayer", "no service: the character is built the same way in every world"},
    {"Teams", "no service: players are one list"},
    {"SoundService", "the .wav files in sounds/, and the built-in sounds"},
    {"TextChatService", "no service: chat is native, with its own rate limits"},
};

}  // namespace

const std::vector<Service>& services() {
    return kServices;
}

const Service* findService(const std::string& name) {
    for (const Service& service : kServices) {
        if (name == service.name) {
            return &service;
        }
    }
    return nullptr;
}

}  // namespace worldscript
}  // namespace linear
