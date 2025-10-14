#ifndef MESH_H
#define MESH_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <glm/glm.hpp>

// We need the full definition for Shader and Texture
#include "Shader.h"
#include "Texture.h"
#include "material.h"

// Vertex struct definition
struct Vertex {
	glm::vec3 pos;
	glm::vec3 normal;
	glm::vec2 texCoord;

	static std::vector<struct Vertex> genList(float* vertices, int size);
};

class Mesh
{
public:
	// Member Variables
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;
	Material material; // Material properties from GLTF
	unsigned int VAO;

	// Constructors
	Mesh(); // CORRECTED: Added declaration for the default constructor
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures = {}, Material material = Material::white_plastic);

	// Public Methods
	void render(Shader shade,bool dorender=true);
	void cleanUp();

private:
	// Private Member Variables
	unsigned int VBO, EBO;
	// Private Methods
	void setup();
};

#endif
