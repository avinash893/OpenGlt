#ifndef RIGIDBODY_H
#define RIGIDBODY_H

#include "glm/glm.hpp"

class RigidBody
{
public:
    glm::vec3 pos; // Position of the rigid body
    glm::vec3 vel; // Velocity of the rigid body
    glm::vec3 acc; // Acceleration of the rigid body
    float mass; // Mass of the rigid body	


    RigidBody(float mass = 1.0f,
        glm::vec3 pos = glm::vec3(0.0f),
        glm::vec3 vel = glm::vec3(0.0f),
        glm::vec3 acc = glm::vec3(0.0f));



    void update(float dt);
    void applyForce(glm::vec3 force);
    void applyForce(glm::vec3 direction, float magnitude);
    void applyAcceleration(glm::vec3 acceleration);
    void applyAcceleration(glm::vec3 direction, float magnitude);
    void applyImpulse(glm::vec3 impulse, float dt);
    void applyImpulse(glm::vec3 direction, float magnitude, float dt);

    void transferEnergy(float joules, glm::vec3 direction);
    void getPosition(glm::vec3& position);

    
};

#endif // !RIGIDBODY_H
