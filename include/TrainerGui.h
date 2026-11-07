#pragma once

#include <deque>
#include <string>
#include <vector>

#include <glm/glm.hpp>

#include "Part.h"

// The optional window AiTrainer opens with --gui: the course seen from above
// with the roof left off, and a dot for every agent training in it. The window
// keeps the simulation at arm's length - it is handed copies of the positions it
// draws - so training behaves the same whether or not anyone is watching.
//
// GLFW and ImGui are deliberately not named here: only the implementation needs
// them, and the trainer's other translation units stay free of both.
namespace rblx {
namespace trainer {

// One agent as the window draws it.
struct AgentMarker {
    glm::vec3 position{0.0f};
    glm::vec3 target{0.0f};
    int targetsPassed = 0;
    float reward = 0.0f;
    bool reached = false;
    bool fell = false;
    std::vector<glm::vec3> trail;
};

// Everything one frame needs: the world's parts, where the agents are on it, the
// checkpoints and a line of text for the header.
struct Frame {
    const std::deque<Part>* parts = nullptr;
    std::vector<AgentMarker> agents;
    std::vector<glm::vec3> targets;
    glm::vec3 spawn{0.0f};
    glm::vec3 goal{0.0f};
    std::string title;
    std::string status;
    int doorCount = 0;
};

class TrainerWindow {
public:
    TrainerWindow() = default;
    ~TrainerWindow();
    TrainerWindow(const TrainerWindow&) = delete;
    TrainerWindow& operator=(const TrainerWindow&) = delete;

    // Opens the window and its GL context. On failure the trainer carries on
    // headless, so a machine without a display is not a reason to stop.
    bool open(const std::string& title, std::string* error);

    // True until the window is closed, which is how a watched run is ended.
    bool running() const { return window_ != nullptr && !closeRequested_; }

    // Draws one frame. Returns false once the window has been closed.
    bool present(const Frame& frame);

private:
    void* window_ = nullptr;  // GLFWwindow*, kept opaque so GLFW stays in the .cpp
    bool closeRequested_ = false;
    // Both are options rather than constants: taking the roof off hides ceilings
    // but also anything a map has built above the course.
    bool hideRoof_ = true;
    bool showTrails_ = true;
    float roofCut_ = 9.0f;
};

}  // namespace trainer
}  // namespace rblx
