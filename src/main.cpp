#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Editor.h"
#include "LinearTextEditor.h"
#include "Part.h"
#include "PhysicsWorld.h"
#include "Raycaster.h"
#include "ShadowMap.h"
#include "WorldLoader.h"
#include "LinearScript.h"
#include "WorldScript.h"
#include "WorldServices.h"
#include "StudioHistory.h"
#include "StudioSelection.h"
#include "Auth.h"

#include <lineargl/lineargl.h>
#include <LINEARWIN/linearwin.h>
#include <linearmath/gtc/matrix_transform.hpp>
#include <linearmath/gtc/type_ptr.hpp>
#include <linearmath/gtx/euler_angles.hpp>
#include <linearmath/gtx/quaternion.hpp>

#include <algorithm>
#include <cstdio>
#include <deque>
#include <exception>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace {
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr float MIN_PART_SCALE = 0.05f;
    constexpr float MAX_PART_SCALE = 512.0f;

    void addDefaultWorld(std::deque<Part>& parts, PhysicsWorld& physicsWorld) {
        parts.clear();
        physicsWorld.reset();

        parts.emplace_back(linearmath::vec3(0.0f, -1.0f, 0.0f), linearmath::vec3(40.0f, 1.0f, 40.0f), linearmath::vec3(0.35f, 0.8f, 0.35f), ShapeType::Cube, "Baseplate");
        parts.back().anchored = true;
        physicsWorld.addPart(&parts.back());

        parts.emplace_back(linearmath::vec3(0.0f, 1.0f, 0.0f), linearmath::vec3(4.0f, 4.0f, 4.0f), linearmath::vec3(0.8f, 0.25f, 0.2f), ShapeType::Cube, "Part");
        parts.back().anchored = true;
        physicsWorld.addPart(&parts.back());
    }

    bool saveWorld(const std::string& filename, const std::deque<Part>& parts) {
        std::ofstream out(filename);
        if (!out.is_open()) {
            return false;
        }

        // A deleted part is not part of the world, so it is not written. The file
        // therefore lists only what is still there, which means a parent or a weld
        // naming a deleted part has to be renumbered for the list being written,
        // or left out when the part it named is gone.
        std::vector<int> written(parts.size(), -1);
        int liveCount = 0;
        for (std::size_t index = 0; index < parts.size(); ++index) {
            if (parts[index].deleted) {
                continue;
            }
            written[index] = liveCount++;
        }

        out << liveCount << '\n';
        for (std::size_t index = 0; index < parts.size(); ++index) {
            const Part& part = parts[index];
            if (part.deleted) {
                continue;
            }

            std::vector<int> weldTargets;
            for (const Weld& weld : part.welds) {
                int target = -1;
                for (std::size_t candidate = 0; candidate < parts.size(); ++candidate) {
                    if (&parts[candidate] == weld.part1) {
                        target = written[candidate];
                        break;
                    }
                }
                if (target >= 0) {
                    weldTargets.push_back(target);
                }
            }

            const int parent = part.parentIndex >= 0 &&
                    part.parentIndex < static_cast<int>(parts.size())
                ? written[static_cast<std::size_t>(part.parentIndex)]
                : -1;

            out << static_cast<int>(part.shape) << ' '
                << std::quoted(part.name) << ' '
                << parent << ' '
                << part.isFolder << ' '
                << part.isCamera << ' '
                << part.isSpawn << ' '
                << part.position.x << ' ' << part.position.y << ' ' << part.position.z << ' '
                << part.size.x << ' ' << part.size.y << ' ' << part.size.z << ' '
                << part.color.x << ' ' << part.color.y << ' ' << part.color.z << ' '
                << part.rotation.x << ' ' << part.rotation.y << ' ' << part.rotation.z << ' '
                << part.transparency << ' '
                << part.reflectance << ' '
                << part.anchored << ' '
                << part.canCollide << ' '
                << weldTargets.size();

            for (const int target : weldTargets) {
                out << ' ' << target;
            }

            // Material and texture are optional trailing fields: a world that
            // uses neither writes nothing extra and keeps the old byte layout,
            // which older readers still load.
            if (part.material != Material::Plastic || !part.texture.empty()) {
                out << ' ' << static_cast<int>(part.material);
                if (!part.texture.empty()) {
                    out << ' ' << std::quoted(part.texture);
                }
            }

            out << '\n';
        }

        return true;
    }

    std::string readTextFile(const std::string& filename) {
        std::ifstream in(filename, std::ios::binary);
        if (!in.is_open()) {
            return "";
        }

        return std::string(
            std::istreambuf_iterator<char>(in),
            std::istreambuf_iterator<char>()
        );
    }

    std::string readTokenFile() {
        std::ifstream in("auth_token.txt");
        std::string token;
        if (in.is_open()) {
            std::getline(in, token);
        }
        return token;
    }

    void writeTokenFile(const std::string& token) {
        std::ofstream out("auth_token.txt");
        if (out.is_open()) {
            out << token;
        }
    }

    std::string jsonStringField(const std::string& json, const std::string& field) {
        const std::string needle = "\"" + field + "\":\"";
        size_t pos = json.find(needle);
        if (pos == std::string::npos) return "";
        pos += needle.size();
        size_t end = json.find('"', pos);
        if (end == std::string::npos) return "";
        return json.substr(pos, end - pos);
    }

    std::string loginForPublish(const std::string& webUrl, const std::string& username, const std::string& password) {
        const std::string response = Auth::login(username, password, webUrl);
        if (response.empty() || response.find("\"success\":true") == std::string::npos) {
            return "";
        }

        return jsonStringField(response, "token");
    }

    bool publishWorld(
        const std::string& webUrl,
        const std::string& token,
        const std::string& title,
        const std::string& description,
        bool isPublic,
        const std::string& worldText,
        std::string& response
    ) {
        if (token.empty() || worldText.empty()) {
            return false;
        }

        std::ostringstream body;
        body << "{\"title\":\"" << Auth::jsonEscape(title) << "\","
             << "\"description\":\"" << Auth::jsonEscape(description) << "\","
             << "\"isPublic\":" << (isPublic ? "true" : "false") << ","
             << "\"worldText\":\"" << Auth::jsonEscape(worldText) << "\"}";

        response = Auth::httpPost(webUrl + "/api/games/publish", body.str(), token);
        return !response.empty() && response.find("\"success\":true") != std::string::npos;
    }

    linearmath::mat4 partToMatrix(const Part& part) {
        linearmath::mat4 model(1.0f);
        model = linearmath::translate(model, part.position);
        model *= linearmath::toMat4(linearmath::quat(linearmath::radians(part.rotation)));
        model = linearmath::scale(model, part.size);
        return model;
    }

    linearmath::vec3 clampScale(const linearmath::vec3& value) {
        return linearmath::vec3(
            std::clamp(value.x, MIN_PART_SCALE, MAX_PART_SCALE),
            std::clamp(value.y, MIN_PART_SCALE, MAX_PART_SCALE),
            std::clamp(value.z, MIN_PART_SCALE, MAX_PART_SCALE)
        );
    }

    void applyMatrixToPart(const linearmath::mat4& model, Part& part, PhysicsWorld& physicsWorld, bool rebuildShape) {
        float translation[3];
        float rotation[3];
        float scale[3];
        LinearGizmo::DecomposeMatrixToComponents(linearmath::value_ptr(model), translation, rotation, scale);

        part.position = linearmath::vec3(translation[0], translation[1], translation[2]);
        part.rotation = linearmath::vec3(rotation[0], rotation[1], rotation[2]);

        if (rebuildShape) {
            part.size = clampScale(linearmath::vec3(scale[0], scale[1], scale[2]));
            if (part.physicsBody) {
                physicsWorld.removePart(&part);
                physicsWorld.addPart(&part);
            }
        } else {
            physicsWorld.updatePartBody(&part);
        }
    }

    // Puts a snapshot back into the world. The deque is rebuilt from scratch and
    // then the physics bodies and the welds, because every Part the old deque held
    // is gone the moment the first part is pushed into it.
    void applySnapshot(const linear::studio::WorldSnapshot& snapshot, std::deque<Part>& parts,
                       PhysicsWorld& physicsWorld) {
        parts.clear();
        physicsWorld.reset();

        for (const linear::studio::PartSnapshot& saved : snapshot) {
            Part part(saved.position, saved.size, saved.color, saved.shape, saved.name);
            part.parentIndex = saved.parentIndex;
            part.isFolder = saved.isFolder;
            part.isCamera = saved.isCamera;
            part.isSpawn = saved.isSpawn;
            part.deleted = saved.deleted;
            part.rotation = saved.rotation;
            part.transparency = saved.transparency;
            part.reflectance = saved.reflectance;
            part.anchored = saved.anchored;
            part.canCollide = saved.canCollide;
            part.material = saved.material;
            part.texture = saved.texture;
            parts.push_back(part);
        }

        for (Part& part : parts) {
            if (!part.isFolder) {
                physicsWorld.addPart(&part);
            }
        }

        for (std::size_t index = 0; index < snapshot.size(); ++index) {
            for (const int target : snapshot[index].weldTargets) {
                if (target < 0 || target >= static_cast<int>(parts.size())) {
                    continue;
                }
                Weld weld;
                weld.part1 = &parts[target];
                parts[index].welds.push_back(weld);
                if (parts[index].physicsBody != nullptr && parts[target].physicsBody != nullptr) {
                    physicsWorld.createWeld(&parts[index], &parts[target]);
                }
            }
        }
    }

    // Where a part sits in the world, or -1. A weld names a part by pointer while
    // the rest of Studio names parts by index, so this is where the two meet.
    // The world's own hierarchy: a part's folder is Part::parentIndex, so the tree
    // the Explorer draws is the one the file holds rather than a second copy of it.
    // The depth limit is what stops a file whose parent index points into its own
    // children from recursing for ever.
    void drawPartTree(std::deque<Part>& parts, std::vector<int>& selection, int parent,
                      int depth) {
        if (depth > 8) return;
        for (int i = 0; i < static_cast<int>(parts.size()); ++i) {
            Part& part = parts[static_cast<std::size_t>(i)];
            if (part.deleted || part.parentIndex != parent) continue;
            if (part.isFolder) {
                // A folder opens onto its contents. The pointer is the id, because
                // two folders may carry the same name.
                if (LinearUI::TreeNode(static_cast<const void*>(&part), "%s",
                                       part.name.c_str())) {
                    drawPartTree(parts, selection, i, depth + 1);
                    LinearUI::TreePop();
                }
                continue;
            }
            if (LinearUI::Selectable(part.name.c_str(),
                                     linear::studio::isSelected(selection, i))) {
                if (LinearUI::GetIO().KeyCtrl) {
                    linear::studio::toggleSelected(selection, i);
                } else {
                    linear::studio::selectOnly(selection, i);
                }
            }
        }
    }

    int partIndex(const std::deque<Part>& parts, const Part* part) {
        for (std::size_t index = 0; index < parts.size(); ++index) {
            if (&parts[index] == part) {
                return static_cast<int>(index);
            }
        }
        return -1;
    }

    // Deletes the selection. A part is flagged rather than erased, which is what
    // the world file, the game server and the script runtime already expect, and
    // what keeps every other index - and every weld naming one - valid.
    void deleteSelection(std::vector<int>& selection, std::deque<Part>& parts,
                         PhysicsWorld& physicsWorld) {
        const std::vector<int> doomed = linear::studio::withFolderContents(parts, selection);

        for (const int index : doomed) {
            if (index < 0 || index >= static_cast<int>(parts.size())) {
                continue;
            }
            Part& part = parts[static_cast<std::size_t>(index)];
            if (part.deleted) {
                continue;
            }
            part.deleted = true;
            if (part.physicsBody != nullptr) {
                physicsWorld.removePart(&part);
            }
            part.welds.clear();
        }

        // A weld into a part that is gone would hold two bodies to something that
        // is no longer in the world.
        for (Part& part : parts) {
            part.welds.erase(
                std::remove_if(part.welds.begin(), part.welds.end(),
                               [&parts, &doomed](const Weld& weld) {
                                   return linear::studio::isSelected(doomed, partIndex(parts, weld.part1));
                               }),
                part.welds.end());
        }

        selection.clear();
    }

    // Copies the selection and selects the copies. The offset is the primary
    // part's own width, so a copy stands beside its original instead of inside
    // it. A copy keeps its place in a folder, and a weld is copied only when both
    // ends were copied, because the other end still names the original.
    void duplicateSelection(std::vector<int>& selection, std::deque<Part>& parts,
                            PhysicsWorld& physicsWorld) {
        std::vector<int> sources;
        for (const int index : selection) {
            if (index < 0 || index >= static_cast<int>(parts.size())) {
                continue;
            }
            if (parts[static_cast<std::size_t>(index)].deleted) {
                continue;
            }
            sources.push_back(index);
        }
        if (sources.empty()) {
            return;
        }

        const int primary = linear::studio::primarySelection(selection);
        const float offsetX = primary >= 0 && primary < static_cast<int>(parts.size())
            ? parts[static_cast<std::size_t>(primary)].size.x + 0.5f
            : 1.0f;

        std::vector<Part> copies;
        copies.reserve(sources.size());
        for (const int index : sources) {
            Part copy = parts[static_cast<std::size_t>(index)];
            copy.physicsBody = nullptr;
            copy.welds.clear();
            copy.position.x += offsetX;
            copies.push_back(copy);
        }

        const int firstCopy = static_cast<int>(parts.size());
        for (const Part& copy : copies) {
            parts.push_back(copy);
        }

        // The copies now have places, so a parent that was copied too can be
        // pointed at its own copy rather than at the original.
        for (std::size_t offset = 0; offset < sources.size(); ++offset) {
            Part& copy = parts[static_cast<std::size_t>(firstCopy) + offset];
            const int parent = parts[static_cast<std::size_t>(sources[offset])].parentIndex;
            const auto found = std::find(sources.begin(), sources.end(), parent);
            if (found != sources.end()) {
                copy.parentIndex = firstCopy + static_cast<int>(found - sources.begin());
            }
        }

        for (std::size_t offset = 0; offset < sources.size(); ++offset) {
            Part& copy = parts[static_cast<std::size_t>(firstCopy) + offset];
            for (const Weld& weld : parts[static_cast<std::size_t>(sources[offset])].welds) {
                const int target = partIndex(parts, weld.part1);
                const auto found = std::find(sources.begin(), sources.end(), target);
                if (found == sources.end()) {
                    continue;
                }
                Weld copiedWeld;
                copiedWeld.part1 = &parts[static_cast<std::size_t>(firstCopy) +
                                          static_cast<std::size_t>(found - sources.begin())];
                copy.welds.push_back(copiedWeld);
            }
        }

        for (std::size_t offset = 0; offset < sources.size(); ++offset) {
            Part& copy = parts[static_cast<std::size_t>(firstCopy) + offset];
            if (!copy.isFolder) {
                physicsWorld.addPart(&copy);
            }
        }
        for (std::size_t offset = 0; offset < sources.size(); ++offset) {
            Part& copy = parts[static_cast<std::size_t>(firstCopy) + offset];
            for (const Weld& weld : copy.welds) {
                if (copy.physicsBody != nullptr && weld.part1 != nullptr &&
                    weld.part1->physicsBody != nullptr) {
                    physicsWorld.createWeld(&copy, weld.part1);
                }
            }
        }

        selection.clear();
        for (std::size_t offset = 0; offset < sources.size(); ++offset) {
            selection.push_back(firstCopy + static_cast<int>(offset));
        }
    }

    void processCameraInput(Window& window, Camera& camera, float deltaTime) {
        LINEARWINwindow* nativeWindow = window.getNativeWindow();
        const bool sprint = linearwinGetKey(nativeWindow, LINEARWIN_KEY_LEFT_SHIFT) == LINEARWIN_PRESS;
        if (linearwinGetKey(nativeWindow, LINEARWIN_KEY_W) == LINEARWIN_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime, sprint);
        if (linearwinGetKey(nativeWindow, LINEARWIN_KEY_S) == LINEARWIN_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime, sprint);
        if (linearwinGetKey(nativeWindow, LINEARWIN_KEY_A) == LINEARWIN_PRESS) camera.ProcessKeyboard(LEFT, deltaTime, sprint);
        if (linearwinGetKey(nativeWindow, LINEARWIN_KEY_D) == LINEARWIN_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime, sprint);
        if (linearwinGetKey(nativeWindow, LINEARWIN_KEY_E) == LINEARWIN_PRESS) camera.ProcessKeyboard(UP, deltaTime, sprint);
        if (linearwinGetKey(nativeWindow, LINEARWIN_KEY_Q) == LINEARWIN_PRESS) camera.ProcessKeyboard(DOWN, deltaTime, sprint);
    }
}

int main() {
    try {
        Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "Linear Studio");
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        Camera camera(linearmath::vec3(8.0f, 8.0f, 12.0f));
        camera.LookAt(linearmath::vec3(0.0f, 0.0f, 0.0f));

        PhysicsWorld physicsWorld;
        physicsWorld.init();

        std::deque<Part> parts;
        WorldLoader::loadWorld("ServerWorld.world", parts, physicsWorld);
        if (parts.empty()) {
            addDefaultWorld(parts, physicsWorld);
        }

        Renderer renderer;
        renderer.init();

        ShadowMap shadowMap;
        shadowMap.init();

        Editor editor(window.getNativeWindow());
        editor.init();

        std::vector<int> selection;
        linear::studio::selectOnly(selection, parts.size() > 1 ? 1 : 0);
        LinearGizmo::OPERATION operation = LinearGizmo::TRANSLATE;
        LinearGizmo::MODE mode = LinearGizmo::LOCAL;
        bool snapEnabled = false;
        float snap[3] = { 1.0f, 1.0f, 1.0f };
        char worldPath[260] = "ServerWorld.world";
        std::string statusMessage = "Studio ready";
        char publishWebUrl[128] = "http://localhost:3000";
        char publishTitle[64] = "Starter Baseplate";
        char publishDescription[241] = "";
        char publishUsername[32] = "";
        char publishPassword[64] = "";
        bool publishPublic = true;
        std::string publishStatus = "Sign in to publish";

        char scriptPath[260] = "games/sky-platformer.rbx";
        std::string scriptStatus = "No script running";
        std::unique_ptr<linear::script::Interpreter> scriptInterpreter;
        std::unique_ptr<linear::worldscript::WorldRuntime> scriptRuntime;
        bool scriptRunning = false;

        float lastFrame = static_cast<float>(linearwinGetTime());
        double lastMouseX = WINDOW_WIDTH / 2.0;
        double lastMouseY = WINDOW_HEIGHT / 2.0;
        bool firstMouse = true;
        bool wasLeftPressed = false;
        linear::studio::History history;

        while (!window.shouldClose()) {
            float currentFrame = static_cast<float>(linearwinGetTime());
            float deltaTime = currentFrame - lastFrame;
            lastFrame = currentFrame;

            window.pollEvents();

            // A request from the menu, the toolbar or a shortcut is consumed by the
            // frame that made it.
            bool undoRequested = false;
            bool redoRequested = false;
            bool deleteRequested = false;
            bool duplicateRequested = false;

            if (scriptRunning && scriptRuntime) {
                scriptRuntime->heartbeat(deltaTime);
            }

            editor.newFrame();
            LINEARWINwindow* nativeWindow = window.getNativeWindow();

            if (LinearUI::BeginMainMenuBar()) {
                if (LinearUI::BeginMenu("File")) {
                    if (LinearUI::MenuItem("New")) {
                        addDefaultWorld(parts, physicsWorld);
                        linear::studio::selectOnly(selection, parts.size() > 1 ? 1 : 0);
                        // A world that was just created has no steps behind it.
                        history.clear();
                        statusMessage = "Created a new default world";
                    }
                    if (LinearUI::MenuItem("Load")) {
                        WorldLoader::loadWorld(worldPath, parts, physicsWorld);
                        if (parts.empty()) {
                            addDefaultWorld(parts, physicsWorld);
                            linear::studio::selectOnly(selection, parts.size() > 1 ? 1 : 0);
                            history.clear();
                            statusMessage = "Load failed; restored default world";
                        } else {
                            linear::studio::selectOnly(selection, 0);
                            // A step taken in another world cannot be undone into
                            // this one, so loading starts the history again.
                            history.clear();
                            statusMessage = std::string("Loaded ") + worldPath;
                        }
                    }
                    if (LinearUI::MenuItem("Save")) {
                        statusMessage = saveWorld(worldPath, parts) ? std::string("Saved ") + worldPath : std::string("Failed to save ") + worldPath;
                    }
                    if (LinearUI::MenuItem("Exit")) {
                        linearwinSetWindowShouldClose(nativeWindow, true);
                    }
                    LinearUI::EndMenu();
                }
                if (LinearUI::BeginMenu("Edit")) {
                    if (LinearUI::MenuItem("Undo", "Ctrl+Z", false, history.canUndo())) {
                        undoRequested = true;
                    }
                    if (LinearUI::MenuItem("Redo", "Ctrl+Y", false, history.canRedo())) {
                        redoRequested = true;
                    }
                    LinearUI::Separator();
                    if (LinearUI::MenuItem("Duplicate", "Ctrl+D", false, !selection.empty())) {
                        duplicateRequested = true;
                    }
                    if (LinearUI::MenuItem("Delete", "Del", false, !selection.empty())) {
                        deleteRequested = true;
                    }
                    LinearUI::EndMenu();
                }
                LinearUI::EndMainMenuBar();
            }

            // The shortcuts belong to Studio only while nothing else has the
            // keyboard: Ctrl+Z inside the script editor is the editor's own undo.
            LinearUIIO& io = LinearUI::GetIO();
            if (!io.WantCaptureKeyboard) {
                if (io.KeyCtrl && LinearUI::IsKeyPressed(LinearUIKey_Z)) {
                    if (io.KeyShift) {
                        redoRequested = true;
                    } else {
                        undoRequested = true;
                    }
                }
                if (io.KeyCtrl && LinearUI::IsKeyPressed(LinearUIKey_Y)) {
                    redoRequested = true;
                }
                if (LinearUI::IsKeyPressed(LinearUIKey_Delete) || LinearUI::IsKeyPressed(LinearUIKey_Backspace)) {
                    deleteRequested = true;
                }
                if (io.KeyCtrl && LinearUI::IsKeyPressed(LinearUIKey_D)) {
                    duplicateRequested = true;
                }
            }

            LinearUI::Begin("Toolbox");
            LinearUI::InputText("World file", worldPath, sizeof(worldPath));
            if (LinearUI::Button("Add Part")) {
                parts.emplace_back(linearmath::vec3(0.0f, 3.0f, 0.0f), linearmath::vec3(2.0f), linearmath::vec3(0.7f, 0.7f, 0.9f), ShapeType::Cube, "Part");
                parts.back().anchored = true;
                physicsWorld.addPart(&parts.back());
                linear::studio::selectOnly(selection, static_cast<int>(parts.size() - 1));
            }
            LinearUI::SameLine();
            LinearUI::BeginDisabled(selection.empty());
            if (LinearUI::Button("Duplicate")) {
                duplicateRequested = true;
            }
            LinearUI::SameLine();
            if (LinearUI::Button("Delete")) {
                deleteRequested = true;
            }
            LinearUI::EndDisabled();
            LinearUI::SameLine();
            LinearUI::TextDisabled("%zu selected", selection.size());
            LinearUI::BeginDisabled(!history.canUndo());
            if (LinearUI::Button("Undo")) {
                undoRequested = true;
            }
            LinearUI::EndDisabled();
            LinearUI::SameLine();
            LinearUI::BeginDisabled(!history.canRedo());
            if (LinearUI::Button("Redo")) {
                redoRequested = true;
            }
            LinearUI::EndDisabled();
            LinearUI::SameLine();
            LinearUI::TextDisabled("%zu step(s) back", history.undoDepth());
            LinearUI::Separator();
            if (LinearUI::RadioButton("Move", operation == LinearGizmo::TRANSLATE)) operation = LinearGizmo::TRANSLATE;
            if (LinearUI::RadioButton("Rotate", operation == LinearGizmo::ROTATE)) operation = LinearGizmo::ROTATE;
            if (LinearUI::RadioButton("Scale", operation == LinearGizmo::SCALE)) operation = LinearGizmo::SCALE;
            LinearUI::Checkbox("Snap", &snapEnabled);
            LinearUI::InputFloat3("Snap values", snap);
            LinearUI::TextWrapped("%s", statusMessage.c_str());
            LinearUI::End();

            LinearUI::Begin("Publish");
            LinearUI::InputText("Web URL", publishWebUrl, sizeof(publishWebUrl));
            LinearUI::InputText("Username", publishUsername, sizeof(publishUsername));
            LinearUI::InputText("Password", publishPassword, sizeof(publishPassword), LinearUIInputTextFlags_Password);
            if (LinearUI::Button("Login")) {
                const std::string token = loginForPublish(publishWebUrl, publishUsername, publishPassword);
                if (token.empty()) {
                    publishStatus = "Login failed";
                } else {
                    writeTokenFile(token);
                    publishStatus = "Logged in for publishing";
                }
            }
            LinearUI::Separator();
            LinearUI::InputText("Game title", publishTitle, sizeof(publishTitle));
            LinearUI::InputTextMultiline("Description", publishDescription, sizeof(publishDescription), LinearVec2(0.0f, 72.0f));
            LinearUI::Checkbox("Public", &publishPublic);
            if (LinearUI::Button("Publish current world")) {
                if (!saveWorld(worldPath, parts)) {
                    publishStatus = std::string("Could not save ") + worldPath;
                } else {
                    const std::string token = readTokenFile();
                    const std::string worldText = readTextFile(worldPath);
                    std::string response;
                    if (publishWorld(publishWebUrl, token, publishTitle, publishDescription, publishPublic, worldText, response)) {
                        publishStatus = "Published to site";
                    } else if (token.empty()) {
                        publishStatus = "Login before publishing";
                    } else {
                        publishStatus = "Publish failed";
                    }
                }
            }
            LinearUI::SameLine();
            if (LinearUI::Button("Open site")) {
                Auth::openBrowser(std::string(publishWebUrl) + "/create");
            }
            LinearUI::TextWrapped("%s", publishStatus.c_str());
            LinearUI::End();

            LinearUI::Begin("Script");

            // Studio's script panel is a code editor rather than a text box: line
            // numbers, syntax highlighting, selection, undo and the dark palette
            // VS Code uses. Linear Script is Lua-shaped, so the Lua definition is
            // the one that colours it correctly. It is created on first use, once
            // the LinearUI context exists, and it holds the script itself - Save and
            // Run read it back instead of a fixed buffer that would cut a long
            // script short.
            static std::unique_ptr<LinearTextEditor> codeEditor;
            if (!codeEditor) {
                codeEditor = std::make_unique<LinearTextEditor>();
                codeEditor->SetLanguageDefinition(LinearTextEditor::LanguageDefinition::Lua());
                codeEditor->SetPalette(LinearTextEditor::GetDarkPalette());
                codeEditor->SetTabSize(4);
                codeEditor->SetShowWhitespaces(false);
                codeEditor->SetText("-- Press Load to open a .rbx script, or write one here.\n");
            }

            LinearUI::InputText("Script file", scriptPath, sizeof(scriptPath));

            // Everything the panel has, apart from the button row and the status
            // bar under it, belongs to the code.
            const float footerHeight = LinearUI::GetFrameHeightWithSpacing() * 2.0f;
            codeEditor->Render("##scriptEditor", LinearVec2(0.0f, -footerHeight), true);
            if (LinearUI::Button("Load")) {
                const std::string text = readTextFile(scriptPath);
                if (text.empty()) {
                    scriptStatus = std::string("Could not read ") + scriptPath;
                } else {
                    codeEditor->SetText(text);
                    scriptStatus = std::string("Loaded ") + scriptPath;
                }
            }
            LinearUI::SameLine();
            if (LinearUI::Button("Save")) {
                std::ofstream out(scriptPath, std::ios::binary);
                if (out.is_open()) {
                    out << codeEditor->GetText();
                    scriptStatus = std::string("Saved ") + scriptPath;
                } else {
                    scriptStatus = std::string("Could not save ") + scriptPath;
                }
            }
            LinearUI::SameLine();
            if (LinearUI::Button("Run")) {
                scriptRunning = false;
                scriptRuntime.reset();
                scriptInterpreter.reset();
                scriptInterpreter = std::make_unique<linear::script::Interpreter>();
                scriptInterpreter->setPrintHandler(
                    [&scriptStatus](const std::string& message, bool isError) {
                        scriptStatus = (isError ? "[script error] " : "[script] ") + message;
                    });
                scriptRuntime = std::make_unique<linear::worldscript::WorldRuntime>(
                    parts, &physicsWorld, *scriptInterpreter);
                std::string scriptError;
                if (scriptRuntime->runScript(codeEditor->GetText(), scriptPath, scriptError)) {
                    scriptRunning = true;
                    scriptStatus = std::string("Running ") + scriptPath;
                } else {
                    scriptStatus = scriptError;
                }
            }
            LinearUI::SameLine();
            if (LinearUI::Button("Stop")) {
                scriptRunning = false;
                if (scriptRuntime) scriptRuntime->stop();
                scriptStatus = "Stopped";
            }
            const LinearTextEditor::Coordinates cursor = codeEditor->GetCursorPosition();
            LinearUI::TextDisabled("%s   Ln %d, Col %d   Lua   |   %s", scriptPath,
                                cursor.mLine + 1, cursor.mColumn + 1, scriptStatus.c_str());
            LinearUI::End();

            LinearUI::Begin("Explorer");
            // The same tree a script names through game:GetService, in the same
            // order, so the panel and the language cannot disagree about what a
            // world has.
            for (const linear::worldscript::Service& service : linear::worldscript::services()) {
                const bool isWorkspace = std::string(service.name) == "Workspace";
                const LinearUI::LinearUITreeNodeFlags flags =
                    isWorkspace ? LinearUI::LinearUITreeNodeFlags_DefaultOpen
                                : LinearUI::LinearUITreeNodeFlags_None;
                if (LinearUI::CollapsingHeader(service.name, flags)) {
                    LinearUI::Indent();
                    // A part may carry a service's name, and the id stack is what
                    // keeps the two rows from opening each other.
                    LinearUI::PushID("children");
                    if (isWorkspace) {
                        drawPartTree(parts, selection, -1, 0);
                    } else {
                        LinearUI::TextDisabled("%s", service.note);
                    }
                    LinearUI::PopID();
                    LinearUI::Unindent();
                }
            }
            LinearUI::TextDisabled("Ctrl+click adds or removes a part");
            LinearUI::End();

            LinearUI::Begin("Properties");
            const int primaryPart = linear::studio::primarySelection(selection);
            if (selection.size() > 1 && primaryPart >= 0 &&
                primaryPart < static_cast<int>(parts.size())) {
                LinearUI::TextDisabled("%zu parts selected; editing %s. The gizmo moves all of them.",
                                    selection.size(), parts[static_cast<std::size_t>(primaryPart)].name.c_str());
            }
            if (primaryPart >= 0 && primaryPart < static_cast<int>(parts.size())) {
                Part& part = parts[static_cast<std::size_t>(primaryPart)];
                char name[64] = {};
                std::snprintf(name, sizeof(name), "%s", part.name.c_str());
                if (LinearUI::InputText("Name", name, sizeof(name))) {
                    part.name = name;
                }
                bool transformChanged = false;
                transformChanged |= LinearUI::DragFloat3("Position", linearmath::value_ptr(part.position), 0.1f);
                transformChanged |= LinearUI::DragFloat3("Rotation", linearmath::value_ptr(part.rotation), 0.5f);
                if (LinearUI::DragFloat3("Size", linearmath::value_ptr(part.size), 0.1f, MIN_PART_SCALE, MAX_PART_SCALE)) {
                    part.size = clampScale(part.size);
                    if (part.physicsBody) {
                        physicsWorld.removePart(&part);
                        physicsWorld.addPart(&part);
                    }
                } else if (transformChanged) {
                    physicsWorld.updatePartBody(&part);
                }
                LinearUI::ColorEdit3("Color", linearmath::value_ptr(part.color));

                static const char* materialLabels[kMaterialCount] = {
                    "Plastic", "Smooth plastic", "Neon", "Metal",
                    "Wood", "Grass", "Concrete", "Brick"
                };
                int materialIndex = static_cast<int>(part.material);
                if (LinearUI::Combo("Material", &materialIndex, materialLabels, kMaterialCount)) {
                    part.material = static_cast<Material>(materialIndex);
                }

                char texturePath[128] = {};
                std::snprintf(texturePath, sizeof(texturePath), "%s", part.texture.c_str());
                if (LinearUI::InputText("Texture", texturePath, sizeof(texturePath))) {
                    part.texture = texturePath;
                }
                LinearUI::TextDisabled("Tiles over the part; image goes in textures/.");

                LinearUI::Checkbox("Anchored", &part.anchored);
                LinearUI::Checkbox("Can collide", &part.canCollide);
            } else {
                LinearUI::Text("No part selected");
            }
            LinearUI::End();

            // Edits that add or remove parts land after the panels and before the
            // undo block, so the world the history settles on at the end of the
            // frame is the finished edit rather than half of one.
            if (deleteRequested && !selection.empty()) {
                const std::size_t removed = selection.size();
                deleteSelection(selection, parts, physicsWorld);
                statusMessage = "Deleted " + std::to_string(removed) +
                    (removed == 1 ? " part" : " parts");
            }
            if (duplicateRequested && !selection.empty()) {
                const std::size_t copied = selection.size();
                duplicateSelection(selection, parts, physicsWorld);
                statusMessage = "Duplicated " + std::to_string(copied) +
                    (copied == 1 ? " part" : " parts");
            }

            // Undo is applied here, after every panel that can set a request, so a
            // menu item and a button both take effect in the frame they were used.
            if (undoRequested || redoRequested) {
                const linear::studio::WorldSnapshot current = linear::studio::captureWorld(parts);
                linear::studio::WorldSnapshot restored;
                const bool stepped = undoRequested ? history.undo(current, &restored)
                                                   : history.redo(current, &restored);
                if (stepped) {
                    applySnapshot(restored, parts, physicsWorld);
                    // A step can add or remove parts, so what was selected may no
                    // longer be there.
                    linear::studio::pruneSelection(selection, parts);
                    statusMessage = undoRequested ? "Undo" : "Redo";
                } else {
                    statusMessage = undoRequested ? "Nothing to undo" : "Nothing to redo";
                }
            }

            const bool gizmoInUse = LinearGizmo::IsUsing();
            if (!gizmoInUse && !io.WantCaptureKeyboard) {
                processCameraInput(window, camera, deltaTime);
            }

            if (!gizmoInUse && !io.WantCaptureMouse && linearwinGetMouseButton(nativeWindow, LINEARWIN_MOUSE_BUTTON_RIGHT) == LINEARWIN_PRESS) {
                double mouseX = 0.0;
                double mouseY = 0.0;
                linearwinGetCursorPos(nativeWindow, &mouseX, &mouseY);
                if (firstMouse) {
                    lastMouseX = mouseX;
                    lastMouseY = mouseY;
                    firstMouse = false;
                }
                camera.ProcessMouseMovement(static_cast<float>(mouseX - lastMouseX), static_cast<float>(lastMouseY - mouseY));
                lastMouseX = mouseX;
                lastMouseY = mouseY;
            } else {
                firstMouse = true;
            }

            bool leftPressed = linearwinGetMouseButton(nativeWindow, LINEARWIN_MOUSE_BUTTON_LEFT) == LINEARWIN_PRESS;

            linearmath::vec3 lightPos(20.0f, 50.0f, 20.0f);
            linearmath::mat4 lightSpaceMatrix = shadowMap.getLightSpaceMatrix(lightPos);

            shadowMap.bindForWriting();
            for (const Part& part : parts) {
                if (!part.deleted && !part.isFolder) {
                    renderer.drawPartShadow(part, lightSpaceMatrix);
                }
            }
            shadowMap.unbind();

            glViewport(0, 0, window.getWidth(), window.getHeight());
            glClearColor(0.53f, 0.81f, 0.92f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            linearmath::mat4 view = camera.GetViewMatrix();
            linearmath::mat4 projection = camera.GetProjectionMatrix(static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

            if (renderer.shader) {
                renderer.shader->use();
                renderer.shader->setVec3("lightPos", lightPos);
            }

            for (const Part& part : parts) {
                if (!part.deleted && !part.isFolder) {
                    renderer.drawPart(part, view, projection, lightSpaceMatrix, shadowMap.depthMap, 0);
                }
            }

            const int gizmoPart = linear::studio::primarySelection(selection);
            if (gizmoPart >= 0 && gizmoPart < static_cast<int>(parts.size()) &&
                !parts[static_cast<std::size_t>(gizmoPart)].isFolder) {
                Part& part = parts[static_cast<std::size_t>(gizmoPart)];
                linearmath::mat4 model = partToMatrix(part);
                const linearmath::mat4 before = model;

                LinearGizmo::SetOrthographic(false);
                const LinearUIViewport* mainViewport = LinearUI::GetMainViewport();
                LinearGizmo::SetRect(mainViewport->Pos.x, mainViewport->Pos.y, mainViewport->Size.x, mainViewport->Size.y);

                const float* snapValues = snapEnabled ? snap : nullptr;
                if (LinearGizmo::Manipulate(
                        linearmath::value_ptr(view),
                        linearmath::value_ptr(projection),
                        operation,
                        mode,
                        linearmath::value_ptr(model),
                        nullptr,
                        snapValues)) {
                    // LinearGizmo returns the full model matrix. Assign the decomposed size directly;
                    // multiplying by the returned scale each frame causes runaway scale growth.
                    //
                    // The rest of the selection moves by what the gizmo did to the
                    // part it drew: the step is that matrix against the one it
                    // started from, applied to each of them, so a group turns about
                    // the primary and keeps its shape.
                    const linearmath::mat4 delta = model * linearmath::inverse(before);
                    for (const int index : selection) {
                        if (index < 0 || index >= static_cast<int>(parts.size())) {
                            continue;
                        }
                        Part& target = parts[static_cast<std::size_t>(index)];
                        if (target.deleted) {
                            continue;
                        }
                        if (index == gizmoPart) {
                            applyMatrixToPart(model, target, physicsWorld, operation == LinearGizmo::SCALE);
                        } else {
                            applyMatrixToPart(delta * partToMatrix(target), target, physicsWorld,
                                              operation == LinearGizmo::SCALE);
                        }
                    }
                }
            }

            if (leftPressed && !wasLeftPressed && !io.WantCaptureMouse && !LinearGizmo::IsUsing() && !LinearGizmo::IsOver()) {
                double mouseX = 0.0;
                double mouseY = 0.0;
                linearwinGetCursorPos(nativeWindow, &mouseX, &mouseY);
                int picked = Raycaster::GetPartFromMouse(parts, camera, static_cast<float>(mouseX), static_cast<float>(mouseY), static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));
                if (picked >= 0 && picked < static_cast<int>(parts.size())) {
                    if (io.KeyCtrl) {
                        linear::studio::toggleSelected(selection, picked);
                    } else {
                        linear::studio::selectOnly(selection, picked);
                    }
                } else if (!io.KeyCtrl) {
                    // A click on nothing lets go of the whole selection, which is
                    // the only way to pick a single part again after a group.
                    selection.clear();
                }
            }
            wasLeftPressed = leftPressed;

            // One step is one finished interaction: IsAnyItemActive covers a slider
            // being dragged and a name being typed, and the gizmo is not an LinearUI
            // item, so it is asked separately.
            const bool interacting = LinearUI::IsAnyItemActive() || LinearGizmo::IsUsing();
            history.note(linear::studio::captureWorld(parts), interacting);

            editor.render();
            window.swapBuffers();
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Studio failed: " << error.what() << std::endl;
        return 1;
    }
}
