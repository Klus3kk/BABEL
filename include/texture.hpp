#pragma once
#include <string>
#include <gl/glew.h>

class Texture {
public:
    // Load texture from file path, returns OpenGL texture ID
    // flip parameter controls whether to flip texture vertically (some formats need this)
    static GLuint load(const std::string& path, bool flip = true);
};