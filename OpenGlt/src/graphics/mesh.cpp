#include "mesh.h"
#include <assimp/material.h> // Include for aiTextureType definitions
#include <string>
#include <vector>

std::vector<Vertex> Vertex::genList(float* vertices, int noVertices)
{
    std::vector<Vertex> ret(noVertices);

    // Stride is the number of floats per vertex (pos:3, normal:3, texCoord:2)
    int stride = sizeof(Vertex) / sizeof(float);

    for (int i = 0; i < noVertices; i++)
    {
        ret[i].pos = glm::vec3(
            vertices[i * stride + 0],
            vertices[i * stride + 1],
            vertices[i * stride + 2]
        );

        ret[i].normal = glm::vec3(
            vertices[i * stride + 3],
            vertices[i * stride + 4],
            vertices[i * stride + 5]
        );

        ret[i].texCoord = glm::vec2(
            vertices[i * stride + 6],
            vertices[i * stride + 7]
        );
    }

    return ret;
}

Mesh::Mesh()
{
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, Material material)
    : vertices(vertices), indices(indices), textures(textures), material(material)
{
    setup();
}

void Mesh::render(Shader shader,bool doRender)
{
    // Set material properties in the shader
    shader.set3Float("material.ambient", material.ambient);
    shader.set3Float("material.diffuse", material.diffuse);
    shader.set3Float("material.specular", material.specular);
    shader.setFloat("material.shininess", material.shininess);
    
    // Activating textures
    unsigned int diffuseIdx = 1;  // Start at 1 for naming convention (e.g., diffuse1)
    unsigned int specularIdx = 1; // Start at 1 for naming convention (e.g., specular1)

    if (textures.empty()) {
        // No textures available, bind default white textures
        Texture& defaultDiffuse = Texture::getDefaultDiffuseTexture();
        Texture& defaultSpecular = Texture::getDefaultSpecularTexture();
        
        // Bind default diffuse texture to texture unit 0
        glActiveTexture(GL_TEXTURE0);
        shader.setInt("diffuse0", 0);
        defaultDiffuse.bind();
        
        // Bind default specular texture to texture unit 1
        glActiveTexture(GL_TEXTURE1);
        shader.setInt("specular0", 1);
        defaultSpecular.bind();
    } else {
        // Bind actual textures
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            // Activate texture unit before binding
            glActiveTexture(GL_TEXTURE0 + i);

            // Retrieve texture type and name
            std::string name;
            std::string number;
            switch (textures[i].type)
            {
            case aiTextureType_DIFFUSE:
                name = "material.diffuse";
                number = std::to_string(diffuseIdx++);
                break;
            case aiTextureType_SPECULAR:
                name = "material.specular";
                number = std::to_string(specularIdx++);
                break;
            default:
                name = "texture"; // Fallback name
                number = std::to_string(i);
                break;
            }

            // Set the texture uniform in the shader (e.g., "material.diffuse1")
            shader.setInt((name + number), static_cast<int>(i));
            textures[i].bind();
        }
    }

    
    // Render the mesh

    if (doRender)
    {
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);

        // Always good practice to set everything back to defaults once configured.
        glActiveTexture(GL_TEXTURE0);
    }



}

void Mesh::cleanUp()
{
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Mesh::setup()
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // VBO (Vertex Buffer Object) - Stores vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vertex),
        vertices.data(),
        GL_STATIC_DRAW
    );

    // EBO (Element Buffer Object) - Stores indices
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        indices.size() * sizeof(unsigned int),
        indices.data(),
        GL_STATIC_DRAW
    );

    // Vertex Attributes
    // Position (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, pos)
    );

    // Normals (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 3, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, normal)
    );

    // Texture Coordinates (location = 2)
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2, 2, GL_FLOAT, GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, texCoord)
    );

    glBindVertexArray(0); // Unbind VAO
}