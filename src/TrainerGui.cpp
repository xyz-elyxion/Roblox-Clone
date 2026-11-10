// The optional trainer window. The game's own renderer needs a camera, shaders,
// a shadow map and materials; the trainer has none of that and does not want it,
// because what it has to show is flat: where the walls are, where the roof was,
// and where each agent is walking. So the course is drawn as rectangles on an
// RblxDrawList, one per part, highest surface last, and parts whose underside sits
// above the roof cut are left out - that is the "roof deleted" view.

#include "TrainerGui.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <utility>

#define RBLXWIN_INCLUDE_NONE
#include <rblxgl/rblxgl.h>
#include <RBLXWIN/rblxwin.h>
#include <rblxui.h>
#include <rblxui_impl_rblxwin.h>
#include <rblxui_impl_opengl3.h>

#include "UiTheme.h"

namespace rblx {
namespace trainer {

namespace {

// A part standing on the floor is drawn as floor, anything taller as wall. The
// same threshold --watch uses, so both views call the same thing a wall.
constexpr float kWallHeight = 1.5f;
constexpr float kAgentRadius = 4.5f;

// Where the course sits on the canvas: world X runs right, world Z runs down, so
// the view is the map seen from above with north up.
struct Canvas {
    RblxVec2 origin{0.0f, 0.0f};
    RblxVec2 size{0.0f, 0.0f};
    float centreX = 0.0f;
    float centreZ = 0.0f;
    float scale = 1.0f;

    RblxVec2 point(float x, float z) const {
        return RblxVec2(origin.x + size.x * 0.5f + (x - centreX) * scale,
                      origin.y + size.y * 0.5f + (z - centreZ) * scale);
    }
};

RblxU32 toColor(const RblxVec4& color) {
    return RblxUI::ColorConvertFloat4ToU32(color);
}

// World colours are data, not chrome, so they are kept but mixed toward the
// window's background: a bright floor still reads as itself without fighting the
// palette it sits in.
RblxU32 partFill(const rblxmath::vec3& color, const RblxVec4& background, float keep) {
    const float red = color.x * keep + background.x * (1.0f - keep);
    const float green = color.y * keep + background.y * (1.0f - keep);
    const float blue = color.z * keep + background.z * (1.0f - keep);
    return toColor(RblxVec4(red, green, blue, 1.0f));
}

// Four roles from the one palette rather than four invented hues; a fifth agent
// starts the list again. accentStrong is the shade that reads on both palettes.
RblxVec4 agentColor(std::size_t index) {
    const ui::theme::Colors& colors = ui::theme::colors();
    switch (index % 4) {
        case 0: return colors.accentStrong;
        case 1: return colors.ok;
        case 2: return colors.warning;
        default: return colors.error;
    }
}

// The course's extent in X and Z, grown to include the spawn and the goal so a
// short course is not cropped. The parts never move, so this is the same answer
// every frame; it is cheap enough at a few hundred parts not to be cached.
bool courseBounds(const Frame& frame, float* minX, float* maxX, float* minZ, float* maxZ) {
    if (frame.parts == nullptr) {
        return false;
    }
    float lowX = 1.0e30f;
    float highX = -1.0e30f;
    float lowZ = 1.0e30f;
    float highZ = -1.0e30f;
    for (const Part& part : *frame.parts) {
        if (part.deleted || part.isFolder) continue;
        lowX = std::min(lowX, part.position.x - part.size.x * 0.5f);
        highX = std::max(highX, part.position.x + part.size.x * 0.5f);
        lowZ = std::min(lowZ, part.position.z - part.size.z * 0.5f);
        highZ = std::max(highZ, part.position.z + part.size.z * 0.5f);
    }
    for (const rblxmath::vec3& point : frame.targets) {
        lowX = std::min(lowX, point.x);
        highX = std::max(highX, point.x);
        lowZ = std::min(lowZ, point.z);
        highZ = std::max(highZ, point.z);
    }
    if (!(highX > lowX) || !(highZ > lowZ)) {
        return false;
    }
    *minX = lowX;
    *maxX = highX;
    *minZ = lowZ;
    *maxZ = highZ;
    return true;
}

void drawCourse(RblxDrawList* draw, const Frame& frame, const Canvas& canvas, bool hideRoof,
                float roofCut) {
    if (frame.parts == nullptr) {
        return;
    }

    // Lowest surface first, so a wall's footprint is painted over the floor it
    // stands on and the picture reads as a top-down plan.
    std::vector<const Part*> visible;
    visible.reserve(frame.parts->size());
    for (const Part& part : *frame.parts) {
        if (part.deleted || part.isFolder || part.transparency >= 0.95f) continue;
        // The roof: a part whose underside is above the cut covers what is below
        // it when seen from above, which is exactly what is being taken off.
        if (hideRoof && part.position.y - part.size.y * 0.5f > roofCut) continue;
        visible.push_back(&part);
    }
    std::sort(visible.begin(), visible.end(), [](const Part* left, const Part* right) {
        return left->position.y + left->size.y * 0.5f < right->position.y + right->size.y * 0.5f;
    });

    const RblxVec4& background = ui::theme::background();
    const RblxU32 border = toColor(ui::theme::border());
    const float canvasRight = canvas.origin.x + canvas.size.x;
    const float canvasBottom = canvas.origin.y + canvas.size.y;

    for (const Part* part : visible) {
        const float halfX = part->size.x * 0.5f;
        const float halfZ = part->size.z * 0.5f;
        const RblxVec2 low = canvas.point(part->position.x - halfX, part->position.z - halfZ);
        const RblxVec2 high = canvas.point(part->position.x + halfX, part->position.z + halfZ);
        if (high.x < canvas.origin.x || low.x > canvasRight) continue;
        if (high.y < canvas.origin.y || low.y > canvasBottom) continue;

        const bool standable =
            part->position.y + part->size.y * 0.5f <= frame.spawn.y + kWallHeight;
        draw->AddRectFilled(low, high, partFill(part->color, background, standable ? 0.85f : 1.0f));
        if (!standable) {
            // A wall's edge, so a corridor is legible even when the two are the
            // same colour in the world file.
            draw->AddRect(low, high, border, 0.0f, 0, 1.0f);
        }
    }
}

void drawMarkers(RblxDrawList* draw, const Frame& frame, const Canvas& canvas, bool showTrails) {
    const ui::theme::Colors& colors = ui::theme::colors();
    const RblxU32 muted = toColor(colors.muted);
    const RblxU32 ok = toColor(colors.ok);
    const RblxU32 error = toColor(colors.error);
    const RblxU32 panel = toColor(colors.panel);

    // Every checkpoint but the last, which is the goal and gets its own mark.
    for (std::size_t index = 0; index + 1 < frame.targets.size(); ++index) {
        const RblxVec2 point = canvas.point(frame.targets[index].x, frame.targets[index].z);
        draw->AddRect(RblxVec2(point.x - 3.0f, point.y - 3.0f), RblxVec2(point.x + 3.0f, point.y + 3.0f),
                      muted, 0.0f, 0, 1.0f);
    }

    const RblxVec2 spawn = canvas.point(frame.spawn.x, frame.spawn.z);
    draw->AddRect(RblxVec2(spawn.x - 5.0f, spawn.y - 5.0f), RblxVec2(spawn.x + 5.0f, spawn.y + 5.0f),
                  muted, 0.0f, 0, 2.0f);
    draw->AddText(RblxVec2(spawn.x + 8.0f, spawn.y - 7.0f), muted, "spawn");

    const RblxVec2 goal = canvas.point(frame.goal.x, frame.goal.z);
    draw->AddCircle(goal, 6.0f, ok, 16, 2.0f);
    draw->AddText(RblxVec2(goal.x + 9.0f, goal.y - 7.0f), ok, "goal");

    for (std::size_t index = 0; index < frame.agents.size(); ++index) {
        const AgentMarker& agent = frame.agents[index];
        const RblxVec4 color = agentColor(index);
        const RblxU32 fill = toColor(color);

        if (showTrails && agent.trail.size() > 1) {
            std::vector<RblxVec2> points;
            points.reserve(agent.trail.size());
            for (const rblxmath::vec3& visited : agent.trail) {
                points.push_back(canvas.point(visited.x, visited.z));
            }
            RblxVec4 faded = color;
            faded.w = 0.35f;
            draw->AddPolyline(points.data(), static_cast<int>(points.size()), toColor(faded),
                              RblxDrawFlags_None, 1.5f);
        }

        // A thin line to the checkpoint it is walking to, which is what makes a
        // wandering agent obvious at a glance.
        const RblxVec2 point = canvas.point(agent.position.x, agent.position.z);
        const RblxVec2 target = canvas.point(agent.target.x, agent.target.z);
        RblxVec4 line = color;
        line.w = 0.30f;
        draw->AddLine(point, target, toColor(line), 1.0f);

        draw->AddCircleFilled(point, kAgentRadius, fill);
        draw->AddCircle(point, kAgentRadius + 1.5f, panel, 16, 1.0f);
        if (agent.reached) {
            draw->AddCircle(point, kAgentRadius + 5.0f, ok, 16, 1.5f);
        } else if (agent.fell) {
            draw->AddLine(RblxVec2(point.x - 6.0f, point.y - 6.0f),
                          RblxVec2(point.x + 6.0f, point.y + 6.0f), error, 1.5f);
            draw->AddLine(RblxVec2(point.x - 6.0f, point.y + 6.0f),
                          RblxVec2(point.x + 6.0f, point.y - 6.0f), error, 1.5f);
        }
    }
}

void drawLegend(const Frame& frame) {
    const ui::theme::Colors& colors = ui::theme::colors();
    for (std::size_t index = 0; index < frame.agents.size(); ++index) {
        const AgentMarker& agent = frame.agents[index];
        if (index > 0) {
            RblxUI::SameLine();
        }
        RblxUI::TextColored(agentColor(index), "AI %zu", index + 1);
        RblxUI::SameLine();
        if (frame.doorCount > 0) {
            RblxUI::TextColored(colors.muted, "%d/%d doors  %.1f", agent.targetsPassed,
                               frame.doorCount, agent.reward);
        } else {
            RblxUI::TextColored(colors.muted, "%.1f", agent.reward);
        }
    }
}

}  // namespace

TrainerWindow::~TrainerWindow() {
    if (window_ == nullptr) {
        return;
    }
    RblxUI_ImplOpenGL3_Shutdown();
    RblxUI_ImplRblxWin_Shutdown();
    RblxUI::DestroyContext();
    rblxwinDestroyWindow(static_cast<RBLXWINwindow*>(window_));
    rblxwinTerminate();
    window_ = nullptr;
}

bool TrainerWindow::open(const std::string& title, std::string* error) {
    if (window_ != nullptr) {
        return true;
    }

    rblxwinSetErrorCallback([](int, const char* description) {
        std::fprintf(stderr, "Trainer window: %s\n",
                     description == nullptr ? "unknown error" : description);
    });
    if (rblxwinInit() == 0) {
        *error = "RBLXWIN could not start (is there a display?)";
        return false;
    }

    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MAJOR, 3);
    rblxwinWindowHint(RBLXWIN_CONTEXT_VERSION_MINOR, 3);
    rblxwinWindowHint(RBLXWIN_OPENGL_PROFILE, RBLXWIN_OPENGL_CORE_PROFILE);

    RBLXWINwindow* window = rblxwinCreateWindow(1100, 780, title.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        rblxwinTerminate();
        *error = "the window could not be created";
        return false;
    }
    rblxwinSetWindowSizeLimits(window, 720, 480, RBLXWIN_DONT_CARE, RBLXWIN_DONT_CARE);
    rblxwinMakeContextCurrent(window);
    // The trainer's simulation is faster than the screen can show, so letting the
    // swap wait keeps the picture watchable instead of a blur.
    rblxwinSwapInterval(1);

    if (rblxglLoadGLLoader(reinterpret_cast<RBLXGLloadproc>(rblxwinGetProcAddress)) == 0) {
        rblxwinDestroyWindow(window);
        rblxwinTerminate();
        *error = "OpenGL 3.3 could not be loaded";
        return false;
    }

    RBLXUI_CHECKVERSION();
    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= RblxUIConfigFlags_NavEnableKeyboard;
    ui::theme::apply();

    RblxUI_ImplRblxWin_InitForOpenGL(window, true);
    RblxUI_ImplOpenGL3_Init("#version 330");

    window_ = window;
    closeRequested_ = false;
    // The console keeps its progress lines: the window is a view, not a
    // replacement for the trainer's output.
    return true;
}

bool TrainerWindow::present(const Frame& frame) {
    if (!running()) {
        return false;
    }
    RBLXWINwindow* window = static_cast<RBLXWINwindow*>(window_);

    rblxwinPollEvents();
    if (rblxwinWindowShouldClose(window) != 0) {
        closeRequested_ = true;
        return false;
    }

    RblxUI_ImplOpenGL3_NewFrame();
    RblxUI_ImplRblxWin_NewFrame();
    RblxUI::NewFrame();

    const RblxUIViewport* viewport = RblxUI::GetMainViewport();
    RblxUI::SetNextWindowPos(viewport->WorkPos);
    RblxUI::SetNextWindowSize(viewport->WorkSize);
    RblxUI::SetNextWindowBgAlpha(1.0f);
    const RblxUIWindowFlags flags =
        RblxUIWindowFlags_NoTitleBar | RblxUIWindowFlags_NoResize | RblxUIWindowFlags_NoMove |
        RblxUIWindowFlags_NoCollapse | RblxUIWindowFlags_NoBringToFrontOnFocus |
        RblxUIWindowFlags_NoSavedSettings;
    RblxUI::Begin("RBLX AI trainer", nullptr, flags);

    const ui::theme::Colors& colors = ui::theme::colors();
    RblxUI::TextColored(colors.text, "%s", frame.title.c_str());
    if (!frame.status.empty()) {
        RblxUI::SameLine();
        RblxUI::TextColored(colors.muted, "%s", frame.status.c_str());
    }

    RblxUI::Checkbox("Hide roof", &hideRoof_);
    RblxUI::SameLine();
    RblxUI::Checkbox("Trails", &showTrails_);
    RblxUI::SameLine();
    RblxUI::SetNextItemWidth(180.0f);
    RblxUI::SliderFloat("Roof cut", &roofCut_, -1.0f, 30.0f, "%.1f");
    if (RblxUI::IsItemHovered()) {
        RblxUI::SetTooltip("Parts whose underside is above this are not drawn.");
    }

    if (!frame.agents.empty()) {
        drawLegend(frame);
    }

    if (RblxUI::BeginChild("course", RblxVec2(0.0f, 0.0f), true)) {
        Canvas canvas;
        canvas.origin = RblxUI::GetCursorScreenPos();
        canvas.size = RblxUI::GetContentRegionAvail();
        float minX = 0.0f;
        float maxX = 0.0f;
        float minZ = 0.0f;
        float maxZ = 0.0f;
        if (canvas.size.x > 1.0f && canvas.size.y > 1.0f &&
            courseBounds(frame, &minX, &maxX, &minZ, &maxZ)) {
            constexpr float kPadding = 14.0f;
            canvas.centreX = (minX + maxX) * 0.5f;
            canvas.centreZ = (minZ + maxZ) * 0.5f;
            canvas.scale = std::min((canvas.size.x - kPadding * 2.0f) / (maxX - minX),
                                    (canvas.size.y - kPadding * 2.0f) / (maxZ - minZ));
            canvas.scale = std::max(canvas.scale, 0.01f);

            RblxDrawList* draw = RblxUI::GetWindowDrawList();
            draw->AddRectFilled(canvas.origin,
                                RblxVec2(canvas.origin.x + canvas.size.x,
                                       canvas.origin.y + canvas.size.y),
                                toColor(colors.background));
            drawCourse(draw, frame, canvas, hideRoof_, roofCut_);
            drawMarkers(draw, frame, canvas, showTrails_);
        }
        // Claim the space the draw list just used, so the child scrolls nothing.
        RblxUI::Dummy(canvas.size);
    }
    RblxUI::EndChild();

    if (RblxUI::IsKeyPressed(RblxUIKey_Escape)) {
        closeRequested_ = true;
    }
    RblxUI::End();

    RblxUI::Render();
    int width = 0;
    int height = 0;
    rblxwinGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    const RblxVec4 background = colors.background;
    glClearColor(background.x, background.y, background.z, background.w);
    glClear(GL_COLOR_BUFFER_BIT);
    RblxUI_ImplOpenGL3_RenderDrawData(RblxUI::GetDrawData());
    rblxwinSwapBuffers(window);
    return running();
}

}  // namespace trainer
}  // namespace rblx
