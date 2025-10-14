#include "model.h"
#include "mesh.h"

// Includes for Assimp and standard libraries
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include <string>
#include <vector>
#include <cstring>

Model::Model(glm::vec3 pos, glm::vec3 size, bool noTextures)
	: pos(pos), size(size), noTextures(noTextures), boundingRegion(BoundTypes::SPHERE)
{
	// Initialize bounding region as a sphere centered at the model position
	// with radius based on the largest dimension of the size
	float maxSize = (std::max)(size.x, (std::max)(size.y, size.z));
	boundingRegion = BoundingRegion(pos, maxSize * 0.5f);
}

void Model::init()
{
}

// Render the model. "dt", "setModel", and "doRender" params are accepted to
// match the interface; current implementation renders immediately.
void Model::render(Shader shader, float /*dt*/, bool /*setModel*/, bool doRender)
{
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, pos);
	modelM = glm::scale(modelM, size);
	shader.setMat4("model", modelM);
	shader.setFloat("material.shininess", 1.0f);
	for (Mesh& mesh : meshes)
	{
		mesh.render(shader, doRender);
	}
}

void Model::cleanup()
{
	for (Mesh& mesh : meshes)
	{
		mesh.cleanUp();
	}
}


// Load the model from the specified path using Assimp
void Model::loadModel(std::string path)
{
	// Diagnostics: show target path and, on Windows, current working directory
	std::cout << "Loading model: " << path << std::endl;
#ifdef _WIN32
	char cwdBuf[MAX_PATH];
	if (GetCurrentDirectoryA(MAX_PATH, cwdBuf)) {
		std::cout << "CWD: " << cwdBuf << std::endl;
	}
#endif

	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "cant load model at   " << path << std::endl << import.GetErrorString() << std::endl;
		return;
	}
	
	// Handle both Windows and Unix path separators
	size_t lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		directory = path.substr(0, lastSlash);
	} else {
		directory = ""; // No directory specified
	}
	processNode(scene->mRootNode, scene);
	std::cout << "Loaded model OK. Meshes: "
		<< scene->mNumMeshes << ", Materials: " << scene->mNumMaterials << std::endl;
}


void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// Then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> texture;

	// Process vertex positions, normals and texture coordinates
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		// Position
		vertex.pos = glm::vec3
		{
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z
		};

		// Normals
		if (mesh->HasNormals())
		{
			vertex.normal = glm::vec3
			{
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z
			};
		}

		// Texture coordinates
		if (mesh->mTextureCoords[0]) // Does the mesh contain texture coordinates?
		{
			vertex.texCoord = glm::vec2{
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y
			};
		}
		else
			vertex.texCoord = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	// Process indices
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// Process material (only if textures are enabled for this model)
	if (mesh->mMaterialIndex >= 0 && !noTextures)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		// Diffuse maps
		std::vector<Texture> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE);
		texture.insert(texture.end(), diffuseMaps.begin(), diffuseMaps.end());
		// Specular maps
		std::vector<Texture> specularMaps = loadTextures(material, aiTextureType_SPECULAR);
		texture.insert(texture.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, texture);
}

std::vector<Texture> Model::loadTextures(aiMaterial* mat, aiTextureType type)
{
	std::vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		bool skip = false;
		// Check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{   // If texture hasn't been loaded already, load it
			Texture tex(directory,str.C_Str(),type);
			tex.load(false); // Load the texture from file
			textures.push_back(tex);
			
			textures_loaded.push_back(tex); // Add to loaded textures
		}
	}
	return textures;
}

// Update bounding region based on current position and size
void Model::updateBoundingRegion()
{
	// Update the center position of the bounding region to match the model's current position
	boundingRegion.center = pos;
	
	// Update the radius based on the current size
	// Use the largest dimension to ensure the sphere encompasses the entire model
	float maxSize = (std::max)(size.x, (std::max)(size.y, size.z));
	boundingRegion.radius = maxSize * 0.5f;
}

// Check collision with another model
bool Model::checkCollision(const Model& other) const
{
	// Use the bounding region's intersect method to check for collision
	return boundingRegion.intersects(other.boundingRegion);
}

// Check if a point is inside this model's bounding region
bool Model::containsPoint(const glm::vec3& point) const
{
	// Use the bounding region's containsPoint method
	return boundingRegion.containsPoint(point);
}
