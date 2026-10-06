#include "texture.h"

#include <iostream>
#include <algorithm>
Texture::Texture()
{
    id = 0;
    type = aiTextureType_NONE;
    name = "";
    dir = "";
    path = "";
}
unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma)
{
    Texture texture(directory, path, aiTextureType_DIFFUSE);
    texture.load();
    return texture.id;
}

Texture::Texture(std::string name)
    : name(name), type(aiTextureType_NONE) {
    generate();
}

// initialize with image path and type
Texture::Texture(std::string dir, std::string path, aiTextureType type)
    : dir(dir), path(path), type(type) {
    generate();
}

// generate texture id
void Texture::generate() {
    glGenTextures(1, &id);
}

// load texture from path
void Texture::load(bool flip) {
    stbi_set_flip_vertically_on_load(flip);

    int width, height, nChannels;

    // Handle path construction (support both Windows and Unix separators)
    std::string fullPath;
    if (dir.empty()) {
        fullPath = path;
    } else {
        // Normalize path separators
        std::string normalizedDir = dir;
        std::string normalizedPath = path;
        
        // Replace backslashes with forward slashes for consistency
        std::replace(normalizedDir.begin(), normalizedDir.end(), '\\', '/');
        std::replace(normalizedPath.begin(), normalizedPath.end(), '\\', '/');
        
        // Remove trailing slash from directory if present
        if (!normalizedDir.empty() && normalizedDir.back() == '/') {
            normalizedDir.pop_back();
        }
        
        // Remove leading slash from path if present
        if (!normalizedPath.empty() && normalizedPath.front() == '/') {
            normalizedPath.erase(0, 1);
        }
        
        fullPath = normalizedDir + "/" + normalizedPath;
    }

    unsigned char* data = stbi_load(fullPath.c_str(), &width, &height, &nChannels, 0);

    GLenum colorMode = GL_RGB;

    switch (nChannels) {
    case 1:
        colorMode = GL_RED;
        break;
    case 4:
        colorMode = GL_RGBA;
        break;
    };

    if (data) {
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, colorMode, width, height, 0, colorMode, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        std::cout << "Successfully loaded texture: " << fullPath << std::endl;
    }
    else {
        std::cout << "ERROR: Failed to load texture at: " << fullPath << std::endl;
        std::cout << "  Directory: " << dir << ", Path: " << path << std::endl;
        // Create a default white texture as fallback
        unsigned char whitePixel[] = { 255, 255, 255, 255 };
        glBindTexture(GL_TEXTURE_2D, id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    stbi_image_free(data);
}

void Texture::allocate(GLenum format, GLuint width, GLuint height, GLenum type) {
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, type, NULL);
}

void Texture::setParams(GLenum texMinFilter, GLenum texMagFilter, GLenum wrapS, GLenum wrapT) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, texMinFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, texMagFilter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
}

// bind texture id
void Texture::bind() {
    glBindTexture(GL_TEXTURE_2D, id);
}

void Texture::cleanup() {
    glDeleteTextures(1, &id);
}

Texture Texture::createDefaultWhiteTexture() {
    Texture texture;
    texture.generate();
    
    // Create a 1x1 white texture
    unsigned char whitePixel[] = { 255, 255, 255, 255 }; // RGBA white
    
    glBindTexture(GL_TEXTURE_2D, texture.id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    texture.type = aiTextureType_DIFFUSE;
    texture.name = "default_white";
    
    return texture;
}

Texture& Texture::getDefaultDiffuseTexture() {
    static Texture defaultDiffuse;
    static bool initialized = false;
    
    if (!initialized) {
        defaultDiffuse = createDefaultWhiteTexture();
        initialized = true;
    }
    
    return defaultDiffuse;
}

Texture& Texture::getDefaultSpecularTexture() {
    static Texture defaultSpecular;
    static bool initialized = false;
    
    if (!initialized) {
        defaultSpecular = createDefaultWhiteTexture();
        defaultSpecular.type = aiTextureType_SPECULAR;
        defaultSpecular.name = "default_specular";
        initialized = true;
    }
    
    return defaultSpecular;
}