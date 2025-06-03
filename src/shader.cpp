#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>

Shader::Shader(const char* vertexPath, const char* fragmentPath) {
    // Read shader source code from files
    std::ifstream vShaderFile(vertexPath), fShaderFile(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;

    // Read file contents into string streams
    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    // Convert to C strings for OpenGL
    std::string vertexCode = vShaderStream.str();
    std::string fragmentCode = fShaderStream.str();
    const char* vCode = vertexCode.c_str();
    const char* fCode = fragmentCode.c_str();

    // Compile vertex shader
    GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vCode, NULL);    // Upload source code
    glCompileShader(vertex);                    // Compile to GPU bytecode

    // Compile fragment shader
    GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fCode, NULL);  // Upload source code
    glCompileShader(fragment);                  // Compile to GPU bytecode

    // Create shader program and link shaders together
    ID = glCreateProgram();
    glAttachShader(ID, vertex);      // Attach vertex shader
    glAttachShader(ID, fragment);    // Attach fragment shader
    glLinkProgram(ID);               // Link into complete program

    // Clean up individual shader objects (no longer needed after linking)
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

void Shader::use() const {
    glUseProgram(ID);  // Make this shader program active for rendering
}

// Utility functions for setting shader uniforms
void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
}

void Shader::setMat4(const std::string& name, const float* mat) const {
    // Upload 4x4 matrix (GL_FALSE means don't transpose)
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, mat);
}