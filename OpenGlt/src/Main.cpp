


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <windows.h>
#endif

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>


#include "graphics/texture.h"
#include "graphics/Shader.h"
#include <stb/stb_image.h>
#include "graphics/light.h"
#include "graphics/models/sphere.hpp"

//inputs
#include "io/keyboard.h"
#include "io/Mouse.h"
#include "io/joystick.h"
#include "io/Camera.h"
#include "io/Screen.h"

//graphics
#include"graphics/models/cube.hpp"
#include "graphics/models/lamp.hpp"
#include "graphics/model.h"
#include "graphics/models/gun.hpp"  
#include "physics/rigidbody.h"  
#include"physics/environment.h"


 #include "assimp/Importer.hpp"
 #include "assimp/scene.h"
 #include "assimp/postprocess.h"

// Global variables
float mixval = 0.5f;
glm::mat4 mouseTransform = glm::mat4(1.0f);
Screen screen;

// Function declarations
void launchItem(float deltaTime);
void processInput(Joystick& joystick, float dt);

unsigned int SCR_WIDTH = 800, SCR_HEIGHT = 600;

// Single default camera instead of array
Camera defaultCamera(glm::vec3(0.0f, 2.0f, 5.0f));

float deltaTime = 0.0f;
float lastFrame = 0.0f; // will be used to calculate deltaTime

float x, y, z;

bool spotlightEnabled = true; // Flag to toggle spotlight
bool showBoundingRegions = false; // Flag to toggle bounding region visualization


SphereArray spheres; // Array to hold multiple spheres




static void setWorkingDirectoryToExecutable()
{
#ifdef _WIN32
    char modulePath[MAX_PATH];
    DWORD len = GetModuleFileNameA(NULL, modulePath, MAX_PATH);
    if (len > 0 && len < MAX_PATH)
    {
        // Find last backslash to isolate directory
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

int main()
{
    setWorkingDirectoryToExecutable();
    // === GLFW and window setup ===
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     
    if (!screen.init())
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    screen.setParameters();
    glEnable(GL_DEPTH_TEST); // Enable depth testing for 3D rendering

    // === Initialize Joystick ===
    Joystick joystick(0);
    joystick.update();
    if (joystick.isPresent())
        std::cout << joystick.getName() << " joystick is present" << std::endl;
    else
        std::cout << "Joystick not present" << std::endl;

    // === Shader setup ===
   // Shader shader("assets/vertexShader.glsl", "assets/fragmentShader.glsl");
    Shader shader("assets/vertexShader.glsl", "assets/fragmentShader.glsl");
	Shader lampShader("assets/instance/instance.glsl", "assets/lampFragmentShader.glsl");
	Shader launchShader("assets/instance/instance.glsl", "assets/fragmentShader.glsl");
    // === Vertex data ===

    
    /*// Create a cube for rendering
	        Cube cube(Material::emerald, glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.75f));
	        cube.init();

            // Create multiple cubes for rendering
            glm::vec3 cubePositions[] = {
                glm::vec3(0.0f,  0.0f,  0.0f),
                glm::vec3(2.0f,  5.0f, -15.0f),
                glm::vec3(-1.5f, -2.2f, -2.5f),
                glm::vec3(-3.8f, -2.0f, -12.3f),
                glm::vec3(2.4f, -0.4f, -3.5f),
                glm::vec3(-1.7f,  3.0f, -7.5f),
                glm::vec3(1.3f, -2.0f, -2.5f),
                glm::vec3(1.5f,  2.0f, -2.5f),
                glm::vec3(1.5f,  0.2f, -1.5f),
                glm::vec3(-1.3f,  1.0f, -1.5f)
            };

    std::vector<Cube> cubes;
    for (unsigned int i = 0; i < 10; i++) {
        cubes.push_back(Cube(Material::gold, cubePositions[i], glm::vec3(1.0f)));
        cubes[i].init();
    }
    */








Model m(glm::vec3(0.0f, 0.0f, -2.0f), glm::vec3(2.0f), false);
    m.loadModel("assets/Models/lotr/scene.gltf"); 
     //Gun g;
     //g.loadModel("assets/Models/mk18/scene.gltf");
	spheres.init(); // Initialize the sphere array
 






  glm::vec3 pointLightPositions[] = {
            glm::vec3(0.7f,  0.2f,  2.0f),
            glm::vec3(2.3f, -3.3f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3(0.0f,  0.0f, -3.0f)
    };
    Lamp lamps[4];
    for (unsigned int i = 0; i < 4; i++) {
        lamps[i] = Lamp(glm::vec3(1.0f),
            glm::vec3(0.05f), glm::vec3(0.8f), glm::vec3(1.0f),
            1.0f, 0.07f, 0.032f,
            pointLightPositions[i], glm::vec3(0.25f));
        lamps[i].init();
    }

    SpotLight s = {

        defaultCamera.cameraPos, // Position
        defaultCamera.cameraFront, // Direction
        glm::cos(glm::radians(5.0f)), // Cutoff angle
        glm::cos(glm::radians(10.0f)), // Outer cutoff angle
		1.0f, 
        0.07f,
        0.03f,

		glm::vec3(0.0f), // Ambient color
		glm::vec3(1.0f), // Diffuse color
		glm::vec3(1.0f), // Specular color
    };

	 DirLight dirLight = { glm::vec3(-0.2f,-1.0f,-0.3f), glm::vec3(0.1f), glm::vec3(0.4f), glm::vec3(0.75f) };

	Lamp lamp(glm::vec3(1.0f),glm::vec3(1.0f),glm::vec3(1.0f),glm::vec3(1.0f),
        1.0f, 0.09f, 0.032f, // Constant, Linear, Quadratic attenuation
        
        glm::vec3(-1.0f,-0.5f,-0.5f),glm::vec3(0.5f));
	    lamp.init(); 
		Sphere staticSphere(glm::vec3(2.0f, 0.0f, -2.0f), glm::vec3(1.0f), false);
        staticSphere.init(); // Use the init method instead of loading directly

  
    // === Main render loop ===
    while (!screen.shouldclose())
    {
        double currentTime = glfwGetTime();
        deltaTime = currentTime - lastFrame;
        lastFrame = currentTime;
        processInput(joystick, deltaTime);

        screen.update();
		shader.activate();
		shader.setInt("useDebugColor", 0);

		shader.set3Float("viewPos", defaultCamera.cameraPos);
		
		// Set up basic lighting so the model is visible
		shader.set3Float("dirLight.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
		shader.set3Float("dirLight.ambient", glm::vec3(0.3f));
		shader.set3Float("dirLight.diffuse", glm::vec3(0.7f));
		shader.set3Float("dirLight.specular", glm::vec3(0.5f));
		
		// Set point lights count to 0
		shader.setInt("noPointLights", 0);
		shader.setInt("noSpotLights", 0);
       
        if (spotlightEnabled)
        {
            s.position = defaultCamera.cameraPos; // Update spotlight position
            s.direction = defaultCamera.cameraFront; // Update spotlight direction
            shader.setInt("noSpotLights", 1); // Update the number of spotlights in the shader
            s.render(shader, 0);
        }
        else {
            shader.setInt("noSpotLights", 0); // Disable spotlight rendering
        }
      
        //lights
         for (int i = 0;i < 4;i++)
        {
            lamps[i].pointLight.render(shader,i);
            shader.setInt("noPointLights", 4); // Update the number of point lights in the shader
        }

		dirLight.render(shader);

        
        
       
        //shader.setFloat("mixval", mixval);
     
        //Create transformation matrix for screen view and projection matrix
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);
      
        float time = glfwGetTime();
 
        glm::mat4 transform = glm::mat4(1.0f);
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(time * 90.0f), glm::vec3(0.0f, 1.0f, 1.0f));

        view = defaultCamera.getViewMatrix(); // Get the view matrix from the camera class
        projection = glm::perspective(
            glm::radians(defaultCamera.getZoom()), 
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 
            0.1f, 
            100.0f
        );

        shader.setMat4("projection", projection); // Update projection matrix
        shader.setMat4("view", view); // Update view matrix
        shader.setInt("useInstancing", 0); // Ensure instancing is off for main models
       
        // Debug: Check if main model has meshes (only once)
        static bool debugPrinted = false;
        if (!debugPrinted && m.meshes.empty()) {
            std::cout << "WARNING: Main model has no meshes!" << std::endl;
            debugPrinted = true;
        }
        
        m.render(shader, deltaTime, false, true);
       /* g.render(shade,);*/
		//g.render(shader);
		shader.setInt("useDebugColor", 1);
		shader.set3Float("debugColor", glm::vec3(1.0f, 0.2f, 0.2f));
		
		// Debug: Check if static sphere has meshes (only once)
		static bool sphereDebugPrinted = false;
		if (!sphereDebugPrinted && staticSphere.meshes.empty()) {
            std::cout << "WARNING: Static sphere has no meshes!" << std::endl;
            sphereDebugPrinted = true;
        }
		
		staticSphere.render(shader, deltaTime, false, true);
		shader.setInt("useDebugColor", 0);

        // Update physics for all launched spheres
        for (auto& sphere : spheres.instances)
        {
            sphere.rb.update(deltaTime);
            sphere.pos = sphere.rb.pos; // Update model position to match physics
            sphere.updateBoundingRegion(); // Update bounding region after position change
        }
        
        // Update bounding regions for static models
        m.updateBoundingRegion(); // Update main model's bounding region
        staticSphere.updateBoundingRegion(); // Update static sphere's bounding region
        
        if (spheres.instances.size() > 0)
        {
            // Debug: Print sphere count only when it changes
            static int lastSphereCount = 0;
            if (spheres.instances.size() != lastSphereCount) {
                std::cout << "Rendering " << spheres.instances.size() << " spheres" << std::endl;
                lastSphereCount = spheres.instances.size();
            }
            
            // Render each sphere individually instead of using instancing
            shader.setInt("useInstancing", 0);
            for (auto& sphere : spheres.instances)
            {
                sphere.render(shader, deltaTime, false, true);
            }
            
            // Check for collisions between spheres and main models
            for (auto& sphere : spheres.instances)
            {
                // Check collision with main LOTR model
                if (sphere.checkCollision(m))
                {
                    std::cout << "Sphere collided with LOTR model!" << std::endl;
                    // You can add collision response here (bounce, stop, etc.)
                }
                
                // Check collision with static sphere
                if (sphere.checkCollision(staticSphere))
                {
                    std::cout << "Sphere collided with static sphere!" << std::endl;
                    // You can add collision response here
                }
            }
            
            // Check for collisions between spheres
            for (size_t i = 0; i < spheres.instances.size(); i++)
            {
                for (size_t j = i + 1; j < spheres.instances.size(); j++)
                {
                    if (spheres.instances[i].checkCollision(spheres.instances[j]))
                    {
                        std::cout << "Sphere " << i << " collided with sphere " << j << "!" << std::endl;
                        // You can add collision response here
                    }
                }
            }
        }
       /* // Draw cubes
        for (int i = 0;i < 10;i++)
        {
            cubes[i].render(shader);
        }
        */

		lampShader.activate();
		lampShader.setMat4("view", view);

		//dirLight.direction = glm::rotate(glm::mat4(1.0f), glm::radians(0.5f), glm::vec3(1.0f, 0.0f, 0.0f)) * glm::vec4(dirLight.direction, 1.0f);

		lampShader.setMat4("projection", projection);

	
        
/*   for (int i = 0;i < 4;i++)
        {
			lamps[i].render(lampShader);
        } */
       

        screen.newFrame();
    }

    glfwTerminate();
    
    // Cleanup

	spheres.cleanup();
    /*g.cleanup();*/
	//g.cleanup();
   staticSphere.cleanup();
    m.cleanup();
/*  for (auto& cube : cubes)
    {
        cube.cleanup();
    }
    for (int i = 0; i < 4; i++)
    {
        lamps[i].cleanup();
    } */
   
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
    SCR_HEIGHT = height;
    SCR_WIDTH = width;
}

void processInput(Joystick& joystick, float dt)
{





    //mouse Inputs

    /*if (Keyboard::key(GLFW_KEY_W))
    {
        mouseTransform = glm::translate(mouseTransform, glm::vec3(0.0f, 0.1f, 0.0f));
    }
    if (Keyboard::key(GLFW_KEY_S))
    {
        mouseTransform = glm::translate(mouseTransform, glm::vec3(0.0f, -0.1f, 0.0f));
    }
    if (Keyboard::key(GLFW_KEY_A))
    {
        mouseTransform = glm::translate(mouseTransform, glm::vec3(-0.1f, 0.0f, 0.0f));
    }
    if (Keyboard::key(GLFW_KEY_D))
    {
        mouseTransform = glm::translate(mouseTransform, glm::vec3(0.1f, 0.0f, 0.0f));
    }*/



    if(Keyboard::keyDown(GLFW_KEY_T))
    {
		spotlightEnabled = !spotlightEnabled; // Toggle spotlight on/off
	}
    
    if(Keyboard::keyDown(GLFW_KEY_B))
    {
		showBoundingRegions = !showBoundingRegions; // Toggle bounding region visualization
		std::cout << "Bounding regions: " << (showBoundingRegions ? "ON" : "OFF") << std::endl;
	}



    if (Keyboard::key(GLFW_KEY_ESCAPE))
        screen.setShouldClose(true);

    if (Keyboard::key(GLFW_KEY_UP))
    {
        mixval += 0.005f;
        if (mixval > 1.0f) mixval = 1.0f;
    }

    if (Keyboard::key(GLFW_KEY_DOWN))
    {
        mixval -= 0.005f;
        if (mixval < 0.0f) mixval = 0.0f;
    }

    joystick.update();

    float lx = joystick.axesState(GLFW_JOYSTICK_AXIS_LEFT_X);
    float ly = -joystick.axesState(GLFW_JOYSTICK_AXIS_LEFT_Y);

    if (std::abs(lx) > 0.05f) x -= lx / 10.0f;
    if (std::abs(ly) > 0.05f) y -= ly / 10.0f;

    if (joystick.buttonState(GLFW_JOYSTICK_BTN_DOWN) == GLFW_PRESS)
        z += 0.1f;
    if (joystick.buttonState(GLFW_JOYSTICK_BTN_RIGHT) == GLFW_PRESS)
        z -= 0.1f;

    if (Keyboard::key(GLFW_KEY_W)) defaultCamera.updateCameraPos(CameraDirection::FORWARD, dt);
    if (Keyboard::key(GLFW_KEY_S)) defaultCamera.updateCameraPos(CameraDirection::BACKWARD, dt);
    if (Keyboard::key(GLFW_KEY_D)) defaultCamera.updateCameraPos(CameraDirection::RIGHT, dt);
    if (Keyboard::key(GLFW_KEY_A)) defaultCamera.updateCameraPos(CameraDirection::LEFT, dt);
    if (Keyboard::key(GLFW_KEY_SPACE)) defaultCamera.updateCameraPos(CameraDirection::UP, dt);
    if (Keyboard::key(GLFW_KEY_LEFT_SHIFT)) defaultCamera.updateCameraPos(CameraDirection::DOWN, dt);

    // Use per-frame mouse deltas instead of absolute positions to avoid accumulation
    double dx = Mouse::DX();
    double dy = Mouse::DY();
    if (dx != 0.0 || dy != 0.0)
    {
        defaultCamera.updateCameraDirection(dx, dy);
    }

    double scrollY = Mouse::getScrollDY();
    if (scrollY != 0.0)
    {
        defaultCamera.updateCameraZoom(scrollY);
    }
    if( Keyboard::keyDown(GLFW_KEY_L))
    {
       // sphere.rb.applyImpulse(defaultCamera.cameraRight, 5.0f, dt);
	}
    if (Keyboard::keyDown(GLFW_KEY_R))
    {
       // sphere.rb.applyImpulse(defaultCamera.cameraRight, -5.0f, dt);
    }
    if(Keyboard::keyDown(GLFW_KEY_E))
    {
        launchItem(deltaTime);
	}
}
void launchItem(float deltaTime)
{
    // Create a new Sphere at the camera position
    Sphere sphere(defaultCamera.cameraPos, glm::vec3(0.25f));
    sphere.init();

    // Initialize the rigidbody with mass and position
    sphere.rb = RigidBody(1.0f, defaultCamera.cameraPos); // 1kg mass, at camera position

    // Apply physics to its rigidbody
	sphere.rb.transferEnergy(5.0f, defaultCamera.cameraFront); // Transfer some energy to the sphere
    sphere.rb.applyAcceleration(Environment::gravity);

    // Add to the sphere array
    spheres.instances.push_back(sphere);
    
    std::cout << "Launched sphere at position: " << defaultCamera.cameraPos.x << ", " 
              << defaultCamera.cameraPos.y << ", " << defaultCamera.cameraPos.z << std::endl;
}
 



