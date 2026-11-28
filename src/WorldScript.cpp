#include "WorldScript.h"

#include "SoundBank.h"
#include "WorldServices.h"

#include <linearphysics/linearphysics.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace linear {
namespace worldscript {

namespace {

using linear::script::Interpreter;
using linear::script::ScriptObject;
using linear::script::Signal;
using linear::script::Table;
using linear::script::Value;

Part* partAt(const std::deque<Part>& parts, int index) {
    if (index < 0 || index >= static_cast<int>(parts.size())) return nullptr;
    return const_cast<Part*>(&parts[static_cast<std::size_t>(index)]);
}

// A script that builds its own level still has to fit through the join
// handshake: the server sends at most PacketProtocol::MAX_WORLD_PARTS parts and
// the rest are dropped, so an oversized world would leave its far end missing
// instead of failing loudly.
constexpr std::size_t kMaxScriptParts = 4096;

Value makeVector3(double x, double y, double z);

bool readVector3(const Value& value, linearmath::vec3& out);

class Vector3Object : public ScriptObject {
public:
    Vector3Object(double x, double y, double z) : x_(x), y_(y), z_(z) {}

    const char* typeName() const override { return "Vector3"; }

    double x() const { return x_; }
    double y() const { return y_; }
    double z() const { return z_; }

    bool get(const std::string& name, Value& out) override {
        if (name == "x" || name == "X") {
            out = Value::numberValue(x_);
            return true;
        }
        if (name == "y" || name == "Y") {
            out = Value::numberValue(y_);
            return true;
        }
        if (name == "z" || name == "Z") {
            out = Value::numberValue(z_);
            return true;
        }
        return false;
    }

    bool set(const std::string& name, const Value& value) override {
        if (!value.isNumber()) return false;
        if (name == "x" || name == "X") {
            x_ = value.number;
            return true;
        }
        if (name == "y" || name == "Y") {
            y_ = value.number;
            return true;
        }
        if (name == "z" || name == "Z") {
            z_ = value.number;
            return true;
        }
        return false;
    }

private:
    double x_ = 0.0;
    double y_ = 0.0;
    double z_ = 0.0;
};

Value makeVector3(double x, double y, double z) {
    return Value::objectValue(std::make_shared<Vector3Object>(x, y, z));
}

bool readVector3(const Value& value, linearmath::vec3& out) {
    if (value.isObject() && value.object) {
        Value x;
        Value y;
        Value z;
        if (value.object->get("x", x) && value.object->get("y", y) &&
            value.object->get("z", z) && x.isNumber() && y.isNumber() && z.isNumber()) {
            out = linearmath::vec3(static_cast<float>(x.number), static_cast<float>(y.number),
                            static_cast<float>(z.number));
            return true;
        }
        return false;
    }
    if (value.isTable() && value.table) {
        const Value x = value.table->fields.count("x") ? value.table->fields.at("x") : Value::nil();
        const Value y = value.table->fields.count("y") ? value.table->fields.at("y") : Value::nil();
        const Value z = value.table->fields.count("z") ? value.table->fields.at("z") : Value::nil();
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            out = linearmath::vec3(static_cast<float>(x.number), static_cast<float>(y.number),
                            static_cast<float>(z.number));
            return true;
        }
    }
    return false;
}

}  // namespace

struct WorldRuntimeState : public std::enable_shared_from_this<WorldRuntimeState> {
    std::deque<Part>* parts = nullptr;
    PhysicsWorld* physics = nullptr;
    Interpreter* interpreter = nullptr;
    bool running = false;

    std::shared_ptr<Signal> heartbeatSignal = std::make_shared<Signal>();
    std::map<int, std::shared_ptr<Signal>> touchedSignals;
    std::set<std::pair<int, std::string>> touching;
    std::function<std::vector<ScriptPlayer>()> playerSource;
    std::function<void(const std::string&, const linearmath::vec3&)> teleportHandler;
    std::function<void(const std::string&, float)> damageHandler;
    std::function<void(const std::string&, const linearmath::vec3&, float, bool)> soundHandler;
    std::function<void(const linearmath::vec3&, float)> environmentHandler;
    std::function<void(const std::string&, int)> scoreHandler;

    // The players the last tick saw. PlayerAdded and PlayerRemoving are the
    // difference between two of these, which is why a host only has to keep
    // supplying the list it already has and the events still land on the tick
    // the list changed - the server does not have to remember to call in.
    std::set<std::string> knownPlayers;

    // The one number the world keeps per player, held here as well as on the host
    // because a script reads it back with player.Score and PlayerRemoving hands
    // over a player who is no longer in the list.
    std::map<std::string, int> scores;

    std::shared_ptr<Signal> playerAddedSignal = std::make_shared<Signal>();
    std::shared_ptr<Signal> playerRemovingSignal = std::make_shared<Signal>();

    // What the fog is: the world file's values, or what a script has changed them
    // to since. Kept here so Lighting answers with the numbers the clients were
    // given rather than with zeros.
    linearmath::vec3 fogColor{0.0f};
    float fogDensity = 0.0f;

    // The service tree, built once: game:GetService has to hand back the same
    // object each time, because Lighting carries state and two copies of it would
    // disagree.
    std::map<std::string, Value> serviceValues;

    int findPart(const std::string& name) const;
    const Value* service(const std::string& name) const;
    void applyEnvironment();
    Value partValue(int index);
    Value playerValue(const std::string& name);
    std::vector<ScriptPlayer> players() const;
    std::shared_ptr<Signal> touchedFor(int index);
    void movePart(int index, const linearmath::vec3& position, bool stopMotion);
    void refreshBody(Part& part);
    void refreshPlayers();
    void updateTouches();
};

int WorldRuntimeState::findPart(const std::string& name) const {
    if (parts == nullptr) return -1;
    for (std::size_t i = 0; i < parts->size(); ++i) {
        const Part& part = (*parts)[i];
        if (part.deleted || part.isFolder) continue;
        if (part.name == name) return static_cast<int>(i);
    }
    return -1;
}

std::shared_ptr<Signal> WorldRuntimeState::touchedFor(int index) {
    auto& signal = touchedSignals[index];
    if (!signal) signal = std::make_shared<Signal>();
    return signal;
}

void WorldRuntimeState::movePart(int index, const linearmath::vec3& position, bool stopMotion) {
    Part* part = partAt(*parts, index);
    if (part == nullptr || part->deleted) return;
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) {
        return;
    }

    part->position = position;
    if (part->physicsBody == nullptr || physics == nullptr) return;

    auto* body = static_cast<linearphysics::RigidBody*>(part->physicsBody);
    const linearphysics::Transform current = body->getTransform();
    body->setTransform(linearphysics::Transform(
        linearphysics::Vector3(position.x, position.y, position.z), current.getOrientation()));
    if (stopMotion) {
        body->setLinearVelocity(linearphysics::Vector3(0.0f, 0.0f, 0.0f));
        body->setAngularVelocity(linearphysics::Vector3(0.0f, 0.0f, 0.0f));
    }
}

void WorldRuntimeState::refreshBody(Part& part) {
    if (physics == nullptr) return;
    physics->removePart(&part);
    physics->addPart(&part);
}

std::vector<ScriptPlayer> WorldRuntimeState::players() const {
    if (!playerSource) return {};
    return playerSource();
}

void WorldRuntimeState::refreshPlayers() {
    if (!playerSource) return;
    const std::vector<ScriptPlayer> current = players();
    std::set<std::string> next;
    for (const ScriptPlayer& player : current) {
        next.insert(player.name);
        scores[player.name] = player.score;
    }

    // The first tick after the script starts is the script's "who is already
    // here": PlayerAdded fires for the players in the game when it ran, which is
    // what a round loop needs to count them without polling for the change.
    for (const ScriptPlayer& player : current) {
        if (knownPlayers.count(player.name) != 0) continue;
        playerAddedSignal->fire(*interpreter, {playerValue(player.name)});
    }
    for (const std::string& name : knownPlayers) {
        if (next.count(name) != 0) continue;
        playerRemovingSignal->fire(*interpreter, {playerValue(name)});
    }
    knownPlayers.swap(next);
}

const Value* WorldRuntimeState::service(const std::string& name) const {
    const auto found = serviceValues.find(name);
    return found == serviceValues.end() ? nullptr : &found->second;
}

void WorldRuntimeState::applyEnvironment() {
    if (!environmentHandler) return;
    environmentHandler(fogColor, fogDensity);
}

void WorldRuntimeState::updateTouches() {
    if (parts == nullptr) return;
    const std::vector<ScriptPlayer> currentPlayers = players();
    std::set<std::pair<int, std::string>> next;

    for (auto& entry : touchedSignals) {
        if (!entry.second || entry.second->empty()) continue;
        Part* listener = partAt(*parts, entry.first);
        if (listener == nullptr || listener->deleted || listener->isFolder) continue;

        const linearmath::vec3 listenerHalf = listener->size * 0.5f;
        for (const ScriptPlayer& player : currentPlayers) {
            // Player characters are roughly a 2 x 5 x 1 box, matching the client.
            const linearmath::vec3 playerHalf(1.0f, 2.5f, 0.5f);
            const bool overlaps =
                std::abs(listener->position.x - player.position.x) <=
                    std::abs(listenerHalf.x) + playerHalf.x &&
                std::abs(listener->position.y - player.position.y) <=
                    std::abs(listenerHalf.y) + playerHalf.y &&
                std::abs(listener->position.z - player.position.z) <=
                    std::abs(listenerHalf.z) + playerHalf.z;
            if (!overlaps) continue;

            const std::pair<int, std::string> pair{entry.first, player.name};
            next.insert(pair);
            if (touching.count(pair) == 0) {
                entry.second->fire(*interpreter, {playerValue(player.name)});
            }
        }
    }

    touching.swap(next);
}

namespace {

// workspace:PlaySound and SoundService:Play are the same call: the script names a
// sound and the host puts it where the ears are, because the world a script runs
// in has none.
bool playSound(WorldRuntimeState& state, const std::string& method,
               const std::vector<Value>& args, Value& out, std::string& error) {
    if (args.empty() || !args[0].isString()) {
        error = method + " expects a sound name";
        return false;
    }
    // Checked on this side of the wire: the same typo would reach the client as a
    // name it cannot resolve, which is a noise that never comes and nothing to
    // look at.
    if (linear::audio::soundByName(args[0].string) == nullptr) {
        std::string known;
        for (const std::string& name : linear::audio::soundNames()) {
            if (!known.empty()) {
                known += ", ";
            }
            known += name;
        }
        error = "there is no sound '" + args[0].string + "': the sounds folder has no " +
                args[0].string + ".wav. This build ships " + known + ", and takes any other "
                "name a .wav in that folder has";
        return false;
    }
    if (!state.soundHandler) {
        error = "sounds cannot be played here";
        return false;
    }

    linearmath::vec3 position(0.0f);
    bool spatial = false;
    if (args.size() >= 2) {
        if (!readVector3(args[1], position)) {
            error = method + " expects a Vector3 position";
            return false;
        }
        spatial = true;
    }
    float volume = 1.0f;
    if (args.size() >= 3) {
        if (!args[2].isNumber()) {
            error = method + " expects a number for its volume";
            return false;
        }
        volume = static_cast<float>(args[2].number);
    }

    state.soundHandler(args[0].string, position, volume, spatial);
    out = Value::nil();
    return true;
}

class PartObject : public ScriptObject {
public:
    PartObject(std::shared_ptr<WorldRuntimeState> state, int index)
        : state_(std::move(state)), index_(index) {}

    const char* typeName() const override { return "Part"; }

    bool get(const std::string& name, Value& out) override {
        Part* part = partAt(*state_->parts, index_);
        if (part == nullptr || part->deleted) return false;

        if (name == "Name") {
            out = Value::stringValue(part->name);
            return true;
        }
        if (name == "ClassName") {
            out = Value::stringValue("Part");
            return true;
        }
        if (name == "Position") {
            out = makeVector3(part->position.x, part->position.y, part->position.z);
            return true;
        }
        if (name == "Size") {
            out = makeVector3(part->size.x, part->size.y, part->size.z);
            return true;
        }
        if (name == "Color") {
            out = makeVector3(part->color.x, part->color.y, part->color.z);
            return true;
        }
        if (name == "Transparency") {
            out = Value::numberValue(part->transparency);
            return true;
        }
        if (name == "Anchored") {
            out = Value::booleanValue(part->anchored);
            return true;
        }
        if (name == "CanCollide") {
            out = Value::booleanValue(part->canCollide);
            return true;
        }
        return false;
    }

    bool set(const std::string& name, const Value& value) override {
        Part* part = partAt(*state_->parts, index_);
        if (part == nullptr || part->deleted) return false;

        if (name == "Name") {
            if (!value.isString()) return false;
            part->name = value.string;
            return true;
        }
        if (name == "Position") {
            linearmath::vec3 position;
            if (!readVector3(value, position)) return false;
            state_->movePart(index_, position, false);
            return true;
        }
        if (name == "Size") {
            linearmath::vec3 size;
            if (!readVector3(value, size)) return false;
            part->size.x = std::clamp(size.x, 0.05f, 512.0f);
            part->size.y = std::clamp(size.y, 0.05f, 512.0f);
            part->size.z = std::clamp(size.z, 0.05f, 512.0f);
            state_->refreshBody(*part);
            return true;
        }
        if (name == "Color") {
            linearmath::vec3 color;
            if (!readVector3(value, color)) return false;
            part->color.x = std::clamp(color.x, 0.0f, 1.0f);
            part->color.y = std::clamp(color.y, 0.0f, 1.0f);
            part->color.z = std::clamp(color.z, 0.0f, 1.0f);
            return true;
        }
        if (name == "Transparency") {
            if (!value.isNumber()) return false;
            part->transparency =
                std::clamp(static_cast<float>(value.number), 0.0f, 1.0f);
            return true;
        }
        if (name == "Anchored") {
            if (value.type != Value::Type::Boolean) return false;
            part->anchored = value.boolean;
            state_->refreshBody(*part);
            return true;
        }
        if (name == "CanCollide") {
            if (value.type != Value::Type::Boolean) return false;
            part->canCollide = value.boolean;
            state_->refreshBody(*part);
            return true;
        }
        return false;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>& args,
              Value& out, std::string& error) override {
        Part* part = partAt(*state_->parts, index_);
        if (part == nullptr || part->deleted) {
            error = "part no longer exists";
            return false;
        }

        if (method == "Teleport") {
            linearmath::vec3 position;
            if (args.empty() || !readVector3(args[0], position)) {
                error = "Teleport expects a Vector3";
                return false;
            }
            state_->movePart(index_, position, true);
            out = Value::nil();
            return true;
        }
        if (method == "Destroy") {
            part->deleted = true;
            part->transparency = 1.0f;
            part->canCollide = false;
            if (state_->physics) state_->physics->removePart(part);
            out = Value::nil();
            return true;
        }

        error = "Part has no method '" + method + "'";
        return false;
    }

    Signal* signal(const std::string& name) override {
        if (name != "Touched") return nullptr;
        state_->touchedFor(index_);
        return state_->touchedSignals[index_].get();
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
    int index_ = -1;
};

class PlayerObject : public ScriptObject {
public:
    PlayerObject(std::shared_ptr<WorldRuntimeState> state, std::string name)
        : state_(std::move(state)), name_(std::move(name)) {}

    const char* typeName() const override { return "Player"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name") {
            out = Value::stringValue(name_);
            return true;
        }
        if (name == "ClassName") {
            out = Value::stringValue("Player");
            return true;
        }
        if (name == "Score") {
            const auto found = state_->scores.find(name_);
            const int score = found == state_->scores.end() ? 0 : found->second;
            out = Value::numberValue(static_cast<double>(score));
            return true;
        }
        if (name == "Position") {
            for (const ScriptPlayer& player : state_->players()) {
                if (player.name != name_) continue;
                out = makeVector3(player.position.x, player.position.y, player.position.z);
                return true;
            }
            out = makeVector3(0.0, 0.0, 0.0);
            return true;
        }
        return false;
    }

    bool set(const std::string& name, const Value& value) override {
        if (name != "Score") return false;
        // Whole numbers only, and finite: a score is a counter, and a HUD that
        // rounds two clients' copies differently is a scoreboard that disagrees.
        if (!value.isNumber() || !std::isfinite(value.number)) return false;
        const int score = static_cast<int>(
            std::llround(std::clamp(value.number, -1000000.0, 1000000.0)));
        state_->scores[name_] = score;
        if (state_->scoreHandler) {
            state_->scoreHandler(name_, score);
        }
        return true;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>& args,
              Value& out, std::string& error) override {
        if (method == "Teleport") {
            linearmath::vec3 position;
            if (args.empty() || !readVector3(args[0], position)) {
                error = "Teleport expects a Vector3";
                return false;
            }
            if (!state_->teleportHandler) {
                error = "players cannot be teleported here";
                return false;
            }
            state_->teleportHandler(name_, position);
            out = Value::nil();
            return true;
        }
        if (method == "TakeDamage") {
            if (args.empty() || !args[0].isNumber()) {
                error = "TakeDamage expects a number";
                return false;
            }
            if (!state_->damageHandler) {
                error = "players cannot take damage here";
                return false;
            }
            state_->damageHandler(name_, static_cast<float>(args[0].number));
            out = Value::nil();
            return true;
        }
        error = "Player has no method '" + method + "'";
        return false;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
    std::string name_;
};

class WorkspaceObject : public ScriptObject {
public:
    explicit WorkspaceObject(std::shared_ptr<WorldRuntimeState> state)
        : state_(std::move(state)) {}

    const char* typeName() const override { return "Workspace"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue("Workspace");
            return true;
        }
        const int index = state_->findPart(name);
        if (index < 0) return false;
        out = state_->partValue(index);
        return true;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>& args,
              Value& out, std::string& error) override {
        if (method == "FindFirstChild") {
            if (args.empty() || !args[0].isString()) {
                error = "FindFirstChild expects a part name";
                return false;
            }
            const int index = state_->findPart(args[0].string);
            out = index < 0 ? Value::nil() : state_->partValue(index);
            return true;
        }
        if (method == "GetChildren") {
            Value children = linear::script::makeTable();
            for (std::size_t i = 0; i < state_->parts->size(); ++i) {
                const Part& part = (*state_->parts)[i];
                if (part.deleted || part.isFolder) continue;
                children.table->elements.push_back(state_->partValue(static_cast<int>(i)));
            }
            out = children;
            return true;
        }
        if (method == "PlaySound") {
            return playSound(*state_, method, args, out, error);
        }
        if (method == "CreatePart") {
            // The way a script makes a level instead of decorating one: a room
            // builder needs parts that were never in the world file.
            if (state_->parts == nullptr) {
                error = "the workspace cannot build parts here";
                return false;
            }
            if (args.size() < 4 || !args[0].isString()) {
                error = "CreatePart expects a name, a position, a size and a color";
                return false;
            }
            linearmath::vec3 position;
            linearmath::vec3 size;
            linearmath::vec3 color;
            if (!readVector3(args[1], position) || !readVector3(args[2], size) ||
                !readVector3(args[3], color)) {
                error = "CreatePart expects Vector3 values for position, size and color";
                return false;
            }
            for (const linearmath::vec3& value : {position, size, color}) {
                if (!std::isfinite(value.x) || !std::isfinite(value.y) ||
                    !std::isfinite(value.z)) {
                    error = "CreatePart refuses non-finite values";
                    return false;
                }
            }
            if (state_->parts->size() >= kMaxScriptParts) {
                error = "the workspace is full; a script can build at most " +
                        std::to_string(kMaxScriptParts) + " parts";
                return false;
            }

            Part part(linearmath::vec3(0.0f), linearmath::vec3(1.0f), linearmath::vec3(0.6f), ShapeType::Cube,
                      args[0].string);
            part.position = position;
            part.size.x = std::clamp(size.x, 0.05f, 512.0f);
            part.size.y = std::clamp(size.y, 0.05f, 512.0f);
            part.size.z = std::clamp(size.z, 0.05f, 512.0f);
            part.color.x = std::clamp(color.x, 0.0f, 1.0f);
            part.color.y = std::clamp(color.y, 0.0f, 1.0f);
            part.color.z = std::clamp(color.z, 0.0f, 1.0f);

            state_->parts->push_back(part);
            Part& created = state_->parts->back();
            if (state_->physics != nullptr) {
                state_->physics->addPart(&created);
            }
            out = state_->partValue(static_cast<int>(state_->parts->size()) - 1);
            return true;
        }
        error = "Workspace has no method '" + method + "'";
        return false;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

class RunServiceObject : public ScriptObject {
public:
    explicit RunServiceObject(std::shared_ptr<WorldRuntimeState> state)
        : state_(std::move(state)) {}

    const char* typeName() const override { return "RunService"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue("RunService");
            return true;
        }
        if (name != "Heartbeat") return false;
        out = linear::script::makeSignalValue(*state_->heartbeatSignal);
        return true;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

class PlayersObject : public ScriptObject {
public:
    explicit PlayersObject(std::shared_ptr<WorldRuntimeState> state)
        : state_(std::move(state)) {}

    const char* typeName() const override { return "Players"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue("Players");
            return true;
        }
        if (name == "PlayerCount") {
            out = Value::numberValue(static_cast<double>(state_->players().size()));
            return true;
        }
        if (name == "PlayerAdded") {
            out = linear::script::makeSignalValue(*state_->playerAddedSignal);
            return true;
        }
        if (name == "PlayerRemoving") {
            out = linear::script::makeSignalValue(*state_->playerRemovingSignal);
            return true;
        }
        return false;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>&,
              Value& out, std::string& error) override {
        if (method == "GetPlayers") {
            Value players = linear::script::makeTable();
            for (const ScriptPlayer& player : state_->players()) {
                players.table->elements.push_back(state_->playerValue(player.name));
            }
            out = players;
            return true;
        }
        error = "Players has no method '" + method + "'";
        return false;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

// The world's air. The fog the world file sets travels to a client once, with
// the join, so a script that changes it has to hand the new pair to the host: the
// clients already in the game will not be sent a second welcome.
class LightingObject : public ScriptObject {
public:
    explicit LightingObject(std::shared_ptr<WorldRuntimeState> state) : state_(std::move(state)) {}

    const char* typeName() const override { return "Lighting"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue("Lighting");
            return true;
        }
        if (name == "FogColor") {
            out = makeVector3(state_->fogColor.x, state_->fogColor.y, state_->fogColor.z);
            return true;
        }
        if (name == "FogDensity") {
            out = Value::numberValue(state_->fogDensity);
            return true;
        }
        return false;
    }

    bool set(const std::string& name, const Value& value) override {
        if (name == "FogColor") {
            linearmath::vec3 color;
            if (!readVector3(value, color)) return false;
            state_->fogColor = linearmath::vec3(std::clamp(color.x, 0.0f, 1.0f),
                                               std::clamp(color.y, 0.0f, 1.0f),
                                               std::clamp(color.z, 0.0f, 1.0f));
            state_->applyEnvironment();
            return true;
        }
        if (name == "FogDensity") {
            if (!value.isNumber()) return false;
            // The client stops thickening the haze at 0.2, so a script cannot ask
            // for fog that hides the world outright.
            state_->fogDensity = std::clamp(static_cast<float>(value.number), 0.0f, 0.2f);
            state_->applyEnvironment();
            return true;
        }
        return false;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

// The service a script plays sounds through, which is the workspace's PlaySound
// under the name the familiar tree gives it.
class SoundServiceObject : public ScriptObject {
public:
    explicit SoundServiceObject(std::shared_ptr<WorldRuntimeState> state)
        : state_(std::move(state)) {}

    const char* typeName() const override { return "SoundService"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue("SoundService");
            return true;
        }
        return false;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>& args,
              Value& out, std::string& error) override {
        if (method == "Play") {
            return playSound(*state_, method, args, out, error);
        }
        error = "SoundService has no method '" + method + "'; this build has Play";
        return false;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
};

// A service this build does not fill. It exists so a script written against the
// familiar tree meets a service that says what is missing at the line that asked,
// instead of a nil that fails somewhere else with no explanation.
class EmptyServiceObject : public ScriptObject {
public:
    EmptyServiceObject(std::string name, std::string note)
        : name_(std::move(name)), note_(std::move(note)) {}

    const char* typeName() const override { return name_.c_str(); }

    bool get(const std::string& name, Value& out) override {
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue(name_);
            return true;
        }
        return false;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>&, Value& out,
              std::string& error) override {
        (void)out;
        error = name_ + " has no method '" + method + "' in this build: " + note_;
        return false;
    }

private:
    std::string name_;
    std::string note_;
};

// game itself: the tree a script reaches every service through.
class GameObject : public ScriptObject {
public:
    GameObject(std::shared_ptr<WorldRuntimeState> state, Value workspace)
        : state_(std::move(state)), workspace_(std::move(workspace)) {}

    const char* typeName() const override { return "game"; }

    bool get(const std::string& name, Value& out) override {
        if (name == "Workspace") {
            out = workspace_;
            return true;
        }
        if (name == "Name" || name == "ClassName") {
            out = Value::stringValue("game");
            return true;
        }
        return false;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>& args, Value& out,
              std::string& error) override {
        if (method != "GetService") {
            error = "game has no method '" + method + "'; this build has GetService";
            return false;
        }
        if (args.empty() || !args[0].isString()) {
            error = "GetService expects a service name";
            return false;
        }
        const Value* service = state_->service(args[0].string);
        if (service == nullptr) {
            std::string known;
            for (const linear::worldscript::Service& candidate : services()) {
                if (!known.empty()) {
                    known += ", ";
                }
                known += candidate.name;
            }
            error = "there is no service '" + args[0].string + "'; this build has " + known;
            return false;
        }
        out = *service;
        return true;
    }

private:
    std::shared_ptr<WorldRuntimeState> state_;
    Value workspace_;
};

}  // namespace

Value WorldRuntimeState::partValue(int index) {
    return Value::objectValue(std::make_shared<PartObject>(shared_from_this(), index));
}

Value WorldRuntimeState::playerValue(const std::string& name) {
    return Value::objectValue(std::make_shared<PlayerObject>(shared_from_this(), name));
}

WorldRuntime::WorldRuntime(std::deque<Part>& parts, PhysicsWorld* physicsWorld,
                           linear::script::Interpreter& interpreter) {
    state_ = std::make_shared<WorldRuntimeState>();
    state_->parts = &parts;
    state_->physics = physicsWorld;
    state_->interpreter = &interpreter;

    const Value workspace = Value::objectValue(std::make_shared<WorkspaceObject>(state_));
    interpreter.setGlobal("workspace", workspace);

    Value vector3 = linear::script::makeTable();
    vector3.table->fields["new"] = linear::script::makeNativeFunction(
        [](Interpreter&, const std::vector<Value>& args) {
            const double x = args.size() > 0 ? args[0].asNumber() : 0.0;
            const double y = args.size() > 1 ? args[1].asNumber() : 0.0;
            const double z = args.size() > 2 ? args[2].asNumber() : 0.0;
            return makeVector3(x, y, z);
        });
    interpreter.setGlobal("Vector3", vector3);

    const Value runService = Value::objectValue(std::make_shared<RunServiceObject>(state_));
    const Value players = Value::objectValue(std::make_shared<PlayersObject>(state_));
    interpreter.setGlobal("RunService", runService);
    interpreter.setGlobal("Players", players);

    // The tree a script asks for by name. The services with state are made here and
    // every other name in the shared table gets an object that says what this build
    // keeps instead of existing as a nil.
    state_->serviceValues["Workspace"] = workspace;
    state_->serviceValues["Players"] = players;
    state_->serviceValues["RunService"] = runService;
    state_->serviceValues["Lighting"] =
        Value::objectValue(std::make_shared<LightingObject>(state_));
    state_->serviceValues["SoundService"] =
        Value::objectValue(std::make_shared<SoundServiceObject>(state_));
    for (const linear::worldscript::Service& service : services()) {
        if (state_->serviceValues.count(service.name) != 0) {
            continue;
        }
        state_->serviceValues[service.name] =
            Value::objectValue(std::make_shared<EmptyServiceObject>(service.name, service.note));
    }

    interpreter.setGlobal(
        "game", Value::objectValue(std::make_shared<GameObject>(state_, workspace)));
}

WorldRuntime::~WorldRuntime() = default;

void WorldRuntime::setPlayerSource(std::function<std::vector<ScriptPlayer>()> source) {
    state_->playerSource = std::move(source);
}

void WorldRuntime::setTeleportHandler(
    std::function<void(const std::string&, const linearmath::vec3&)> handler) {
    state_->teleportHandler = std::move(handler);
}

void WorldRuntime::setDamageHandler(
    std::function<void(const std::string&, float)> handler) {
    state_->damageHandler = std::move(handler);
}

void WorldRuntime::setSoundHandler(
    std::function<void(const std::string&, const linearmath::vec3&, float, bool)> handler) {
    state_->soundHandler = std::move(handler);
}

void WorldRuntime::setEnvironment(const linearmath::vec3& fogColor, float fogDensity) {
    // The numbers the world file gave the clients. Setting them here is not a
    // change to send: this is what everybody was already told at the join.
    state_->fogColor = fogColor;
    state_->fogDensity = fogDensity;
}

void WorldRuntime::setEnvironmentHandler(
    std::function<void(const linearmath::vec3&, float)> handler) {
    state_->environmentHandler = std::move(handler);
}

void WorldRuntime::setScoreHandler(std::function<void(const std::string&, int)> handler) {
    state_->scoreHandler = std::move(handler);
}

bool WorldRuntime::runScript(const std::string& source, const std::string& chunkName,
                             std::string& error) {
    state_->running = false;
    if (!state_->interpreter->run(source, chunkName, error)) {
        return false;
    }
    state_->running = true;
    return true;
}

void WorldRuntime::heartbeat(float deltaTime) {
    if (!state_->running) return;
    state_->refreshPlayers();
    state_->heartbeatSignal->fire(*state_->interpreter,
                                  {Value::numberValue(static_cast<double>(deltaTime))});
    state_->updateTouches();
}

bool WorldRuntime::isRunning() const { return state_->running; }

void WorldRuntime::stop() { state_->running = false; }

}  // namespace worldscript
}  // namespace linear
