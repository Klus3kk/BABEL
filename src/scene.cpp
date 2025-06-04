#include "scene.hpp"
#include <cmath>
#include <cstdlib>

SceneObject::SceneObject(const Model* modelPtr, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
    : model(modelPtr), position(pos), rotation(rot), scale(scl), basePosition(pos), orbitCenter(pos) {
    updateModelMatrix();

    // Randomize animation start times so objects don't all sync up
    floatTime = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    orbitTime = static_cast<float>(rand()) / RAND_MAX * 6.28f;
    pulseTime = static_cast<float>(rand()) / RAND_MAX * 6.28f;
}

void SceneObject::updateModelMatrix() {
    // Build transformation matrix: Scale * Rotate * Translate
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);

    // Apply rotations in XYZ order
    modelMatrix = glm::rotate(modelMatrix, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::rotate(modelMatrix, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    modelMatrix = glm::scale(modelMatrix, scale);
}

void SceneObject::update(float deltaTime) {
    // Orbital motion - objects move in circles
    if (orbiting) {
        orbitTime += orbitSpeed * deltaTime;
        position.x = orbitCenter.x + orbitRadius * cos(orbitTime);
        position.z = orbitCenter.z + orbitRadius * sin(orbitTime);
        rotation.y = orbitTime + glm::radians(90.0f); // Make orbiting objects face their movement direction
    }

    // Floating motion - gentle up/down bobbing
    if (floating) {
        floatTime += floatSpeed * deltaTime;
        if (!orbiting) {
            // Simple floating around base position
            position.y = basePosition.y + sin(floatTime) * floatAmplitude;
        }
        else {
            // If orbiting too, add floating to orbital motion
            position.y = orbitCenter.y + sin(floatTime) * floatAmplitude;
        }
    }

    // Rotation around Y axis
    if (rotating) {
        rotation.y += rotationSpeed * deltaTime;

        // Keep rotation in reasonable range to prevent float precision issues
		if (rotation.y > 6.28f) // 2pi radians 
            rotation.y -= 6.28f;
        if (rotation.y < -6.28f)
            rotation.y += 6.28f;
    }

    updateModelMatrix();  // Rebuild transformation matrix with new values
}

void SceneObject::rotate(float yawAmount, float pitchAmount, float rollAmount) {
    // Manual rotation adjustment
    rotation.y += yawAmount;   // Yaw (left/right)
    rotation.x += pitchAmount; // Pitch (up/down)
    rotation.z += rollAmount;  // Roll (twist)
    updateModelMatrix();
}

// animations 
void SceneObject::setRotating(bool enabled, float speed) {
    rotating = enabled;
    rotationSpeed = speed;  // Radians per second
}

void SceneObject::setFloating(bool enabled, float amplitude, float speed) {
    floating = enabled;
    floatAmplitude = amplitude;  // How far up/down to float
    floatSpeed = speed;          // How fast to bob
    if (enabled && !orbiting) {
        basePosition = position; // Remember current position as baseline
    }
}

void SceneObject::setOrbiting(bool enabled, const glm::vec3& center, float radius, float speed) {
    orbiting = enabled;
    orbitCenter = center;
    orbitRadius = radius;
    orbitSpeed = speed;
    if (enabled) {
        // Calculate starting angle from current position
        glm::vec3 offset = position - center;
        orbitTime = atan2(offset.z, offset.x);  // Start at current angle
    }
}

// SCNENE CLASS IMPLEMENTATION
void Scene::addObject(const Model* model, const glm::vec3& position, const glm::vec3& rotation, const glm::vec3& scale) {
    // Add new object to scene with specified transform
    objects.emplace_back(model, position, rotation, scale);
}

void Scene::update(float deltaTime) {
    // Update all objects' animations
    for (auto& obj : objects) {
        obj.update(deltaTime);
    }
}

void Scene::draw(Shader& shader) const {
    // Render all objects using provided shader
    for (const auto& obj : objects) {
        // Set model matrix uniform for this object
        shader.setMat4("model", &obj.modelMatrix[0][0]);
        obj.model->draw();  // Render the mesh
    }
}