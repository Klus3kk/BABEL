#include "model.hpp"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>

Model::Model(const std::string& path) {
	std::vector<float> vertexData; // Vector to hold vertex data (positions, normals, UVs)

    // Load OBJ file using TinyObjLoader library
    tinyobj::attrib_t attrib;                    // Vertex attributes (positions, normals, UVs)
    std::vector<tinyobj::shape_t> shapes;        // Mesh shapes/objects
    std::vector<tinyobj::material_t> materials;  // Material definitions (unused)
    std::string warn, err;                       // Warning and error messages

    bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str());
    if (!success) {
        std::cerr << "Failed to load OBJ: " << err << std::endl;
        vertexCount = 0;
        return;
    }

    // Process all shapes in the loaded model
    for (const auto& shape : shapes) {
        // Process each triangle face
        for (const auto& index : shape.mesh.indices) {
            // Extract vertex position from attribute arrays
            float vx = attrib.vertices[3 * index.vertex_index + 0];  // X coordinate
            float vy = attrib.vertices[3 * index.vertex_index + 1];  // Y coordinate
            float vz = attrib.vertices[3 * index.vertex_index + 2];  // Z coordinate

            // Extract vertex normal (or use default if model has no normals)
            float nx = attrib.normals.empty() ? 0.0f : attrib.normals[3 * index.normal_index + 0];
            float ny = attrib.normals.empty() ? 0.0f : attrib.normals[3 * index.normal_index + 1];
            float nz = attrib.normals.empty() ? 0.0f : attrib.normals[3 * index.normal_index + 2];

            // Extract texture coordinates (or use default if model has no UVs)
            float tx = attrib.texcoords.empty() ? 0.0f : attrib.texcoords[2 * index.texcoord_index + 0];
            float ty = attrib.texcoords.empty() ? 0.0f : attrib.texcoords[2 * index.texcoord_index + 1];

            // Pack vertex data: [position(3) + normal(3) + texcoord(2)] = 8 floats per vertex
            vertexData.insert(vertexData.end(), { vx, vy, vz, nx, ny, nz, tx, ty });
        }
    }

    vertexCount = vertexData.size() / 8; // Each vertex is 8 floats

    // Create OpenGL buffer objects
    glGenVertexArrays(1, &VAO);  // Vertex Array Object - stores vertex attribute setup
	glGenBuffers(1, &VBO);       // Vertex Buffer Object - stores actual vertex data (raw data)
	glBindVertexArray(VAO);      // Bind VAO to set up vertex attributes

    // Upload vertex data to GPU
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Position attribute (location = 0 in vertex shader)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute (location = 1 in vertex shader)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute (location = 2 in vertex shader)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);  // Unbind VAO to prevent accidental modification
}

void Model::draw() const {
    glBindVertexArray(VAO);                                          // Bind this model's VAO
    glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount)); // Draw all triangles
    glBindVertexArray(0);                                            // Unbind VAO
}