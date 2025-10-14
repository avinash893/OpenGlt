#include "Shader.h"

Shader::Shader(const char* vertexShaderPath, const char* fragmentShaderPath)
{
    int success;
    char infoLog[512];

    // Compile shaders
    GLuint vertexShader = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
    GLuint fragmentShader = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

    // Link shader program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    // Check for linking errors
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "SHADER PROGRAM LINKING FAILED\n" << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::activate()
{
    glUseProgram(ID);
}

std::string Shader::loadShaderSrc(const char* filepath)
{
    std::ifstream file(filepath);
    std::stringstream buffer;

    if (file.is_open())
    {
        buffer << file.rdbuf();
        return buffer.str();
    }
    else
    {
        std::cout << "Failed to open shader file: " << filepath << std::endl;
        return "";
    }
}

GLuint Shader::compileShader(const char* filepath, GLenum type)
{
    std::string src = loadShaderSrc(filepath);
    const char* srcCode = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &srcCode, NULL); 
    glCompileShader(shader);

    int success;
    char infoLog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cout << "SHADER COMPILATION FAILED\n" << infoLog << std::endl;
    }

    return shader;
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}


void Shader::set3Float(const std::string& name, glm::vec3 v)
{
	set3Float(name, v.x, v.y, v.z);
}
void Shader::set3Float(const std::string& name, float v1, float v2, float v3)
{
	glUniform3f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3);    
}
void Shader::set4Float(const std::string& name, float v1, float v2, float v3,float v4)
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), v1, v2, v3,v4);
}