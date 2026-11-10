#include "WorldScript.h"

#include "SoundBank.h"

#include <rblxphysics/rblxphysics.h>

#include <algorithm>
#include <cmath>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace rblx {
namespace worldscript {

namespace {

using rblx::script::Interpreter;
using rblx::script::ScriptObject;
using rblx::script::Signal;
using rblx::script::Table;
using rblx::script::Value;

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

bool readVector3(const Value& value, rblxmath::vec3& out);

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

bool readVector3(const Value& value, rblxmath::vec3& out) {
    if (value.isObject() && value.object) {
        Value x;
        Value y;
        Value z;
        if (value.object->get("x", x) && value.object->get("y", y) &&
            value.object->get("z", z) && x.isNumber() && y.isNumber() && z.isNumber()) {
            out = rblxmath::vec3(static_cast<float>(x.number), static_cast<float>(y.number),
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
            out = rblxmath::vec3(static_cast<float>(x.number), static_cast<float>(y.number),
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
    std::function<void(const std::string&, const rblxmath::vec3&)> teleportHandler;
    std::function<void(const std::string&, float)> damageHandler;
    std::function<void(const std::string&, const rblxmath::vec3&, float, bool)> soundHandler;

    int findPart(const std::string& name) const;
    Value partValue(int index);
    Value playerValue(const std::string& name);
    std::vector<ScriptPlayer> players() const;
    std::shared_ptr<Signal> touchedFor(int index);
    void movePart(int index, const rblxmath::vec3& position, bool stopMotion);
    void refreshBody(Part& part);
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

void WorldRuntimeState::movePart(int index, const rblxmath::vec3& position, bool stopMotion) {
    Part* part = partAt(*parts, index);
    if (part == nullptr || part->deleted) return;
    if (!std::isfinite(position.x) || !std::isfinite(position.y) || !std::isfinite(position.z)) {
        return;
    }

    part->position = position;
    if (part->physicsBody == nullptr || physics == nullptr) return;

    auto* body = static_cast<rblxphysics::RigidBody*>(part->physicsBody);
    const rblxphysics::Transform current = body->getTransform();
    body->setTransform(rblxphysics::Transform(
        rblxphysics::Vector3(position.x, position.y, position.z), current.getOrientation()));
    if (stopMotion) {
        body->setLinearVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
        body->setAngularVelocity(rblxphysics::Vector3(0.0f, 0.0f, 0.0f));
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

void WorldRuntimeState::updateTouches() {
    if (parts == nullptr) return;
    const std::vector<ScriptPlayer> currentPlayers = players();
    std::set<std::pair<int, std::string>> next;

    for (auto& entry : touchedSignals) {
        if (!entry.second || entry.second->empty()) continue;
        Part* listener = partAt(*parts, entry.first);
        if (listener == nullptr || listener->deleted || listener->isFolder) continue;

        const rblxmath::vec3 listenerHalf = listener->size * 0.5f;
        for (const ScriptPlayer& player : currentPlayers) {
            // Player characters are roughly a 2 x 5 x 1 box, matching the client.
            const rblxmath::vec3 playerHalf(1.0f, 2.5f, 0.5f);
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
            rblxmath::vec3 position;
            if (!readVector3(value, position)) return false;
            state_->movePart(index_, position, false);
            return true;
        }
        if (name == "Size") {
            rblxmath::vec3 size;
            if (!readVector3(value, size)) return false;
            part->size.x = std::clamp(size.x, 0.05f, 512.0f);
            part->size.y = std::clamp(size.y, 0.05f, 512.0f);
            part->size.z = std::clamp(size.z, 0.05f, 512.0f);
            state_->refreshBody(*part);
            return true;
        }
        if (name == "Color") {
            rblxmath::vec3 color;
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
            rblxmath::vec3 position;
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

    bool call(Interpreter&, const std::string& method, const std::vector<Value>& args,
              Value& out, std::string& error) override {
        if (method == "Teleport") {
            rblxmath::vec3 position;
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
            Value children = rblx::script::makeTable();
            for (std::size_t i = 0; i < state_->parts->size(); ++i) {
                const Part& part = (*state_->parts)[i];
                if (part.deleted || part.isFolder) continue;
                children.table->elements.push_back(state_->partValue(static_cast<int>(i)));
            }
            out = children;
            return true;
        }
        if (method == "PlaySound") {
            if (args.empty() || !args[0].isString()) {
                error = "PlaySound expects a sound name";
                return false;
            }
            // Checked on this side of the wire: the same typo would reach the
            // client as a name it cannot resolve, which is a noise that never
            // comes and nothing to look at.
            if (rblx::audio::soundByName(args[0].string) == nullptr) {
                std::string known;
                for (const std::string& name : rblx::audio::soundNames()) {
                    if (!known.empty()) {
                        known += ", ";
                    }
                    known += name;
                }
                error = "there is no sound '" + args[0].string + "'; this build has " + known;
                return false;
            }
            if (!state_->soundHandler) {
                error = "sounds cannot be played here";
                return false;
            }

            rblxmath::vec3 position(0.0f);
            bool spatial = false;
            if (args.size() >= 2) {
                if (!readVector3(args[1], position)) {
                    error = "PlaySound expects a Vector3 position";
                    return false;
                }
                spatial = true;
            }
            float volume = 1.0f;
            if (args.size() >= 3) {
                if (!args[2].isNumber()) {
                    error = "PlaySound expects a number for its volume";
                    return false;
                }
                volume = static_cast<float>(args[2].number);
            }

            state_->soundHandler(args[0].string, position, volume, spatial);
            out = Value::nil();
            return true;
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
            rblxmath::vec3 position;
            rblxmath::vec3 size;
            rblxmath::vec3 color;
            if (!readVector3(args[1], position) || !readVector3(args[2], size) ||
                !readVector3(args[3], color)) {
                error = "CreatePart expects Vector3 values for position, size and color";
                return false;
            }
            for (const rblxmath::vec3& value : {position, size, color}) {
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

            Part part(rblxmath::vec3(0.0f), rblxmath::vec3(1.0f), rblxmath::vec3(0.6f), ShapeType::Cube,
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
        if (name != "Heartbeat") return false;
        out = rblx::script::makeSignalValue(*state_->heartbeatSignal);
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
        if (name == "PlayerCount") {
            out = Value::numberValue(static_cast<double>(state_->players().size()));
            return true;
        }
        return false;
    }

    bool call(Interpreter&, const std::string& method, const std::vector<Value>&,
              Value& out, std::string& error) override {
        if (method == "GetPlayers") {
            Value players = rblx::script::makeTable();
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

}  // namespace

Value WorldRuntimeState::partValue(int index) {
    return Value::objectValue(std::make_shared<PartObject>(shared_from_this(), index));
}

Value WorldRuntimeState::playerValue(const std::string& name) {
    return Value::objectValue(std::make_shared<PlayerObject>(shared_from_this(), name));
}

WorldRuntime::WorldRuntime(std::deque<Part>& parts, PhysicsWorld* physicsWorld,
                           rblx::script::Interpreter& interpreter) {
    state_ = std::make_shared<WorldRuntimeState>();
    state_->parts = &parts;
    state_->physics = physicsWorld;
    state_->interpreter = &interpreter;

    const Value workspace = Value::objectValue(std::make_shared<WorkspaceObject>(state_));
    interpreter.setGlobal("workspace", workspace);

    Value vector3 = rblx::script::makeTable();
    vector3.table->fields["new"] = rblx::script::makeNativeFunction(
        [](Interpreter&, const std::vector<Value>& args) {
            const double x = args.size() > 0 ? args[0].asNumber() : 0.0;
            const double y = args.size() > 1 ? args[1].asNumber() : 0.0;
            const double z = args.size() > 2 ? args[2].asNumber() : 0.0;
            return makeVector3(x, y, z);
        });
    interpreter.setGlobal("Vector3", vector3);

    interpreter.setGlobal(
        "RunService", Value::objectValue(std::make_shared<RunServiceObject>(state_)));
    interpreter.setGlobal(
        "Players", Value::objectValue(std::make_shared<PlayersObject>(state_)));

    Value game = rblx::script::makeTable();
    game.table->fields["Workspace"] = workspace;
    game.table->fields["Name"] = Value::stringValue("Sky Platformer");
    interpreter.setGlobal("game", game);
}

WorldRuntime::~WorldRuntime() = default;

void WorldRuntime::setPlayerSource(std::function<std::vector<ScriptPlayer>()> source) {
    state_->playerSource = std::move(source);
}

void WorldRuntime::setTeleportHandler(
    std::function<void(const std::string&, const rblxmath::vec3&)> handler) {
    state_->teleportHandler = std::move(handler);
}

void WorldRuntime::setDamageHandler(
    std::function<void(const std::string&, float)> handler) {
    state_->damageHandler = std::move(handler);
}

void WorldRuntime::setSoundHandler(
    std::function<void(const std::string&, const rblxmath::vec3&, float, bool)> handler) {
    state_->soundHandler = std::move(handler);
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
    state_->heartbeatSignal->fire(*state_->interpreter,
                                  {Value::numberValue(static_cast<double>(deltaTime))});
    state_->updateTouches();
}

bool WorldRuntime::isRunning() const { return state_->running; }

void WorldRuntime::stop() { state_->running = false; }

}  // namespace worldscript
}  // namespace rblx
