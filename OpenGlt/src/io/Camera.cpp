#include "Camera.h"

Camera::Camera(glm::vec3 position)
{
	cameraPos = position;
	cameraWorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	yaw = 0.0f;
	pitch = 0.0f;
	movementSpeed = 2.5f;
	zoom = 45.0f;
	cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
	updateCameraVectors();
}

void Camera::updateCameraDirection(double dx, double dy)
{
	float sensitivity = 0.1f; // Mouse sensitivity
	yaw += dx*sensitivity;
	pitch -= dy*sensitivity;

	if (pitch > 89.0f)
		pitch = 89.0f;
	else if (pitch < -89.0f)
		pitch = -89.0f;

	updateCameraVectors(); 
}

void Camera::updateCameraPos(CameraDirection direction, float deltaTime)
{
	float velocity = movementSpeed * deltaTime;

	if (direction == CameraDirection::FORWARD)
		cameraPos += cameraFront * velocity;
	if (direction == CameraDirection::BACKWARD)
		cameraPos -= cameraFront * velocity;
	if (direction == CameraDirection::LEFT)
		cameraPos -= cameraRight * velocity;
	if (direction == CameraDirection::RIGHT)
		cameraPos += cameraRight * velocity;
	if (direction == CameraDirection::UP)
		cameraPos += cameraUp * velocity;
	if (direction == CameraDirection::DOWN)
		cameraPos -= cameraUp * velocity;

	updateCameraVectors();
}

void Camera::updateCameraZoom(double dy)
{
	if (zoom >= 1.0f && zoom <= 45.0f)
		zoom -= dy;

	if (zoom <= 1.0f)
		zoom = 1.0f;
	if (zoom >= 45.0f)
		zoom = 45.0f;
}

glm::mat4 Camera::getViewMatrix()
{
	return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
}

void Camera::updateCameraVectors()
{
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
	cameraRight = glm::normalize(glm::cross(cameraFront, cameraWorldUp));
	cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));
}

float Camera::getZoom()
{
	return zoom;
}
