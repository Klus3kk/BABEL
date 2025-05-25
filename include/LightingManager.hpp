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

    // Animation properties
    bool flickering = false;
    float flickerSpeed = 5.0f;
    float flickerIntensity = 0.3f;
    float baseIntensity;
    float flickerTime = 0.0f;

    // Movement properties
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

    // Ambient lighting
    glm::vec3 ambientColor = glm::vec3(0.15f, 0.1f, 0.2f); // Slight purple tint
    float ambientStrength = 0.3f;

    // LIGHT STRENGTH CONTROLS
    void setGlobalLightIntensity(float multiplier);
    void setTorchIntensity(float intensity);
    void setAmbientDarkness(float darkness); // 0.0 = pitch black, 1.0 = normal
    void setDramaticMode(bool enabled);

    // Torch-specific controls
    void increaseTorchFlicker();
    void decreaseTorchFlicker();

    // Add lights
    void addPointLight(const PointLight& light);
    void addDirectionalLight(const DirectionalLight& light);

    // Setup specific lighting scenarios for the library
    void setupLibraryLighting(float roomRadius, float roomHeight);

    // Update animated lights
    void update(float deltaTime);

    // Bind all lights to shader
    void bindToShader(Shader& shader) const;

    // Animation controls
    void setLightFlickering(int lightIndex, bool enabled, float speed = 5.0f, float intensity = 0.3f);
    void setLightMoving(int lightIndex, bool enabled, const glm::vec3& center, float radius = 1.0f, float speed = 1.0f);

    // Color and intensity controls
    void setAmbientColor(const glm::vec3& color, float strength);
    void updatePointLightColor(int lightIndex, const glm::vec3& color);
    void updatePointLightIntensity(int lightIndex, float intensity);
};