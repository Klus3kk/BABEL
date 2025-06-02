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

// Replace the setupLibraryLighting function in src/LightingManager.cpp
void LightingManager::setupLibraryLighting(float roomRadius, float roomHeight) {
    pointLights.clear();
    directionalLights.clear();

    std::cout << "🕯️ Setting up simple library lighting..." << std::endl;

    // MODERATE AMBIENT - not too bright, not too dark
    ambientColor = glm::vec3(0.15f, 0.12f, 0.18f); // Subtle purple ambient
    ambientStrength = 0.3f; // Reasonable ambient

    // 1. CENTRAL LAMP - Main light source
    PointLight centralLamp(
        glm::vec3(0.0f, roomHeight + 0.5f, 0.0f),
        glm::vec3(1.0f, 0.9f, 0.7f), // Warm white
        2.0f, // Moderate intensity
        1.0f, 0.09f, 0.032f // Standard attenuation
    );
    centralLamp.flickering = true;
    centralLamp.flickerSpeed = 2.0f;
    centralLamp.flickerIntensity = 0.1f;
    addPointLight(centralLamp);

    // 2. TORCH LIGHTS - One for each torch (4 total)
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        glm::vec3 torchPos = glm::vec3(
            3.2f * cos(angle) + 1.2f * cos(angle + glm::radians(90.0f)),
            3.0f,
            3.2f * sin(angle) + 1.2f * sin(angle + glm::radians(90.0f))
        );

        PointLight torchLight(
            torchPos,
            glm::vec3(1.0f, 0.6f, 0.2f), // Orange flame color
            1.5f, // Moderate torch brightness
            1.0f, 0.22f, 0.20f // Localized light
        );
        torchLight.flickering = true;
        torchLight.flickerSpeed = 5.0f + (i * 0.8f);
        torchLight.flickerIntensity = 0.3f;
        addPointLight(torchLight);
    }

    // 3. SIMPLE DIRECTIONAL FILL
    DirectionalLight skylight(
        glm::vec3(0.2f, -1.0f, 0.3f), // Slight angle from above
        glm::vec3(0.6f, 0.6f, 0.8f), // Cool blue-white
        0.4f // Gentle fill light
    );
    addDirectionalLight(skylight);

    std::cout << "💡 Simple lighting setup complete!" << std::endl;
    std::cout << "   📊 Point lights: " << pointLights.size() << std::endl;
    std::cout << "   📊 Directional lights: " << directionalLights.size() << std::endl;
}

void LightingManager::update(float deltaTime) {
    // Update animated point lights
    for (auto& light : pointLights) {
        // Handle flickering with more natural variation
        if (light.flickering) {
            light.flickerTime += light.flickerSpeed * deltaTime;

            // Use multiple sine waves for more natural flicker
            float flicker1 = sin(light.flickerTime) * 0.5f + 0.5f;
            float flicker2 = sin(light.flickerTime * 1.7f + 0.5f) * 0.3f + 0.7f;
            float flicker3 = sin(light.flickerTime * 2.3f + 1.2f) * 0.2f + 0.8f;

            float combinedFlicker = (flicker1 + flicker2 + flicker3) / 3.0f;
            light.intensity = light.baseIntensity * (1.0f - light.flickerIntensity + light.flickerIntensity * combinedFlicker);
        }

        // Handle smooth orbital movement
        if (light.moving) {
            light.moveTime += light.moveSpeed * deltaTime;
            float x = light.moveCenter.x + light.moveRadius * cos(light.moveTime);
            float z = light.moveCenter.z + light.moveRadius * sin(light.moveTime);
            // Add subtle vertical bobbing
            float y = light.moveCenter.y + sin(light.moveTime * 2.0f) * 0.1f;
            light.position = glm::vec3(x, y, z);
        }
    }
}

void LightingManager::bindToShader(Shader& shader) const {
    // Bind ambient lighting
    shader.setVec3("ambientColor", ambientColor.x, ambientColor.y, ambientColor.z);
    shader.setFloat("ambientStrength", ambientStrength);

    // Bind number of lights (clamped to shader limits)
    int numPointLights = std::min(static_cast<int>(pointLights.size()), 8);
    int numDirLights = std::min(static_cast<int>(directionalLights.size()), 2);

    shader.setInt("numPointLights", numPointLights);
    shader.setInt("numDirLights", numDirLights);

    // Bind point lights
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

    // Bind directional lights
    for (int i = 0; i < numDirLights; i++) {
        std::string base = "dirLights[" + std::to_string(i) + "]";
        const auto& light = directionalLights[i];

        shader.setVec3(base + ".direction", light.direction.x, light.direction.y, light.direction.z);
        shader.setVec3(base + ".color", light.color.x, light.color.y, light.color.z);
        shader.setFloat(base + ".intensity", light.intensity);
    }
}

void LightingManager::setLightFlickering(int lightIndex, bool enabled, float speed, float intensity) {
    if (lightIndex >= 0 && lightIndex < static_cast<int>(pointLights.size())) {
        pointLights[lightIndex].flickering = enabled;
        pointLights[lightIndex].flickerSpeed = speed;
        pointLights[lightIndex].flickerIntensity = intensity;
    }
}

void LightingManager::setLightMoving(int lightIndex, bool enabled, const glm::vec3& center, float radius, float speed) {
    if (lightIndex >= 0 && lightIndex < static_cast<int>(pointLights.size())) {
        pointLights[lightIndex].moving = enabled;
        pointLights[lightIndex].moveCenter = center;
        pointLights[lightIndex].moveRadius = radius;
        pointLights[lightIndex].moveSpeed = speed;
        pointLights[lightIndex].basePosition = center;
    }
}

void LightingManager::setAmbientColor(const glm::vec3& color, float strength) {
    ambientColor = color;
    ambientStrength = strength;
}

void LightingManager::updatePointLightColor(int lightIndex, const glm::vec3& color) {
    if (lightIndex >= 0 && lightIndex < static_cast<int>(pointLights.size())) {
        pointLights[lightIndex].color = color;
    }
}

void LightingManager::updatePointLightIntensity(int lightIndex, float intensity) {
    if (lightIndex >= 0 && lightIndex < static_cast<int>(pointLights.size())) {
        pointLights[lightIndex].baseIntensity = intensity;
        pointLights[lightIndex].intensity = intensity;
    }
}

// ENHANCED LIGHTING CONTROL METHODS
void LightingManager::setGlobalLightIntensity(float multiplier) {
    for (auto& light : pointLights) {
        light.baseIntensity *= multiplier;
        light.intensity = light.baseIntensity;
    }
    for (auto& light : directionalLights) {
        light.intensity *= multiplier;
    }
    std::cout << "🔆 Global light intensity adjusted by " << multiplier << "x" << std::endl;
}

void LightingManager::setTorchIntensity(float intensity) {
    // Wall torches (indices 1-8) and column torches (indices 9-16)
    for (int i = 1; i < std::min(17, static_cast<int>(pointLights.size())); i++) {
        pointLights[i].baseIntensity = intensity;
        pointLights[i].intensity = intensity;
    }
    std::cout << "🔥 Torch intensity set to " << intensity << std::endl;
}

void LightingManager::setAmbientDarkness(float darkness) {
    // darkness: 0.0 = pitch black, 1.0 = normal lighting
    float ambientLevel = 0.15f * darkness; // Max 0.15 for good portal visibility
    ambientStrength = ambientLevel;

    // Adjust ambient color warmth based on darkness
    float warmth = 0.08f * darkness;
    ambientColor = glm::vec3(warmth, warmth * 0.75f, warmth * 1.5f);

    std::cout << "🌙 Ambient darkness set to " << (1.0f - darkness) * 100.0f << "%" << std::endl;
}

void LightingManager::setDramaticMode(bool enabled) {
    if (enabled) {
        std::cout << "🎭 DRAMATIC MODE: Enabling cinematic lighting" << std::endl;
        setAmbientDarkness(0.2f); // Very dark ambient
        setTorchIntensity(3.5f);   // Brighter torches for high contrast

        // Increase flicker for dramatic effect
        for (auto& light : pointLights) {
            if (light.flickering) {
                light.flickerIntensity = std::min(0.7f, light.flickerIntensity * 1.5f);
                light.flickerSpeed *= 1.3f;
            }
        }
    }
    else {
        std::cout << "✨ STANDARD MODE: Balanced atmospheric lighting" << std::endl;
        setAmbientDarkness(0.5f); // Moderate ambient
        setTorchIntensity(2.5f);   // Comfortable torch brightness

        // Reset flicker to normal
        for (auto& light : pointLights) {
            if (light.flickering) {
                light.flickerIntensity = std::max(0.2f, light.flickerIntensity / 1.5f);
                light.flickerSpeed /= 1.3f;
            }
        }
    }
}

void LightingManager::increaseTorchFlicker() {
    for (int i = 1; i < std::min(17, static_cast<int>(pointLights.size())); i++) {
        pointLights[i].flickerIntensity = std::min(0.8f, pointLights[i].flickerIntensity + 0.1f);
        pointLights[i].flickerSpeed += 0.5f;
    }
    std::cout << "🔥 Increased torch flicker intensity" << std::endl;
}

void LightingManager::decreaseTorchFlicker() {
    for (int i = 1; i < std::min(17, static_cast<int>(pointLights.size())); i++) {
        pointLights[i].flickerIntensity = std::max(0.1f, pointLights[i].flickerIntensity - 0.1f);
        pointLights[i].flickerSpeed = std::max(1.0f, pointLights[i].flickerSpeed - 0.5f);
    }
    std::cout << "🔥 Decreased torch flicker intensity" << std::endl;
}