#include "scene.hpp"

// SceneObject implementation
SceneObject::SceneObject(const Model* modelPtr, const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scl)
    : model(modelPtr), position(pos), rotation(rot), scale(scl) {
    updateModelMatrix();
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
    if (rotating) {
        // Rotate around Y axis based on rotation speed
        rotation.y += rotationSpeed * deltaTime;

        // Keep rotation angle in reasonable range
        if (rotation.y > 6.28f) // 2π
            rotation.y -= 6.28f;

        updateModelMatrix();
    }
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