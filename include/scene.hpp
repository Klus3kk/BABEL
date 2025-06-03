#pragma once
#include <vector>
#include <memory>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shader.hpp"
#include "model.hpp"

class SceneObject {
public:
    const Model* model;
    glm::vec3 position;
    glm::vec3 rotation;  // In radians
    glm::vec3 scale;
    glm::mat4 modelMatrix;

    // Animation types (only the ones actually used)
    bool rotating = false;
    bool floating = false;
    bool orbiting = false;
    bool pulsing = false;

    // Animation parameters
    glm::vec3 basePosition;     // Original position for floating/orbiting
    glm::vec3 orbitCenter;      // Center point for orbital motion
    float orbitRadius = 2.0f;   // Radius of orbit
    float orbitSpeed = 1.0f;    // Speed of orbital motion
    float floatAmplitude = 0.3f; // How high/low to float
    float floatSpeed = 1.0f;    // Speed of floating motion
    float pulseAmplitude = 0.1f; // Scale variation for pulsing
    float pulseSpeed = 2.0f;    // Speed of pulsing
    float rotationSpeed = 0.0f; // Rotation speed

    // Animation time tracking
    float animationTime = 0.0f;
    float orbitTime = 0.0f;
    float floatTime = 0.0f;
    float pulseTime = 0.0f;

    // Constructor
    SceneObject(const Model* modelPtr,
        const glm::vec3& pos = glm::vec3(0.0f),
        const glm::vec3& rot = glm::vec3(0.0f),
        const glm::vec3& scl = glm::vec3(1.0f));

    // Core methods
    void updateModelMatrix();
    void update(float deltaTime);
    void rotate(float yawAmount, float pitchAmount = 0.0f, float rollAmount = 0.0f);

    // Animation control methods (only the ones used)
    void setRotating(bool enabled, float speed = 1.0f);
    void setFloating(bool enabled, float amplitude = 0.3f, float speed = 1.0f);
    void setOrbiting(bool enabled, const glm::vec3& center, float radius = 2.0f, float speed = 1.0f);
    void setPulsing(bool enabled, float amplitude = 0.1f, float speed = 2.0f);
};

// Scene class to manage all objects
class Scene {
public:
    std::vector<SceneObject> objects;

    // Core scene methods
    void addObject(const Model* model,
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f));

    void update(float deltaTime);
    void draw(Shader& shader) const;

    // Utility methods
    size_t getObjectCount() const { return objects.size(); }
    SceneObject& getObject(size_t index) { return objects[index]; }
    const SceneObject& getObject(size_t index) const { return objects[index]; }
};