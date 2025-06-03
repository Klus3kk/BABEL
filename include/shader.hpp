#pragma once
#include <string>
#include <gl/glew.h>

class Shader {
public:
    GLuint ID;  // OpenGL shader program ID

    // Constructor loads vertex and fragment shader files, compiles and links them
    Shader(const char* vertexPath, const char* fragmentPath);

    // Make this shader active for rendering
    void use() const;

    // Utility functions for setting shader uniforms (shader variables)
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, float x, float y, float z) const;
    void setMat4(const std::string& name, const float* mat) const;  // Upload 4x4 matrix
};
