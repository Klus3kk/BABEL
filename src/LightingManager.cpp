// Warm LightingManager.cpp - Warmer atmosphere with moving torch lights
#include "LightingManager.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

void LightingManager::addPointLight(const PointLight& light) {
    pointLights.push_back(light);
}

void LightingManager::addDirectionalLight(const DirectionalLight& light) {
    directionalLights.push_back(light);
}

void LightingManager::setupLibraryLighting(float roomRadius, float roomHeight) {
    pointLights.clear();
    directionalLights.clear();

    std::cout << "Setting up WARM atmospheric lighting..." << std::endl;

    // Warmer ambient for cozy stone library
    ambientColor = glm::vec3(0.025f, 0.015f, 0.008f); // Warm amber tint
    ambientStrength = 0.12f; // Slightly higher for warmth

    // 1. CENTRAL LAMP - Warm atmospheric light
    PointLight centralLamp(
        glm::vec3(0.0f, 7.0f, 0.0f),
        glm::vec3(1.0f, 0.9f, 0.7f), // Warm white with golden tint
        2.2f, // Slightly brighter for warmth
        1.0f, 0.09f, 0.032f
    );
    addPointLight(centralLamp);

    // 2. TORCH LIGHTS - Warm moving flames (positions will be updated)
    for (int i = 0; i < 4; i++) {
        // Initial positions - will be updated by updateTorchPositions
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        glm::vec3 torchPos = glm::vec3(
            3.2f * cos(angle) + 1.2f * cos(angle + glm::radians(90.0f)),
            3.0f,
            3.2f * sin(angle) + 1.2f * sin(angle + glm::radians(90.0f))
        );

        PointLight torchLight(
            torchPos,
            glm::vec3(1.0f, 0.6f, 0.2f), // Warm orange-amber flame
            2.0f, // Good intensity for warm atmosphere
            1.0f, 0.18f, 0.15f // Slightly wider spread for warmth
        );
        addPointLight(torchLight);
    }

    std::cout << "LAMP: Warm golden light intensity 2.2" << std::endl;
    std::cout << "TORCHES: Warm amber flames intensity 2.0 (MOVING)" << std::endl;
    std::cout << "AMBIENT: Warm amber atmosphere" << std::endl;
    std::cout << "Total lights: " << pointLights.size() << std::endl;
}

void LightingManager::updateTorchPositions(const std::vector<glm::vec3>& torchPositions) {
    // Update torch light positions (skip lamp at index 0)
    int torchesUpdated = 0;
    for (int i = 0; i < torchPositions.size() && (i + 1) < pointLights.size(); i++) {
        pointLights[i + 1].position = torchPositions[i];
        torchesUpdated++;
    }

    // Debug output only occasionally to avoid spam
    static int updateCount = 0;
    updateCount++;
    if (updateCount % 60 == 0) { // Every 60 updates
        std::cout << "Updated " << torchesUpdated << " torch light positions" << std::endl;
    }
}

void LightingManager::bindToShader(Shader& shader) const {
    shader.setVec3("ambientColor", ambientColor.x, ambientColor.y, ambientColor.z);
    shader.setFloat("ambientStrength", ambientStrength);

    int numPointLights = std::min(static_cast<int>(pointLights.size()), 32);
    shader.setInt("numPointLights", numPointLights);
    shader.setInt("numDirLights", 0); // No directional lights

    for (int i = 0; i < numPointLights; i++) {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        const auto& light = pointLights[i];

        shader.setVec3(base + ".position", light.position.x, light.position.y, light.position.z);
        shader.setVec3(base + ".color", light.color.x, light.color.y, light.color.z);
        shader.setFloat(base + ".intensity", light.intensity);
        shader.setFloat(base + ".constant", light.constant);
        shader.setFloat(base + ".linear", light.linear);
        shader.setFloat(base + ".quadratic", light.quadratic);
    }
}

// WARM INTENSITY CONTROLS

void LightingManager::setDramaticMode(bool enabled) {
    if (enabled) {
        std::cout << "DRAMATIC MODE: Warmer, brighter lighting" << std::endl;

        // LAMP becomes warmer and brighter
        if (!pointLights.empty()) {
            pointLights[0].color = glm::vec3(1.0f, 0.8f, 0.5f); // More golden
            pointLights[0].baseIntensity = 3.0f;
            pointLights[0].intensity = 3.0f;
        }

        // TORCHES become warmer and brighter
        for (int i = 1; i < pointLights.size(); i++) {
            pointLights[i].color = glm::vec3(1.0f, 0.7f, 0.3f); // Warmer orange
            pointLights[i].baseIntensity = 2.8f;
            pointLights[i].intensity = 2.8f;
        }

        // Warmer ambient
        ambientColor = glm::vec3(0.04f, 0.025f, 0.015f);
        ambientStrength = 0.15f;
    }
    else {
        std::cout << "NORMAL MODE: Standard warm lighting" << std::endl;

        // Reset to normal warm values
        if (!pointLights.empty()) {
            pointLights[0].color = glm::vec3(1.0f, 0.9f, 0.7f);
            pointLights[0].baseIntensity = 2.2f;
            pointLights[0].intensity = 2.2f;
        }

        for (int i = 1; i < pointLights.size(); i++) {
            pointLights[i].color = glm::vec3(1.0f, 0.6f, 0.2f);
            pointLights[i].baseIntensity = 2.0f;
            pointLights[i].intensity = 2.0f;
        }

        ambientColor = glm::vec3(0.025f, 0.015f, 0.008f);
        ambientStrength = 0.12f;
    }
}

void LightingManager::setTorchIntensity(float intensity) {
    intensity = std::max(1.0f, std::min(intensity, 4.0f));

    std::cout << "Setting torch intensity to " << intensity << std::endl;

    for (int i = 1; i < pointLights.size(); i++) {
        pointLights[i].baseIntensity = intensity;
        pointLights[i].intensity = intensity;
    }
}

void LightingManager::setGlobalLightIntensity(float multiplier) {
    multiplier = std::max(0.5f, std::min(multiplier, 2.5f));

    for (auto& light : pointLights) {
        float newIntensity = light.baseIntensity * multiplier;
        light.intensity = std::max(0.5f, std::min(newIntensity, 6.0f));
    }
}

void LightingManager::setAmbientDarkness(float darkness) {
    // Keep warm ambient
    float baseStrength = 0.12f;
    ambientStrength = std::max(0.02f, std::min(0.25f, baseStrength - darkness * 0.1f));
}

void LightingManager::setAmbientColor(const glm::vec3& color, float strength) {
    ambientColor = color;
    ambientStrength = std::max(0.02f, std::min(strength, 0.3f));
}

void LightingManager::updatePointLightColor(int lightIndex, const glm::vec3& color) {
    if (lightIndex >= 0 && lightIndex < pointLights.size()) {
        pointLights[lightIndex].color = color;
    }
}

void LightingManager::updatePointLightIntensity(int lightIndex, float intensity) {
    if (lightIndex >= 0 && lightIndex < pointLights.size()) {
        intensity = std::max(0.5f, std::min(intensity, 6.0f));
        pointLights[lightIndex].baseIntensity = intensity;
        pointLights[lightIndex].intensity = intensity;
    }
}