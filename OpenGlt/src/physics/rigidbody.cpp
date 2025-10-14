#include "rigidbody.h"	
#include <glm/glm.hpp>
#include <cmath>

RigidBody::RigidBody(float mass, glm::vec3 pos, glm::vec3 vel, glm::vec3 acc)
    : mass(mass), pos(pos), vel(vel), acc(acc)
{
}

void RigidBody::update(float dt) {
    // Update position based on velocity and acceleration
    pos += vel * dt + 0.5f * acc * dt * dt;

    // Update velocity based on acceleration
    vel += acc * dt;
}

void RigidBody::applyForce(glm::vec3 force)
{
    acc = force / mass; // Correct assignment
}

void RigidBody::applyForce(glm::vec3 direction, float magnitude)
{
    applyForce(direction * magnitude);
}

void RigidBody::applyImpulse(glm::vec3 impulse, float dt)
{
    vel += (impulse / mass) * dt; // Apply impulse (Force*dt = Impulse)
}

void RigidBody::applyImpulse(glm::vec3 direction, float magnitude, float dt)
{
    applyImpulse(direction * magnitude, dt);
}

void RigidBody::transferEnergy(float joules,glm::vec3 direction)
{
    if (joules == 0.0f) return;

    // Use float math to avoid implicit double-to-float and template ambiguities
    const float kineticEnergyMagnitude = std::fabs(joules);
    const float speedDelta = std::sqrtf(2.0f * kineticEnergyMagnitude / mass);
    glm::vec3 deltaV = speedDelta * direction;
	vel += joules > 0 ? deltaV : -deltaV; // Apply energy transfer based on the sign of joules
}

void RigidBody::applyAcceleration(glm::vec3 acceleration)
{
    acc += acceleration; // Fixed (+= instead of =+)
}

void RigidBody::applyAcceleration(glm::vec3 direction, float magnitude)
{
    applyAcceleration(direction * magnitude);
}

void RigidBody::getPosition(glm::vec3& position)
{
    position = pos;
}
