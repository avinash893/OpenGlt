#ifndef MODELARRAY_HPP
#define MODELARRAY_HPP

#include "../model.h"

#define UPPER_BOUND 100 // Maximum number of instances in the array

template <class T>
class ModelArray
{
public:
    std::vector<T> instances; // Store actual models (Sphere, Cube, etc.)

    void init()
    {
        model.init();

        // --- Position buffer for instancing ---
        glGenBuffers(1, &posVBO);
        glBindBuffer(GL_ARRAY_BUFFER, posVBO);
        glBufferData(GL_ARRAY_BUFFER, UPPER_BOUND * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // --- Size buffer for instancing ---
        glGenBuffers(1, &sizeVBO);
        glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
        glBufferData(GL_ARRAY_BUFFER, UPPER_BOUND * 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // --- Set attribute pointers for each mesh ---
        for (unsigned int i = 0, sz = model.meshes.size(); i < sz; i++)
        {
            glBindVertexArray(model.meshes[i].VAO);

            // Position attribute (layout = 3 in shader)
            glBindBuffer(GL_ARRAY_BUFFER, posVBO);
            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(3);

            // Size attribute (layout = 4 in shader)
            glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
            glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(4);

            // Unbind VBO
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            // Mark these attributes as "per instance"
            glVertexAttribDivisor(3, 1);
            glVertexAttribDivisor(4, 1);

            glBindVertexArray(0); // Unbind VAO
        }

        // Reserve memory to avoid reallocations
        positions.reserve(UPPER_BOUND);
        sizes.reserve(UPPER_BOUND);
    }

    void render(Shader shader, float dt, bool setList = true)
    {
        if (setList)
        {
            positions.clear();
            sizes.clear();
            for (T& instance : instances)
            {
                // If T is Sphere, and Sphere has a RigidBody member named rb
                // Use dynamic_cast if T is a base pointer, otherwise direct access
                positions.push_back(instance.rb.pos); // Use instance.rb.pos for Sphere
                sizes.push_back(model.size);
            }
        }
        int size = std::min(UPPER_BOUND, (int)positions.size());

        shader.setMat4("model", glm::mat4(1.0f));
        shader.setInt("useInstancing", 1);
        // Do not draw the prototype model when instancing; just ensure VAO state set above.
        model.render(shader, dt, true);
        if (positions.size() != 0)
        {
            glBindBuffer(GL_ARRAY_BUFFER, posVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3 * sizeof(float), &positions[0]);
            glBindBuffer(GL_ARRAY_BUFFER, sizeVBO);
            glBufferSubData(GL_ARRAY_BUFFER, 0, size * 3 * sizeof(float), &sizes[0]);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        for (unsigned int i = 0, size = model.meshes.size(); i < size; i++)
        {
            glBindVertexArray(model.meshes[i].VAO);
            glDrawElementsInstanced(GL_TRIANGLES, model.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, positions.size());
            glBindVertexArray(0);
        }
        shader.setInt("useInstancing", 0);
    }

    void setSize(glm::vec3 size)
    {
        for (auto& instance : instances)
        {
            instance.size = size;
        }
    }

    void cleanup()
    {
        for (auto& instance : instances)
        {
            instance.cleanup();
        }
    }

protected:
    T model; // prototype instance for base setup
    unsigned int posVBO = 0;
    unsigned int sizeVBO = 0;

    std::vector<glm::vec3> positions; // per-instance positions
    std::vector<glm::vec3> sizes;     // per-instance sizes
};

#endif // MODELARRAY_HPP
