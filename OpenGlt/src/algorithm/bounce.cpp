#define GLM_ENABLE_EXPERIMENTAL
#include "bounce.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtx/norm.hpp>

BoundingRegion::BoundingRegion(BoundTypes Type)
    : type(Type), center(glm::vec3(0.0f)), radius(0.0f), min(glm::vec3(0.0f)), max(glm::vec3(0.0f))
{
}

BoundingRegion::BoundingRegion(glm::vec3 center, float radius)
    : type(BoundTypes::SPHERE), center(center), radius(radius), min(glm::vec3(0.0f)), max(glm::vec3(0.0f))
{
}

BoundingRegion::BoundingRegion(glm::vec3 min, glm::vec3 max)
    : type(BoundTypes::AABB), center(glm::vec3(0.0f)), radius(0.0f), min(min), max(max)
{}

glm::vec3 BoundingRegion::calculateCenter() {
    return (type == BoundTypes::AABB) ? (min + max) / 2.0f : center;
}

glm::vec3 BoundingRegion::calculateDimensions() {
    return (type == BoundTypes::AABB) ? (max - min) : glm::vec3(radius * 2.0f);
}

bool BoundingRegion::containsPoint(glm::vec3 point) const {
    if (type == BoundTypes::AABB) {
        return (point.x >= min.x) && (point.x <= max.x) &&
               (point.y >= min.y) && (point.y <= max.y) &&
               (point.z >= min.z) && (point.z <= max.z);
    } else {
        float disr = glm::distance2(point, center);
        return disr <= (radius * radius);
    }
}

bool BoundingRegion::containsRegion(const BoundingRegion& br) const {
    if (type == BoundTypes::AABB && br.type == BoundTypes::SPHERE) {
        // Check if the sphere's center is inside the AABB and its radius fits
        return containsPoint(br.center) &&
               (br.center.x - br.radius >= min.x) && (br.center.x + br.radius <= max.x) &&
               (br.center.y - br.radius >= min.y) && (br.center.y + br.radius <= max.y) &&
               (br.center.z - br.radius >= min.z) && (br.center.z + br.radius <= max.z);
    } else if (type == BoundTypes::SPHERE && br.type == BoundTypes::SPHERE) {
        float distance = glm::distance(center, br.center);
        return distance + br.radius <= radius;
    } else if (type == BoundTypes::SPHERE && br.type == BoundTypes::AABB) {
        glm::vec3 corners[8] = {
            glm::vec3(br.min.x, br.min.y, br.min.z),
            glm::vec3(br.min.x, br.min.y, br.max.z),
            glm::vec3(br.min.x, br.max.y, br.min.z),
            glm::vec3(br.min.x, br.max.y, br.max.z),
            glm::vec3(br.max.x, br.min.y, br.min.z),
            glm::vec3(br.max.x, br.min.y, br.max.z),
            glm::vec3(br.max.x, br.max.y, br.min.z),
            glm::vec3(br.max.x, br.max.y, br.max.z)
        };
        for (const auto& corner : corners) {
            if (!containsPoint(corner))
                return false;
        }
        return true;
    }
    return false;
}

bool BoundingRegion::intersects(const BoundingRegion& br) const {
    if (type == BoundTypes::AABB && br.type == BoundTypes::AABB) {
        return (min.x <= br.max.x && max.x >= br.min.x) &&
               (min.y <= br.max.y && max.y >= br.min.y) &&
               (min.z <= br.max.z && max.z >= br.min.z);
    } else if (type == BoundTypes::SPHERE && br.type == BoundTypes::SPHERE) {
        float distance = glm::distance(center, br.center);
        return distance <= (radius + br.radius);
    } else if (type == BoundTypes::AABB && br.type == BoundTypes::SPHERE) {
        glm::vec3 closestPoint = glm::clamp(br.center, min, max);
        float distanceSquared = glm::distance2(closestPoint, br.center);
        return distanceSquared <= (br.radius * br.radius);
    } else if (type == BoundTypes::SPHERE && br.type == BoundTypes::AABB) {
        glm::vec3 closestPoint = glm::clamp(center, br.min, br.max);
        float distanceSquared = glm::distance2(closestPoint, center);
        return distanceSquared <= (radius * radius);
    }
    return false;
}