// Headless trainer: runs a .world with no window, lets a small policy drive the
// same Character and PhysicsWorld the game runs, and improves that policy with
// the cross-entropy method. A policy learned here is a plain table of weights, so
// the result can be replayed or copied into anything that can read five floats.
//
// The point of reusing Character instead of simulating a box: walk speed, jump,
// friction and the death plane all behave exactly as they do in the game, so a
// policy is not trained against a world that the game does not have.

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <functional>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <rblxmath/rblxmath.hpp>

#include "Camera.h"
#include "Character.h"
#include "HardwareId.h"
#include "HttpClient.h"
#include "Network.h"
#include "Part.h"
#include "PhysicsWorld.h"
#include "TrainerGui.h"
#include "UdpTransport.h"
#include "WorldLoader.h"
#include "WebJson.h"

namespace {

// The window's own types live in rblx::trainer; named here so the training code
// reads the same as the rest of this file.
using AgentMarker = rblx::trainer::AgentMarker;
using TrainerFrame = rblx::trainer::Frame;
using TrainerWindow = rblx::trainer::TrainerWindow;

constexpr float kStepSeconds = 1.0f / 60.0f;
constexpr int kObservationCount = 8;
constexpr int kActionCount = 6;  // forward, backward, left, right, jump, idle
constexpr int kParameterCount = kObservationCount * kActionCount;
constexpr float kGoalRadius = 4.0f;
// How far below the spawn the character may drop before the episode is a fall.
// The game's own death plane sits at -185; stopping well above it keeps a round
// short instead of letting it burn its whole step budget in free fall.
constexpr float kFallLimit = 60.0f;
constexpr int kDefaultSteps = 900;
constexpr int kDefaultGenerations = 60;
constexpr int kDefaultCandidates = 12;
constexpr int kDefaultEpisodesPerCandidate = 1;
// --watch redraws the map every kFrameStride steps and then waits, so a run
// plays at a speed a person can follow instead of finishing in one blink.
constexpr int kFrameStride = 4;
constexpr int kFrameDelayMilliseconds = 16;
// Agents stand this far apart across X when a batch starts, so four of them
// spawn beside each other instead of inside each other.
constexpr float kAgentSpacing = 2.0f;
// Positions kept per agent for the window's trail. A hotel run is sixteen
// thousand steps long, so a complete trail would cost more memory than the
// picture is worth.
constexpr std::size_t kTrailPoints = 2048;
constexpr int kMaxAgents = 8;

// One row of weights per action, one column per observation. The idle action
// scores exactly zero, so an action has to earn a positive score to be taken.
struct Policy {
    std::vector<float> weights = std::vector<float>(kParameterCount, 0.0f);

    int act(const float* observation) const {
        int bestAction = kActionCount - 1;
        float bestScore = 0.0f;
        for (int action = 0; action < kActionCount; ++action) {
            float score = 0.0f;
            for (int i = 0; i < kObservationCount; ++i) {
                score += weights[static_cast<std::size_t>(action) * kObservationCount + i] *
                         observation[i];
            }
            if (score > bestScore + 1.0e-6f) {
                bestScore = score;
                bestAction = action;
            }
        }
        return bestAction;
    }
};

struct EpisodeResult {
    float reward = 0.0f;
    float distance = 0.0f;
    rblxmath::vec3 finalPosition{0.0f, 0.0f, 0.0f};
    int steps = 0;
    bool reached = false;
    bool fell = false;
    // How many checkpoints the episode got past: one for a map course, and a door
    // count for the hotel.
    int targetsPassed = 0;
};

// What the trainer is asked to learn. Map walks to a part in a .world file.
// DoorsHotel builds the hundred-door hotel the Doors script generates, from the
// same seed and the same measurements, and trains the run to the elevator past
// door 100. The hotel is generated here because the script builds it at run time:
// a trainer that only read the .world file would see the lobby and nothing else.
enum class Course { Map, DoorsHotel };

constexpr int kHotelDoors = 100;
constexpr float kHotelPitch = 30.0f;
constexpr float kHotelCornerLength = 26.0f;
constexpr float kHotelHalfWidth = 13.0f;
constexpr float kHotelDoorOneZ = -22.0f;
constexpr float kHotelLobbyEndZ = 26.5f;
// The script's seed, and the same hotel on both sides. 71234 walked its path
// back over itself twice, which this trainer's top-down window drew as a room
// sitting on another; 71241 keeps all hundred rooms clear of each other.
constexpr double kHotelSeed = 71241.0;
// A checkpoint sits at the height the character's own origin has once it stands
// on a floor, so the distance to it is a distance along the corridor rather than
// mostly a difference in height.
constexpr float kHotelWalkY = 3.0f;

// The script's own mixing function, in doubles so the same seed builds the same
// hotel here that the game server builds: an integer-valued double is exact well
// past the largest product below.
double hotelHash(int index, int salt) {
    double n = std::fmod(static_cast<double>(index) * 73856093.0 +
                             static_cast<double>(salt) * 19349663.0 +
                             kHotelSeed * 2654435761.0,
                         1000003.0);
    n = std::fmod(n * 1103515245.0 + 12345.0, 2147483648.0);
    return n;
}

int hotelPick(int index, int salt, int count) {
    return static_cast<int>(std::fmod(hotelHash(index, salt), static_cast<double>(count))) + 1;
}

bool hotelChance(int index, int salt, int percent) {
    return std::fmod(hotelHash(index, salt), 100.0) < static_cast<double>(percent);
}

class TrainingWorld {
public:
    // One character in the world. Its policy is handed to the batch step rather
    // than stored here, so all this holds is where it is and how far it has got.
    struct Agent {
        Character* character = nullptr;
        // This character's own root part, so its feeler rays ignore the part
        // they start inside.
        Part* root = nullptr;
        rblxmath::vec3 position{0.0f};
        rblxmath::vec3 target{0.0f};
        std::size_t targetIndex = 0;
        float previousDistance = 0.0f;
        int lastAction = kActionCount - 1;
        std::deque<rblxmath::vec3> trail;
        EpisodeResult result;
        bool finished = false;
    };

    TrainingWorld(Course course, const std::string& worldPath, const std::string& goalName,
                  std::string* error)
        : course_(course) {
        // The policy acts on fixed world axes: forward is -Z and right is +X, the
        // same mapping the game uses when the camera looks along -Z. Using a
        // rotating camera here would hand the policy a free heading.
        inputCamera_.Yaw = -90.0f;

        physics_.init();
        if (course_ == Course::DoorsHotel) {
            if (!buildHotel(error)) {
                return;
            }
        } else if (!loadMap(worldPath, goalName, error)) {
            return;
        }

        if (targets_.empty()) {
            *error = "the course has nothing to walk to";
            return;
        }

        // One agent is all the constructor has to prove works; a batch asks for
        // the rest the first time it runs.
        prepareAgents(1);
    }

    ~TrainingWorld() {
        for (Agent& agent : agents_) {
            delete agent.character;
            agent.character = nullptr;
        }
    }

    bool ready() const { return !agents_.empty() && agents_.front().character != nullptr; }
    bool doorsCourse() const { return course_ == Course::DoorsHotel; }
    std::size_t partCount() const { return parts_.size(); }
    const std::deque<Part>& parts() const { return parts_; }
    const std::string& goalName() const { return goalName_; }
    rblxmath::vec3 spawn() const { return spawn_; }
    rblxmath::vec3 goal() const { return targets_.back(); }
    std::size_t targetCount() const { return targets_.size(); }
    float startDistance() const { return rblxmath::length(targets_.front() - spawn_); }
    const std::vector<rblxmath::vec3>& targets() const { return targets_; }
    // The agents as the last batch left them, which is what the window draws.
    const std::vector<Agent>& agents() const { return agents_; }

    // Whether steps keep a trail for the window to draw. Off by default: a
    // console run never shows one, and a long episode would pay for it.
    void recordTrails(bool record) { recordTrails_ = record; }

    // What one live step did, which is what --play sends to a game server.
    struct LiveStep {
        rblxmath::vec3 position{0.0f};
        float yaw = 0.0f;
        bool walking = false;
        bool jumping = false;
        float reward = 0.0f;
        int targetsPassed = 0;
        bool reached = false;
        bool fell = false;
    };

    // Puts the one agent a live run drives back on the spawn line. Separate from
    // runBatch because a live step is paced by the network, not by a step loop.
    void beginLiveRun() {
        prepareAgents(1);
        liveStep_ = 0;
    }

    // One step of the policy and where it ended up: exactly what a trained
    // episode does, one step at a time. --play sends this to a game server, so
    // the run watched there is on the course the policy was taught.
    LiveStep stepLive(const Policy& policy) {
        Agent& agent = agents_.front();
        float observation[kObservationCount];
        observe(agent, observation);
        agent.lastAction = policy.act(observation);
        applyAction(agent, agent.lastAction);
        agent.character->update(kStepSeconds, nullptr, false);
        physics_.update(kStepSeconds);
        advance(agent, liveStep_++);

        LiveStep result;
        result.position = agent.position;
        result.yaw = agent.character->debugCurrentYaw;
        result.walking = agent.lastAction >= 0 && agent.lastAction <= 3;
        // A jump keeps the character in the air for several steps after the
        // action that started it, so the character is asked rather than the policy.
        result.jumping = agent.character->isJumping;
        result.reward = agent.result.reward;
        result.targetsPassed = agent.result.targetsPassed;
        result.reached = agent.result.reached;
        result.fell = agent.result.fell;
        return result;
    }

    // The server rejected a step and sent the position it believes in. Follow it:
    // a character left where the trainer wanted it would be corrected every step.
    void acceptServerPosition(const rblxmath::vec3& position) {
        if (agents_.empty() || agents_.front().character == nullptr) {
            return;
        }
        Agent& agent = agents_.front();
        agent.character->teleportTo(position);
        agent.character->update(0.0f, nullptr, false);
        agent.position = agent.character->getPosition();
        agent.previousDistance = rblxmath::length(agent.target - agent.position);
    }

    // Called once per simulated step, which is how --watch turns an episode into
    // frames without teaching the trainer anything about drawing.
    using StepObserver = std::function<void(int step, int action, const rblxmath::vec3& position,
                                            float distance, float reward)>;

    // Called once per step of a batch; returning false stops the batch, which is
    // how closing the window ends a watched run early.
    using BatchObserver = std::function<bool(int step)>;

    EpisodeResult runEpisode(const Policy& policy, int maxSteps,
                             const StepObserver& observer = StepObserver()) {
        const std::vector<const Policy*> policies(1, &policy);
        BatchObserver tick;
        if (observer) {
            tick = [this, &observer](int step) {
                const Agent& agent = agents_.front();
                observer(step, agent.lastAction, agent.position, agent.previousDistance,
                         agent.result.reward);
                return true;
            };
        }
        return runBatch(policies, maxSteps, tick).front();
    }

    // Runs one episode for every policy at once and returns one result per
    // policy. Every agent observes, acts and moves in the same step, and one
    // physics step moves all of them, so a batch costs a physics update per step
    // rather than one per agent.
    std::vector<EpisodeResult> runBatch(const std::vector<const Policy*>& policies, int maxSteps,
                                        const BatchObserver& observer = BatchObserver()) {
        if (policies.empty()) {
            return std::vector<EpisodeResult>(1);
        }
        const std::size_t count = policies.size();
        prepareAgents(count);

        std::vector<EpisodeResult> results(count);
        std::size_t running = count;
        for (int step = 0; step < maxSteps && running > 0; ++step) {
            for (std::size_t index = 0; index < count; ++index) {
                Agent& agent = agents_[index];
                if (agent.finished) continue;
                float observation[kObservationCount];
                observe(agent, observation);
                agent.lastAction = policies[index]->act(observation);
                applyAction(agent, agent.lastAction);
                agent.character->update(kStepSeconds, nullptr, false);
            }
            physics_.update(kStepSeconds);

            for (std::size_t index = 0; index < count; ++index) {
                Agent& agent = agents_[index];
                if (agent.finished) continue;
                advance(agent, step);
                if (agent.result.reached || agent.result.fell) {
                    agent.finished = true;
                    --running;
                }
            }

            if (observer && !observer(step)) {
                break;
            }
        }

        for (std::size_t index = 0; index < count; ++index) {
            Agent& agent = agents_[index];
            agent.result.distance = agent.previousDistance;
            agent.result.finalPosition = agent.position;
            results[index] = agent.result;
        }
        return results;
    }

private:
    // One step of reward and checkpoint bookkeeping for a single agent. Split out
    // of runBatch so the batch loop stays three lines long per phase.
    void advance(Agent& agent, int step) {
        agent.position = agent.character->getPosition();
        const float distance = rblxmath::length(agent.target - agent.position);
        // Progress toward the checkpoint being walked to is the reward, with a
        // small per-step cost so dawdling loses to walking a straight line.
        agent.result.reward += (agent.previousDistance - distance) - 0.01f;
        agent.previousDistance = distance;
        agent.result.steps = step + 1;
        if (recordTrails_) {
            agent.trail.push_back(agent.position);
            if (agent.trail.size() > kTrailPoints) {
                agent.trail.pop_front();
            }
        }

        if (distance <= kGoalRadius) {
            ++agent.result.targetsPassed;
            agent.targetIndex = static_cast<std::size_t>(agent.result.targetsPassed);
            if (agent.targetIndex >= targets_.size()) {
                agent.result.reward += 50.0f;
                agent.result.reached = true;
                return;
            }
            // The hotel run is long enough that the final bonus says nothing
            // about how far a policy got, so every door is worth something on its
            // own and the score reads as a door count.
            if (course_ == Course::DoorsHotel) {
                agent.result.reward += 4.0f;
            }
            agent.target = targets_[agent.targetIndex];
            agent.previousDistance = rblxmath::length(agent.target - agent.position);
            return;
        }
        if (agent.position.y < spawn_.y - kFallLimit) {
            agent.result.reward -= 25.0f;
            agent.result.fell = true;
        }
    }

    // Makes sure there are `count` characters and puts every one of them back on
    // the spawn line. The lateral offset keeps them from starting inside each
    // other, which the physics engine would spend the first steps resolving.
    void prepareAgents(std::size_t count) {
        while (agents_.size() < count) {
            Agent agent;
            const std::size_t firstPart = parts_.size();
            agent.character = new Character(spawn_, &physics_, &parts_);
            agent.character->update(0.0f, nullptr, false);
            for (std::size_t index = firstPart; index < parts_.size(); ++index) {
                Part& part = parts_[index];
                if (!part.deleted && part.name == "HumanoidRootPart") {
                    agent.root = &part;
                    break;
                }
            }
            agents_.push_back(agent);
        }

        for (std::size_t index = 0; index < count; ++index) {
            Agent& agent = agents_[index];
            const rblxmath::vec3 start = spawnFor(index, count);
            agent.character->teleportTo(start);
            agent.character->update(0.0f, nullptr, false);
            agent.targetIndex = 0;
            agent.target = targets_.front();
            agent.lastAction = kActionCount - 1;
            agent.result = EpisodeResult();
            agent.trail.clear();
            agent.finished = false;
        }
        // One step settles every character on the spawn floor before the first
        // observation, exactly as the single-agent episode used to.
        physics_.update(kStepSeconds);
        for (std::size_t index = 0; index < count; ++index) {
            Agent& agent = agents_[index];
            agent.position = agent.character->getPosition();
            agent.previousDistance = rblxmath::length(agent.target - agent.position);
        }
    }

    // Every course here runs along Z, so agents line up across X, the direction
    // with room to stand beside each other.
    rblxmath::vec3 spawnFor(std::size_t index, std::size_t count) const {
        const float centre = (static_cast<float>(count) - 1.0f) * 0.5f;
        return spawn_ +
               rblxmath::vec3((static_cast<float>(index) - centre) * kAgentSpacing, 0.0f, 0.0f);
    }

    bool loadMap(const std::string& worldPath, const std::string& goalName, std::string* error) {
        WorldLoader::loadWorld(worldPath, parts_, physics_);
        if (parts_.empty()) {
            *error = "the world file has no parts";
            return false;
        }

        bool hasSpawn = false;
        for (const Part& part : parts_) {
            if (!part.deleted && part.isSpawn) {
                spawn_ = part.position + rblxmath::vec3(0.0f, 5.0f, 0.0f);
                hasSpawn = true;
                break;
            }
        }
        if (!hasSpawn) {
            spawn_ = rblxmath::vec3(0.0f, 10.0f, 0.0f);
        }

        const Part* goalPart = nullptr;
        for (const Part& part : parts_) {
            if (!part.deleted && !part.isFolder && part.name == goalName) {
                goalPart = &part;
                break;
            }
        }
        if (goalPart == nullptr) {
            // No part by that name: fall back to the part furthest from the spawn,
            // which is the end of the map for the worlds this ships with.
            float furthest = -1.0f;
            for (const Part& part : parts_) {
                if (part.deleted || part.isFolder) continue;
                const float distance = rblxmath::length(part.position - spawn_);
                if (distance > furthest) {
                    furthest = distance;
                    goalPart = &part;
                }
            }
        }
        if (goalPart == nullptr) {
            *error = "the world has no part to walk to";
            return false;
        }
        targets_.assign(1, goalPart->position);
        goalName_ = goalPart->name;
        return true;
    }

    // The hotel, built the way games/doors-hotel.rbx builds it: the path is laid
    // out first, one room at a time, and each room then places its floor, walls
    // and doorway in its own frame of (along, across). Rooms the seed turns are
    // square and hand the corridor off through a side wall; the rest run straight
    // or hold a divider or a pillar for the policy to walk around.
    bool buildHotel(std::string* error) {
        struct Room {
            int dirX = 0;
            int dirZ = -1;
            int acrX = 1;
            int acrZ = 0;
            double entryX = 0.0;
            double entryZ = 0.0;
            float length = kHotelPitch;
            int turn = 0;
            int kind = 1;
        };
        static const int kDirX[4] = {0, 1, 0, -1};
        static const int kDirZ[4] = {-1, 0, 1, 0};

        std::vector<Room> rooms(static_cast<std::size_t>(kHotelDoors));
        int dir = 1;
        double ex = 0.0;
        double ez = kHotelDoorOneZ;
        int sinceTurn = 0;
        for (int index = 1; index <= kHotelDoors; ++index) {
            Room& room = rooms[static_cast<std::size_t>(index - 1)];
            room.dirX = kDirX[dir - 1];
            room.dirZ = kDirZ[dir - 1];
            room.acrX = -room.dirZ;
            room.acrZ = room.dirX;
            room.entryX = ex;
            room.entryZ = ez;

            // A turn needs straight rooms on either side of it, and the first and
            // last rooms keep the lobby and the elevator where the world expects.
            int side = 0;
            if (sinceTurn >= 3 && index > 3 && index < kHotelDoors - 2 &&
                hotelChance(index, 21, 24)) {
                side = hotelPick(index, 22, 2);
                if (side == 2) {
                    side = -1;
                }
            }
            ++sinceTurn;

            int kind = 1;
            float length = kHotelPitch;
            if (side != 0) {
                kind = 2;
                length = kHotelCornerLength;
                sinceTurn = 0;
            } else {
                const int rolled = hotelPick(index, 23, 3);
                if (rolled == 2) {
                    kind = 3;
                }
                if (rolled == 3) {
                    kind = 4;
                }
            }
            room.turn = side;
            room.kind = kind;
            room.length = length;

            if (side == 0) {
                ex += room.dirX * length;
                ez += room.dirZ * length;
            } else {
                ex += room.dirX * (length * 0.5) + room.acrX * (kHotelHalfWidth * side);
                ez += room.dirZ * (length * 0.5) + room.acrZ * (kHotelHalfWidth * side);
                dir += side;
                if (dir > 4) {
                    dir -= 4;
                }
                if (dir < 1) {
                    dir += 4;
                }
            }
        }

        const rblxmath::vec3 floorColor(0.11f, 0.095f, 0.115f);
        const rblxmath::vec3 wallColor(0.16f, 0.15f, 0.17f);
        auto add = [this](const std::string& name, double x, float y, double z, float sizeX,
                          float sizeY, float sizeZ, const rblxmath::vec3& color) {
            Part part(rblxmath::vec3(static_cast<float>(x), y, static_cast<float>(z)),
                      rblxmath::vec3(sizeX, sizeY, sizeZ), color, ShapeType::Cube, name);
            parts_.push_back(part);
            physics_.addPart(&parts_.back());
        };
        // A part in a room's own frame. The size is given along the heading and
        // across it, so a room on either axis is the same call.
        auto place = [&add](const Room& room, const std::string& stem, int index, double u,
                            double v, float y, float sizeAlong, float sizeAcross, float sizeY,
                            const rblxmath::vec3& color) {
            const double x = room.entryX + room.dirX * u + room.acrX * v;
            const double z = room.entryZ + room.dirZ * u + room.acrZ * v;
            const float sizeX = std::fabs(static_cast<float>(room.dirX)) * sizeAlong +
                                std::fabs(static_cast<float>(room.acrX)) * sizeAcross;
            const float sizeZ = std::fabs(static_cast<float>(room.dirZ)) * sizeAlong +
                                std::fabs(static_cast<float>(room.acrZ)) * sizeAcross;
            add(stem + std::to_string(index), x, y, z, sizeX, sizeY, sizeZ, color);
        };

        const double lobbyCentre = (kHotelLobbyEndZ + kHotelDoorOneZ) * 0.5;
        const float lobbyLength = static_cast<float>(kHotelLobbyEndZ - kHotelDoorOneZ);
        add("LobbyFloor", 0.0, -0.5f, lobbyCentre, 26.0f, 1.0f, lobbyLength, floorColor);
        add("LobbyWallWest", -13.5, 6.0f, lobbyCentre, 1.0f, 12.0f, lobbyLength, wallColor);
        add("LobbyWallEast", 13.5, 6.0f, lobbyCentre, 1.0f, 12.0f, lobbyLength, wallColor);
        add("LobbyBackWall", 0.0, 6.0f, kHotelLobbyEndZ + 0.5, 28.0f, 12.0f, 1.0f, wallColor);

        targets_.clear();
        for (int index = 1; index <= kHotelDoors; ++index) {
            const Room& room = rooms[static_cast<std::size_t>(index - 1)];
            const float length = room.length;
            const int side = room.turn;

            place(room, "RoomFloor", index, length * 0.5, 0.0, -0.5f, length,
                  kHotelHalfWidth * 2.0f, 1.0f, floorColor);
            if (side == 0) {
                place(room, "RoomWallWest", index, length * 0.5, -kHotelHalfWidth - 0.5, 6.0f,
                      length, 1.0f, 12.0f, wallColor);
                place(room, "RoomWallEast", index, length * 0.5, kHotelHalfWidth + 0.5, 6.0f,
                      length, 1.0f, 12.0f, wallColor);
            } else {
                place(room, "RoomWallSide", index, length * 0.5,
                      -side * (kHotelHalfWidth + 0.5), 6.0f, length, 1.0f, 12.0f, wallColor);
                place(room, "RoomBack", index, length + 0.5, 0.0, 6.0f, 1.0f,
                      kHotelHalfWidth * 2.0f, 12.0f, wallColor);
            }
            // The doorway into the room: the two partitions leave a six-wide gap,
            // which is where the game's door slides up out of the way.
            place(room, "PartitionWest", index, 0.0, -8.0, 6.0f, 1.0f, 10.0f, 12.0f, wallColor);
            place(room, "PartitionEast", index, 0.0, 8.0, 6.0f, 1.0f, 10.0f, 12.0f, wallColor);

            if (room.kind == 3) {
                const int gap = hotelPick(index, 24, 2) == 2 ? 1 : -1;
                place(room, "Divider", index, length * 0.35, gap * 4.5, 6.0f, 1.0f, 17.0f, 12.0f,
                      wallColor);
            }
            if (room.kind == 4) {
                place(room, "Pillar", index, length * 0.5, 0.0, 5.5f, 6.0f, 8.0f, 11.0f,
                      wallColor);
            }

            targets_.push_back(rblxmath::vec3(
                static_cast<float>(room.entryX + room.dirX * (length * 0.5)), kHotelWalkY,
                static_cast<float>(room.entryZ + room.dirZ * (length * 0.5))));
        }

        // The elevator, four studs short of the last room's far wall.
        const Room& last = rooms.back();
        const float elevatorU = last.length - 4.0f;
        targets_.push_back(rblxmath::vec3(
            static_cast<float>(last.entryX + last.dirX * elevatorU), kHotelWalkY,
            static_cast<float>(last.entryZ + last.dirZ * elevatorU)));

        spawn_ = rblxmath::vec3(0.0f, 6.2f, 0.0f);
        goalName_ = "Door100";
        if (!error) {
            return true;
        }
        return true;
    }

    void observe(const Agent& agent, float* out) {
        const rblxmath::vec3 delta = agent.target - agent.position;
        const float distance = rblxmath::length(delta);
        out[0] = std::clamp(delta.x / 40.0f, -1.0f, 1.0f);
        out[1] = std::clamp(delta.z / 40.0f, -1.0f, 1.0f);
        out[2] = std::clamp(distance / 100.0f, 0.0f, 1.0f);
        out[3] = std::clamp((agent.position.y - spawn_.y) / 10.0f, -1.0f, 1.0f);
        out[4] = 1.0f;  // bias, so a weight can act as a per-action threshold
        // One feeler per movement axis. Without these the policy can only learn
        // to walk straight at the goal, which parks it against the first wall or
        // parked car in the way; with them it can learn to steer around one.
        out[5] = freeAhead(agent, rblxmath::vec3(0.0f, 0.0f, -1.0f));
        out[6] = freeAhead(agent, rblxmath::vec3(-1.0f, 0.0f, 0.0f));
        out[7] = freeAhead(agent, rblxmath::vec3(1.0f, 0.0f, 0.0f));
    }

    // How much of a fixed-length probe in this direction is clear, 0 at a wall
    // to 1 when nothing is within reach.
    float freeAhead(const Agent& agent, const rblxmath::vec3& direction) {
        constexpr float kFeelDistance = 12.0f;
        PhysicsWorld::RaycastResult hit;
        if (physics_.raycast(agent.position, agent.position + direction * kFeelDistance, hit,
                             const_cast<Part*>(agent.root))) {
            return std::clamp(hit.hitFraction, 0.0f, 1.0f);
        }
        return 1.0f;
    }

    void applyAction(Agent& agent, int action) {
        const bool forward = action == 0;
        const bool backward = action == 1;
        const bool left = action == 2;
        const bool right = action == 3;
        const bool jump = action == 4;
        agent.character->processInput(forward, backward, left, right, jump, inputCamera_, 0.0f,
                                      kStepSeconds);
    }

    std::deque<Part> parts_;
    PhysicsWorld physics_;
    std::vector<Agent> agents_;
    Course course_ = Course::Map;
    rblxmath::vec3 spawn_{0.0f, 0.0f, 0.0f};
    // The checkpoint being walked to, and every checkpoint after it. A map course
    // has one; the hotel has a door per room and the elevator after the last.
    std::vector<rblxmath::vec3> targets_;
    std::string goalName_;
    Camera inputCamera_;
    bool recordTrails_ = false;
    int liveStep_ = 0;
};

// Top-down view used by --watch. Rotated parts are drawn from their axis-aligned
// size, so an angled wall covers a little more than it really does; that is fine
// for a view whose job is to show where the policy walked.
struct MapView {
    float originX = 0.0f;
    float originZ = 0.0f;
    float cellSize = 1.0f;
    int columns = 0;
    int rows = 0;
    std::vector<char> terrain;
    std::vector<unsigned char> tall;
};

MapView buildMapView(const std::deque<Part>& parts, const rblxmath::vec3& spawn) {
    MapView view;
    float minX = 1.0e30f;
    float maxX = -1.0e30f;
    float minZ = 1.0e30f;
    float maxZ = -1.0e30f;
    for (const Part& part : parts) {
        if (part.deleted || part.isFolder) continue;
        minX = std::min(minX, part.position.x - part.size.x * 0.5f);
        maxX = std::max(maxX, part.position.x + part.size.x * 0.5f);
        minZ = std::min(minZ, part.position.z - part.size.z * 0.5f);
        maxZ = std::max(maxZ, part.position.z + part.size.z * 0.5f);
    }
    if (!(maxX > minX) || !(maxZ > minZ)) {
        return view;
    }

    constexpr int kMaxColumns = 74;
    constexpr int kMaxRows = 26;
    const float spanX = maxX - minX;
    const float spanZ = maxZ - minZ;
    view.cellSize = std::max(std::max(spanX / kMaxColumns, spanZ / kMaxRows), 0.5f);
    view.columns = std::max(1, static_cast<int>(std::ceil(spanX / view.cellSize)));
    view.rows = std::max(1, static_cast<int>(std::ceil(spanZ / view.cellSize)));
    view.originX = minX;
    view.originZ = minZ;
    view.terrain.assign(static_cast<std::size_t>(view.columns * view.rows), ' ');
    view.tall.assign(static_cast<std::size_t>(view.columns * view.rows), 0);

    for (int row = 0; row < view.rows; ++row) {
        for (int column = 0; column < view.columns; ++column) {
            const float x = view.originX + (static_cast<float>(column) + 0.5f) * view.cellSize;
            const float z = view.originZ + (static_cast<float>(row) + 0.5f) * view.cellSize;
            float top = -1.0e30f;
            for (const Part& part : parts) {
                if (part.deleted || part.isFolder) continue;
                if (x < part.position.x - part.size.x * 0.5f) continue;
                if (x > part.position.x + part.size.x * 0.5f) continue;
                if (z < part.position.z - part.size.z * 0.5f) continue;
                if (z > part.position.z + part.size.z * 0.5f) continue;
                top = std::max(top, part.position.y + part.size.y * 0.5f);
            }
            if (top > -1.0e29f) {
                const std::size_t index = static_cast<std::size_t>(row * view.columns + column);
                // Anything the character could stand on is drawn as floor; only
                // geometry standing above it counts as a wall, otherwise a large
                // ground slab would fill the whole map and hide the walk.
                const bool obstacle = top > spawn.y + 1.5f;
                view.terrain[index] = obstacle ? '#' : '_';
                view.tall[index] = obstacle ? 1 : 0;
            }
        }
    }
    return view;
}

const char* actionName(int action) {
    static const char* names[kActionCount] = {"forward", "backward", "left",
                                              "right",   "jump",     "idle"};
    return action >= 0 && action < kActionCount ? names[action] : "idle";
}

void drawMapFrame(const MapView& view, const rblxmath::vec3& spawn, const rblxmath::vec3& goal,
                  const rblxmath::vec3& position, const std::deque<rblxmath::vec3>& trail,
                  const std::string& status) {
    if (view.columns <= 0 || view.rows <= 0) {
        return;
    }

    struct Overlay {
        char glyph = ' ';
        const char* color = "";
    };

    const std::size_t cellCount = static_cast<std::size_t>(view.columns * view.rows);
    std::vector<Overlay> overlays(cellCount);
    const auto mark = [&view, &overlays, cellCount](const rblxmath::vec3& point, char glyph,
                                                    const char* color) {
        const int column = static_cast<int>(std::floor((point.x - view.originX) / view.cellSize));
        const int row = static_cast<int>(std::floor((point.z - view.originZ) / view.cellSize));
        if (column < 0 || column >= view.columns || row < 0 || row >= view.rows) return;
        const std::size_t index = static_cast<std::size_t>(row * view.columns + column);
        if (index >= cellCount) return;
        overlays[index].glyph = glyph;
        overlays[index].color = color;
    };

    for (const rblxmath::vec3& visited : trail) {
        mark(visited, '.', "\x1b[34m");
    }
    mark(spawn, 'S', "\x1b[36m");
    mark(goal, 'G', "\x1b[32m");
    mark(position, '@', "\x1b[33m");

    std::string frame = "\x1b[H\x1b[0m";
    const char* color = "\x1b[0m";
    for (int row = 0; row < view.rows; ++row) {
        for (int column = 0; column < view.columns; ++column) {
            const std::size_t index = static_cast<std::size_t>(row * view.columns + column);
            char glyph = view.terrain[index];
            const char* glyphColor = view.tall[index] ? "\x1b[31m" : "\x1b[90m";
            if (glyph == ' ') {
                glyphColor = "\x1b[0m";
            }
            if (overlays[index].glyph != ' ') {
                glyph = overlays[index].glyph;
                glyphColor = overlays[index].color;
            }
            if (std::strcmp(glyphColor, color) != 0) {
                color = glyphColor;
                frame += color;
            }
            frame += glyph;
        }
        frame += "\x1b[0m\x1b[K\n";
        color = "\x1b[0m";
    }
    frame += status;
    frame += "\n\x1b[J";
    std::fputs(frame.c_str(), stdout);
    std::fflush(stdout);
}

EpisodeResult runWatchedEpisode(TrainingWorld& world, const Policy& policy, int maxSteps,
                                const std::string& label) {
    const MapView view = buildMapView(world.parts(), world.spawn());
    std::deque<rblxmath::vec3> trail;
    // Clear once so the first frame starts from a blank screen; later frames only
    // home the cursor, which avoids a full-screen flash on every step.
    std::fputs("\x1b[2J\x1b[H", stdout);

    const EpisodeResult result = world.runEpisode(
        policy, maxSteps,
        [&](int step, int action, const rblxmath::vec3& position, float distance, float reward) {
            trail.push_back(position);
            if (trail.size() > 4096) {
                trail.pop_front();
            }
            if (step % kFrameStride != 0) {
                return;
            }
            char status[512];
            std::snprintf(status, sizeof(status),
                          "%s   step %d/%d   distance %.1f   action %s   reward %.1f   "
                          "(@ character, G goal, # wall, _ floor)",
                          label.c_str(), step + 1, maxSteps, distance, actionName(action),
                          reward);
            drawMapFrame(view, world.spawn(), world.goal(), position, trail, status);
            std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMilliseconds));
        });

    char status[512];
    std::snprintf(status, sizeof(status),
                  "%s finished   reward %.2f   closest %.1f   %d steps   %s", label.c_str(),
                  result.reward, result.distance, result.steps,
                  result.reached ? "reached the goal"
                                 : (result.fell ? "fell" : "ran out of time"));
    drawMapFrame(view, world.spawn(), world.goal(), result.finalPosition, trail, status);
    std::fputs("\n", stdout);
    return result;
}

// One frame of the window from the world as it stands. Training, replay and the
// closing demonstration all fill it the same way, so they all show the same
// thing: the course, the checkpoints, and every agent on it.
void fillTrainerFrame(TrainingWorld& world, TrainerFrame* frame) {
    frame->parts = &world.parts();
    frame->targets = world.targets();
    frame->spawn = world.spawn();
    frame->goal = world.goal();
    frame->agents.clear();
    for (const TrainingWorld::Agent& agent : world.agents()) {
        AgentMarker marker;
        marker.position = agent.position;
        marker.target = agent.target;
        marker.targetsPassed = agent.result.targetsPassed;
        marker.reward = agent.result.reward;
        marker.reached = agent.result.reached;
        marker.fell = agent.result.fell;
        if (!agent.trail.empty()) {
            marker.trail.assign(agent.trail.begin(), agent.trail.end());
        }
        frame->agents.push_back(std::move(marker));
    }
}

// One episode with the window on: --gui --replay and the closing demonstration
// both run one known policy this way.
EpisodeResult runGuiEpisode(TrainingWorld& world, TrainerWindow& window, const Policy& policy,
                           int maxSteps, const std::string& label, int doorCount) {
    TrainerFrame frame;
    frame.title = label;
    frame.doorCount = doorCount;
    fillTrainerFrame(world, &frame);
    window.present(frame);

    const std::vector<const Policy*> policies(1, &policy);
    const std::vector<EpisodeResult> results = world.runBatch(policies, maxSteps, [&](int step) {
        if (step % kFrameStride != 0) {
            return true;
        }
        fillTrainerFrame(world, &frame);
        const TrainingWorld::Agent& agent = world.agents().front();
        char status[512];
        std::snprintf(status, sizeof(status),
                      "step %d/%d   action %s   reward %.1f   closest %.1f", step + 1, maxSteps,
                      actionName(agent.lastAction), agent.result.reward, agent.previousDistance);
        frame.status = status;
        return window.present(frame);
    });
    return results.front();
}

// A live run stops when the server has been silent this long: a server that died
// must not leave the trainer walking a course only it can see.
constexpr uint64_t kLiveTimeoutMs = 15000;

// The live game server --play drives: one guest connection, joined the way
// Client.exe joins, then one state packet per simulated step. The server accepts
// a client that owns its own character, so a policy in place of a keyboard is a
// client like any other; the movement guard checks its claims like any player's.
class LiveServer {
public:
    ~LiveServer() { close(); }

    bool join(const std::string& host, int port, const std::string& name, std::string* error);
    void close();

    // Reads whatever the server sent: the welcome, a chat line (a refused join is
    // explained there), or the position it moved the character back to.
    void poll(rblxmath::vec3* correction, bool* corrected, std::string* notice);
    bool sendState(const rblxmath::vec3& position, float yaw, bool walking, bool jumping,
                   std::string* error);

    int playerId() const { return playerId_; }
    bool joined() const { return playerId_ >= 0; }
    bool lost() const { return lost_; }
    // Steps the server refused to believe since the join.
    int corrections() const { return corrections_; }
    // The server drops a dead player's movement, so a run that never moves may
    // simply be a dead AI rather than a broken packet.
    bool dead() const { return dead_; }

private:
    uint64_t nowMs() const;
    bool sendDatagram(const char* data, size_t size);
    void handleFrame(const PacketProtocol::ParsedPacket& frame, rblxmath::vec3* correction,
                     bool* corrected, std::string* notice);

    SOCKET socket_ = INVALID_SOCKET;
    rblx::net::UdpSession session_;
    // Bytes of frames that arrived but have not been parsed yet, because a message
    // can hold a frame and a half or several at once.
    std::vector<char> pending_;
    int playerId_ = -1;
    bool lost_ = false;
    int corrections_ = 0;
    bool dead_ = false;
    std::chrono::steady_clock::time_point startedAt_;
};

uint64_t LiveServer::nowMs() const {
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - startedAt_)
                                     .count());
}

bool LiveServer::sendDatagram(const char* data, size_t size) {
    if (socket_ == INVALID_SOCKET) {
        return false;
    }
    return send(socket_, data, static_cast<int>(size), 0) == static_cast<int>(size);
}

bool LiveServer::join(const std::string& host, int port, const std::string& name,
                      std::string* error) {
    close();
    if (!Network::init()) {
        *error = "could not start the network stack";
        return false;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_port = htons(static_cast<u_short>(port));
    if (inet_pton(AF_INET, host.c_str(), &address.sin_addr) != 1) {
        *error = host + " is not an address this machine can join";
        close();
        return false;
    }

    socket_ = Network::createUdpSocket();
    if (socket_ == INVALID_SOCKET) {
        *error = "could not create the game socket";
        close();
        return false;
    }
    Network::setIoBufferSize(socket_, 4 << 20);
    Network::setNonBlocking(socket_);
    if (connect(socket_, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        *error = "could not reach " + host + ":" + std::to_string(port);
        close();
        return false;
    }

    startedAt_ = std::chrono::steady_clock::now();
    session_.reset(0);

    // A guest join: the trainer holds a developer token, not an account, and the
    // server's guest path is what a player without one gets. The machine still
    // goes with the join, so a hardware ban reaches the AI as well.
    PacketConnect packet;
    std::memset(&packet, 0, sizeof(packet));
    std::snprintf(packet.username, sizeof(packet.username), "%s", name.c_str());
    std::snprintf(packet.hardwareId, sizeof(packet.hardwareId), "%s",
                  rblx::hwid::machineId().c_str());
    packet.protocolRevision = PacketProtocol::kProtocolRevision;
    packet.packetLayout = PacketProtocol::packetLayoutSignature();

    const std::vector<char> frame = PacketProtocol::buildStructPacket(PacketType::CONNECT, packet);
    if (!session_.sendMessage(frame.data(), frame.size(), true)) {
        *error = "could not queue the join packet";
        close();
        return false;
    }
    session_.pump(nowMs(), [this](const char* data, size_t size) { return sendDatagram(data, size); });
    return true;
}

void LiveServer::close() {
    if (socket_ == INVALID_SOCKET) {
        return;
    }
    closesocket(socket_);
    socket_ = INVALID_SOCKET;
    pending_.clear();
    playerId_ = -1;
    lost_ = false;
    corrections_ = 0;
    dead_ = false;
    Network::cleanup();
}

void LiveServer::handleFrame(const PacketProtocol::ParsedPacket& frame, rblxmath::vec3* correction,
                             bool* corrected, std::string* notice) {
    if (frame.type == PacketType::WELCOME && frame.payloadSize == sizeof(PacketWelcome)) {
        PacketWelcome welcome;
        std::memcpy(&welcome, frame.payload, sizeof(welcome));
        playerId_ = welcome.playerId;
        return;
    }
    if (frame.type == PacketType::TELEPORT && frame.payloadSize == sizeof(PacketPlayerState)) {
        PacketPlayerState state;
        std::memcpy(&state, frame.payload, sizeof(state));
        ++corrections_;
        *correction = state.position;
        *corrected = true;
        return;
    }
    if (frame.type == PacketType::PLAYER_HEALTH &&
        frame.payloadSize == sizeof(PacketPlayerHealth)) {
        PacketPlayerHealth health;
        std::memcpy(&health, frame.payload, sizeof(health));
        if (playerId_ < 0 || health.playerId != playerId_) {
            return;
        }
        dead_ = health.health <= 0.0f;
        return;
    }
    if (frame.type == PacketType::CHAT && frame.payloadSize == sizeof(PacketChat)) {
        PacketChat chat;
        std::memcpy(&chat, frame.payload, sizeof(chat));
        // A refused join arrives here with the reason, before the server drops the
        // socket, so it is printed instead of retrying into silence.
        notice->append("server: ");
        notice->append(PacketProtocol::fixedString(chat.message, sizeof(chat.message)));
        notice->append("\n");
    }
}

void LiveServer::poll(rblxmath::vec3* correction, bool* corrected, std::string* notice) {
    if (socket_ == INVALID_SOCKET) {
        return;
    }

    std::vector<std::vector<char>> messages;
    char datagram[2048];
    const uint64_t stamp = nowMs();
    for (int i = 0; i < 32; ++i) {
        const int received = recv(socket_, datagram, sizeof(datagram), 0);
        if (received == SOCKET_ERROR) {
            const int code = WSAGetLastError();
            if (code == WSAEWOULDBLOCK) break;
            // An ICMP port-unreachable arrives as WSAECONNRESET; the server may
            // simply not be listening yet, and the join is retried by the session.
            if (code == WSAECONNRESET) continue;
            break;
        }
        if (received <= 0) break;
        session_.receiveDatagram(datagram, static_cast<size_t>(received), stamp, messages);
    }
    session_.pump(stamp, [this](const char* data, size_t size) { return sendDatagram(data, size); });

    if (playerId_ >= 0 && session_.timedOut(stamp, kLiveTimeoutMs)) {
        lost_ = true;
    }

    for (const std::vector<char>& message : messages) {
        pending_.insert(pending_.end(), message.begin(), message.end());
    }

    PacketProtocol::ParsedPacket frame;
    for (;;) {
        const PacketProtocol::ParseStatus status = PacketProtocol::tryReadFrame(pending_, frame);
        if (status == PacketProtocol::ParseStatus::NeedMoreData) {
            break;
        }
        if (status == PacketProtocol::ParseStatus::Invalid) {
            // Nothing after a malformed frame can be trusted to be aligned, and a
            // different build of the server is the usual reason.
            pending_.clear();
            break;
        }
        handleFrame(frame, correction, corrected, notice);
        PacketProtocol::consumeFrame(pending_, frame.frameSize);
    }
}

bool LiveServer::sendState(const rblxmath::vec3& position, float yaw, bool walking, bool jumping,
                           std::string* error) {
    if (socket_ == INVALID_SOCKET || playerId_ < 0) {
        *error = "the game server never answered the join request";
        return false;
    }

    PacketPlayerState state;
    std::memset(&state, 0, sizeof(state));
    state.playerId = playerId_;
    state.position = position;
    state.rotationY = yaw;
    state.isWalking = walking;
    state.isJumping = jumping;
    // The server replaces health with its own, but a zero would read as a dead
    // character on the first packet if it ever stopped doing that.
    state.health = 100.0f;
    // The movement guard measures speed against the client's own clock, so this
    // has to be the time the step was taken rather than the time it is sent.
    state.clientTimeMs = static_cast<uint32_t>(nowMs());

    const std::vector<char> frame = PacketProtocol::buildStructPacket(PacketType::PLAYER_STATE, state);
    if (!session_.sendMessage(frame.data(), frame.size(), false)) {
        *error = "could not queue the player state";
        return false;
    }
    session_.pump(nowMs(), [this](const char* data, size_t size) { return sendDatagram(data, size); });
    return true;
}

struct Options {
    // Where WebApp.exe answers. The trainer has nothing to train without a token
    // from that site, so the address is part of every run rather than a fallback.
    std::string site = "http://127.0.0.1:3000";
    std::string token;
    int gameId = 0;
    std::string out;
    int generations = kDefaultGenerations;
    int candidates = kDefaultCandidates;
    int episodes = kDefaultEpisodesPerCandidate;
    // 0 asks the course for its own budget: a map is a short walk, a hundred-door
    // hotel is not.
    int steps = 0;
    unsigned int seed = 1337;
    bool replay = false;
    bool watch = false;
    // --play shows the saved policy on a running game server instead of training
    // here, so the run is the game's own rather than this process's copy of it.
    bool play = false;
    std::string server = "127.0.0.1";
    int port = 7777;
    std::string playerName = "AiTrainer";
    // The optional window, and how many policies walk the course at once inside
    // it. A batch also trains faster headless, so the count is independent of it.
    bool gui = false;
    int agents = 1;
};

// A hundred rooms of thirty studs and fourteen corners is a little under 3000
// studs of corridor, which a character walking at game speed covers in about four
// minutes of simulated time.
constexpr int kDefaultHotelSteps = 16000;

int defaultStepsFor(const std::string& course) {
    return course == "doors" ? kDefaultHotelSteps : kDefaultSteps;
}

void printUsage() {
    std::printf(
        "Usage: AiTrainer --token <token> --game <id> [options]\n"
        "  --token <token>      developer token from the site's developer portal\n"
        "  --game <id>          game to train on; its owner must have marked it AI\n"
        "                       compatible, and the site names the map and the course\n"
        "  --site <url>         where WebApp.exe answers (default %s)\n"
        "  --generations <n>    training generations (default %d)\n"
        "  --candidates <n>     policies sampled per generation (default %d)\n"
        "  --episodes <n>       episodes per candidate (default %d)\n"
        "  --steps <n>          steps per episode at 60 Hz (default %d, or %d for the\n"
        "                       hundred-door hotel)\n"
        "  --out <file>         where to write the trained policy\n"
        "                       (default models/<world>.policy, or models/doors-hotel.policy)\n"
        "  --replay             load --out and run one episode with it\n"
        "  --play               join a running Server.exe and walk the course\n"
        "                       there with the saved policy, instead of training\n"
        "  --server <host>      game server --play joins (default 127.0.0.1)\n"
        "  --port <n>           its port (default 7777)\n"
        "  --name <text>        the name the AI joins under (default AiTrainer)\n"
        "  --watch              draw the episode as a top-down map while it runs\n"
        "  --gui                draw the course in a window while it trains, seen\n"
        "                       from above with the roof left off\n"
        "  --agents <n>         policies walking the course at once (default 1)\n"
        "  --seed <n>           sampling seed (default 1337)\n",
        "http://127.0.0.1:3000", kDefaultGenerations, kDefaultCandidates,
        kDefaultEpisodesPerCandidate, kDefaultSteps, kDefaultHotelSteps);
}

// Plays a trained policy on a running game server. Nothing is trained and nothing
// is decided locally about the course: the trainer joins the server as a guest,
// walks the same world with the same observations, and sends the position of each
// step, so the run happens in the game. Paced in real time because the server
// measures speed between packets, and a burst would be corrected as a speed hack.
int runLiveGame(TrainingWorld& world, TrainerWindow& window, bool gui, const Options& options,
                int maxSteps, const Policy& policy) {
    LiveServer server;
    std::string error;
    if (!server.join(options.server, options.port, options.playerName, &error)) {
        std::fprintf(stderr, "Could not join %s:%d: %s\n", options.server.c_str(), options.port,
                     error.c_str());
        return 1;
    }

    TrainerFrame frame;
    if (gui) {
        frame.title = "RBLX AI trainer - live";
        frame.doorCount = world.doorsCourse() ? kHotelDoors : 0;
        fillTrainerFrame(world, &frame);
        window.present(frame);
    }

    // The welcome carries the player id every state packet needs, and a refused
    // join is explained in chat before the server drops the socket, so both are
    // read before the run starts instead of after the first packet vanishes.
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    while (!server.joined() && std::chrono::steady_clock::now() < deadline) {
        rblxmath::vec3 correction(0.0f);
        bool corrected = false;
        std::string notice;
        server.poll(&correction, &corrected, &notice);
        std::fputs(notice.c_str(), stdout);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    if (!server.joined()) {
        std::fprintf(stderr,
                     "No welcome from %s:%d. Server.exe has to be running this game on that "
                     "port, built from the same source as this trainer.\n",
                     options.server.c_str(), options.port);
        return 1;
    }
    std::printf("Joined %s:%d as player %d; the run happens on that server.\n",
                options.server.c_str(), options.port, server.playerId());

    world.beginLiveRun();
    // The server chose where its copy of this player stands (the world's
    // SpawnLocation) and measures every step from there. Sending this one position
    // before walking turns a world the trainer loaded differently into a mismatch
    // that is reported once, instead of a correction the guard sends every step.
    const rblxmath::vec3 courseStart = world.agents().front().position;
    if (!server.sendState(courseStart, 0.0f, false, false, &error)) {
        std::fprintf(stderr, "Lost the game server: %s\n", error.c_str());
        return 1;
    }
    const auto syncDeadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(500);
    while (std::chrono::steady_clock::now() < syncDeadline) {
        rblxmath::vec3 correction(0.0f);
        bool corrected = false;
        std::string notice;
        server.poll(&correction, &corrected, &notice);
        std::fputs(notice.c_str(), stdout);
        if (!corrected) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            continue;
        }
        std::printf("The server has the AI at %.1f, %.1f, %.1f, not at this course's start "
                    "(%.1f, %.1f, %.1f). Following the server.\n",
                    correction.x, correction.y, correction.z, courseStart.x, courseStart.y,
                    courseStart.z);
        world.acceptServerPosition(correction);
        break;
    }

    TrainingWorld::LiveStep sample;
    int steps = 0;
    for (int step = 0; step < maxSteps; ++step) {
        sample = world.stepLive(policy);

        rblxmath::vec3 correction(0.0f);
        bool corrected = false;
        std::string notice;
        server.poll(&correction, &corrected, &notice);
        std::fputs(notice.c_str(), stdout);
        if (server.lost()) {
            std::fprintf(stderr, "The game server stopped answering; the run ends here.\n");
            return 1;
        }
        if (server.dead()) {
            std::fprintf(stderr,
                         "The server says the AI is dead, and it ignores a dead player's "
                         "movement, so this run cannot move. Respawn it in the game and run "
                         "--play again.\n");
            return 1;
        }
        if (corrected) {
            if (server.corrections() >= 60) {
                std::fprintf(stderr,
                             "The server has undone 60 steps in a row: it is running a "
                             "different world, spawn or game from the one this course was "
                             "built from, so the walk cannot be accepted. Check that "
                             "Server.exe loaded the same game the trainer trained on.\n");
                return 1;
            }
            if (server.corrections() <= 3) {
                std::printf("The server moved the AI back to %.1f, %.1f, %.1f (step %d).\n",
                            correction.x, correction.y, correction.z, step + 1);
            }
            world.acceptServerPosition(correction);
        }
        const TrainingWorld::Agent& agent = world.agents().front();
        if (!server.sendState(agent.position, sample.yaw, sample.walking, sample.jumping,
                              &error)) {
            std::fprintf(stderr, "Lost the game server: %s\n", error.c_str());
            return 1;
        }

        steps = step + 1;
        if (gui) {
            fillTrainerFrame(world, &frame);
            char status[512];
            std::snprintf(status, sizeof(status),
                          "server player %d   step %d/%d   action %s   doors %d/%d   "
                          "reward %.1f",
                          server.playerId(), step + 1, maxSteps, actionName(agent.lastAction),
                          sample.targetsPassed, world.doorsCourse() ? kHotelDoors : 0,
                          sample.reward);
            frame.status = status;
            if (!window.present(frame)) {
                break;
            }
        }

        if (sample.reached || sample.fell) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(kFrameDelayMilliseconds));
    }

    if (world.doorsCourse()) {
        std::printf("Live run: doors %d/%d, %d steps, %d server corrections, %s\n",
                    sample.targetsPassed, kHotelDoors, steps, server.corrections(),
                    sample.reached ? "reached the elevator"
                                   : (sample.fell ? "fell" : "ran out of time"));
    } else {
        std::printf("Live run: %d steps, %d server corrections, %s\n", steps,
                    server.corrections(),
                    sample.reached ? "reached the goal"
                                   : (sample.fell ? "fell" : "ran out of time"));
    }
    return sample.reached ? 0 : 3;
}

// What the site said to train on. The trainer never decides on its own that a game
// may be played by an AI: the portal's flag and the course live on the game record.
struct Session {
    int gameId = 0;
    std::string title;
    std::string world;
    std::string course = "map";
    std::string goal;
};

std::string withoutTrailingSlash(std::string url) {
    while (!url.empty() && url.back() == '/') {
        url.pop_back();
    }
    return url;
}

bool signIn(const Options& options, Session* session, std::string* error) {
    const std::string url = withoutTrailingSlash(options.site) + "/api/ai/login";
    rblx::web::Json request = rblx::web::Json::makeObject();
    request.set("gameId", options.gameId);
    const rblx::http::Response response =
        rblx::http::request("POST", url, request.dump(), options.token, 8000);
    if (response.networkError()) {
        *error = "could not reach " + url + " (" + response.error +
                 "). Is WebApp.exe running?";
        return false;
    }
    std::string parseError;
    const rblx::web::Json body = rblx::web::Json::parse(response.body, &parseError);
    if (!response.ok() || !body.member("success").asBool(false)) {
        std::string message = body.member("error").asString();
        if (message.empty()) {
            message = "the site answered HTTP " + std::to_string(response.status);
        }
        *error = message;
        return false;
    }

    const rblx::web::Json& game = body.member("game");
    session->gameId = static_cast<int>(game.member("id").asInteger(0));
    session->title = game.member("title").asString();
    session->world = game.member("world").asString();
    session->course = game.member("course").asString("map");
    session->goal = game.member("goal").asString();
    if (session->world.empty()) {
        *error = "the site did not name a world file";
        return false;
    }
    if (session->course != "doors" && session->course != "map") {
        *error = "the game asks for a course this build does not have: " + session->course;
        return false;
    }
    return true;
}

std::string defaultOutputPath(const std::string& worldPath) {
    const std::filesystem::path world(worldPath);
    return (std::filesystem::path("models") / (world.stem().string() + ".policy")).string();
}

bool savePolicy(const Policy& policy, const std::string& path, const TrainingWorld& world,
                std::string* error) {
    const std::filesystem::path target(path);
    if (target.has_parent_path()) {
        std::error_code ignored;
        std::filesystem::create_directories(target.parent_path(), ignored);
    }
    std::ofstream out(path, std::ios::trunc);
    if (!out) {
        *error = "could not open " + path + " for writing";
        return false;
    }
    out << "# RBLX AI policy v1\n";
    out << "course " << (world.doorsCourse() ? "doors" : "map") << '\n';
    out << "goal " << world.goalName() << ' ' << world.goal().x << ' ' << world.goal().y << ' '
        << world.goal().z << '\n';
    out << "observations " << kObservationCount << '\n';
    out << "actions " << kActionCount << '\n';
    for (int action = 0; action < kActionCount; ++action) {
        for (int i = 0; i < kObservationCount; ++i) {
            if (i != 0) out << ' ';
            out << policy.weights[static_cast<std::size_t>(action) * kObservationCount + i];
        }
        out << '\n';
    }
    return true;
}

bool loadPolicy(Policy* policy, const std::string& path, std::string* error) {
    std::ifstream in(path);
    if (!in) {
        *error = "could not open " + path;
        return false;
    }
    std::string line;
    int row = 0;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        if (line.rfind("goal ", 0) == 0 || line.rfind("observations ", 0) == 0 ||
            line.rfind("actions ", 0) == 0 || line.rfind("course ", 0) == 0) {
            continue;
        }
        std::vector<float> rowWeights;
        const char* cursor = line.c_str();
        char* end = nullptr;
        for (int i = 0; i < kObservationCount; ++i) {
            const float value = std::strtof(cursor, &end);
            if (end == cursor) break;
            rowWeights.push_back(value);
            cursor = end;
        }
        if (static_cast<int>(rowWeights.size()) == kObservationCount && row < kActionCount) {
            for (int i = 0; i < kObservationCount; ++i) {
                policy->weights[static_cast<std::size_t>(row) * kObservationCount + i] =
                    rowWeights[static_cast<std::size_t>(i)];
            }
            ++row;
        }
    }
    if (row != kActionCount) {
        *error = path + " does not contain " + std::to_string(kActionCount) + " action rows";
        return false;
    }
    return true;
}

std::vector<float> meanOf(const std::vector<const std::vector<float>*>& population) {
    std::vector<float> mean(kParameterCount, 0.0f);
    if (population.empty()) return mean;
    for (const std::vector<float>* weights : population) {
        for (int i = 0; i < kParameterCount; ++i) {
            mean[static_cast<std::size_t>(i)] += (*weights)[static_cast<std::size_t>(i)];
        }
    }
    const float scale = 1.0f / static_cast<float>(population.size());
    for (float& value : mean) value *= scale;
    return mean;
}

std::vector<float> deviationOf(const std::vector<const std::vector<float>*>& population,
                               const std::vector<float>& mean, float floorValue) {
    std::vector<float> deviation(kParameterCount, 0.0f);
    if (population.size() < 2) {
        std::fill(deviation.begin(), deviation.end(), floorValue);
        return deviation;
    }
    for (const std::vector<float>* weights : population) {
        for (int i = 0; i < kParameterCount; ++i) {
            const float delta = (*weights)[static_cast<std::size_t>(i)] - mean[static_cast<std::size_t>(i)];
            deviation[static_cast<std::size_t>(i)] += delta * delta;
        }
    }
    const float scale = 1.0f / static_cast<float>(population.size());
    for (float& value : deviation) {
        value = std::max(std::sqrt(value * scale), floorValue);
    }
    return deviation;
}

}  // namespace

int main(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        const bool hasValue = i + 1 < argc;
        if (arg == "--token" && hasValue) {
            options.token = argv[++i];
        } else if (arg == "--site" && hasValue) {
            options.site = argv[++i];
        } else if (arg == "--game" && hasValue) {
            options.gameId = std::atoi(argv[++i]);
        } else if (arg == "--out" && hasValue) {
            options.out = argv[++i];
        } else if (arg == "--generations" && hasValue) {
            options.generations = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--candidates" && hasValue) {
            options.candidates = std::max(2, std::atoi(argv[++i]));
        } else if (arg == "--episodes" && hasValue) {
            options.episodes = std::max(1, std::atoi(argv[++i]));
        } else if (arg == "--steps" && hasValue) {
            options.steps = std::max(60, std::atoi(argv[++i]));
        } else if (arg == "--seed" && hasValue) {
            options.seed = static_cast<unsigned int>(std::strtoul(argv[++i], nullptr, 10));
        } else if (arg == "--replay") {
            options.replay = true;
        } else if (arg == "--play") {
            options.play = true;
        } else if (arg == "--server" && hasValue) {
            options.server = argv[++i];
        } else if (arg == "--port" && hasValue) {
            options.port = std::max(1, std::min(65535, std::atoi(argv[++i])));
        } else if (arg == "--name" && hasValue) {
            options.playerName = argv[++i];
        } else if (arg == "--watch") {
            options.watch = true;
        } else if (arg == "--gui") {
            options.gui = true;
        } else if (arg == "--agents" && hasValue) {
            options.agents = std::max(1, std::min(kMaxAgents, std::atoi(argv[++i])));
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::fprintf(stderr, "Unknown option: %s\n", arg.c_str());
            printUsage();
            return 2;
        }
    }

    if (options.token.empty() || options.gameId <= 0) {
        std::fprintf(stderr,
                     "AiTrainer signs in before it runs.\n"
                     "Create a developer token at %s/developers, mark the game AI\n"
                     "compatible there, then pass the token and the game:\n\n"
                     "  AiTrainer.exe --token rblxai_... --game <id>\n\n",
                     options.site.c_str());
        printUsage();
        return 2;
    }

    std::string error;
    Session session;
    if (!signIn(options, &session, &error)) {
        std::fprintf(stderr, "Could not sign in: %s\n", error.c_str());
        return 1;
    }
    const Course course = session.course == "doors" ? Course::DoorsHotel : Course::Map;
    const int steps = options.steps > 0 ? options.steps : defaultStepsFor(session.course);

    std::printf("Signed in for %s (game %d)\n", session.title.c_str(), session.gameId);
    TrainingWorld world(course, session.world, session.goal, &error);
    if (!world.ready()) {
        std::fprintf(stderr, "Could not build a training world for %s: %s\n",
                     session.title.c_str(), error.c_str());
        return 1;
    }
    if (options.out.empty()) {
        options.out = course == Course::DoorsHotel
                          ? std::string("models/doors-hotel.policy")
                          : defaultOutputPath(session.world);
    }

    std::printf("World: %s (%zu parts)\n", session.world.c_str(), world.partCount());
    if (world.doorsCourse()) {
        std::printf("Course: %d doors, spawn (%.1f, %.1f, %.1f), elevator (%.1f, %.1f, %.1f), "
                    "%.1f studs of corridor\n",
                    kHotelDoors, world.spawn().x, world.spawn().y, world.spawn().z, world.goal().x,
                    world.goal().y, world.goal().z, world.startDistance());
    } else {
        std::printf("Spawn: (%.1f, %.1f, %.1f)  Goal: %s (%.1f, %.1f, %.1f)  distance %.1f\n",
                    world.spawn().x, world.spawn().y, world.spawn().z, world.goalName().c_str(),
                    world.goal().x, world.goal().y, world.goal().z, world.startDistance());
    }

    // Opening the window is two optional steps: --gui asks for it, and the
    // machine has to allow it. Everything below reads `gui`, so a refused window
    // falls back to the headless run instead of failing.
    TrainerWindow window;
    bool gui = options.gui;
    if (gui) {
        if (!window.open("RBLX AI trainer - " + session.title, &error)) {
            std::fprintf(stderr, "No window: %s; training headless.\n", error.c_str());
            gui = false;
        } else {
            // Trails cost memory on every step, so they are only kept when
            // something is going to draw them.
            world.recordTrails(true);
        }
    }

    // --play is a demonstration, not a lesson: the policy is read from disk the
    // same way --replay reads it, and the server is what runs the course.
    if (options.play) {
        Policy policy;
        if (!loadPolicy(&policy, options.out, &error)) {
            std::fprintf(stderr, "Could not load %s: %s\n", options.out.c_str(), error.c_str());
            return 1;
        }
        return runLiveGame(world, window, gui, options, steps, policy);
    }

    if (options.replay) {
        Policy policy;
        if (!loadPolicy(&policy, options.out, &error)) {
            std::fprintf(stderr, "Could not load %s: %s\n", options.out.c_str(), error.c_str());
            return 1;
        }
        const EpisodeResult result =
            gui ? runGuiEpisode(world, window, policy, steps, "replay",
                                world.doorsCourse() ? kHotelDoors : 0)
                : (options.watch ? runWatchedEpisode(world, policy, steps, "replay")
                                 : world.runEpisode(policy, steps));
        if (world.doorsCourse()) {
            std::printf("Replay %s: reward %.2f, doors %d/%d at (%.1f, %.1f, %.1f), %d steps, %s\n",
                        options.out.c_str(), result.reward, result.targetsPassed, kHotelDoors,
                        result.finalPosition.x, result.finalPosition.y, result.finalPosition.z,
                        result.steps,
                        result.reached ? "reached the elevator"
                                       : (result.fell ? "fell" : "ran out of time"));
        } else {
            std::printf("Replay %s: reward %.2f, closest %.2f at (%.1f, %.1f, %.1f), %d steps, %s\n",
                        options.out.c_str(), result.reward, result.distance,
                        result.finalPosition.x, result.finalPosition.y, result.finalPosition.z,
                        result.steps,
                        result.reached ? "reached the goal"
                                       : (result.fell ? "fell" : "ran out of time"));
        }
        return result.reached ? 0 : 3;
    }

    std::mt19937 rng(options.seed);
    std::vector<float> mean(kParameterCount, 0.0f);
    std::vector<float> deviation(kParameterCount, 0.6f);
    Policy best;
    float bestReward = -1.0e30f;
    EpisodeResult bestResult;

    for (int generation = 0; generation < options.generations; ++generation) {
        std::vector<Policy> candidates(static_cast<std::size_t>(options.candidates));
        std::vector<float> scores(static_cast<std::size_t>(options.candidates), 0.0f);
        std::vector<EpisodeResult> results(static_cast<std::size_t>(options.candidates));

        // Candidates are evaluated in groups of --agents so several policies can
        // walk the course at the same time: that is what the window draws, and a
        // group of four settles four candidates in the time of one.
        int candidate = 0;
        while (candidate < options.candidates) {
            const int batch = std::min(options.agents, options.candidates - candidate);
            for (int i = 0; i < batch; ++i) {
                Policy& policy = candidates[static_cast<std::size_t>(candidate + i)];
                for (int parameter = 0; parameter < kParameterCount; ++parameter) {
                    std::normal_distribution<float> distribution(
                        mean[static_cast<std::size_t>(parameter)],
                        deviation[static_cast<std::size_t>(parameter)]);
                    policy.weights[static_cast<std::size_t>(parameter)] = distribution(rng);
                }
            }

            std::vector<EpisodeResult> summaries(static_cast<std::size_t>(batch));
            for (int episode = 0; episode < options.episodes; ++episode) {
                std::vector<const Policy*> batchPolicies;
                batchPolicies.reserve(static_cast<std::size_t>(batch));
                for (int i = 0; i < batch; ++i) {
                    batchPolicies.push_back(&candidates[static_cast<std::size_t>(candidate + i)]);
                }

                std::vector<EpisodeResult> batchResults;
                if (gui) {
                    TrainerFrame frame;
                    frame.title = "RBLX AI trainer - " + session.title;
                    frame.doorCount = world.doorsCourse() ? kHotelDoors : 0;
                    fillTrainerFrame(world, &frame);
                    window.present(frame);
                    batchResults = world.runBatch(batchPolicies, steps, [&](int step) {
                        if (step % kFrameStride != 0) {
                            return true;
                        }
                        fillTrainerFrame(world, &frame);
                        char status[512];
                        std::snprintf(status, sizeof(status),
                                      "gen %d/%d   candidates %d-%d of %d   episode %d/%d   "
                                      "step %d/%d",
                                      generation + 1, options.generations, candidate + 1,
                                      candidate + batch, options.candidates, episode + 1,
                                      options.episodes, step + 1, steps);
                        frame.status = status;
                        return window.present(frame);
                    });
                } else {
                    batchResults.resize(static_cast<std::size_t>(batch));
                    for (int i = 0; i < batch; ++i) {
                        batchResults[static_cast<std::size_t>(i)] = world.runEpisode(
                            *batchPolicies[static_cast<std::size_t>(i)], steps);
                    }
                }

                for (int i = 0; i < batch; ++i) {
                    const EpisodeResult& result = batchResults[static_cast<std::size_t>(i)];
                    EpisodeResult& summary = summaries[static_cast<std::size_t>(i)];
                    summary.reward += result.reward / static_cast<float>(options.episodes);
                    summary.distance += result.distance / static_cast<float>(options.episodes);
                    summary.steps = result.steps;
                    summary.reached = summary.reached || result.reached;
                    summary.fell = summary.fell || result.fell;
                    // The door count is what a hotel run is judged by, and the
                    // average across episodes is clearer than whichever episode ran
                    // longest.
                    summary.targetsPassed += result.targetsPassed;
                    summary.finalPosition = result.finalPosition;
                }

                if (gui && !window.running()) {
                    break;
                }
            }

            for (int i = 0; i < batch; ++i) {
                EpisodeResult& summary = summaries[static_cast<std::size_t>(i)];
                if (options.episodes > 1) {
                    summary.targetsPassed /= options.episodes;
                }
                const int index = candidate + i;
                scores[static_cast<std::size_t>(index)] = summary.reward;
                results[static_cast<std::size_t>(index)] = summary;
                if (summary.reward > bestReward) {
                    bestReward = summary.reward;
                    best = candidates[static_cast<std::size_t>(index)];
                    bestResult = summary;
                }
            }

            candidate += batch;
            if (gui && !window.running()) {
                std::printf("Window closed; saving the best policy so far.\n");
                break;
            }
        }

        std::vector<int> order(static_cast<std::size_t>(options.candidates));
        for (int i = 0; i < options.candidates; ++i) order[static_cast<std::size_t>(i)] = i;
        std::sort(order.begin(), order.end(), [&scores](int a, int b) {
            return scores[static_cast<std::size_t>(a)] > scores[static_cast<std::size_t>(b)];
        });

        const int eliteCount = std::max(2, options.candidates / 4);
        std::vector<const std::vector<float>*> elite;
        elite.reserve(static_cast<std::size_t>(eliteCount));
        float generationBest = -1.0e30f;
        float generationMean = 0.0f;
        for (int i = 0; i < options.candidates; ++i) {
            generationMean += scores[static_cast<std::size_t>(i)] / static_cast<float>(options.candidates);
        }
        for (int i = 0; i < eliteCount; ++i) {
            const int index = order[static_cast<std::size_t>(i)];
            elite.push_back(&candidates[static_cast<std::size_t>(index)].weights);
            generationBest = std::max(generationBest, scores[static_cast<std::size_t>(index)]);
        }

        mean = meanOf(elite);
        // A high floor keeps sampling alive: with a tiny floor the elite mean
        // collapses onto one policy within a few generations and stops exploring
        // the actions it has not tried yet.
        deviation = deviationOf(elite, mean, 0.25f);

        if (world.doorsCourse()) {
            std::printf("gen %2d  best %8.2f  mean %8.2f  doors %2d/%d  reached %d/%d\n",
                        generation + 1, generationBest, generationMean,
                        results[static_cast<std::size_t>(order.front())].targetsPassed,
                        kHotelDoors,
                        static_cast<int>(std::count_if(
                            results.begin(), results.end(),
                            [](const EpisodeResult& result) { return result.reached; })),
                        options.candidates);
        } else {
            std::printf("gen %2d  best %8.2f  mean %8.2f  elite closest %.1f  reached %d/%d\n",
                        generation + 1, generationBest, generationMean,
                        results[static_cast<std::size_t>(order.front())].distance,
                        static_cast<int>(std::count_if(
                            results.begin(), results.end(),
                            [](const EpisodeResult& result) { return result.reached; })),
                        options.candidates);
        }
    }

    if (!savePolicy(best, options.out, world, &error)) {
        std::fprintf(stderr, "Could not save the policy: %s\n", error.c_str());
        return 1;
    }

    if (gui) {
        // Closing the window is how a watched run ends, so only show the best
        // policy if it is still there; the file is written either way.
        if (window.running()) {
            runGuiEpisode(world, window, best, steps, "best policy",
                          world.doorsCourse() ? kHotelDoors : 0);
        }
    } else if (options.watch) {
        runWatchedEpisode(world, best, steps, "best policy");
    }

    if (world.doorsCourse()) {
        std::printf("\nBest policy: reward %.2f, doors %d/%d at (%.1f, %.1f, %.1f), %d steps, %s\n",
                    bestReward, bestResult.targetsPassed, kHotelDoors, bestResult.finalPosition.x,
                    bestResult.finalPosition.y, bestResult.finalPosition.z, bestResult.steps,
                    bestResult.reached ? "reached the elevator" : "ran out of time");
    } else {
        std::printf("\nBest policy: reward %.2f, closest %.2f at (%.1f, %.1f, %.1f), %d steps, %s\n",
                    bestReward, bestResult.distance, bestResult.finalPosition.x,
                    bestResult.finalPosition.y, bestResult.finalPosition.z, bestResult.steps,
                    bestResult.reached ? "reached the goal"
                                       : (bestResult.fell ? "fell" : "ran out of time"));
    }
    std::printf("Saved to %s\n", options.out.c_str());
    return 0;
}
