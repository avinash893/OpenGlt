#ifndef CAMERA_H
#define CAMERA_H
#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>


enum class CameraDirection
{
	NONE=0,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

class Camera
{
public:
	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;
	glm::vec3 cameraRight;
	glm::vec3 cameraWorldUp;


	float yaw;  
	float pitch;
	float movementSpeed;
	float zoom;


	Camera(glm::vec3 position);
	void updateCameraDirection(double dx, double dy);
	void updateCameraPos(CameraDirection direction, float deltaTime);
	void updateCameraZoom(double dy);

	float getZoom();
	glm::mat4 getViewMatrix();

private:
	void updateCameraVectors();

	

};


#endif //
