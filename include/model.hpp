#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Model {
public:
    GLuint VAO, VBO;
    size_t vertexCount;

    // Load model from file
    Model(const std::string& path);

    // Draw the model
    void draw() const;
};