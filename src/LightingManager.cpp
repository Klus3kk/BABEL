#include "LightingManager.hpp"
#include <cmath>
#include <iostream>
#include <algorithm>

void LightingManager::addPointLight(const PointLight& light) {
    pointLights.push_back(light);
}

void LightingManager::setupLibraryLighting(float roomRadius, float roomHeight) {
    pointLights.clear();

    std::cout << "Setting up atmospheric lighting..." << std::endl;

    // Set warm ambient for cozy stone library atmosphere
    ambientColor = glm::vec3(0.025f, 0.015f, 0.008f); // Warm amber tint
    ambientStrength = 0.12f; // Low but noticeable ambient

    // CENTRAL LAMP - Main atmospheric light source
    PointLight centralLamp(
        glm::vec3(0.0f, 7.0f, 0.0f),          // High center position
        glm::vec3(1.0f, 0.9f, 0.7f),          // Warm white with golden tint
        2.2f,                                  // Good intensity for warmth
        1.0f, 0.09f, 0.032f                   // Attenuation: constant, linear, quadratic
    );
    addPointLight(centralLamp);

    // TORCH LIGHTS - Four moving flame lights
    for (int i = 0; i < 4; i++) {
        // Calculate initial torch positions (updated later by animation system)
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        glm::vec3 torchPos = glm::vec3(
            3.2f * cos(angle) + 1.2f * cos(angle + glm::radians(90.0f)),
            3.0f,  // Height
            3.2f * sin(angle) + 1.2f * sin(angle + glm::radians(90.0f))
        );

        PointLight torchLight(
            torchPos,
            glm::vec3(1.0f, 0.6f, 0.2f),          // Warm orange-amber flame color
            2.0f,                                   // Good intensity for cozy feel
            1.0f, 0.18f, 0.15f                     // Wider spread than central lamp
        );
        addPointLight(torchLight);
    }

    std::cout << "Total lights: " << pointLights.size() << std::endl;
}

void LightingManager::updateTorchPositions(const std::vector<glm::vec3>& torchPositions) {
    // Sync light positions with animated torch objects (skip index 0 which is the central lamp)
    int torchesUpdated = 0;
    for (int i = 0; i < torchPositions.size() && (i + 1) < pointLights.size(); i++) {
        pointLights[i + 1].position = torchPositions[i];  // Update torch light position
        torchesUpdated++;
    }
}

void LightingManager::bindToShader(Shader& shader) const {
    // Send ambient lighting to shader
    shader.setVec3("ambientColor", ambientColor.x, ambientColor.y, ambientColor.z);
    shader.setFloat("ambientStrength", ambientStrength);

    // Send number of lights (clamped to shader maximum)
    int numPointLights = std::min(static_cast<int>(pointLights.size()), 32);
    shader.setInt("numPointLights", numPointLights);

    // Send each point light's properties to shader array
    for (int i = 0; i < numPointLights; i++) {
        std::string base = "pointLights[" + std::to_string(i) + "]";
        const auto& light = pointLights[i];

        // Upload light properties
        shader.setVec3(base + ".position", light.position.x, light.position.y, light.position.z);
        shader.setVec3(base + ".color", light.color.x, light.color.y, light.color.z);
        shader.setFloat(base + ".intensity", light.intensity);
        shader.setFloat(base + ".constant", light.constant);    // Distance attenuation factors
        shader.setFloat(base + ".linear", light.linear);
        shader.setFloat(base + ".quadratic", light.quadratic);
    }
}

void LightingManager::setDramaticMode(bool enabled) {
    if (enabled) {
        std::cout << "DRAMATIC MODE: Warmer, brighter lighting" << std::endl;

        // Make central lamp warmer and brighter
        if (!pointLights.empty()) {
            pointLights[0].color = glm::vec3(1.0f, 0.8f, 0.5f);  // More golden
            pointLights[0].baseIntensity = 3.0f;
            pointLights[0].intensity = 3.0f;
        }

        // Make torches warmer and brighter
        for (int i = 1; i < pointLights.size(); i++) {
            pointLights[i].color = glm::vec3(1.0f, 0.7f, 0.3f);  // Warmer orange
            pointLights[i].baseIntensity = 2.8f;
            pointLights[i].intensity = 2.8f;
        }

        // Increase ambient warmth
        ambientColor = glm::vec3(0.04f, 0.025f, 0.015f);
        ambientStrength = 0.15f;
    }
    else {
        std::cout << "NORMAL MODE: Standard warm lighting" << std::endl;

        // Reset central lamp to normal warm values
        if (!pointLights.empty()) {
            pointLights[0].color = glm::vec3(1.0f, 0.9f, 0.7f);
            pointLights[0].baseIntensity = 2.2f;
            pointLights[0].intensity = 2.2f;
        }

        // Reset torches to normal
        for (int i = 1; i < pointLights.size(); i++) {
            pointLights[i].color = glm::vec3(1.0f, 0.6f, 0.2f);
            pointLights[i].baseIntensity = 2.0f;
            pointLights[i].intensity = 2.0f;
        }

        // Reset ambient to normal
        ambientColor = glm::vec3(0.025f, 0.015f, 0.008f);
        ambientStrength = 0.12f;
    }
}

void LightingManager::setTorchIntensity(float intensity) {
    // Clamp intensity to reasonable range
    intensity = std::max(1.0f, std::min(intensity, 4.0f));

    std::cout << "Setting torch intensity to " << intensity << std::endl;

    // Update all torch lights (skip central lamp at index 0)
    for (int i = 1; i < pointLights.size(); i++) {
        pointLights[i].baseIntensity = intensity;
        pointLights[i].intensity = intensity;
    }
}

void LightingManager::setGlobalLightIntensity(float multiplier) {
    // Scale all lights by multiplier (clamped for safety)
    multiplier = std::max(0.5f, std::min(multiplier, 2.5f));

    for (auto& light : pointLights) {
        float newIntensity = light.baseIntensity * multiplier;
        light.intensity = std::max(0.5f, std::min(newIntensity, 6.0f));  // Final clamp
    }
}

void LightingManager::setAmbientDarkness(float darkness) {
    // Adjust ambient strength based on darkness level
    float baseStrength = 0.12f;
    ambientStrength = std::max(0.02f, std::min(0.25f, baseStrength - darkness * 0.1f));
}

void LightingManager::setAmbientColor(const glm::vec3& color, float strength) {
    ambientColor = color;
    ambientStrength = std::max(0.02f, std::min(strength, 0.3f));  // Clamp strength
}

void LightingManager::updatePointLightColor(int lightIndex, const glm::vec3& color) {
    // Change specific light color (bounds checking)
    if (lightIndex >= 0 && lightIndex < pointLights.size()) {
        pointLights[lightIndex].color = color;
    }
}

void LightingManager::updatePointLightIntensity(int lightIndex, float intensity) {
    // Change specific light intensity (bounds checking)
    if (lightIndex >= 0 && lightIndex < pointLights.size()) {
        intensity = std::max(0.5f, std::min(intensity, 6.0f));  // Clamp
        pointLights[lightIndex].baseIntensity = intensity;
        pointLights[lightIndex].intensity = intensity;
    }
}