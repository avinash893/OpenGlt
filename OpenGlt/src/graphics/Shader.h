#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
    unsigned int ID;

    // Constructor that builds the shader from vertex and fragment shader paths
    Shader(const char* vertexShaderPath, const char* fragmentShaderPath);

    // Activate the shader program
    void activate();

    // Utility functions to set uniforms
	void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;

    void set3Float(const std::string& name, glm::vec3 v);
	void set3Float(const std::string& name, float v1, float v2, float v3);

	void set4Float(const std::string& name, float v1,float v2,float v3,float v4);
    void setMat4(const std::string& name, const glm::mat4& value) const;

private:
    std::string loadShaderSrc(const char* filepath);
    GLuint compileShader(const char* filepath, GLenum type);
};

#endif
