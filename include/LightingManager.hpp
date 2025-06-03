// Enhanced LightingManager.hpp - Atmospheric Mystical Library Lighting
#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "shader.hpp"

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;

    // Enhanced animation properties
    bool flickering = false;
    float flickerSpeed = 5.0f;
    float flickerIntensity = 0.3f;
    float baseIntensity;
    float flickerTime = 0.0f;

    // Enhanced movement properties
    bool moving = false;
    glm::vec3 moveCenter;
    float moveRadius = 1.0f;
    float moveSpeed = 1.0f;
    float moveTime = 0.0f;
    glm::vec3 basePosition;

    PointLight(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f,
        float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f)
        : position(pos), color(col), intensity(intens), baseIntensity(intens),
        constant(constant), linear(linear), quadratic(quadratic), basePosition(pos) {
    }
};

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;

    DirectionalLight(const glm::vec3& dir, const glm::vec3& col, float intens = 1.0f)
        : direction(dir), color(col), intensity(intens) {
    }
};

class LightingManager {
public:
    std::vector<PointLight> pointLights;
    std::vector<DirectionalLight> directionalLights;

    // Ambient lighting with mystical properties
    glm::vec3 ambientColor = glm::vec3(0.08f, 0.05f, 0.12f); // Deep mystical purple
    float ambientStrength = 0.15f;

    // CORE LIGHTING SETUP
    void setupLibraryLighting(float roomRadius, float roomHeight);
    void addPointLight(const PointLight& light);
    void addDirectionalLight(const DirectionalLight& light);
    void bindToShader(Shader& shader) const;
    void updateTorchPositions(const std::vector<glm::vec3>& torchPositions);
    void setTorchIntensity(float intensity);        // Direct torch control
    void setGlobalLightIntensity(float multiplier); // Scale all lights
    void setAmbientDarkness(float darkness);       // 0.0 = pitch black, 1.0 = bright

    // COLOR AND AMBIENCE
    void setAmbientColor(const glm::vec3& color, float strength);
    void updatePointLightColor(int lightIndex, const glm::vec3& color);
    void updatePointLightIntensity(int lightIndex, float intensity);

    // LEGACY COMPATIBILITY (keep existing methods)
    void setDramaticMode(bool enabled);
};