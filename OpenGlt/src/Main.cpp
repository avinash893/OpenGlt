#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "graphics/texture.h"
#include "graphics/Shader.h"
#include <stb/stb_image.h>
#include "graphics/light.h"
#include "graphics/models/sphere.hpp"

// Inputs
#include "io/keyboard.h"
#include "io/Mouse.h"
#include "io/joystick.h"
#include "io/Camera.h"
#include "io/Screen.h"

// Graphics & Physics
#include "graphics/models/cube.hpp"
#include "graphics/models/lamp.hpp"
#include "graphics/model.h"
#include "graphics/models/gun.hpp"
#include "physics/rigidbody.h"
#include "physics/environment.h"

// ImGui & Scene Management
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "SceneManager.h"

// Global variables
float mixval = 0.5f;
Screen screen;
Camera defaultCamera(glm::vec3(0.0f, 2.0f, 6.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float x = 0.0f, y = 0.0f, z = 0.0f;

bool spotlightEnabled = true;
bool showBoundingRegions = false;
bool cameraLookMode = false; // False = Editor mode (mouse free), True = Fly look mode

SphereArray spheres; // Array to hold launched physics spheres

// Function declarations
void launchItem(float deltaTime);
void processInput(Joystick& joystick, float dt, const glm::mat4& projection);
void drop_callback(GLFWwindow* window, int count, const char** paths);

static std::string resolveShaderPath(const std::string& path)
{
    std::vector<std::string> candidates = {
        path,
        "assets/" + path,
        "OpenGlt/" + path,
        "OpenGlt/assets/" + path,
        "../" + path,
        "../OpenGlt/" + path,
        "../../OpenGlt/" + path,
        "C:/Users/avina/Desktop/Life/OpenGlt/OpenGlt/" + path
    };
    for (const auto& c : candidates) {
        if (std::filesystem::exists(c)) {
            return c;
        }
    }
    return path;
}

static void setWorkingDirectoryToExecutable()
{
#ifdef _WIN32
    char modulePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
    {
        for (int i = static_cast<int>(len) - 1; i >= 0; --i)
        {
            if (modulePath[i] == '\\' || modulePath[i] == '/')
            {
                modulePath[i] = '\0';
                break;
            }
        }
        SetCurrentDirectoryA(modulePath);
    }
#endif
}

void drop_callback(GLFWwindow* /*window*/, int count, const char** paths)
{
    for (int i = 0; i < count; i++) {
        if (paths && paths[i]) {
            SceneManager::getInstance().queueDropFile(paths[i]);
        }
    }
}

int main()
{
    setWorkingDirectoryToExecutable();
    char cwd[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, cwd);
    std::cout << "[LOG] Current working directory: " << cwd << std::endl;

    std::cout << "[LOG] Starting GLFW initialization" << std::endl;
    if (!glfwInit()) {
        std::cerr << "[ERROR] glfwInit() failed" << std::endl;
        return -1;
    }
    std::cout << "[LOG] glfwInit() succeeded" << std::endl;

    // GLFW window setup
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    std::cout << "[LOG] Creating screen/window" << std::endl;
    if (!screen.init())
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    std::cout << "[LOG] screen.init() succeeded" << std::endl;

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "[ERROR] Failed to initialize GLAD\n";
        return -1;
    }
    std::cout << "[LOG] GLAD initialized" << std::endl;

    screen.setParameters();
    screen.setCursorMode(GLFW_CURSOR_NORMAL); // Start in editor mode
    glEnable(GL_DEPTH_TEST);

    // Register Drag and Drop callback
    glfwSetDropCallback(screen.getWindow(), drop_callback);

    // Initialize Dear ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(screen.getWindow(), true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Initialize Joystick
    Joystick joystick(0);
    joystick.update();
    if (joystick.isPresent())
        std::cout << joystick.getName() << " joystick is present" << std::endl;

    // Setup Shaders
    std::cout << "[LOG] Loading shaders" << std::endl;
    std::string vShader = resolveShaderPath("assets/vertexShader.glsl");
    std::string fShader = resolveShaderPath("assets/fragmentShader.glsl");
    std::string instShader = resolveShaderPath("assets/instance/instance.glsl");
    std::string lampFShader = resolveShaderPath("assets/lampFragmentShader.glsl");

    Shader shader(vShader.c_str(), fShader.c_str());
    Shader lampShader(instShader.c_str(), lampFShader.c_str());
    std::cout << "[LOG] Shaders loaded" << std::endl;

    // Procedural triangle vertex data
    float triangleVertices[] = {
        // positions          // normals           // texcoords
        -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.5f, 1.0f
    };

    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Textures for triangle
    GLuint whiteTexture, specularTexture;
    glGenTextures(1, &whiteTexture);
    glBindTexture(GL_TEXTURE_2D, whiteTexture);
    unsigned char whitePixel[] = { 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &specularTexture);
    glBindTexture(GL_TEXTURE_2D, specularTexture);
    unsigned char specularPixel[] = { 32, 32, 32 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, specularPixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Lighting setup
    SpotLight s = {
        defaultCamera.cameraPos,
        defaultCamera.cameraFront,
        glm::cos(glm::radians(7.5f)),
        glm::cos(glm::radians(12.5f)),
        1.0f, 0.07f, 0.03f,
        glm::vec3(0.0f),
        glm::vec3(1.0f),
        glm::vec3(1.0f)
    };

    DirLight dirLight = {
        glm::vec3(-0.2f, -1.0f, -0.3f),
        glm::vec3(0.2f),
        glm::vec3(0.7f),
        glm::vec3(0.8f)
    };

    Lamp lamp(
        glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f), glm::vec3(1.0f),
        1.0f, 0.09f, 0.032f,
        glm::vec3(-1.0f, 2.0f, -0.5f), glm::vec3(0.25f)
    );
    lamp.init();

    // Initialize Scene Manager with models
    SceneManager::getInstance().init(defaultCamera);

    // Main render loop
    std::cout << "[LOG] Entering main render loop" << std::endl;
    while (!screen.shouldclose())
    {
        double currentTime = glfwGetTime();
        deltaTime = (float)(currentTime - lastFrame);
        lastFrame = (float)currentTime;

        glm::mat4 view = defaultCamera.getViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(defaultCamera.getZoom()),
            (float)Screen::SCR_WIDTH / (float)Screen::SCR_HEIGHT,
            0.1f,
            100.0f
        );

        processInput(joystick, deltaTime, projection);

        screen.update();

        // 3D Scene Rendering
        shader.activate();
        shader.setInt("useDebugColor", 0);
        shader.set3Float("viewPos", defaultCamera.cameraPos);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setInt("useInstancing", 0);

        // Lighting uniforms
        dirLight.render(shader);

        if (spotlightEnabled)
        {
            s.position = defaultCamera.cameraPos;
            s.direction = defaultCamera.cameraFront;
            shader.setInt("noSpotLights", 1);
            s.render(shader, 0);
        }
        else {
            shader.setInt("noSpotLights", 0);
        }

        // Point light lamp
        lamp.pointLight.render(shader, 0);
        shader.setInt("noPointLights", 1);

        // Render reference triangle
        glm::mat4 triModel = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.8f, -2.0f));
        shader.setMat4("model", triModel);
        shader.set3Float("material.ambient", glm::vec3(0.2f, 0.2f, 0.2f));
        shader.set3Float("material.diffuse", glm::vec3(0.8f, 0.5f, 0.31f));
        shader.set3Float("material.specular", glm::vec3(0.5f));
        shader.setFloat("material.shininess", 32.0f);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, whiteTexture);
        shader.setInt("diffuse0", 0);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularTexture);
        shader.setInt("specular0", 1);

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glBindVertexArray(0);

        // Update & Render all scene hierarchy objects
        bool activeLook = cameraLookMode || Mouse::button(GLFW_MOUSE_BUTTON_RIGHT);
        SceneManager::getInstance().update(deltaTime, defaultCamera, screen.getWindow(), activeLook);
        SceneManager::getInstance().render(shader, deltaTime);

        // Render launched physics spheres
        for (auto& sp : spheres.instances) {
            sp.rb.update(deltaTime);
            sp.pos = sp.rb.pos;
            sp.render(shader, deltaTime, false, true);
        }

        // Render point lamp visual
        lampShader.activate();
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        lamp.render(lampShader, deltaTime, true, true);

        // Render ImGui GUI Overlay
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        SceneManager::getInstance().renderUI(defaultCamera, screen.getWindow(), cameraLookMode);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        screen.newFrame();
    }

    // Cleanup ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Cleanup OpenGL buffers
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteTextures(1, &whiteTexture);
    glDeleteTextures(1, &specularTexture);
    lamp.cleanup();
    spheres.cleanup();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* /*window*/, int width, int height)
{
    glViewport(0, 0, width, height);
    Screen::SCR_WIDTH = width;
    Screen::SCR_HEIGHT = height;
}

void processInput(Joystick& joystick, float dt, const glm::mat4& projection)
{
    // Mode management: Right mouse button or TAB toggles camera look
    if (Keyboard::keyDown(GLFW_KEY_TAB))
    {
        cameraLookMode = !cameraLookMode;
        Mouse::resetFirstMouse();
    }

    bool rmbHeld = Mouse::button(GLFW_MOUSE_BUTTON_RIGHT);
    bool activeLook = cameraLookMode || rmbHeld;

    if (activeLook)
    {
        screen.setCursorMode(GLFW_CURSOR_DISABLED);
        double dx = Mouse::DX();
        double dy = Mouse::DY();
        if (dx != 0.0 || dy != 0.0)
        {
            defaultCamera.updateCameraDirection(dx, dy);
        }

        // Camera movement only while in look mode or WASD
        if (Keyboard::key(GLFW_KEY_W)) defaultCamera.updateCameraPos(CameraDirection::FORWARD, dt);
        if (Keyboard::key(GLFW_KEY_S)) defaultCamera.updateCameraPos(CameraDirection::BACKWARD, dt);
        if (Keyboard::key(GLFW_KEY_D)) defaultCamera.updateCameraPos(CameraDirection::RIGHT, dt);
        if (Keyboard::key(GLFW_KEY_A)) defaultCamera.updateCameraPos(CameraDirection::LEFT, dt);
        if (Keyboard::key(GLFW_KEY_SPACE)) defaultCamera.updateCameraPos(CameraDirection::UP, dt);
        if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) defaultCamera.updateCameraPos(CameraDirection::DOWN, dt);
    }
    else
    {
        screen.setCursorMode(GLFW_CURSOR_NORMAL);
        // Clear delta so it doesn't accumulate
        Mouse::DX();
        Mouse::DY();

        // Object Raycast Picking via Left-Click in 3D viewport
        if (Mouse::buttonDown(GLFW_MOUSE_BUTTON_LEFT))
        {
            ImGuiIO& io = ImGui::GetIO();
            if (!io.WantCaptureMouse)
            {
                SceneManager::getInstance().selectByScreenPos(
                    Mouse::getX(), Mouse::getY(),
                    Screen::SCR_WIDTH, Screen::SCR_HEIGHT,
                    defaultCamera, projection
                );
            }
        }
    }

    // Scroll zoom
    double scrollY = Mouse::getScrollDY();
    if (scrollY != 0.0)
    {
        ImGuiIO& io = ImGui::GetIO();
        if (!io.WantCaptureMouse)
        {
            defaultCamera.updateCameraZoom(scrollY);
        }
    }

    // Shortcuts: Duplicate (Shift + D)
    bool isShift = (Keyboard::key(GLFW_KEY_LEFT_SHIFT) || Keyboard::key(GLFW_KEY_RIGHT_SHIFT));
    if (isShift && Keyboard::keyDown(GLFW_KEY_D))
    {
        SceneManager::getInstance().duplicateSelected();
    }

    // Shortcut: Delete (Delete or Backspace)
    if (Keyboard::keyDown(GLFW_KEY_DELETE) || Keyboard::keyDown(GLFW_KEY_BACKSPACE))
    {
        SceneManager::getInstance().deleteSelected();
    }

    // Shortcut: Focus camera on selected (F)
    if (Keyboard::keyDown(GLFW_KEY_F) && SceneManager::getInstance().selectedEntity)
    {
        SceneManager::getInstance().focusCameraOn(SceneManager::getInstance().selectedEntity, defaultCamera);
    }

    // Shortcut: Grab/Move object with mouse (G)
    if (Keyboard::keyDown(GLFW_KEY_G) && SceneManager::getInstance().selectedEntity)
    {
        SceneManager::getInstance().isGrabMode = !SceneManager::getInstance().isGrabMode;
        if (SceneManager::getInstance().isGrabMode) {
            SceneManager::getInstance().grabStartPos = SceneManager::getInstance().selectedEntity->pos;
            glfwGetCursorPos(screen.getWindow(), &SceneManager::getInstance().lastMouseX, &SceneManager::getInstance().lastMouseY);
            SceneManager::getInstance().setStatus("Grab Mode Active: Move mouse to translate, Click/Enter to confirm");
        }
    }

    // SpotLight toggle
    if (Keyboard::keyDown(GLFW_KEY_T))
    {
        spotlightEnabled = !spotlightEnabled;
    }

    // Launch physics sphere
    if (Keyboard::keyDown(GLFW_KEY_H))
    {
        launchItem(dt);
    }

    // Quit application
    if (Keyboard::key(GLFW_KEY_ESCAPE) && !SceneManager::getInstance().isGrabMode)
    {
        screen.setShouldClose(true);
    }

    // Joystick support
    joystick.update();
    float lx = joystick.axesState(GLFW_JOYSTICK_AXIS_LEFT_X);
    float ly = -joystick.axesState(GLFW_JOYSTICK_AXIS_LEFT_Y);
    if (std::abs(lx) > 0.05f) x -= lx / 10.0f;
    if (std::abs(ly) > 0.05f) y -= ly / 10.0f;
}

void launchItem(float /*deltaTime*/)
{
    Sphere sphere(defaultCamera.cameraPos, glm::vec3(0.25f));
    sphere.init();
    sphere.rb = RigidBody(1.0f, defaultCamera.cameraPos);
    sphere.rb.transferEnergy(6.0f, defaultCamera.cameraFront);
    sphere.rb.applyAcceleration(Environment::gravity);
    spheres.instances.push_back(sphere);
    SceneManager::getInstance().setStatus("Launched physics projectile sphere!");
}
