#include"light.h"

void PointLight::render(Shader shader, int idx)
{
	std::string name = "pointLights["+std::to_string(idx)+"]";
	shader.set3Float(name + ".position", position);

	shader.set3Float(name + ".ambient", ambient);
	shader.set3Float(name + ".diffuse", diffuse);
	shader.set3Float(name + ".specular", specular);

	shader.setFloat(name + ".k0",k0);
	shader.setFloat(name + ".k1", k1);
	shader.setFloat(name + ".k2", k2);




}

void SpotLight::render(Shader shader,int idx)
{
	 std::string name = "spotLights[" + std::to_string(idx) + "]";
	shader.set3Float(name + ".position", position);
	shader.set3Float(name + ".direction", direction);

	shader.setFloat(name + ".cutOff", cutOff);
	shader.setFloat(name + ".outerCutOff", outerCutOff);

	shader.set3Float(name + ".ambient", ambient);
	shader.set3Float(name + ".diffuse", diffuse);
	shader.set3Float(name + ".specular", specular);


	shader.setFloat(name + ".k0", k0);
	shader.setFloat(name + ".k1", k1);
	shader.setFloat(name + ".k2", k2);

}


void DirLight::render(Shader shader)
{
	std::string name = "dirLight";

	shader.set3Float(name + ".direction", direction);

	shader.set3Float(name + ".ambient", ambient);
	shader.set3Float(name + ".diffuse", diffuse);
	shader.set3Float(name + ".specular", specular);

}