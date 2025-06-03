#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "shader.hpp"

// Simplified point light structure - removed all unused animation properties
struct PointLight {
    glm::vec3 position;      // Light position in world space
    glm::vec3 color;         // Light color (RGB)
    float intensity;         // Light brightness
    float constant;          // Distance attenuation factors
    float linear;            // (used in 1/(constant + linear*d + quadratic*d²))
    float quadratic;
    float baseIntensity;     // Original intensity before modifications (used for drama mode)

    // Constructor with reasonable defaults
    PointLight(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f,
        float constant = 1.0f, float linear = 0.09f, float quadratic = 0.032f)
        : position(pos), color(col), intensity(intens), baseIntensity(intens),
        constant(constant), linear(linear), quadratic(quadratic) {
    }
};

class LightingManager {
public:
    std::vector<PointLight> pointLights;           // All point lights in scene

    // Global ambient lighting - fixed to actual values used
    glm::vec3 ambientColor = glm::vec3(0.025f, 0.015f, 0.008f); // Warm amber tint
    float ambientStrength = 0.12f;

    // Main setup and management
    void setupLibraryLighting(float roomRadius, float roomHeight); // Create initial lighting setup
    void addPointLight(const PointLight& light);                   // Add new point light
    void bindToShader(Shader& shader) const;                      // Upload all lights to shader
    void updateTorchPositions(const std::vector<glm::vec3>& torchPositions); // Sync lights with animated objects

    // Lighting controls
    void setTorchIntensity(float intensity);        // Control torch brightness
    void setGlobalLightIntensity(float multiplier); // Scale all lights
    void setAmbientDarkness(float darkness);       // Control ambient level

    // Color and individual light controls
    void setAmbientColor(const glm::vec3& color, float strength);
    void updatePointLightColor(int lightIndex, const glm::vec3& color);
    void updatePointLightIntensity(int lightIndex, float intensity);

    // Preset modes
    void setDramaticMode(bool enabled); // Toggle warmer/brighter lighting
};