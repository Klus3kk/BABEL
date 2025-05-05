#pragma once
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"
#include "model.hpp"

// Scene object class
class SceneObject {
public:
    const Model* model;
    glm::vec3 position;
    glm::vec3 rotation;  // In radians
    glm::vec3 scale;
    glm::mat4 modelMatrix;

    // For rotating objects
    bool rotating = false;
    float rotationSpeed = 0.0f;

    // Constructor
    SceneObject(const Model* modelPtr,
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f));

    // Update the model matrix
    void updateModelMatrix();

    // Update rotation if needed
    void update(float deltaTime);

    // Rotate the object
    void rotate(float yawAmount, float pitchAmount = 0.0f, float rollAmount = 0.0f);

    // Enable continuous rotation
    void setRotating(bool enabled, float speed = 1.0f);
};

// Scene class to manage all objects
class Scene {
public:
    std::vector<SceneObject> objects;

    // Add an object to the scene
    void addObject(const Model* model,
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f));

    // Update all objects
    void update(float deltaTime);

    // Draw all objects
    void draw(Shader& shader) const;
};