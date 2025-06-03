#pragma once
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include "shader.hpp"

class TextureManager {
public:
    // Load texture from file and store with given name
    static GLuint loadTexture(const std::string& name, const std::string& filePath);

    // Get previously loaded texture by name
    static GLuint getTexture(const std::string& name);

    // Load all textures needed for the project
    static void loadAllTextures();

    // Bind appropriate textures for different object types
    // This handles PBR material setup (base color + roughness + metallic)
    static void bindTextureForObject(const std::string& objectType, Shader& shader);

    // Free all loaded textures
    static void cleanup();

private:
    // Static storage for all loaded textures (name -> OpenGL texture ID)
    static std::unordered_map<std::string, GLuint> textures;
};