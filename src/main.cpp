
#define GLM_ENABLE_EXPERIMENTAL

#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Editor.h"
#include "TextEditor.h"
#include "Part.h"
#include "PhysicsWorld.h"
#include "Raycaster.h"
#include "ShadowMap.h"
#include "WorldLoader.h"
#include "RblxScript.h"
#include "WorldScript.h"
#include "StudioHistory.h"
#include "StudioSelection.h"
#include "Auth.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>

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

        parts.emplace_back(glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(40.0f, 1.0f, 40.0f), glm::vec3(0.35f, 0.8f, 0.35f), ShapeType::Cube, "Baseplate");
        parts.back().anchored = true;
        physicsWorld.addPart(&parts.back());

        parts.emplace_back(glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.8f, 0.25f, 0.2f), ShapeType::Cube, "Part");
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

    glm::mat4 partToMatrix(const Part& part) {
        glm::mat4 model(1.0f);
        model = glm::translate(model, part.position);
        model *= glm::toMat4(glm::quat(glm::radians(part.rotation)));
        model = glm::scale(model, part.size);
        return model;
    }

    glm::vec3 clampScale(const glm::vec3& value) {
        return glm::vec3(
            std::clamp(value.x, MIN_PART_SCALE, MAX_PART_SCALE),
            std::clamp(value.y, MIN_PART_SCALE, MAX_PART_SCALE),
            std::clamp(value.z, MIN_PART_SCALE, MAX_PART_SCALE)
        );
    }

    void applyMatrixToPart(const glm::mat4& model, Part& part, PhysicsWorld& physicsWorld, bool rebuildShape) {
        float translation[3];
        float rotation[3];
        float scale[3];
        ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(model), translation, rotation, scale);

        part.position = glm::vec3(translation[0], translation[1], translation[2]);
        part.rotation = glm::vec3(rotation[0], rotation[1], rotation[2]);

        if (rebuildShape) {
            part.size = clampScale(glm::vec3(scale[0], scale[1], scale[2]));
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
    void applySnapshot(const rblx::studio::WorldSnapshot& snapshot, std::deque<Part>& parts,
                       PhysicsWorld& physicsWorld) {
        parts.clear();
        physicsWorld.reset();

        for (const rblx::studio::PartSnapshot& saved : snapshot) {
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
        const std::vector<int> doomed = rblx::studio::withFolderContents(parts, selection);

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
                                   return rblx::studio::isSelected(doomed, partIndex(parts, weld.part1));
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

        const int primary = rblx::studio::primarySelection(selection);
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
        GLFWwindow* nativeWindow = window.getNativeWindow();
        const bool sprint = glfwGetKey(nativeWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
        if (glfwGetKey(nativeWindow, GLFW_KEY_W) == GLFW_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime, sprint);
        if (glfwGetKey(nativeWindow, GLFW_KEY_S) == GLFW_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime, sprint);
        if (glfwGetKey(nativeWindow, GLFW_KEY_A) == GLFW_PRESS) camera.ProcessKeyboard(LEFT, deltaTime, sprint);
        if (glfwGetKey(nativeWindow, GLFW_KEY_D) == GLFW_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime, sprint);
        if (glfwGetKey(nativeWindow, GLFW_KEY_E) == GLFW_PRESS) camera.ProcessKeyboard(UP, deltaTime, sprint);
        if (glfwGetKey(nativeWindow, GLFW_KEY_Q) == GLFW_PRESS) camera.ProcessKeyboard(DOWN, deltaTime, sprint);
    }
}

int main() {
    try {
        Window window(WINDOW_WIDTH, WINDOW_HEIGHT, "RBLX Clone Studio");
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        Camera camera(glm::vec3(8.0f, 8.0f, 12.0f));
        camera.LookAt(glm::vec3(0.0f, 0.0f, 0.0f));

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
        rblx::studio::selectOnly(selection, parts.size() > 1 ? 1 : 0);
        ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
        ImGuizmo::MODE mode = ImGuizmo::LOCAL;
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
        std::unique_ptr<rblx::script::Interpreter> scriptInterpreter;
        std::unique_ptr<rblx::worldscript::WorldRuntime> scriptRuntime;
        bool scriptRunning = false;

        float lastFrame = static_cast<float>(glfwGetTime());
        double lastMouseX = WINDOW_WIDTH / 2.0;
        double lastMouseY = WINDOW_HEIGHT / 2.0;
        bool firstMouse = true;
        bool wasLeftPressed = false;
        rblx::studio::History history;

        while (!window.shouldClose()) {
            float currentFrame = static_cast<float>(glfwGetTime());
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
            GLFWwindow* nativeWindow = window.getNativeWindow();

            if (ImGui::BeginMainMenuBar()) {
                if (ImGui::BeginMenu("File")) {
                    if (ImGui::MenuItem("New")) {
                        addDefaultWorld(parts, physicsWorld);
                        rblx::studio::selectOnly(selection, parts.size() > 1 ? 1 : 0);
                        // A world that was just created has no steps behind it.
                        history.clear();
                        statusMessage = "Created a new default world";
                    }
                    if (ImGui::MenuItem("Load")) {
                        WorldLoader::loadWorld(worldPath, parts, physicsWorld);
                        if (parts.empty()) {
                            addDefaultWorld(parts, physicsWorld);
                            rblx::studio::selectOnly(selection, parts.size() > 1 ? 1 : 0);
                            history.clear();
                            statusMessage = "Load failed; restored default world";
                        } else {
                            rblx::studio::selectOnly(selection, 0);
                            // A step taken in another world cannot be undone into
                            // this one, so loading starts the history again.
                            history.clear();
                            statusMessage = std::string("Loaded ") + worldPath;
                        }
                    }
                    if (ImGui::MenuItem("Save")) {
                        statusMessage = saveWorld(worldPath, parts) ? std::string("Saved ") + worldPath : std::string("Failed to save ") + worldPath;
                    }
                    if (ImGui::MenuItem("Exit")) {
                        glfwSetWindowShouldClose(nativeWindow, true);
                    }
                    ImGui::EndMenu();
                }
                if (ImGui::BeginMenu("Edit")) {
                    if (ImGui::MenuItem("Undo", "Ctrl+Z", false, history.canUndo())) {
                        undoRequested = true;
                    }
                    if (ImGui::MenuItem("Redo", "Ctrl+Y", false, history.canRedo())) {
                        redoRequested = true;
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Duplicate", "Ctrl+D", false, !selection.empty())) {
                        duplicateRequested = true;
                    }
                    if (ImGui::MenuItem("Delete", "Del", false, !selection.empty())) {
                        deleteRequested = true;
                    }
                    ImGui::EndMenu();
                }
                ImGui::EndMainMenuBar();
            }

            // The shortcuts belong to Studio only while nothing else has the
            // keyboard: Ctrl+Z inside the script editor is the editor's own undo.
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureKeyboard) {
                if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z)) {
                    if (io.KeyShift) {
                        redoRequested = true;
                    } else {
                        undoRequested = true;
                    }
                }
                if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Y)) {
                    redoRequested = true;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Delete) || ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
                    deleteRequested = true;
                }
                if (io.KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D)) {
                    duplicateRequested = true;
                }
            }

            ImGui::Begin("Toolbox");
            ImGui::InputText("World file", worldPath, sizeof(worldPath));
            if (ImGui::Button("Add Part")) {
                parts.emplace_back(glm::vec3(0.0f, 3.0f, 0.0f), glm::vec3(2.0f), glm::vec3(0.7f, 0.7f, 0.9f), ShapeType::Cube, "Part");
                parts.back().anchored = true;
                physicsWorld.addPart(&parts.back());
                rblx::studio::selectOnly(selection, static_cast<int>(parts.size() - 1));
            }
            ImGui::SameLine();
            ImGui::BeginDisabled(selection.empty());
            if (ImGui::Button("Duplicate")) {
                duplicateRequested = true;
            }
            ImGui::SameLine();
            if (ImGui::Button("Delete")) {
                deleteRequested = true;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextDisabled("%zu selected", selection.size());
            ImGui::BeginDisabled(!history.canUndo());
            if (ImGui::Button("Undo")) {
                undoRequested = true;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::BeginDisabled(!history.canRedo());
            if (ImGui::Button("Redo")) {
                redoRequested = true;
            }
            ImGui::EndDisabled();
            ImGui::SameLine();
            ImGui::TextDisabled("%zu step(s) back", history.undoDepth());
            ImGui::Separator();
            if (ImGui::RadioButton("Move", operation == ImGuizmo::TRANSLATE)) operation = ImGuizmo::TRANSLATE;
            if (ImGui::RadioButton("Rotate", operation == ImGuizmo::ROTATE)) operation = ImGuizmo::ROTATE;
            if (ImGui::RadioButton("Scale", operation == ImGuizmo::SCALE)) operation = ImGuizmo::SCALE;
            ImGui::Checkbox("Snap", &snapEnabled);
            ImGui::InputFloat3("Snap values", snap);
            ImGui::TextWrapped("%s", statusMessage.c_str());
            ImGui::End();

            ImGui::Begin("Publish");
            ImGui::InputText("Web URL", publishWebUrl, sizeof(publishWebUrl));
            ImGui::InputText("Username", publishUsername, sizeof(publishUsername));
            ImGui::InputText("Password", publishPassword, sizeof(publishPassword), ImGuiInputTextFlags_Password);
            if (ImGui::Button("Login")) {
                const std::string token = loginForPublish(publishWebUrl, publishUsername, publishPassword);
                if (token.empty()) {
                    publishStatus = "Login failed";
                } else {
                    writeTokenFile(token);
                    publishStatus = "Logged in for publishing";
                }
            }
            ImGui::Separator();
            ImGui::InputText("Game title", publishTitle, sizeof(publishTitle));
            ImGui::InputTextMultiline("Description", publishDescription, sizeof(publishDescription), ImVec2(0.0f, 72.0f));
            ImGui::Checkbox("Public", &publishPublic);
            if (ImGui::Button("Publish current world")) {
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
            ImGui::SameLine();
            if (ImGui::Button("Open site")) {
                Auth::openBrowser(std::string(publishWebUrl) + "/create");
            }
            ImGui::TextWrapped("%s", publishStatus.c_str());
            ImGui::End();

            ImGui::Begin("Script");

            // Studio's script panel is a code editor rather than a text box: line
            // numbers, syntax highlighting, selection, undo and the dark palette
            // VS Code uses. RBLX Script is Lua-shaped, so the Lua definition is
            // the one that colours it correctly. It is created on first use, once
            // the ImGui context exists, and it holds the script itself - Save and
            // Run read it back instead of a fixed buffer that would cut a long
            // script short.
            static std::unique_ptr<TextEditor> codeEditor;
            if (!codeEditor) {
                codeEditor = std::make_unique<TextEditor>();
                codeEditor->SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());
                codeEditor->SetPalette(TextEditor::GetDarkPalette());
                codeEditor->SetTabSize(4);
                codeEditor->SetShowWhitespaces(false);
                codeEditor->SetText("-- Press Load to open a .rbx script, or write one here.\n");
            }

            ImGui::InputText("Script file", scriptPath, sizeof(scriptPath));

            // Everything the panel has, apart from the button row and the status
            // bar under it, belongs to the code.
            const float footerHeight = ImGui::GetFrameHeightWithSpacing() * 2.0f;
            codeEditor->Render("##scriptEditor", ImVec2(0.0f, -footerHeight), true);
            if (ImGui::Button("Load")) {
                const std::string text = readTextFile(scriptPath);
                if (text.empty()) {
                    scriptStatus = std::string("Could not read ") + scriptPath;
                } else {
                    codeEditor->SetText(text);
                    scriptStatus = std::string("Loaded ") + scriptPath;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Save")) {
                std::ofstream out(scriptPath, std::ios::binary);
                if (out.is_open()) {
                    out << codeEditor->GetText();
                    scriptStatus = std::string("Saved ") + scriptPath;
                } else {
                    scriptStatus = std::string("Could not save ") + scriptPath;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Run")) {
                scriptRunning = false;
                scriptRuntime.reset();
                scriptInterpreter.reset();
                scriptInterpreter = std::make_unique<rblx::script::Interpreter>();
                scriptInterpreter->setPrintHandler(
                    [&scriptStatus](const std::string& message, bool isError) {
                        scriptStatus = (isError ? "[script error] " : "[script] ") + message;
                    });
                scriptRuntime = std::make_unique<rblx::worldscript::WorldRuntime>(
                    parts, &physicsWorld, *scriptInterpreter);
                std::string scriptError;
                if (scriptRuntime->runScript(codeEditor->GetText(), scriptPath, scriptError)) {
                    scriptRunning = true;
                    scriptStatus = std::string("Running ") + scriptPath;
                } else {
                    scriptStatus = scriptError;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop")) {
                scriptRunning = false;
                if (scriptRuntime) scriptRuntime->stop();
                scriptStatus = "Stopped";
            }
            const TextEditor::Coordinates cursor = codeEditor->GetCursorPosition();
            ImGui::TextDisabled("%s   Ln %d, Col %d   Lua   |   %s", scriptPath,
                                cursor.mLine + 1, cursor.mColumn + 1, scriptStatus.c_str());
            ImGui::End();

            ImGui::Begin("Explorer");
            for (size_t i = 0; i < parts.size(); ++i) {
                if (parts[i].deleted) continue;
                if (ImGui::Selectable(parts[i].name.c_str(),
                                      rblx::studio::isSelected(selection, static_cast<int>(i)))) {
                    if (io.KeyCtrl) {
                        rblx::studio::toggleSelected(selection, static_cast<int>(i));
                    } else {
                        rblx::studio::selectOnly(selection, static_cast<int>(i));
                    }
                }
            }
            ImGui::TextDisabled("Ctrl+click adds or removes a part");
            ImGui::End();

            ImGui::Begin("Properties");
            const int primaryPart = rblx::studio::primarySelection(selection);
            if (selection.size() > 1 && primaryPart >= 0 &&
                primaryPart < static_cast<int>(parts.size())) {
                ImGui::TextDisabled("%zu parts selected; editing %s. The gizmo moves all of them.",
                                    selection.size(), parts[static_cast<std::size_t>(primaryPart)].name.c_str());
            }
            if (primaryPart >= 0 && primaryPart < static_cast<int>(parts.size())) {
                Part& part = parts[static_cast<std::size_t>(primaryPart)];
                char name[64] = {};
                std::snprintf(name, sizeof(name), "%s", part.name.c_str());
                if (ImGui::InputText("Name", name, sizeof(name))) {
                    part.name = name;
                }
                bool transformChanged = false;
                transformChanged |= ImGui::DragFloat3("Position", glm::value_ptr(part.position), 0.1f);
                transformChanged |= ImGui::DragFloat3("Rotation", glm::value_ptr(part.rotation), 0.5f);
                if (ImGui::DragFloat3("Size", glm::value_ptr(part.size), 0.1f, MIN_PART_SCALE, MAX_PART_SCALE)) {
                    part.size = clampScale(part.size);
                    if (part.physicsBody) {
                        physicsWorld.removePart(&part);
                        physicsWorld.addPart(&part);
                    }
                } else if (transformChanged) {
                    physicsWorld.updatePartBody(&part);
                }
                ImGui::ColorEdit3("Color", glm::value_ptr(part.color));

                static const char* materialLabels[kMaterialCount] = {
                    "Plastic", "Smooth plastic", "Neon", "Metal",
                    "Wood", "Grass", "Concrete", "Brick"
                };
                int materialIndex = static_cast<int>(part.material);
                if (ImGui::Combo("Material", &materialIndex, materialLabels, kMaterialCount)) {
                    part.material = static_cast<Material>(materialIndex);
                }

                char texturePath[128] = {};
                std::snprintf(texturePath, sizeof(texturePath), "%s", part.texture.c_str());
                if (ImGui::InputText("Texture", texturePath, sizeof(texturePath))) {
                    part.texture = texturePath;
                }
                ImGui::TextDisabled("Tiles over the part; image goes in textures/.");

                ImGui::Checkbox("Anchored", &part.anchored);
                ImGui::Checkbox("Can collide", &part.canCollide);
            } else {
                ImGui::Text("No part selected");
            }
            ImGui::End();

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
                const rblx::studio::WorldSnapshot current = rblx::studio::captureWorld(parts);
                rblx::studio::WorldSnapshot restored;
                const bool stepped = undoRequested ? history.undo(current, &restored)
                                                   : history.redo(current, &restored);
                if (stepped) {
                    applySnapshot(restored, parts, physicsWorld);
                    // A step can add or remove parts, so what was selected may no
                    // longer be there.
                    rblx::studio::pruneSelection(selection, parts);
                    statusMessage = undoRequested ? "Undo" : "Redo";
                } else {
                    statusMessage = undoRequested ? "Nothing to undo" : "Nothing to redo";
                }
            }

            const bool gizmoInUse = ImGuizmo::IsUsing();
            if (!gizmoInUse && !io.WantCaptureKeyboard) {
                processCameraInput(window, camera, deltaTime);
            }

            if (!gizmoInUse && !io.WantCaptureMouse && glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
                double mouseX = 0.0;
                double mouseY = 0.0;
                glfwGetCursorPos(nativeWindow, &mouseX, &mouseY);
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

            bool leftPressed = glfwGetMouseButton(nativeWindow, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;

            glm::vec3 lightPos(20.0f, 50.0f, 20.0f);
            glm::mat4 lightSpaceMatrix = shadowMap.getLightSpaceMatrix(lightPos);

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

            glm::mat4 view = camera.GetViewMatrix();
            glm::mat4 projection = camera.GetProjectionMatrix(static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));

            if (renderer.shader) {
                renderer.shader->use();
                renderer.shader->setVec3("lightPos", lightPos);
            }

            for (const Part& part : parts) {
                if (!part.deleted && !part.isFolder) {
                    renderer.drawPart(part, view, projection, lightSpaceMatrix, shadowMap.depthMap, 0);
                }
            }

            const int gizmoPart = rblx::studio::primarySelection(selection);
            if (gizmoPart >= 0 && gizmoPart < static_cast<int>(parts.size()) &&
                !parts[static_cast<std::size_t>(gizmoPart)].isFolder) {
                Part& part = parts[static_cast<std::size_t>(gizmoPart)];
                glm::mat4 model = partToMatrix(part);
                const glm::mat4 before = model;

                ImGuizmo::SetOrthographic(false);
                const ImGuiViewport* mainViewport = ImGui::GetMainViewport();
                ImGuizmo::SetRect(mainViewport->Pos.x, mainViewport->Pos.y, mainViewport->Size.x, mainViewport->Size.y);

                const float* snapValues = snapEnabled ? snap : nullptr;
                if (ImGuizmo::Manipulate(
                        glm::value_ptr(view),
                        glm::value_ptr(projection),
                        operation,
                        mode,
                        glm::value_ptr(model),
                        nullptr,
                        snapValues)) {
                    // ImGuizmo returns the full model matrix. Assign the decomposed size directly;
                    // multiplying by the returned scale each frame causes runaway scale growth.
                    //
                    // The rest of the selection moves by what the gizmo did to the
                    // part it drew: the step is that matrix against the one it
                    // started from, applied to each of them, so a group turns about
                    // the primary and keeps its shape.
                    const glm::mat4 delta = model * glm::inverse(before);
                    for (const int index : selection) {
                        if (index < 0 || index >= static_cast<int>(parts.size())) {
                            continue;
                        }
                        Part& target = parts[static_cast<std::size_t>(index)];
                        if (target.deleted) {
                            continue;
                        }
                        if (index == gizmoPart) {
                            applyMatrixToPart(model, target, physicsWorld, operation == ImGuizmo::SCALE);
                        } else {
                            applyMatrixToPart(delta * partToMatrix(target), target, physicsWorld,
                                              operation == ImGuizmo::SCALE);
                        }
                    }
                }
            }

            if (leftPressed && !wasLeftPressed && !io.WantCaptureMouse && !ImGuizmo::IsUsing() && !ImGuizmo::IsOver()) {
                double mouseX = 0.0;
                double mouseY = 0.0;
                glfwGetCursorPos(nativeWindow, &mouseX, &mouseY);
                int picked = Raycaster::GetPartFromMouse(parts, camera, static_cast<float>(mouseX), static_cast<float>(mouseY), static_cast<float>(window.getWidth()), static_cast<float>(window.getHeight()));
                if (picked >= 0 && picked < static_cast<int>(parts.size())) {
                    if (io.KeyCtrl) {
                        rblx::studio::toggleSelected(selection, picked);
                    } else {
                        rblx::studio::selectOnly(selection, picked);
                    }
                } else if (!io.KeyCtrl) {
                    // A click on nothing lets go of the whole selection, which is
                    // the only way to pick a single part again after a group.
                    selection.clear();
                }
            }
            wasLeftPressed = leftPressed;

            // One step is one finished interaction: IsAnyItemActive covers a slider
            // being dragged and a name being typed, and the gizmo is not an ImGui
            // item, so it is asked separately.
            const bool interacting = ImGui::IsAnyItemActive() || ImGuizmo::IsUsing();
            history.note(rblx::studio::captureWorld(parts), interacting);

            editor.render();
            window.swapBuffers();
        }

        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Studio failed: " << error.what() << std::endl;
        return 1;
    }
}
