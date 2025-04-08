#pragma once
#include <string>
#include <gl/glew.h>

class Texture {
public:
    static GLuint load(const std::string& path, bool flip = true);
};
