#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <vector>
#include <string>

#include "../physics/rigidbody.h"
#include "../algorithm/bounce.h"  // Include bounding region header

#include "Shader.h"
#include "mesh.h"

class Model {
public:
    std::vector<Mesh> meshes;
    RigidBody rb; // Physics body for the model
    BoundingRegion boundingRegion; // Bounding region for collision detection
    glm::vec3 pos;
    glm::vec3 size;
    std::vector<Texture> textures_loaded;
    bool noTextures; // <-- Add this line to declare noTextures as a member

    Model(glm::vec3 pos = glm::vec3(0.0f), glm::vec3 size = glm::vec3(1.0f), bool noTextures = true);

    virtual void init();
    virtual void render(Shader shader, float dt, bool setModel=false,bool doRender=false);
    void cleanup();

    void loadModel(std::string path);
    
    // Bounding region methods
    void updateBoundingRegion(); // Update bounding region based on current position and size
    bool checkCollision(const Model& other) const; // Check collision with another model
    bool containsPoint(const glm::vec3& point) const; // Check if point is inside this model

protected:
  
    std::string directory;

private:
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadTextures(aiMaterial* mat, aiTextureType type);
};

#endif
