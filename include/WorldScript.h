#pragma once

#include "Part.h"
#include "PhysicsWorld.h"
#include "RblxScript.h"

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace rblx {
namespace worldscript {

struct WorldRuntimeState;

// Snapshot of a connected player, supplied by the server so scripts can react to
// touches and teleport players.
struct ScriptPlayer {
    std::string name;
    glm::vec3 position{0.0f};
};

// Connects a script interpreter to a world: exposes workspace, part properties,
// Vector3, RunService.Heartbeat and part.Touched, and drives those events.
class WorldRuntime {
public:
    WorldRuntime(std::deque<Part>& parts, PhysicsWorld* physicsWorld,
                 rblx::script::Interpreter& interpreter);
    ~WorldRuntime();

    WorldRuntime(const WorldRuntime&) = delete;
    WorldRuntime& operator=(const WorldRuntime&) = delete;

    bool runScript(const std::string& source, const std::string& chunkName,
                   std::string& error);

    // Supplies the connected players. The server wires these callbacks so Touched
    // can report players and scripts can teleport them.
    void setPlayerSource(std::function<std::vector<ScriptPlayer>()> source);
    void setTeleportHandler(
        std::function<void(const std::string& name, const glm::vec3& position)> handler);

    // Script-driven damage. The host owns health, so a script asks for damage
    // (player:TakeDamage(amount)) and the server applies it and decides when the
    // player respawns.
    void setDamageHandler(std::function<void(const std::string& name, float amount)> handler);

    // Script-driven noise. The world a script runs on has no ears, so
    // workspace:PlaySound asks the host to make the sound somewhere else - on the
    // clients, where the listeners are.
    void setSoundHandler(std::function<void(const std::string& sound, const glm::vec3& position,
                                           float volume, bool spatial)> handler);

    // Fires RunService.Heartbeat handlers and checks parts for new Touched pairs.
    void heartbeat(float deltaTime);

    bool isRunning() const;
    void stop();

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

}  // namespace worldscript
}  // namespace rblx
