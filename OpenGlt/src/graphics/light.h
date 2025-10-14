#ifndef LIGHT_H
#define LIGHT_H


#include <glm/glm.hpp>	
#include "Shader.h"

struct PointLight {
	glm::vec3 position;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;



	// Attenuation coefficients
	float k0;
	float k1;
	float k2;


	void render(Shader shader,int idx);
};


struct SpotLight {
	glm::vec3 position;
	glm::vec3 direction;

	float cutOff;
	float outerCutOff;

	float k0;
	float k1;
	float k2;


	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;


	// Attenuation coefficients

	void render(Shader shader, int idx);
};


struct DirLight {
	glm::vec3 direction;
	glm::vec3 ambient;
	glm::vec3 diffuse;
	glm::vec3 specular;
	void render(Shader shader);
};


#endif // !LIGHT_H
