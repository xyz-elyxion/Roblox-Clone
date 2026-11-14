#include "Window.h"
#include "Renderer.h"
#include "Camera.h"
#include "Part.h"
#include "Raycaster.h"
#include "Skybox.h"
#include "PhysicsWorld.h"
#include "ShadowMap.h"
#include "UiScale.h"
#include "UiTheme.h"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <deque>
#include <vector>
#include <rblxmath/gtc/type_ptr.hpp>
#include <iostream>
#include <iomanip>

#define RBLXWIN_EXPOSE_NATIVE_WIN32
#include <RBLXWIN/rblxwinnative.h>

#include "rblxui.h"
#include "rblxui_impl_rblxwin.h"
#include "rblxui_impl_opengl3.h"

// Settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// Camera
Camera camera(rblxmath::vec3(0.0f, 5.0f, 10.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
bool rightMousePressed = false; // In Player mode, usually locked unless ESC?

// State
bool isCursorLocked = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// --- Serialization Helpers (Duplicated for now, should be in Utils.h) ---
void loadWorld(const std::string& filename, std::deque<Part>& parts, PhysicsWorld& physicsWorld) {
    std::ifstream in(filename);
    if (!in.is_open()) {
        std::cerr << "Failed to open world file: " << filename << std::endl;
        return;
    }
    
    physicsWorld.reset();
    parts.clear();
    
    size_t count;
    in >> count;
    
    struct TempWeld {
        int partIndex;
        std::vector<int> targets;
    };
    std::vector<TempWeld> tempWelds;
    
    for (size_t i = 0; i < count; i++) {
        int shapeInt;
        in >> shapeInt;
        
        Part p;
        p.shape = (ShapeType)shapeInt;
        
        in >> std::ws;
        if (in.peek() == '"') {
            in >> std::quoted(p.name);
        } else {
            in >> p.name; 
        }
        
        in >> p.parentIndex >> p.isFolder >> p.isCamera >> p.isSpawn;
        
        in >> p.position.x >> p.position.y >> p.position.z
           >> p.size.x >> p.size.y >> p.size.z
           >> p.color.x >> p.color.y >> p.color.z
           >> p.rotation.x >> p.rotation.y >> p.rotation.z
           >> p.transparency >> p.reflectance
           >> p.anchored >> p.canCollide;
           
        size_t weldCount;
        in >> weldCount;
        
        if (weldCount > 0) {
            TempWeld tw;
            tempWelds.push_back({ (int)i, {} });
            for (size_t j=0; j<weldCount; j++) {
                int targetIdx;
                in >> targetIdx;
                tempWelds.back().targets.push_back(targetIdx);
            }
        }
        
        parts.push_back(p);
        if (!p.isFolder) physicsWorld.addPart(&parts.back());
    }
    
    for (const auto& tw : tempWelds) {
        for (int target : tw.targets) {
            if (target >= 0 && target < parts.size()) {
                Weld w;
                w.part1 = &parts[target];
                parts[tw.partIndex].welds.push_back(w);
            }
        }
    }
    
    in.close();
}

// Windows File Dialog
#include <windows.h>
#include <commdlg.h>

std::string openFileDialog(RBLXWINwindow* window) {
    OPENFILENAMEA ofn;
    char szFile[260] = {0};
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = rblxwinGetWin32Window(window);
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "World Files\0*.world\0All Files\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

    if (GetOpenFileNameA(&ofn) == TRUE) {
        return std::string(ofn.lpstrFile);
    }
    return "";
}

void mouse_callback(RBLXWINwindow* window, double xposIn, double yposIn) {
    if (RblxUI::GetIO().WantCaptureMouse) return;
    if (!isCursorLocked) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

#include "Character.h" // Include Character

// Global Character
Character* myCharacter = nullptr;

int main() {
    Window window(SCR_WIDTH, SCR_HEIGHT, "RBLX Clone Player");
    rblxwinSetCursorPosCallback(window.getNativeWindow(), mouse_callback);
    
    // Lock cursor by default
    rblxwinSetInputMode(window.getNativeWindow(), RBLXWIN_CURSOR, RBLXWIN_CURSOR_DISABLED);

    // Setup RblxUI
    RBLXUI_CHECKVERSION();
    RblxUI::CreateContext();
    RblxUIIO& io = RblxUI::GetIO(); (void)io;
    rblx::ui::theme::apply();
    const float uiScale = UiScale::Apply(window.getNativeWindow());
    RblxUI_ImplRblxWin_InitForOpenGL(window.getNativeWindow(), true);
    RblxUI_ImplOpenGL3_Init("#version 330");

    Renderer renderer;
    renderer.init();

    Skybox skybox;
    skybox.init();

    PhysicsWorld physicsWorld;
    physicsWorld.init();
    
    ShadowMap shadowMap;
    shadowMap.init();

    std::deque<Part> parts;
    std::string currentWorldFile = "";
    
    // Load World on Start
    currentWorldFile = openFileDialog(window.getNativeWindow());
    // Reset lastFrame after waiting for file dialog to avoid huge deltaTime
    lastFrame = static_cast<float>(rblxwinGetTime());
    
    if (!currentWorldFile.empty()) {
        loadWorld(currentWorldFile, parts, physicsWorld);
    } else {
        Part baseplate(rblxmath::vec3(0.0f, -2.0f, 0.0f), rblxmath::vec3(100.0f, 1.0f, 100.0f), rblxmath::vec3(0.3f, 0.5f, 0.3f));
        baseplate.name = "Baseplate";
        parts.push_back(baseplate);
        physicsWorld.addPart(&parts.back());
    }
    
    // Helper to reset and setup physics/joints
    auto setupPhysics = [&]() {
        if (myCharacter) { delete myCharacter; myCharacter = nullptr; }
        physicsWorld.reset();
        for (auto& part : parts) {
            if (!part.isFolder) physicsWorld.addPart(&part);
        }
        for (auto& part : parts) {
             for (const auto& weld : part.welds) {
                 if (weld.part1) {
                     physicsWorld.createWeld(&part, weld.part1);
                 }
             }
        }
        // Spawn Location Logic
        Part* spawnPart = nullptr;
        Part* cameraPart = nullptr;
        
        for(auto& p : parts) {
            if (p.isSpawn && !spawnPart) spawnPart = &p;
            if (p.isCamera && !cameraPart) cameraPart = &p;
        }
        
        if (spawnPart) {
             rblxmath::vec3 spawnPos = spawnPart->position + rblxmath::vec3(0, 2, 0);
             myCharacter = new Character(spawnPos, &physicsWorld, &parts);
             return (Part*)nullptr; // Character controls camera
        } else if (cameraPart) {
            camera.Position = cameraPart->position;
            camera.Yaw = cameraPart->rotation.y - 90.0f; 
            camera.Pitch = cameraPart->rotation.x;
            camera.ProcessMouseMovement(0,0);
            return cameraPart;
        }
        return (Part*)nullptr;
    };

    Part* activeCameraPart = setupPhysics();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    while (!window.shouldClose()) {
        float currentFrame = static_cast<float>(rblxwinGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_ESCAPE) == RBLXWIN_PRESS) {
            isCursorLocked = false;
            rblxwinSetInputMode(window.getNativeWindow(), RBLXWIN_CURSOR, RBLXWIN_CURSOR_NORMAL);
        }
        
        // RblxUI Frame
        RblxUI_ImplOpenGL3_NewFrame();
        RblxUI_ImplRblxWin_NewFrame();
        RblxUI::NewFrame();
        
        // Overlay UI
        RblxUI::SetNextWindowPos(RblxVec2(UiScale::Px(10.0f, uiScale), UiScale::Px(10.0f, uiScale)));
        if (RblxUI::Begin("Player Controls", NULL, RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_AlwaysAutoResize | RblxUIWindowFlags_NoSavedSettings | RblxUIWindowFlags_NoFocusOnAppearing | RblxUIWindowFlags_NoNav)) {
            if (RblxUI::Button("Load (Test)")) {
                std::string newFile = openFileDialog(window.getNativeWindow());
                // Reset timer after dialog
                lastFrame = static_cast<float>(rblxwinGetTime());
                
                if (!newFile.empty()) {
                    currentWorldFile = newFile;
                    loadWorld(currentWorldFile, parts, physicsWorld);
                    activeCameraPart = setupPhysics();
                }
            }
            RblxUI::SameLine();
            if (RblxUI::Button("Restart")) {
                if (!currentWorldFile.empty()) {
                    loadWorld(currentWorldFile, parts, physicsWorld);
                    activeCameraPart = setupPhysics();
                } else {
                    activeCameraPart = setupPhysics();
                }
            }
            RblxUI::SameLine();
            if (RblxUI::Button("Stop")) {
                rblxwinSetWindowShouldClose(window.getNativeWindow(), true);
            }
            RblxUI::SameLine();
            if (RblxUI::Button(isCursorLocked ? "Unlock Cursor (ESC)" : "Lock Cursor")) {
                isCursorLocked = !isCursorLocked;
                rblxwinSetInputMode(window.getNativeWindow(), RBLXWIN_CURSOR, isCursorLocked ? RBLXWIN_CURSOR_DISABLED : RBLXWIN_CURSOR_NORMAL);
            }
        }
        RblxUI::End();

        // Health Bar (Copied from main.cpp logic)
        if (myCharacter) {
            RblxUIWindowFlags window_flags = RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_AlwaysAutoResize | RblxUIWindowFlags_NoSavedSettings | RblxUIWindowFlags_NoFocusOnAppearing | RblxUIWindowFlags_NoNav;
            window_flags |= RblxUIWindowFlags_NoMove;
            
            float PAD = UiScale::Px(10.0f, uiScale);
            const RblxUIViewport* viewport = RblxUI::GetMainViewport();
            RblxVec2 work_pos = viewport->WorkPos; 
            RblxVec2 work_size = viewport->WorkSize;
            RblxVec2 window_pos, window_pos_pivot;
            window_pos.x = work_pos.x + work_size.x - PAD;
            window_pos.y = work_pos.y + work_size.y - PAD;
            window_pos_pivot.x = 1.0f;
            window_pos_pivot.y = 1.0f;
            RblxUI::SetNextWindowPos(window_pos, RblxUICond_Always, window_pos_pivot);
            RblxUI::SetNextWindowBgAlpha(0.35f);
            
            if (RblxUI::Begin("HealthOverlay", NULL, window_flags)) {
                RblxUI::Text("Health");
                float health = myCharacter->health;
                float maxHealth = myCharacter->maxHealth;
                
                RblxVec4 color = RblxVec4(0.2f, 0.8f, 0.2f, 1.0f);
                if (health < 30) color = RblxVec4(0.8f, 0.2f, 0.2f, 1.0f);
                
                RblxUI::PushStyleColor(RblxUICol_PlotHistogram, color);
                char buf[32];
                std::snprintf(buf, sizeof(buf), "%.0f / %.0f", health, maxHealth);
                RblxUI::ProgressBar(health / maxHealth, UiScale::Size(200.0f, 20.0f, uiScale), buf);
                RblxUI::PopStyleColor();
            }
            RblxUI::End();
        }

        // Health Bar
        if (myCharacter) {
            // ... existing health bar code ...
        }
        
        // Player List (Leaderboard)
        {
            float PAD = UiScale::Px(10.0f, uiScale);
            const RblxUIViewport* viewport = RblxUI::GetMainViewport();
            RblxVec2 work_pos = viewport->WorkPos; 
            RblxVec2 work_size = viewport->WorkSize;
            RblxVec2 window_pos, window_pos_pivot;
            window_pos.x = work_pos.x + work_size.x - PAD;
            window_pos.y = work_pos.y + PAD; // Top Right
            window_pos_pivot.x = 1.0f;
            window_pos_pivot.y = 0.0f;
            RblxUI::SetNextWindowPos(window_pos, RblxUICond_Always, window_pos_pivot);
            RblxUI::SetNextWindowBgAlpha(0.5f); // Dark translucent
            
            RblxUIWindowFlags window_flags = RblxUIWindowFlags_NoDecoration | RblxUIWindowFlags_AlwaysAutoResize | RblxUIWindowFlags_NoSavedSettings | RblxUIWindowFlags_NoFocusOnAppearing | RblxUIWindowFlags_NoNav | RblxUIWindowFlags_NoMove;
            
            if (RblxUI::Begin("Leaderboard", NULL, window_flags)) {
                RblxUI::Text("Players");
                RblxUI::Separator();
                RblxUI::Text("Player1"); // Placeholder
            }
            RblxUI::End();
        }

        if (isCursorLocked) {
            if (myCharacter) {
                 bool fwd = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_W) == RBLXWIN_PRESS;
                 bool bwd = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_S) == RBLXWIN_PRESS;
                 bool l = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_A) == RBLXWIN_PRESS;
                 bool r = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_D) == RBLXWIN_PRESS;
                 bool j = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_SPACE) == RBLXWIN_PRESS;
                 
                 float zoom = 0.0f;
                 if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_UP) == RBLXWIN_PRESS) zoom = 1.0f;
                 if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_DOWN) == RBLXWIN_PRESS) zoom = -1.0f;
                 
                 myCharacter->processInput(fwd, bwd, l, r, j, camera, zoom, deltaTime);
                 
                 // Camera is updated in update() loop
            }
            // If Camera Part exists, lock position to it
            else if (activeCameraPart) {
                camera.Position = activeCameraPart->position;
            } else {
                // Free cam
                bool isSprinting = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_LEFT_SHIFT) == RBLXWIN_PRESS;
                if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_W) == RBLXWIN_PRESS) camera.ProcessKeyboard(FORWARD, deltaTime, isSprinting);
                if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_S) == RBLXWIN_PRESS) camera.ProcessKeyboard(BACKWARD, deltaTime, isSprinting);
                if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_A) == RBLXWIN_PRESS) camera.ProcessKeyboard(LEFT, deltaTime, isSprinting);
                if (rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_D) == RBLXWIN_PRESS) camera.ProcessKeyboard(RIGHT, deltaTime, isSprinting);
            }
        }

        // Update Character
        if (myCharacter) {
             bool isSprinting = rblxwinGetKey(window.getNativeWindow(), RBLXWIN_KEY_LEFT_SHIFT) == RBLXWIN_PRESS;
             myCharacter->update(deltaTime, &camera, isSprinting);
        }

        // Update Physics
        physicsWorld.update(deltaTime);
        
        // --- Render ---
        rblxmath::vec3 lightPos(20.0f, 50.0f, 20.0f);
        rblxmath::mat4 lightSpaceMatrix = shadowMap.getLightSpaceMatrix(lightPos);
        
        shadowMap.bindForWriting();
        for (const auto& part : parts) {
            // In Player mode, hide camera part?
            // User said "face of the part black showing the fron that camera is what you will see out of"
            // So keep rendering it.
            renderer.drawPartShadow(part, lightSpaceMatrix);
        }
        shadowMap.unbind();
        
        glViewport(0, 0, window.getWidth(), window.getHeight());
        glClearColor(0.53f, 0.81f, 0.92f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        rblxmath::mat4 view = camera.GetViewMatrix();
        rblxmath::mat4 projection = camera.GetProjectionMatrix((float)window.getWidth(), (float)window.getHeight());

        renderer.shader->use();
        renderer.shader->setVec3("lightPos", lightPos);

        glDepthMask(GL_FALSE);
        skybox.draw(view, projection);
        glDepthMask(GL_TRUE);

        for (const auto& part : parts) {
            // Camera part logic handled in shader (isCamera uniform)
            // Renderer::drawPart sets isCamera uniform based on part.isCamera
            renderer.drawPart(part, view, projection, lightSpaceMatrix, shadowMap.depthMap);
        }

        RblxUI::Render();
        RblxUI_ImplOpenGL3_RenderDrawData(RblxUI::GetDrawData());

        window.swapBuffers();
        window.pollEvents();
    }
    
    RblxUI_ImplOpenGL3_Shutdown();
    RblxUI_ImplRblxWin_Shutdown();
    RblxUI::DestroyContext();

    return 0;
}
