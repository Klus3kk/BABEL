#include "scene.hpp"
#include <cmath>
#include <cstdlib>

SceneObject::SceneObject(const Model* modelPtr, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
    : model(modelPtr), position(pos), rotation(rot), scale(scl), basePosition(pos), orbitCenter(pos) {
    updateModelMatrix();

    // Initialize random animation offsets for variety
    floatTime = static_cast<float>(rand()) / RAND_MAX * 6.28f; // Random phase
    orbitTime = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    pulseTime = static_cast<float>(rand()) / RAND_MAX * 6.28f;
}

void SceneObject::updateModelMatrix() {
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, scale);
}

void SceneObject::update(float deltaTime) {
    animationTime += deltaTime;

    // Handle orbital motion
    if (orbiting) {
        orbitTime += orbitSpeed * deltaTime;
        float x = orbitCenter.x + orbitRadius * cos(orbitTime);
        float z = orbitCenter.z + orbitRadius * sin(orbitTime);
        position.x = x;
        position.z = z;

        // Make orbiting objects face their movement direction
        rotation.y = orbitTime + glm::radians(90.0f);
    }

    // Handle floating motion (bobbing up and down)
    if (floating) {
        floatTime += floatSpeed * deltaTime;
        if (!orbiting) {
            position.y = basePosition.y + sin(floatTime) * floatAmplitude;
        }
        else {
            // If both orbiting and floating, add floating to orbital motion
            position.y = orbitCenter.y + sin(floatTime) * floatAmplitude;
        }
    }

    // Handle pulsing (scale variation)
    if (pulsing) {
        pulseTime += pulseSpeed * deltaTime;
        float pulseFactor = 1.0f + sin(pulseTime) * pulseAmplitude;
        scale = glm::vec3(pulseFactor);
    }

    // Handle rotation
    if (rotating) {
        // Rotate around Y axis based on rotation speed
        rotation.y += rotationSpeed * deltaTime;

        // Keep rotation angle in reasonable range
        if (rotation.y > 6.28f) // 2π
            rotation.y -= 6.28f;
        if (rotation.y < -6.28f)
            rotation.y += 6.28f;
    }

    updateModelMatrix();
}

void SceneObject::rotate(float yawAmount, float pitchAmount, float rollAmount) {
    rotation.y += yawAmount;
    rotation.x += pitchAmount;
    rotation.z += rollAmount;
    updateModelMatrix();
}

void SceneObject::setRotating(bool enabled, float speed) {
    rotating = enabled;
    rotationSpeed = speed;
}

void SceneObject::setFloating(bool enabled, float amplitude, float speed) {
    floating = enabled;
    floatAmplitude = amplitude;
    floatSpeed = speed;
    if (enabled && !orbiting) {
        basePosition = position; // Store current position as base
    }
}

void SceneObject::setOrbiting(bool enabled, const glm::vec3& center, float radius, float speed) {
    orbiting = enabled;
    orbitCenter = center;
    orbitRadius = radius;
    orbitSpeed = speed;
    if (enabled) {
        // Start orbit from current position
        glm::vec3 offset = position - center;
        orbitTime = atan2(offset.z, offset.x);
    }
}

void SceneObject::setPulsing(bool enabled, float amplitude, float speed) {
    pulsing = enabled;
    pulseAmplitude = amplitude;
    pulseSpeed = speed;
}

// Scene implementation
void Scene::addObject(const Model* model, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
    objects.emplace_back(model, position, rotation, scale);
}

void Scene::update(float deltaTime) {
    for (auto& obj : objects) {
        obj.update(deltaTime);
    }
}

void Scene::draw(Shader& shader) const {
    for (const auto& obj : objects) {
        shader.setMat4("model", &obj.modelMatrix[0][0]);
        obj.model->draw();
    }
}