#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <commdlg.h>
#endif

#include "SceneManager.h"
#include "imgui.h"
#include "io/Screen.h"
#include <iostream>
#include <algorithm>
#include <fstream>
#include <cmath>

// ==========================================
// SceneEntity Implementation
// ==========================================

SceneEntity::SceneEntity(int id, const std::string& name, EntityType type)
    : id(id), name(name), type(type)
{
}

void SceneEntity::calculateBounds()
{
    if (model && !model->meshes.empty()) {
        glm::vec3 minBound(1e9f);
        glm::vec3 maxBound(-1e9f);
        bool hasVerts = false;

        for (const auto& mesh : model->meshes) {
            for (const auto& v : mesh.vertices) {
                minBound.x = (std::min)(minBound.x, v.pos.x);
                minBound.y = (std::min)(minBound.y, v.pos.y);
                minBound.z = (std::min)(minBound.z, v.pos.z);
                maxBound.x = (std::max)(maxBound.x, v.pos.x);
                maxBound.y = (std::max)(maxBound.y, v.pos.y);
                maxBound.z = (std::max)(maxBound.z, v.pos.z);
                hasVerts = true;
            }
        }

        if (hasVerts) {
            localCenter = (minBound + maxBound) * 0.5f;
            localRadius = glm::length((maxBound - minBound) * 0.5f);
            if (localRadius < 0.1f) localRadius = 0.5f;
            return;
        }
    }

    localCenter = glm::vec3(0.0f);
    localRadius = (type == EntityType::PROCEDURAL_SPHERE) ? 1.0f : 0.8f;
}

glm::vec3 SceneEntity::getWorldCenter() const
{
    return pos + localCenter * scale;
}

float SceneEntity::getWorldRadius() const
{
    float maxS = (std::max)({ std::abs(scale.x), std::abs(scale.y), std::abs(scale.z) });
    return (std::max)(localRadius * maxS, 0.3f);
}

glm::mat4 SceneEntity::getModelMatrix() const
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, pos);
    if (rotation.x != 0.0f) m = glm::rotate(m, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
    if (rotation.y != 0.0f) m = glm::rotate(m, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
    if (rotation.z != 0.0f) m = glm::rotate(m, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
    m = glm::scale(m, scale);
    return m;
}

void SceneEntity::render(Shader& shader, float dt, bool isSelected)
{
    if (!visible) return;

    glm::mat4 modelM = getModelMatrix();
    shader.setMat4("model", modelM);

    if (model) {
        model->pos = pos;
        model->size = scale;
        model->updateBoundingRegion();

        // 1. Normal Solid Render
        shader.setInt("useDebugColor", 0);
        model->render(shader, dt, false, false);

        // 2. Selection Wireframe Highlight
        if (isSelected) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(2.5f);
            shader.setInt("useDebugColor", 1);
            shader.set3Float("debugColor", glm::vec3(1.0f, 0.75f, 0.1f)); // Golden selection glow
            model->render(shader, dt, false, false);
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            shader.setInt("useDebugColor", 0);
        }
    }
}

std::shared_ptr<SceneEntity> SceneEntity::clone(int newId) const
{
    auto copy = std::make_shared<SceneEntity>(newId, name + " (Copy)", type);
    copy->pos = pos + glm::vec3(1.0f, 0.0f, 0.0f); // Offset along X
    copy->rotation = rotation;
    copy->scale = scale;
    copy->color = color;
    copy->visible = visible;
    copy->filePath = filePath;
    copy->model = model; // Shares mesh data safely
    copy->localCenter = localCenter;
    copy->localRadius = localRadius;
    return copy;
}

// ==========================================
// SceneManager Implementation
// ==========================================

static std::string openNativeFileDialog()
{
#ifdef _WIN32
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn;
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = "3D Models (*.gltf;*.glb;*.obj;*.fbx;*.dae;*.stl)\0*.gltf;*.glb;*.obj;*.fbx;*.dae;*.stl\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = "gltf";
    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
#endif
    return "";
}

SceneManager::SceneManager()
{
    try {
        if (std::filesystem::exists("assets/Models")) {
            currentBrowserPath = "assets/Models";
        } else if (std::filesystem::exists("OpenGlt/assets/Models")) {
            currentBrowserPath = "OpenGlt/assets/Models";
        } else {
            currentBrowserPath = ".";
        }
    } catch (...) {
        currentBrowserPath = ".";
    }
}

void SceneManager::init(Camera& camera)
{
    // Add default initial entities
    addCube(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f));
    addSphere(glm::vec3(2.5f, 0.0f, -1.0f), glm::vec3(1.0f));

    // Try loading bundled models if available
    std::vector<std::string> testModels = {
        "assets/Models/mk18/scene.gltf",
        "OpenGlt/assets/Models/mk18/scene.gltf",
        "../OpenGlt/assets/Models/mk18/scene.gltf"
    };

    for (const auto& path : testModels) {
        if (std::filesystem::exists(path)) {
            auto modelEnt = addModel(path, camera, false);
            if (modelEnt) {
                modelEnt->pos = glm::vec3(-2.5f, 0.0f, -1.0f);
                modelEnt->scale = glm::vec3(0.5f);
            }
            break;
        }
    }

    if (!entities.empty()) {
        selectedEntity = entities[0];
    }
}

std::shared_ptr<SceneEntity> SceneManager::addCube(glm::vec3 pos, glm::vec3 size)
{
    auto cubeModel = std::make_shared<Cube>(Material::gold, glm::vec3(0.0f), glm::vec3(1.0f));
    cubeModel->init();

    auto entity = std::make_shared<SceneEntity>(nextEntityId++, "Gold Cube " + std::to_string(nextEntityId - 1), EntityType::PROCEDURAL_CUBE);
    entity->pos = pos;
    entity->scale = size;
    entity->model = cubeModel;
    entity->calculateBounds();

    entities.push_back(entity);
    selectedEntity = entity;
    setStatus("Added Procedural Cube");
    return entity;
}

std::shared_ptr<SceneEntity> SceneManager::addSphere(glm::vec3 pos, glm::vec3 size)
{
    auto sphereModel = std::make_shared<Sphere>(glm::vec3(0.0f), glm::vec3(1.0f), false);
    
    // Look for sphere model assets
    std::vector<std::string> spherePaths = {
        "assets/Models/sphere/scene.gltf",
        "OpenGlt/assets/Models/sphere/scene.gltf",
        "../OpenGlt/assets/Models/sphere/scene.gltf"
    };
    for (const auto& sp : spherePaths) {
        if (std::filesystem::exists(sp)) {
            sphereModel->loadModel(sp);
            break;
        }
    }

    auto entity = std::make_shared<SceneEntity>(nextEntityId++, "Sphere " + std::to_string(nextEntityId - 1), EntityType::PROCEDURAL_SPHERE);
    entity->pos = pos;
    entity->scale = size;
    entity->model = sphereModel;
    entity->calculateBounds();

    entities.push_back(entity);
    selectedEntity = entity;
    setStatus("Added Sphere Model");
    return entity;
}

std::shared_ptr<SceneEntity> SceneManager::addModel(const std::string& path, Camera& camera, bool focus)
{
    setStatus("Loading model: " + path + "...");
    std::cout << "[SceneManager] Loading model from: " << path << std::endl;

    auto model = std::make_shared<Model>(glm::vec3(0.0f), glm::vec3(1.0f), false);
    model->loadModel(path);

    if (model->meshes.empty()) {
        setStatus("ERROR: No meshes found in " + path);
        std::cerr << "[SceneManager] Failed to load meshes for: " << path << std::endl;
        return nullptr;
    }

    std::filesystem::path fsPath(path);
    std::string stemName = fsPath.stem().string();
    if (stemName == "scene" || stemName == "model") {
        stemName = fsPath.parent_path().filename().string();
    }
    if (stemName.empty()) stemName = "Model";

    auto entity = std::make_shared<SceneEntity>(nextEntityId++, stemName + " " + std::to_string(nextEntityId - 1), EntityType::MODEL_FILE);
    entity->filePath = path;
    entity->model = model;
    entity->calculateBounds();

    // Position directly in front of camera if not focusing camera on it
    if (!focus) {
        entity->pos = camera.cameraPos + camera.cameraFront * 3.5f;
    } else {
        entity->pos = glm::vec3(0.0f, 0.0f, 0.0f);
    }

    entities.push_back(entity);
    selectedEntity = entity;

    if (focus) {
        focusCameraOn(entity, camera);
    }

    setStatus("Loaded: " + entity->name);
    return entity;
}

std::shared_ptr<SceneEntity> SceneManager::duplicateSelected()
{
    if (!selectedEntity) {
        setStatus("No object selected to duplicate");
        return nullptr;
    }

    auto copy = selectedEntity->clone(nextEntityId++);
    entities.push_back(copy);
    selectedEntity = copy;
    setStatus("Duplicated: " + copy->name + " (Shift+D)");
    return copy;
}

bool SceneManager::deleteSelected()
{
    if (!selectedEntity) {
        setStatus("No object selected to delete");
        return false;
    }

    std::string deletedName = selectedEntity->name;
    auto it = std::find(entities.begin(), entities.end(), selectedEntity);
    if (it != entities.end()) {
        entities.erase(it);
        selectedEntity = entities.empty() ? nullptr : entities.back();
        setStatus("Deleted: " + deletedName);
        return true;
    }
    return false;
}

void SceneManager::focusCameraOn(std::shared_ptr<SceneEntity> entity, Camera& camera)
{
    if (!entity) return;

    glm::vec3 center = entity->getWorldCenter();
    float radius = entity->getWorldRadius();
    if (radius <= 0.05f) radius = 1.0f;

    float distance = radius * 2.8f + 1.2f;
    camera.focusOn(center, distance);
    setStatus("Focused camera on: " + entity->name);
}

void SceneManager::queueDropFile(const std::string& path)
{
    pendingDropFiles.push_back(path);
}

void SceneManager::processQueuedFiles(Camera& camera)
{
    if (pendingDropFiles.empty()) return;

    for (const auto& path : pendingDropFiles) {
        addModel(path, camera, true);
    }
    pendingDropFiles.clear();
}

bool SceneManager::selectByScreenPos(double mouseX, double mouseY, unsigned int width, unsigned int height, const Camera& camera, const glm::mat4& projection)
{
    if (width == 0 || height == 0 || entities.empty()) return false;

    // Convert mouse coordinates to Normalized Device Coordinates (NDC)
    float x = (2.0f * (float)mouseX) / (float)width - 1.0f;
    float y = 1.0f - (2.0f * (float)mouseY) / (float)height;

    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    glm::vec4 rayEye = glm::inverse(projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::mat4 view = const_cast<Camera&>(camera).getViewMatrix();
    glm::vec3 rayWorld = glm::vec3(glm::inverse(view) * rayEye);
    rayWorld = glm::normalize(rayWorld);

    glm::vec3 rayOrigin = camera.cameraPos;

    std::shared_ptr<SceneEntity> closestHit = nullptr;
    float closestT = 1e9f;

    for (const auto& entity : entities) {
        if (!entity || !entity->visible) continue;

        glm::vec3 center = entity->getWorldCenter();
        float radius = entity->getWorldRadius();

        glm::vec3 oc = rayOrigin - center;
        float b = glm::dot(oc, rayWorld);
        float c = glm::dot(oc, oc) - radius * radius;
        float discriminant = b * b - c;

        if (discriminant >= 0.0f) {
            float t = -b - std::sqrt(discriminant);
            if (t > 0.0f && t < closestT) {
                closestT = t;
                closestHit = entity;
            }
        } else {
            // Distance of closest approach for friendly picking
            float tClosest = -glm::dot(oc, rayWorld);
            if (tClosest > 0.0f && tClosest < closestT) {
                glm::vec3 pClosest = rayOrigin + rayWorld * tClosest;
                float dist = glm::distance(pClosest, center);
                if (dist <= radius * 1.25f) {
                    closestT = tClosest;
                    closestHit = entity;
                }
            }
        }
    }

    if (closestHit) {
        selectedEntity = closestHit;
        setStatus("Selected: " + closestHit->name);
        return true;
    }

    return false;
}

void SceneManager::setStatus(const std::string& msg)
{
    statusMessage = msg;
    statusTimer = 4.0f;
}

void SceneManager::update(float dt, Camera& camera, GLFWwindow* window, bool isCameraLookMode)
{
    processQueuedFiles(camera);

    if (statusTimer > 0.0f) {
        statusTimer -= dt;
    }

    // Interactive Grab / Move mode
    if (isGrabMode && selectedEntity) {
        double currentMouseX, currentMouseY;
        glfwGetCursorPos(window, &currentMouseX, &currentMouseY);

        double dx = currentMouseX - lastMouseX;
        double dy = currentMouseY - lastMouseY;

        float dist = glm::distance(camera.cameraPos, selectedEntity->getWorldCenter());
        float factor = dist * 0.0018f;

        selectedEntity->pos += camera.cameraRight * (float)dx * factor - camera.cameraUp * (float)dy * factor;

        lastMouseX = currentMouseX;
        lastMouseY = currentMouseY;

        // Confirm on left click, cancel on right click or Esc
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsKeyPressed(ImGuiKey_Enter)) {
            isGrabMode = false;
            setStatus("Moved: " + selectedEntity->name);
        } else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right) || ImGui::IsKeyPressed(ImGuiKey_Escape)) {
            selectedEntity->pos = grabStartPos;
            isGrabMode = false;
            setStatus("Move cancelled");
        }
    }
}

void SceneManager::render(Shader& shader, float dt)
{
    for (auto& entity : entities) {
        if (entity) {
            bool isSel = (entity == selectedEntity);
            entity->render(shader, dt, isSel);
        }
    }
}

// ==========================================
// ImGui UI Panels
// ==========================================

void SceneManager::renderUI(Camera& camera, GLFWwindow* window, bool& cameraLookMode)
{
    // Apply modern clean theme styling
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.11f, 0.12f, 0.15f, 0.94f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.24f, 0.32f, 0.44f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.32f, 0.45f, 0.62f, 0.9f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.38f, 0.52f, 0.72f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.20f, 0.28f, 0.38f, 0.85f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.28f, 0.40f, 0.55f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.35f, 0.50f, 0.70f, 1.0f));

    renderHierarchyWindow(camera);
    renderInspectorWindow(camera);
    renderFileBrowserWindow(camera);
    renderHelpOverlay(cameraLookMode);

    ImGui::PopStyleColor(6);
    ImGui::PopStyleVar(2);
}

void SceneManager::renderHierarchyWindow(Camera& camera)
{
    ImGui::SetNextWindowSize(ImVec2(320, 360), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(15, 45), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Scene Hierarchy", nullptr)) {
        ImGui::TextDisabled("Entities: %zu", entities.size());
        ImGui::Separator();

        // Action Toolbar
        if (ImGui::Button("+ Cube")) {
            addCube();
        }
        ImGui::SameLine();
        if (ImGui::Button("+ Sphere")) {
            addSphere();
        }
        ImGui::SameLine();
        if (ImGui::Button("Duplicate (Shift+D)")) {
            duplicateSelected();
        }
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.65f, 0.18f, 0.18f, 0.85f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.85f, 0.22f, 0.22f, 1.0f));
        if (ImGui::Button("Delete (Del)")) {
            deleteSelected();
        }
        ImGui::PopStyleColor(2);

        ImGui::Separator();

        // Hierarchy List
        ImGui::BeginChild("EntityList", ImVec2(0, 0), true);
        for (size_t i = 0; i < entities.size(); ++i) {
            auto& entity = entities[i];
            if (!entity) continue;

            std::string label;
            switch (entity->type) {
                case EntityType::MODEL_FILE:        label = "[3D] " + entity->name; break;
                case EntityType::PROCEDURAL_CUBE:   label = "[Cube] " + entity->name; break;
                case EntityType::PROCEDURAL_SPHERE: label = "[Sphere] " + entity->name; break;
                default:                            label = "[Obj] " + entity->name; break;
            }

            bool isSelected = (entity == selectedEntity);
            if (ImGui::Selectable(label.c_str(), isSelected)) {
                selectedEntity = entity;
                setStatus("Selected: " + entity->name);
            }

            // Right click context menu
            if (ImGui::BeginPopupContextItem()) {
                if (ImGui::MenuItem("Focus Camera (F)")) {
                    focusCameraOn(entity, camera);
                }
                if (ImGui::MenuItem("Duplicate (Shift+D)")) {
                    selectedEntity = entity;
                    duplicateSelected();
                }
                if (ImGui::MenuItem("Delete (Del)")) {
                    selectedEntity = entity;
                    deleteSelected();
                }
                ImGui::EndPopup();
            }
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void SceneManager::renderInspectorWindow(Camera& camera)
{
    ImGui::SetNextWindowSize(ImVec2(320, 260), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(15, 415), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("Object Inspector", nullptr)) {
        if (!selectedEntity) {
            ImGui::TextDisabled("No object selected.");
            ImGui::TextWrapped("Click any 3D object in the viewport or hierarchy to select and modify it.");
        } else {
            char nameBuf[128];
            strncpy_s(nameBuf, selectedEntity->name.c_str(), sizeof(nameBuf));
            if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf))) {
                selectedEntity->name = nameBuf;
            }

            ImGui::Checkbox("Visible", &selectedEntity->visible);
            ImGui::Separator();

            ImGui::TextColored(ImVec4(0.4f, 0.8f, 1.0f, 1.0f), "Transform:");
            ImGui::DragFloat3("Position (XYZ)", &selectedEntity->pos.x, 0.05f);
            ImGui::DragFloat3("Rotation (Deg)", &selectedEntity->rotation.x, 1.0f, -360.0f, 360.0f);
            ImGui::DragFloat3("Scale", &selectedEntity->scale.x, 0.05f, 0.01f, 50.0f);

            ImGui::Separator();
            if (ImGui::Button("Focus Camera (F)")) {
                focusCameraOn(selectedEntity, camera);
            }
            ImGui::SameLine();
            if (ImGui::Button("Move With Mouse (G)")) {
                isGrabMode = true;
                grabStartPos = selectedEntity->pos;
                setStatus("Moving: " + selectedEntity->name + " (Click to place, Esc to cancel)");
            }

            if (!selectedEntity->filePath.empty()) {
                ImGui::Separator();
                ImGui::TextDisabled("Source: %s", selectedEntity->filePath.c_str());
            }
            if (selectedEntity->model) {
                ImGui::TextDisabled("Meshes: %zu", selectedEntity->model->meshes.size());
            }
        }
    }
    ImGui::End();
}

void SceneManager::renderFileBrowserWindow(Camera& camera)
{
    ImGui::SetNextWindowSize(ImVec2(380.0f, 420.0f), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2((float)Screen::SCR_WIDTH - 395.0f, 45.0f), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("File System / Assets", nullptr)) {
        // Drag & Drop Hint Banner
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.16f, 0.22f, 0.30f, 0.9f));
        ImGui::BeginChild("DropHint", ImVec2(0, 50), true);
        ImGui::TextColored(ImVec4(0.3f, 0.9f, 0.5f, 1.0f), "DRAG & DROP 3D FILES ANYWHERE");
        ImGui::TextDisabled(".gltf, .glb, .obj, .fbx, .stl -> Auto-loads & focuses camera!");
        ImGui::EndChild();
        ImGui::PopStyleColor();

        ImGui::Separator();

        // Native File Dialog Button
        if (ImGui::Button("Browse Computer (File Dialog)...")) {
            std::string selectedFile = openNativeFileDialog();
            if (!selectedFile.empty()) {
                addModel(selectedFile, camera, true);
            }
        }

        ImGui::Separator();
        ImGui::Text("Directory: %s", currentBrowserPath.c_str());

        // Quick path shortcuts
        if (ImGui::SmallButton("assets/Models")) {
            if (std::filesystem::exists("assets/Models")) currentBrowserPath = "assets/Models";
            else if (std::filesystem::exists("OpenGlt/assets/Models")) currentBrowserPath = "OpenGlt/assets/Models";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton("Project Root")) {
            currentBrowserPath = ".";
        }
        ImGui::SameLine();
        if (ImGui::SmallButton(".. (Up)")) {
            try {
                std::filesystem::path p(currentBrowserPath);
                if (p.has_parent_path()) currentBrowserPath = p.parent_path().string();
                if (currentBrowserPath.empty()) currentBrowserPath = ".";
            } catch (...) {}
        }

        ImGui::Separator();

        // Directory contents listing
        ImGui::BeginChild("DirList", ImVec2(0, 0), true);
        try {
            if (std::filesystem::exists(currentBrowserPath)) {
                for (const auto& entry : std::filesystem::directory_iterator(currentBrowserPath)) {
                    std::string filename = entry.path().filename().string();
                    if (entry.is_directory()) {
                        if (ImGui::Selectable(("[DIR] " + filename).c_str())) {
                            currentBrowserPath = entry.path().string();
                        }
                    } else {
                        std::string ext = entry.path().extension().string();
                        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
                        bool is3D = (ext == ".gltf" || ext == ".glb" || ext == ".obj" || ext == ".fbx" || ext == ".stl" || ext == ".dae");

                        if (is3D) {
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 0.9f, 1.0f, 1.0f));
                            if (ImGui::Button(("Load##" + filename).c_str())) {
                                addModel(entry.path().string(), camera, true);
                            }
                            ImGui::SameLine();
                            if (ImGui::Selectable(("[3D] " + filename).c_str())) {
                                addModel(entry.path().string(), camera, true);
                            }
                            ImGui::PopStyleColor();
                        } else {
                            ImGui::TextDisabled("     %s", filename.c_str());
                        }
                    }
                }
            }
        } catch (const std::exception& e) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Error reading directory: %s", e.what());
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

void SceneManager::renderHelpOverlay(bool& cameraLookMode)
{
    // Status Bar & Controls at bottom
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
    ImGui::SetNextWindowPos(ImVec2(15.0f, (float)Screen::SCR_HEIGHT - 65.0f));
    ImGui::SetNextWindowBgAlpha(0.75f);

    if (ImGui::Begin("StatusOverlay", nullptr, flags)) {
        ImGui::TextColored(cameraLookMode ? ImVec4(1.0f, 0.8f, 0.2f, 1.0f) : ImVec4(0.3f, 0.9f, 0.5f, 1.0f),
            "Mode: %s (Hold Right-Click or TAB to toggle)",
            cameraLookMode ? "CAMERA FLY LOOK (WASD)" : "EDITOR / SELECTION (Click to select/drag)");

        ImGui::SameLine();
        ImGui::Text(" | ");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(0.9f, 0.9f, 0.9f, 1.0f), "%s", statusMessage.c_str());

        ImGui::TextDisabled("Hotkeys: Left-Click: Select | Shift+D: Duplicate | Del: Delete | G: Grab/Move | F: Focus Camera | H: Launch Sphere");
    }
    ImGui::End();
}
