#pragma once
#include <gl/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>

class Model {
public:
    GLuint VAO, VBO;     // OpenGL objects for rendering
    size_t vertexCount;  // Number of vertices to draw

    // Load 3D model from OBJ file
    Model(const std::string& path);

    // Render the model (assumes shader is already active)
    void draw() const;
};