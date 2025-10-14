#ifndef BOUNCE_H
#define BOUNCE_H

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

enum class BoundTypes
{
	AABB=0x00,
	SPHERE=0x01
};
class BoundingRegion
{
public:
	BoundTypes type;

	glm::vec3 center;
	float radius; // For sphere
	//bounding box values
	glm::vec3 min;
	glm::vec3 max;
	//constructor

	//initialize with tyepe

	BoundingRegion(BoundTypes Type);
	BoundingRegion(glm::vec3 center, float radius = 1.0f);

	BoundingRegion(glm::vec3 min, glm::vec3 max);
	//methods
	glm::vec3 calculateCenter();	
	glm::vec3 calculateDimensions();


	//testing

	bool containsPoint(glm::vec3 point) const;


	bool containsRegion(const BoundingRegion& other) const;	
	bool intersects(const BoundingRegion& other) const;

};
#endif // !BOUNCE_H
