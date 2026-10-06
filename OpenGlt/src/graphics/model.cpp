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
	if (meshes.empty())
	{
		std::cout << "WARNING: Model has no meshes to render!" << std::endl;
		return;
	}
	
	glm::mat4 modelM = glm::mat4(1.0f);
	modelM = glm::translate(modelM, pos);
	modelM = glm::scale(modelM, size);
	shader.setMat4("model", modelM);
	shader.setFloat("material.shininess", 1.0f);
	
	for (Mesh& mesh : meshes)
	{
		// Only render if mesh has vertices and indices
		if (!mesh.vertices.empty() && !mesh.indices.empty())
		{
			mesh.render(shader, doRender);
		}
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
	// Use more comprehensive post-processing for GLTF files
	unsigned int postProcessFlags = aiProcess_Triangulate 
		| aiProcess_FlipUVs 
		| aiProcess_CalcTangentSpace
		| aiProcess_GenNormals;
	
	// Try to load the model file
	const aiScene* scene = import.ReadFile(path, postProcessFlags);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		std::cout << "ERROR: Cannot load model at: " << path << std::endl;
		std::cout << "Assimp error: " << import.GetErrorString() << std::endl;
		
		// Check if file exists
		std::ifstream fileCheck(path);
		if (!fileCheck.good()) {
			std::cout << "ERROR: File does not exist at: " << path << std::endl;
		} else {
			std::cout << "File exists but Assimp cannot load it. Possible issues:" << std::endl;
			std::cout << "  - File is corrupted" << std::endl;
			std::cout << "  - Unsupported format or version" << std::endl;
			std::cout << "  - Missing dependencies or textures" << std::endl;
		}
		fileCheck.close();
		return;
	}
	
	std::cout << "Model file loaded successfully!" << std::endl;
	std::cout << "  Scene has " << scene->mNumMeshes << " meshes" << std::endl;
	std::cout << "  Scene has " << scene->mNumMaterials << " materials" << std::endl;
	std::cout << "  Root node: " << (scene->mRootNode ? "yes" : "no") << std::endl;
	
	// Handle both Windows and Unix path separators
	size_t lastSlash = path.find_last_of("/\\");
	if (lastSlash != std::string::npos) {
		directory = path.substr(0, lastSlash);
	} else {
		directory = ""; // No directory specified
	}
	// Clear any existing meshes before loading
	meshes.clear();
	
	// Process the scene starting from root node
	processNode(scene->mRootNode, scene);
	
	std::cout << "Loaded model OK. Scene meshes: "
		<< scene->mNumMeshes << ", Materials: " << scene->mNumMaterials << std::endl;
	
	// Debug: Print mesh details
	std::cout << "Processed " << meshes.size() << " meshes in Model object" << std::endl;
	if (meshes.empty())
	{
		std::cout << "ERROR: No meshes were processed! This could mean:" << std::endl;
		std::cout << "  1. The GLTF file structure is not being parsed correctly" << std::endl;
		std::cout << "  2. Meshes are empty or have no vertices" << std::endl;
		std::cout << "  3. Node hierarchy doesn't contain mesh references" << std::endl;
		
		// Try processing meshes directly from scene (bypassing node hierarchy)
		std::cout << "Attempting to load meshes directly from scene..." << std::endl;
		for (unsigned int i = 0; i < scene->mNumMeshes; i++)
		{
			aiMesh* aiMesh = scene->mMeshes[i];
			if (aiMesh && aiMesh->mNumVertices > 0)
			{
				std::cout << "  Processing scene mesh " << i << " directly: " 
					<< aiMesh->mNumVertices << " vertices" << std::endl;
				Mesh processedMesh = processMesh(aiMesh, scene);
				if (!processedMesh.vertices.empty() && !processedMesh.indices.empty())
				{
					meshes.push_back(processedMesh);
					std::cout << "    Added mesh with " << processedMesh.vertices.size() << " vertices" << std::endl;
				}
			}
		}
		std::cout << "After direct processing: " << meshes.size() << " meshes" << std::endl;
	}
	
	for (size_t i = 0; i < meshes.size(); i++)
	{
		std::cout << "  Mesh " << i << ": " << meshes[i].vertices.size() << " vertices, " 
			<< meshes[i].indices.size() << " indices, " << meshes[i].textures.size() << " textures" << std::endl;
		if (meshes[i].vertices.empty() || meshes[i].indices.empty())
		{
			std::cout << "    WARNING: Mesh " << i << " is empty!" << std::endl;
		}
	}
}


void Model::processNode(aiNode* node, const aiScene* scene)
{
	// Debug: Print node information
	std::cout << "Processing node: " << node->mName.C_Str() << " (meshes: " << node->mNumMeshes << ", children: " << node->mNumChildren << ")" << std::endl;
	
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		unsigned int meshIndex = node->mMeshes[i];
		if (meshIndex >= scene->mNumMeshes)
		{
			std::cout << "ERROR: Mesh index " << meshIndex << " out of range (total meshes: " << scene->mNumMeshes << ")" << std::endl;
			continue;
		}
		
		aiMesh* mesh = scene->mMeshes[meshIndex];
		if (mesh && mesh->mNumVertices > 0)
		{
			std::cout << "  Processing mesh " << i << ": " << mesh->mNumVertices << " vertices, " << mesh->mNumFaces << " faces" << std::endl;
			Mesh processedMesh = processMesh(mesh, scene);
			// Only add mesh if it has vertices and indices
			if (!processedMesh.vertices.empty() && !processedMesh.indices.empty())
			{
				meshes.push_back(processedMesh);
				std::cout << "    Added mesh with " << processedMesh.vertices.size() << " vertices and " << processedMesh.indices.size() << " indices" << std::endl;
			}
			else
			{
				std::cout << "    WARNING: Processed mesh is empty, not adding to meshes vector" << std::endl;
			}
		}
		else
		{
			std::cout << "  WARNING: Mesh " << i << " is empty or null" << std::endl;
		}
	}
	// Then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	if (!mesh)
	{
		std::cout << "ERROR: processMesh called with null mesh!" << std::endl;
		return Mesh(); // Return empty mesh
	}
	
	if (mesh->mNumVertices == 0)
	{
		std::cout << "WARNING: Mesh has zero vertices, skipping" << std::endl;
		return Mesh(); // Return empty mesh
	}
	
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> texture;

	std::cout << "    Processing mesh with " << mesh->mNumVertices << " vertices and " << mesh->mNumFaces << " faces" << std::endl;

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
		else
		{
			// Generate default normal if missing (pointing up)
			vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);
			std::cout << "WARNING: Mesh missing normals, using default" << std::endl;
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
		if (face.mNumIndices != 3)
		{
			std::cout << "WARNING: Face " << i << " has " << face.mNumIndices << " indices (expected 3)" << std::endl;
		}
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	
	if (indices.empty())
	{
		std::cout << "ERROR: Mesh has no indices!" << std::endl;
	}
	if (vertices.empty())
	{
		std::cout << "ERROR: Mesh has no vertices!" << std::endl;
	}
	// Process material (only if textures are enabled for this model)
	if (mesh->mMaterialIndex >= 0 && !noTextures)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		
		// For GLTF files, try different texture types
		// Start with standard diffuse and specular
		std::vector<Texture> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE);
		
		// GLTF 2.0 uses different texture type constants
		// Try base color texture (common in GLTF PBR materials)
		// Note: Some Assimp versions use different constants
		if (diffuseMaps.empty())
		{
			// Try other possible texture type values for GLTF
			// aiTextureType_BASE_COLOR might be available in newer Assimp
			// For now, we'll stick with DIFFUSE as it's most compatible
			diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE);
		}
		texture.insert(texture.end(), diffuseMaps.begin(), diffuseMaps.end());
		
		// Specular maps
		std::vector<Texture> specularMaps = loadTextures(material, aiTextureType_SPECULAR);
		texture.insert(texture.end(), specularMaps.begin(), specularMaps.end());
		
		std::cout << "  Loaded " << diffuseMaps.size() << " diffuse and " << specularMaps.size() << " specular textures" << std::endl;
	}
	else if (mesh->mMaterialIndex >= 0)
	{
		std::cout << "Textures disabled for this mesh, using default material" << std::endl;
	}
	else
	{
		std::cout << "Mesh has no material index" << std::endl;
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
