#pragma once
#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include "shader.hpp"

class TextureManager {
public:
    // Load a single texture by name and path
    static GLuint loadTexture(const std::string& name, const std::string& filePath);

    // Get a loaded texture
    static GLuint getTexture(const std::string& name);

    // Load all textures for the project
    static void loadAllTextures();

    // Bind texture for an object type
    static void bindTextureForObject(const std::string& objectType, Shader& shader);

    // Clean up
    static void cleanup();

private:
    static std::unordered_map<std::string, GLuint> textures;
};