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

    // Only include animation types that are actually used
    bool rotating = false;
    bool floating = false;
    bool orbiting = false;
    bool pulsing = false;

    // Only the animation parameters that are actually needed
    glm::vec3 basePosition;     // For floating
    glm::vec3 orbitCenter;      // For orbiting
    float orbitRadius = 2.0f;
    float orbitSpeed = 1.0f;
    float floatAmplitude = 0.3f;
    float floatSpeed = 1.0f;
    float pulseAmplitude = 0.1f;
    float pulseSpeed = 2.0f;
    float rotationSpeed = 0.0f;

    // Only the timing variables that are actually used
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

    // Animation control methods - only the ones actually used
    void setRotating(bool enabled, float speed = 1.0f);
    void setFloating(bool enabled, float amplitude = 0.3f, float speed = 1.0f);
    void setOrbiting(bool enabled, const glm::vec3& center, float radius = 2.0f, float speed = 1.0f);
    //void setPulsing(bool enabled, float amplitude = 0.1f, float speed = 2.0f);
};

class Scene {
public:
    std::vector<SceneObject> objects;

    void addObject(const Model* model,
        const glm::vec3& position = glm::vec3(0.0f),
        const glm::vec3& rotation = glm::vec3(0.0f),
        const glm::vec3& scale = glm::vec3(1.0f));

    void update(float deltaTime);
    void draw(Shader& shader) const;

    // Utility methods
    size_t getObjectCount() const { return objects.size(); }
    SceneObject& getObject(size_t index) { return objects[index]; }
    //const SceneObject& getObject(size_t index) const { return objects[index]; }
};