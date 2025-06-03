// NO FLICKERING LightingManager.cpp - ONLY steady lights with strong intensity controls
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

    std::cout << "Setting up STEADY lamp and torch lighting..." << std::endl;

    // PURE DARKNESS - only light sources illuminate
    ambientColor = glm::vec3(0.0f, 0.0f, 0.0f);
    ambientStrength = 0.0f;

    // 1. CENTRAL LAMP - Strong steady light
    PointLight centralLamp(
        glm::vec3(0.0f, 7.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f), // White light
        5.0f, // MUCH STRONGER
        1.0f, 0.09f, 0.032f
    );
    addPointLight(centralLamp);

    // 2. TORCH LIGHTS - Strong steady flames
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        glm::vec3 torchPos = glm::vec3(
            3.2f * cos(angle) + 1.2f * cos(angle + glm::radians(90.0f)),
            3.0f,
            3.2f * sin(angle) + 1.2f * sin(angle + glm::radians(90.0f))
        );

        PointLight torchLight(
            torchPos,
            glm::vec3(1.0f, 0.5f, 0.0f), // Orange
            10.0f, // MUCH STRONGER
            1.0f, 0.22f, 0.20f
        );
        addPointLight(torchLight);
    }

    std::cout << "LAMP: White light intensity 30.0" << std::endl;
    std::cout << "TORCHES: Orange light intensity 20.0" << std::endl;
    std::cout << "AMBIENT: ZERO" << std::endl;
    std::cout << "Total lights: " << pointLights.size() << std::endl;
}

void LightingManager::updateTorchPositions(const std::vector<glm::vec3>& torchPositions) {
    std::cout << "UPDATING " << torchPositions.size() << " torch light positions:" << std::endl;

    // Update torch light positions (skip lamp at index 0)
    for (int i = 0; i < torchPositions.size() && (i + 1) < pointLights.size(); i++) {
        glm::vec3 oldPos = pointLights[i + 1].position;
        pointLights[i + 1].position = torchPositions[i];

        std::cout << "Torch " << i << " light: (" << oldPos.x << "," << oldPos.y << "," << oldPos.z
            << ") -> (" << torchPositions[i].x << "," << torchPositions[i].y << "," << torchPositions[i].z << ")" << std::endl;
    }
    std::cout << "Torch lights updated!" << std::endl;
}

void LightingManager::bindToShader(Shader& shader) const {
    shader.setVec3("ambientColor", ambientColor.x, ambientColor.y, ambientColor.z);
    shader.setFloat("ambientStrength", ambientStrength);

    int numPointLights = std::min(static_cast<int>(pointLights.size()), 32);
    shader.setInt("numPointLights", numPointLights);
    shader.setInt("numDirLights", 0); // NO directional lights

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

// STRONG INTENSITY CONTROLS

void LightingManager::setDramaticMode(bool enabled) {
    if (enabled) {
        std::cout << "DRAMATIC MODE: Making lights MUCH brighter" << std::endl;

        // LAMP becomes very bright
        if (!pointLights.empty()) {
            pointLights[0].baseIntensity = 60.0f;
            pointLights[0].intensity = 60.0f;
        }

        // TORCHES become very bright
        for (int i = 1; i < pointLights.size(); i++) {
            pointLights[i].baseIntensity = 40.0f;
            pointLights[i].intensity = 40.0f;
        }
    }
    else {
        std::cout << "NORMAL MODE: Standard intensities" << std::endl;

        // Reset to normal
        if (!pointLights.empty()) {
            pointLights[0].baseIntensity = 30.0f;
            pointLights[0].intensity = 30.0f;
        }

        for (int i = 1; i < pointLights.size(); i++) {
            pointLights[i].baseIntensity = 20.0f;
            pointLights[i].intensity = 20.0f;
        }
    }
}

void LightingManager::setTorchIntensity(float intensity) {
    std::cout << "Setting torch intensity to " << intensity << std::endl;

    for (int i = 1; i < pointLights.size(); i++) {
        pointLights[i].baseIntensity = intensity;
        pointLights[i].intensity = intensity;
    }
}

void LightingManager::setGlobalLightIntensity(float multiplier) {
    for (auto& light : pointLights) {
        light.baseIntensity *= multiplier;
        light.intensity = light.baseIntensity;
    }
}

// STUB METHODS - NO FLICKERING BULLSHIT
void LightingManager::setAmbientDarkness(float darkness) {
    ambientStrength = 0.0f; // ALWAYS ZERO
}

void LightingManager::setAmbientColor(const glm::vec3& color, float strength) {
    ambientColor = glm::vec3(0.0f); // ALWAYS BLACK
    ambientStrength = 0.0f; // ALWAYS ZERO
}

void LightingManager::updatePointLightColor(int lightIndex, const glm::vec3& color) {
    if (lightIndex >= 0 && lightIndex < pointLights.size()) {
        pointLights[lightIndex].color = color;
    }
}

void LightingManager::updatePointLightIntensity(int lightIndex, float intensity) {
    if (lightIndex >= 0 && lightIndex < pointLights.size()) {
        pointLights[lightIndex].baseIntensity = intensity;
        pointLights[lightIndex].intensity = intensity;
    }
}