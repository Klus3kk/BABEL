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

    std::cout << "🕯️ Setting up enhanced library lighting system..." << std::endl;

    // ATMOSPHERIC AMBIENT LIGHTING
    ambientColor = glm::vec3(0.08f, 0.06f, 0.12f); // Warm purple ambient
    ambientStrength = 0.15f; // Sufficient for portal visibility

    // 1. CENTRAL MYSTICAL LAMP - Primary light source
    PointLight centralLamp(
        glm::vec3(0.0f, roomHeight + 0.8f, 0.0f),
        glm::vec3(1.0f, 0.9f, 0.7f), // Warm golden light
        3.5f, // Strong central illumination
        1.0f, 0.045f, 0.0075f // Wide spread
    );
    centralLamp.flickering = true;
    centralLamp.flickerSpeed = 2.0f;
    centralLamp.flickerIntensity = 0.15f; // Gentle flickering
    addPointLight(centralLamp);

    // 2. WALL TORCHES - Perimeter atmospheric lighting (8 torches)
    std::cout << "🔥 Installing wall torches..." << std::endl;
    for (int i = 0; i < 8; i++) {
        float angle = glm::radians(45.0f * static_cast<float>(i));
        glm::vec3 torchPosition = glm::vec3(
            roomRadius * 0.92f * cos(angle),
            2.0f, // Match torch model height
            roomRadius * 0.92f * sin(angle)
        );

        PointLight wallTorch(
            torchPosition,
            glm::vec3(1.0f, 0.5f, 0.1f), // Orange flame color
            2.2f, // Medium intensity
            1.0f, 0.22f, 0.20f // Medium range
        );
        wallTorch.flickering = true;
        wallTorch.flickerSpeed = 6.0f + (i * 0.8f); // Varied flicker speeds
        wallTorch.flickerIntensity = 0.4f + (i % 3) * 0.1f; // Varied intensities
        addPointLight(wallTorch);
    }

    // 3. COLUMN TORCHES - Strategic placement (8 total, 2 per column)
    std::cout << "🏛️ Setting up column torches..." << std::endl;
    for (int i = 0; i < 4; i++) {
        float columnAngle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 columnCenter = glm::vec3(3.2f * cos(columnAngle), 0.0f, 3.2f * sin(columnAngle));

        // Two torches per column at different heights and positions
        for (int j = 0; j < 2; j++) {
            float offsetAngle = columnAngle + (j == 0 ? glm::radians(45.0f) : glm::radians(-45.0f));
            glm::vec3 torchPosition = columnCenter + glm::vec3(
                1.2f * cos(offsetAngle),
                2.8f + j * 0.5f, // Staggered heights
                1.2f * sin(offsetAngle)
            );

            PointLight columnTorch(
                torchPosition,
                glm::vec3(1.0f, 0.4f, 0.05f), // Deep orange-red
                2.5f, // Stronger for dramatic effect
                1.0f, 0.14f, 0.07f // Focused range
            );
            columnTorch.flickering = true;
            columnTorch.flickerSpeed = 5.0f + (i * 1.5f) + (j * 0.7f);
            columnTorch.flickerIntensity = 0.5f + (j * 0.1f);
            addPointLight(columnTorch);
        }
    }

    // 4. BOOKSHELF READING LIGHTS - Warm study lighting (4 torches)
    std::cout << "📚 Installing bookshelf reading lights..." << std::endl;
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        glm::vec3 shelfPosition = glm::vec3(
            roomRadius * 0.65f * cos(angle),
            4.5f, // High above bookshelves
            roomRadius * 0.65f * sin(angle)
        );

        PointLight readingLight(
            shelfPosition,
            glm::vec3(0.9f, 0.7f, 0.4f), // Warm reading light
            1.8f, // Moderate intensity for comfortable reading
            1.0f, 0.35f, 0.44f // Localized range
        );
        readingLight.flickering = true;
        readingLight.flickerSpeed = 3.0f + (i * 0.5f); // Gentle flicker
        readingLight.flickerIntensity = 0.2f; // Minimal flicker for reading comfort
        addPointLight(readingLight);
    }

    // 5. PORTAL ACCENT LIGHTS - Mystical blue lights near portals (4 lights)
    std::cout << "🌀 Creating portal mystical lighting..." << std::endl;
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 portalPosition = glm::vec3(
            roomRadius * 0.75f * cos(angle),
            2.5f,
            roomRadius * 0.75f * sin(angle)
        );

        PointLight portalLight(
            portalPosition,
            glm::vec3(0.3f, 0.5f, 1.0f), // Mystical blue
            1.2f, // Subtle accent lighting
            1.0f, 0.45f, 0.75f // Short range
        );
        portalLight.moving = true;
        portalLight.moveSpeed = 0.8f + (i * 0.2f);
        portalLight.moveRadius = 0.5f;
        portalLight.moveCenter = portalPosition;
        portalLight.flickering = true;
        portalLight.flickerSpeed = 4.0f;
        portalLight.flickerIntensity = 0.3f;
        addPointLight(portalLight);
    }

    // 6. FLOATING BOOK ACCENT LIGHTS - Subtle magical glow (6 lights)
    std::cout << "✨ Adding magical book aura lights..." << std::endl;
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * static_cast<float>(i));
        glm::vec3 bookLightPos = glm::vec3(
            2.5f * cos(angle),
            3.5f + sin(angle * 2.0f) * 0.5f,
            2.5f * sin(angle)
        );

        PointLight bookLight(
            bookLightPos,
            glm::vec3(0.8f, 0.6f, 1.0f), // Soft purple-white
            0.8f, // Very subtle
            1.0f, 0.8f, 1.6f // Very short range
        );
        bookLight.moving = true;
        bookLight.moveSpeed = 0.3f;
        bookLight.moveRadius = 0.3f;
        bookLight.moveCenter = bookLightPos;
        bookLight.flickering = true;
        bookLight.flickerSpeed = 8.0f;
        bookLight.flickerIntensity = 0.6f;
        addPointLight(bookLight);
    }

    // 7. ATMOSPHERIC DIRECTIONAL LIGHTING
    std::cout << "🌙 Setting up atmospheric directional lights..." << std::endl;

    // Soft moonlight from above
    DirectionalLight moonlight(
        glm::vec3(0.2f, -1.0f, 0.3f),
        glm::vec3(0.4f, 0.4f, 0.6f), // Cool blue moonlight
        0.3f // Subtle ambient enhancement
    );
    addDirectionalLight(moonlight);

    // Warm candlelight atmosphere from sides
    DirectionalLight warmAmbient(
        glm::vec3(1.0f, -0.3f, 0.5f),
        glm::vec3(1.0f, 0.7f, 0.4f), // Warm orange
        0.2f // Very subtle
    );
    addDirectionalLight(warmAmbient);

    std::cout << "💡 Library lighting setup complete!" << std::endl;
    std::cout << "   📊 Total point lights: " << pointLights.size() << std::endl;
    std::cout << "   📊 Total directional lights: " << directionalLights.size() << std::endl;
    std::cout << "   🔥 Flickering lights: " << std::count_if(pointLights.begin(), pointLights.end(),
        [](const PointLight& light) { return light.flickering; }) << std::endl;
    std::cout << "   🌊 Moving lights: " << std::count_if(pointLights.begin(), pointLights.end(),
        [](const PointLight& light) { return light.moving; }) << std::endl;
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