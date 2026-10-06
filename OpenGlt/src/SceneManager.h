#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <string>
#include <memory>
#include <filesystem>

#include "graphics/Shader.h"
#include "graphics/model.h"
#include "graphics/models/cube.hpp"
#include "graphics/models/sphere.hpp"
#include "graphics/models/lamp.hpp"
#include "io/Camera.h"

enum class EntityType {
    MODEL_FILE,
    PROCEDURAL_CUBE,
    PROCEDURAL_SPHERE,
    PROCEDURAL_TRIANGLE,
    LIGHT_LAMP
};

class SceneEntity {
public:
    int id;
    std::string name;
    EntityType type;
    glm::vec3 pos{ 0.0f };
    glm::vec3 rotation{ 0.0f }; // Euler angles in degrees
    glm::vec3 scale{ 1.0f };
    glm::vec3 color{ 1.0f };
    bool visible{ true };
    std::string filePath{ "" };

    std::shared_ptr<Model> model{ nullptr };
    glm::vec3 localCenter{ 0.0f };
    float localRadius{ 1.0f };

    SceneEntity(int id, const std::string& name, EntityType type);

    void calculateBounds();
    glm::vec3 getWorldCenter() const;
    float getWorldRadius() const;
    glm::mat4 getModelMatrix() const;
    void render(Shader& shader, float dt, bool isSelected);
    std::shared_ptr<SceneEntity> clone(int newId) const;
};

class SceneManager {
public:
    static SceneManager& getInstance() {
        static SceneManager instance;
        return instance;
    }

    std::vector<std::shared_ptr<SceneEntity>> entities;
    std::shared_ptr<SceneEntity> selectedEntity{ nullptr };
    std::vector<std::string> pendingDropFiles;

    std::string currentBrowserPath{ "." };
    std::string statusMessage{ "Welcome to OpenGlt Editor! Drag and drop 3D files or use the File Browser." };
    float statusTimer{ 5.0f };

    // Interactive Move (Grab / Drag)
    bool isGrabMode{ false };
    glm::vec3 grabStartPos{ 0.0f };
    bool isViewportDragging{ false };
    double lastMouseX{ 0.0 };
    double lastMouseY{ 0.0 };

    SceneManager();

    void init(Camera& camera);
    std::shared_ptr<SceneEntity> addModel(const std::string& path, Camera& camera, bool focus = true);
    std::shared_ptr<SceneEntity> addCube(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(0.5f));
    std::shared_ptr<SceneEntity> addSphere(glm::vec3 pos = glm::vec3(2.0f, 0.0f, -2.0f), glm::vec3 size = glm::vec3(1.0f));
    
    std::shared_ptr<SceneEntity> duplicateSelected();
    bool deleteSelected();
    void focusCameraOn(std::shared_ptr<SceneEntity> entity, Camera& camera);

    void queueDropFile(const std::string& path);
    void processQueuedFiles(Camera& camera);

    bool selectByScreenPos(double mouseX, double mouseY, unsigned int width, unsigned int height, const Camera& camera, const glm::mat4& projection);
    
    void update(float dt, Camera& camera, GLFWwindow* window, bool isCameraLookMode);
    void render(Shader& shader, float dt);
    void renderUI(Camera& camera, GLFWwindow* window, bool& cameraLookMode);

    void setStatus(const std::string& msg);

private:
    int nextEntityId{ 1 };
    void renderHierarchyWindow(Camera& camera);
    void renderFileBrowserWindow(Camera& camera);
    void renderInspectorWindow(Camera& camera);
    void renderHelpOverlay(bool& cameraLookMode);
};

#endif // SCENE_MANAGER_H
