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
    // Clear existing lights
    pointLights.clear();
    directionalLights.clear();

    std::cout << "Setting up DARK LIBRARY lighting with torches..." << std::endl;

    // DARK ATMOSPHERE SETTINGS
    ambientColor = glm::vec3(0.02f, 0.01f, 0.05f); // Very dark purple
    ambientStrength = 0.05f; // Minimal ambient light

    // 1. Central lamp (dimmed)
    PointLight dimCentralLight(
        glm::vec3(0.0f, roomHeight + 0.5f, 0.0f),
        glm::vec3(0.3f, 0.2f, 0.1f), // Dim warm glow
        0.8f, // Much lower intensity
        1.0f, 0.09f, 0.032f
    );
    dimCentralLight.flickering = true;
    dimCentralLight.flickerSpeed = 3.0f;
    dimCentralLight.flickerIntensity = 0.3f;
    addPointLight(dimCentralLight);

    // 2. WALL TORCHES - Match the wall torch positions
    for (int i = 0; i < 8; i++) {
        float angle = glm::radians(45.0f * static_cast<float>(i));
        glm::vec3 torchLightPos = glm::vec3(
            roomRadius * 0.9f * cos(angle),
            2.3f, // Slightly above torch model
            roomRadius * 0.9f * sin(angle)
        );

        PointLight wallTorch(
            torchLightPos,
            glm::vec3(1.0f, 0.4f, 0.05f), // Orange flame color
            1.8f, // Medium intensity for wall torches
            1.0f, 0.14f, 0.07f
        );
        wallTorch.flickering = true;
        wallTorch.flickerSpeed = 8.0f + (i * 1.5f);
        wallTorch.flickerIntensity = 0.5f;
        addPointLight(wallTorch);
    }

    // 3. COLUMN TORCHES - Two per column (matching the model positions)
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 columnCenter = glm::vec3(3.0f * cos(angle), 0.0f, 3.0f * sin(angle));
        float offsetDistance = 1.0f;

        // First torch on each column
        glm::vec3 torchLight1 = glm::vec3(
            columnCenter.x + offsetDistance * cos(angle),
            2.9f, // Slightly above torch model
            columnCenter.z + offsetDistance * sin(angle)
        );

        PointLight columnTorch1(
            torchLight1,
            glm::vec3(1.0f, 0.3f, 0.0f), // Deep orange-red
            2.0f, // Stronger for column torches
            1.0f, 0.22f, 0.20f
        );
        columnTorch1.flickering = true;
        columnTorch1.flickerSpeed = 6.0f + (i * 2.0f);
        columnTorch1.flickerIntensity = 0.6f;
        addPointLight(columnTorch1);

        // Second torch on opposite side of column
        glm::vec3 torchLight2 = glm::vec3(
            columnCenter.x - offsetDistance * cos(angle),
            2.9f,
            columnCenter.z - offsetDistance * sin(angle)
        );

        PointLight columnTorch2(
            torchLight2,
            glm::vec3(1.0f, 0.3f, 0.0f), // Deep orange-red
            2.0f,
            1.0f, 0.22f, 0.20f
        );
        columnTorch2.flickering = true;
        columnTorch2.flickerSpeed = 7.0f + (i * 1.8f); // Slightly different speed
        columnTorch2.flickerIntensity = 0.6f;
        addPointLight(columnTorch2);
    }

    // 4. BOOKSHELF READING LIGHTS - Match the bookshelf torch positions
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        float x = roomRadius * 0.7f * cos(angle);
        float z = roomRadius * 0.7f * sin(angle);

        PointLight readingLight(
            glm::vec3(x, 4.3f, z), // Above bookshelf torch
            glm::vec3(0.8f, 0.6f, 0.3f), // Warm reading light
            1.2f, // Moderate intensity for reading
            1.0f, 0.35f, 0.44f // Closer range
        );
        readingLight.flickering = true;
        readingLight.flickerSpeed = 4.0f + (i * 1.0f);
        readingLight.flickerIntensity = 0.3f; // Less flicker for reading
        addPointLight(readingLight);
    }

    // 5. MYSTICAL PORTAL LIGHTS - Keep existing
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 portalPos = glm::vec3(roomRadius * 0.8f * cos(angle), 1.0f, roomRadius * 0.8f * sin(angle));

        PointLight portalLight(
            portalPos,
            glm::vec3(0.1f, 0.05f, 0.4f), // Deep blue mystical
            0.6f, // Very dim
            1.0f, 0.35f, 0.44f
        );
        portalLight.moving = true;
        portalLight.moveSpeed = 0.5f;
        portalLight.moveRadius = 0.3f;
        portalLight.moveCenter = portalPos;
        addPointLight(portalLight);
    }

    // 6. Subtle moonlight
    DirectionalLight dimMoonlight(
        glm::vec3(0.3f, -1.0f, 0.2f),
        glm::vec3(0.05f, 0.05f, 0.1f),
        0.1f
    );
    addDirectionalLight(dimMoonlight);

    std::cout << "DARK LIBRARY setup complete!" << std::endl;
    std::cout << "Total lights: " << pointLights.size() << std::endl;
}

void LightingManager::update(float deltaTime) {
    // Update animated point lights
    for (auto& light : pointLights) {
        // Handle flickering
        if (light.flickering) {
            light.flickerTime += light.flickerSpeed * deltaTime;
            float flicker = sin(light.flickerTime) * 0.5f + 0.5f; // 0 to 1
            light.intensity = light.baseIntensity * (1.0f - light.flickerIntensity + light.flickerIntensity * flicker);
        }

        // Handle movement
        if (light.moving) {
            light.moveTime += light.moveSpeed * deltaTime;
            float x = light.moveCenter.x + light.moveRadius * cos(light.moveTime);
            float z = light.moveCenter.z + light.moveRadius * sin(light.moveTime);
            light.position = glm::vec3(x, light.position.y, z);
        }
    }
}

void LightingManager::bindToShader(Shader& shader) const {
    // Bind ambient lighting
    shader.setVec3("ambientColor", ambientColor.x, ambientColor.y, ambientColor.z);
    shader.setFloat("ambientStrength", ambientStrength);

    // Bind number of lights
    shader.setInt("numPointLights", static_cast<int>(pointLights.size()));
    shader.setInt("numDirLights", static_cast<int>(directionalLights.size()));

    // Bind point lights
    for (size_t i = 0; i < pointLights.size() && i < 8; i++) {
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
    for (size_t i = 0; i < directionalLights.size() && i < 2; i++) {
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

// LIGHT STRENGTH CONTROL METHODS
void LightingManager::setGlobalLightIntensity(float multiplier) {
    for (auto& light : pointLights) {
        light.baseIntensity *= multiplier;
        light.intensity = light.baseIntensity;
    }
    for (auto& light : directionalLights) {
        light.intensity *= multiplier;
    }
}

void LightingManager::setTorchIntensity(float intensity) {
    // Assuming first 13 lights are torches (1 central + 8 wall + 4 column)
    for (int i = 1; i < std::min(13, static_cast<int>(pointLights.size())); i++) {
        pointLights[i].baseIntensity = intensity;
        pointLights[i].intensity = intensity;
    }
}

void LightingManager::setAmbientDarkness(float darkness) {
    // darkness: 0.0 = pitch black, 1.0 = normal lighting
    float ambientLevel = 0.05f * darkness; // Max 0.05 for dark atmosphere
    ambientStrength = ambientLevel;

    // Adjust ambient color darkness
    ambientColor = glm::vec3(0.02f * darkness, 0.01f * darkness, 0.05f * darkness);
}

void LightingManager::setDramaticMode(bool enabled) {
    if (enabled) {
        // Ultra dramatic - almost no ambient
        setAmbientDarkness(0.2f);
        setTorchIntensity(2.5f); // Brighter torches for contrast
    }
    else {
        // Normal dark mode
        setAmbientDarkness(0.5f);
        setTorchIntensity(2.0f);
    }
}

void LightingManager::increaseTorchFlicker() {
    for (int i = 1; i < std::min(13, static_cast<int>(pointLights.size())); i++) {
        pointLights[i].flickerIntensity = std::min(0.8f, pointLights[i].flickerIntensity + 0.1f);
        pointLights[i].flickerSpeed += 1.0f;
    }
}

void LightingManager::decreaseTorchFlicker() {
    for (int i = 1; i < std::min(13, static_cast<int>(pointLights.size())); i++) {
        pointLights[i].flickerIntensity = std::max(0.1f, pointLights[i].flickerIntensity - 0.1f);
        pointLights[i].flickerSpeed = std::max(2.0f, pointLights[i].flickerSpeed - 1.0f);
    }
}