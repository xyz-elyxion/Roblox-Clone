// Headless trainer: runs a .world with no window, lets a small policy drive the
// same Character and PhysicsWorld the game runs, and improves that policy with
// the cross-entropy method. A policy learned here is a plain table of weights, so
// the result can be replayed or copied into anything that can read five floats.
//
// The point of reusing Character instead of simulating a box: walk speed, jump,
// friction and the death plane all behave exactly as they do in the game, so a
// policy is not trained against a world that the game does not have.

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <deque>
#include <filesystem>
#include <fstream>
#include <random>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Character.h"
#include "Part.h"
#include "PhysicsWorld.h"
#include "WorldLoader.h"

namespace {

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
    glm::vec3 finalPosition{0.0f, 0.0f, 0.0f};
    int steps = 0;
    bool reached = false;
    bool fell = false;
};

class TrainingWorld {
public:
    TrainingWorld(const std::string& worldPath, const std::string& goalName, std::string* error)
        : goalName_(goalName) {
        // The policy acts on fixed world axes: forward is -Z and right is +X, the
        // same mapping the game uses when the camera looks along -Z. Using a
        // rotating camera here would hand the policy a free heading.
        inputCamera_.Yaw = -90.0f;

        physics_.init();
        WorldLoader::loadWorld(worldPath, parts_, physics_);
        if (parts_.empty()) {
            *error = "the world file has no parts";
            return;
        }

        bool hasSpawn = false;
        for (const Part& part : parts_) {
            if (!part.deleted && part.isSpawn) {
                spawn_ = part.position + glm::vec3(0.0f, 5.0f, 0.0f);
                hasSpawn = true;
                break;
            }
        }
        if (!hasSpawn) {
            spawn_ = glm::vec3(0.0f, 10.0f, 0.0f);
        }

        const Part* goalPart = nullptr;
        for (const Part& part : parts_) {
            if (!part.deleted && !part.isFolder && part.name == goalName_) {
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
                const float distance = glm::length(part.position - spawn_);
                if (distance > furthest) {
                    furthest = distance;
                    goalPart = &part;
                }
            }
        }
        if (goalPart == nullptr) {
            *error = "the world has no part to walk to";
            return;
        }
        goal_ = goalPart->position;
        goalName_ = goalPart->name;

        character_ = new Character(spawn_, &physics_, &parts_);
        character_->update(0.0f, nullptr, false);
        physics_.update(kStepSeconds);
        position_ = character_->getPosition();

        for (Part& part : parts_) {
            if (!part.deleted && part.name == "HumanoidRootPart") {
                rootPart_ = &part;
                break;
            }
        }
    }

    ~TrainingWorld() {
        delete character_;
        character_ = nullptr;
    }

    bool ready() const { return character_ != nullptr; }
    std::size_t partCount() const { return parts_.size(); }
    const std::string& goalName() const { return goalName_; }
    glm::vec3 spawn() const { return spawn_; }
    glm::vec3 goal() const { return goal_; }
    float startDistance() const { return glm::length(goal_ - spawn_); }

    EpisodeResult runEpisode(const Policy& policy, int maxSteps) {
        EpisodeResult result;

        character_->teleportTo(spawn_);
        character_->update(0.0f, nullptr, false);
        physics_.update(kStepSeconds);
        position_ = character_->getPosition();
        previousDistance_ = glm::length(goal_ - position_);

        for (int step = 0; step < maxSteps; ++step) {
            float observation[kObservationCount];
            observe(observation);
            applyAction(policy.act(observation));

            character_->update(kStepSeconds, nullptr, false);
            physics_.update(kStepSeconds);

            position_ = character_->getPosition();
            const float distance = glm::length(goal_ - position_);
            // Progress toward the goal is the reward, with a small per-step cost
            // so dawdling loses to walking a straight line.
            result.reward += (previousDistance_ - distance) - 0.01f;
            previousDistance_ = distance;
            result.steps = step + 1;

            if (distance <= kGoalRadius) {
                result.reward += 50.0f;
                result.reached = true;
                break;
            }
            if (position_.y < spawn_.y - kFallLimit) {
                result.reward -= 25.0f;
                result.fell = true;
                break;
            }
        }

        result.distance = previousDistance_;
        result.finalPosition = position_;
        return result;
    }

private:
    void observe(float* out) {
        const glm::vec3 delta = goal_ - position_;
        const float distance = glm::length(delta);
        out[0] = std::clamp(delta.x / 40.0f, -1.0f, 1.0f);
        out[1] = std::clamp(delta.z / 40.0f, -1.0f, 1.0f);
        out[2] = std::clamp(distance / 100.0f, 0.0f, 1.0f);
        out[3] = std::clamp((position_.y - spawn_.y) / 10.0f, -1.0f, 1.0f);
        out[4] = 1.0f;  // bias, so a weight can act as a per-action threshold
        // One feeler per movement axis. Without these the policy can only learn
        // to walk straight at the goal, which parks it against the first wall or
        // parked car in the way; with them it can learn to steer around one.
        out[5] = freeAhead(glm::vec3(0.0f, 0.0f, -1.0f));
        out[6] = freeAhead(glm::vec3(-1.0f, 0.0f, 0.0f));
        out[7] = freeAhead(glm::vec3(1.0f, 0.0f, 0.0f));
    }

    // How much of a fixed-length probe in this direction is clear, 0 at a wall
    // to 1 when nothing is within reach.
    float freeAhead(const glm::vec3& direction) {
        constexpr float kFeelDistance = 12.0f;
        PhysicsWorld::RaycastResult hit;
        if (physics_.raycast(position_, position_ + direction * kFeelDistance, hit, rootPart_)) {
            return std::clamp(hit.hitFraction, 0.0f, 1.0f);
        }
        return 1.0f;
    }

    void applyAction(int action) {
        const bool forward = action == 0;
        const bool backward = action == 1;
        const bool left = action == 2;
        const bool right = action == 3;
        const bool jump = action == 4;
        character_->processInput(forward, backward, left, right, jump, inputCamera_, 0.0f,
                                 kStepSeconds);
    }

    std::deque<Part> parts_;
    PhysicsWorld physics_;
    Character* character_ = nullptr;
    glm::vec3 spawn_{0.0f, 0.0f, 0.0f};
    glm::vec3 goal_{0.0f, 0.0f, 0.0f};
    std::string goalName_;
    Part* rootPart_ = nullptr;
    Camera inputCamera_;
    glm::vec3 position_{0.0f, 0.0f, 0.0f};
    float previousDistance_ = 0.0f;
};

struct Options {
    std::string world = "games/turbo-circuit.world";
    std::string goal = "FinishLine";
    std::string out;
    int generations = kDefaultGenerations;
    int candidates = kDefaultCandidates;
    int episodes = kDefaultEpisodesPerCandidate;
    int steps = kDefaultSteps;
    unsigned int seed = 1337;
    bool replay = false;
};

void printUsage() {
    std::printf(
        "Usage: AiTrainer [options]\n"
        "  --world <file>        .world file to train on (default %s)\n"
        "  --goal <part>         part to walk to (default FinishLine; a missing part\n"
        "                        falls back to the part furthest from the spawn)\n"
        "  --generations <n>     training generations (default %d)\n"
        "  --candidates <n>      policies sampled per generation (default %d)\n"
        "  --episodes <n>        episodes per candidate (default %d)\n"
        "  --steps <n>           steps per episode at 60 Hz (default %d)\n"
        "  --out <file>          where to write the trained policy\n"
        "                        (default models/<world>.policy)\n"
        "  --replay              load --out and run one episode with it\n"
        "  --seed <n>            sampling seed (default 1337)\n",
        "games/turbo-circuit.world", kDefaultGenerations, kDefaultCandidates,
        kDefaultEpisodesPerCandidate, kDefaultSteps);
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
            line.rfind("actions ", 0) == 0) {
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
        if (arg == "--world" && hasValue) {
            options.world = argv[++i];
        } else if (arg == "--goal" && hasValue) {
            options.goal = argv[++i];
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
        } else if (arg == "--help" || arg == "-h") {
            printUsage();
            return 0;
        } else {
            std::fprintf(stderr, "Unknown option: %s\n", arg.c_str());
            printUsage();
            return 2;
        }
    }
    if (options.out.empty()) {
        options.out = defaultOutputPath(options.world);
    }

    std::string error;
    TrainingWorld world(options.world, options.goal, &error);
    if (!world.ready()) {
        std::fprintf(stderr, "Could not build a training world from %s: %s\n",
                     options.world.c_str(), error.c_str());
        return 1;
    }

    std::printf("World: %s (%zu parts)\n", options.world.c_str(), world.partCount());
    std::printf("Spawn: (%.1f, %.1f, %.1f)  Goal: %s (%.1f, %.1f, %.1f)  distance %.1f\n",
                world.spawn().x, world.spawn().y, world.spawn().z, world.goalName().c_str(),
                world.goal().x, world.goal().y, world.goal().z, world.startDistance());

    if (options.replay) {
        Policy policy;
        if (!loadPolicy(&policy, options.out, &error)) {
            std::fprintf(stderr, "Could not load %s: %s\n", options.out.c_str(), error.c_str());
            return 1;
        }
        const EpisodeResult result = world.runEpisode(policy, options.steps);
        std::printf("Replay %s: reward %.2f, closest %.2f at (%.1f, %.1f, %.1f), %d steps, %s\n",
                    options.out.c_str(), result.reward, result.distance, result.finalPosition.x,
                    result.finalPosition.y, result.finalPosition.z, result.steps,
                    result.reached ? "reached the goal" : (result.fell ? "fell" : "ran out of time"));
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

        for (int candidate = 0; candidate < options.candidates; ++candidate) {
            Policy& policy = candidates[static_cast<std::size_t>(candidate)];
            for (int i = 0; i < kParameterCount; ++i) {
                std::normal_distribution<float> distribution(mean[static_cast<std::size_t>(i)],
                                                             deviation[static_cast<std::size_t>(i)]);
                policy.weights[static_cast<std::size_t>(i)] = distribution(rng);
            }

            EpisodeResult summary;
            for (int episode = 0; episode < options.episodes; ++episode) {
                const EpisodeResult result = world.runEpisode(policy, options.steps);
                summary.reward += result.reward / static_cast<float>(options.episodes);
                summary.distance += result.distance / static_cast<float>(options.episodes);
                summary.steps = result.steps;
                summary.reached = summary.reached || result.reached;
                summary.fell = summary.fell || result.fell;
                summary.finalPosition = result.finalPosition;
            }
            scores[static_cast<std::size_t>(candidate)] = summary.reward;
            results[static_cast<std::size_t>(candidate)] = summary;

            if (summary.reward > bestReward) {
                bestReward = summary.reward;
                best = policy;
                bestResult = summary;
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

        std::printf("gen %2d  best %8.2f  mean %8.2f  elite closest %.1f  reached %d/%d\n",
                    generation + 1, generationBest, generationMean,
                    results[static_cast<std::size_t>(order.front())].distance,
                    static_cast<int>(std::count_if(
                        results.begin(), results.end(),
                        [](const EpisodeResult& result) { return result.reached; })),
                    options.candidates);
    }

    if (!savePolicy(best, options.out, world, &error)) {
        std::fprintf(stderr, "Could not save the policy: %s\n", error.c_str());
        return 1;
    }

    std::printf("\nBest policy: reward %.2f, closest %.2f at (%.1f, %.1f, %.1f), %d steps, %s\n",
                bestReward, bestResult.distance, bestResult.finalPosition.x,
                bestResult.finalPosition.y, bestResult.finalPosition.z, bestResult.steps,
                bestResult.reached ? "reached the goal" : (bestResult.fell ? "fell" : "ran out of time"));
    std::printf("Saved to %s\n", options.out.c_str());
    return 0;
}
