#include "model.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>

Model::Model(const std::string& path) {
    std::vector<float> vertexData;

    // Load the model with TinyObjLoader
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!success) {
        std::cerr << "Failed to load OBJ: " << err << std::endl;
        vertexCount = 0;
        return;
    }

    // Process all model shapes
    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            // Extract vertex position
            float vx = attrib.vertices[3 * index.vertex_index + 0];
            float vy = attrib.vertices[3 * index.vertex_index + 1];
            float vz = attrib.vertices[3 * index.vertex_index + 2];

            // Extract vertex normal (or use default if none)
            float nx = attrib.normals.empty() ? 0.0f : attrib.normals[3 * index.normal_index + 0];
            float ny = attrib.normals.empty() ? 0.0f : attrib.normals[3 * index.normal_index + 1];
            float nz = attrib.normals.empty() ? 0.0f : attrib.normals[3 * index.normal_index + 2];

            // Extract texture coordinates (or use default if none)
            float tx = attrib.texcoords.empty() ? 0.0f : attrib.texcoords[2 * index.texcoord_index + 0];
            float ty = attrib.texcoords.empty() ? 0.0f : attrib.texcoords[2 * index.texcoord_index + 1];

            // Add to vertex data (position + normal + texcoord)
            vertexData.insert(vertexData.end(), { vx, vy, vz, nx, ny, nz, tx, ty });
        }
    }

    vertexCount = vertexData.size() / 8; // 8 floats per vertex

    // Create OpenGL buffers
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Set vertex attribute pointers
    // Position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinates
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Model::draw() const {
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
    glBindVertexArray(0);
}