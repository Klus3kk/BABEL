#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Model {
public:
    GLuint VAO, VBO;
    size_t vertexCount;

    Model(const std::string& path);
    void draw() const;
};
