#pragma once

#include "Part.h"
#include "PhysicsWorld.h"
#include "LinearScript.h"

#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace linear {
namespace worldscript {

struct WorldRuntimeState;

// Snapshot of a connected player, supplied by the server so scripts can react to
// touches and teleport players.
struct ScriptPlayer {
    std::string name;
    linearmath::vec3 position{0.0f};
    // The one number the world keeps for this player (player.Score), as the host
    // holds it: a script reads it back and the clients draw it in their player
    // list, so the two cannot disagree about what the score is.
    int score = 0;
};

// Connects a script interpreter to a world: exposes workspace, part properties,
// Vector3, the Players add/remove events, RunService.Heartbeat and part.Touched,
// and drives those events.
class WorldRuntime {
public:
    WorldRuntime(std::deque<Part>& parts, PhysicsWorld* physicsWorld,
                 linear::script::Interpreter& interpreter);
    ~WorldRuntime();

    WorldRuntime(const WorldRuntime&) = delete;
    WorldRuntime& operator=(const WorldRuntime&) = delete;

    bool runScript(const std::string& source, const std::string& chunkName,
                   std::string& error);

    // Supplies the connected players. The server wires these callbacks so Touched
    // can report players and scripts can teleport them.
    void setPlayerSource(std::function<std::vector<ScriptPlayer>()> source);
    void setTeleportHandler(
        std::function<void(const std::string& name, const linearmath::vec3& position)> handler);

    // Script-driven damage. The host owns health, so a script asks for damage
    // (player:TakeDamage(amount)) and the server applies it and decides when the
    // player respawns.
    void setDamageHandler(std::function<void(const std::string& name, float amount)> handler);

    // Script-driven noise. The world a script runs on has no ears, so
    // workspace:PlaySound asks the host to make the sound somewhere else - on the
    // clients, where the listeners are.
    void setSoundHandler(std::function<void(const std::string& sound, const linearmath::vec3& position,
                                           float volume, bool spatial)> handler);

    // The fog the world file starts with, so Lighting reads back what the clients
    // were given, and the handler a change goes through: a client is handed the
    // environment once, at the join, so the clients already in the game only hear
    // about a new one if the host sends it.
    void setEnvironment(const linearmath::vec3& fogColor, float fogDensity);
    void setEnvironmentHandler(std::function<void(const linearmath::vec3&, float)> handler);

    // The one number a world keeps per player, written by a script
    // (player.Score = n). The host stores it and tells the clients; the server is
    // the only owner, because a score a client decided for itself is a player who
    // always wins.
    void setScoreHandler(std::function<void(const std::string& name, int score)> handler);

    // Fires RunService.Heartbeat, reports the players who joined or left since the
    // last tick, and checks parts for new Touched pairs.
    void heartbeat(float deltaTime);

    bool isRunning() const;
    void stop();

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

}  // namespace worldscript
}  // namespace linear
