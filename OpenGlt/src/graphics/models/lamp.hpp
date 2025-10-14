// lamp.hpp

#ifndef LAMP_HPP
#define LAMP_HPP

#include "cube.hpp"
#include "../light.h"

/*
    Lamp class inherits from Cube, carries a PointLight to be rendered
*/

class Lamp : public Cube {

public:
    glm::vec3 lightColor;

    // light strength values
    PointLight pointLight;

    Lamp() : Cube(Material::gold) {}

    Lamp(
        glm::vec3 lightColor,
        glm::vec3 ambient,
        glm::vec3 diffuse,
        glm::vec3 specular,
        float k0,
        float k1,
        float k2,
        glm::vec3 pos,
        glm::vec3 size
    )
        : Cube(Material::gold, pos, size),
        lightColor(lightColor),
        pointLight({ pos, ambient, diffuse, specular, k0, k1, k2 })
    {
        // Cube constructor already set pos and size
    }

    void render(Shader shader,float deltaTime,bool noModel)
    {
        shader.set3Float("lightColor", lightColor);
        Cube::render(shader, deltaTime,true);
    }

  


};
class lampArray : public ModelArray<Lamp>
{
public:
    std::vector<PointLight> lightInstances; // Store point lights for each lamp
    void init()
    {
        model = Lamp(glm::vec3(1.0f), glm::vec3(0.1f), glm::vec3(0.8f), glm::vec3(1.0f),
            1.0f, 0.09f, 0.032f, glm::vec3(0.0f), glm::vec3(1.0f));
        ModelArray::init();
    }

    void render(Shader shader, float dt, bool setList = true)
    {

        positions.clear();
        sizes.clear();
        for (PointLight& p1 : lightInstances)
        {
            positions.push_back(p1.position);
            sizes.push_back(model.size);

        }
		ModelArray::render(shader, dt, false);
    }
};


#endif // LAMP_HPP
