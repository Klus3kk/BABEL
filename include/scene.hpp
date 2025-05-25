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

    // NEW: Advanced animation types
    bool floating = false;
    bool orbiting = false;
    bool pulsing = false;

    // NEW: Animation parameters
    glm::vec3 basePosition;     // Original position for floating/orbiting
    glm::vec3 orbitCenter;      // Center point for orbital motion
    float orbitRadius = 2.0f;   // Radius of orbit
    float orbitSpeed = 1.0f;    // Speed of orbital motion
    float floatAmplitude = 0.3f; // How high/low to float
    float floatSpeed = 1.0f;    // Speed of floating motion
    float pulseAmplitude = 0.1f; // Scale variation for pulsing
    float pulseSpeed = 2.0f;    // Speed of pulsing

    // NEW: Animation time tracking
    float animationTime = 0.0f;
    float orbitTime = 0.0f;
    float floatTime = 0.0f;
    float pulseTime = 0.0f;

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

    // NEW: Animation control methods
    void setFloating(bool enabled, float amplitude = 0.3f, float speed = 1.0f);
    void setOrbiting(bool enabled, const glm::vec3& center, float radius = 2.0f, float speed = 1.0f);
    void setPulsing(bool enabled, float amplitude = 0.1f, float speed = 2.0f);
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