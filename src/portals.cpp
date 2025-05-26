#include "portals.hpp"
#include <iostream>
#include <cmath>

PortalSystem::PortalSystem() {
    // Create room variations for infinite diversity
    roomVariations = {
        {glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 30.0f},     // Normal room
        {glm::vec3(1.2f, 0.8f, 0.6f), 1.1f, 35.0f},     // Warm, larger
        {glm::vec3(0.6f, 0.8f, 1.2f), 0.9f, 25.0f},     // Cool, smaller
        {glm::vec3(0.8f, 1.2f, 0.8f), 1.3f, 40.0f},     // Green, much larger
        {glm::vec3(1.1f, 0.9f, 1.1f), 0.8f, 20.0f},     // Purple, smaller
        {glm::vec3(1.0f, 1.0f, 0.7f), 1.0f, 30.0f},     // Yellow tint
        {glm::vec3(0.9f, 0.7f, 1.0f), 1.2f, 45.0f},     // Pink, larger
        {glm::vec3(0.7f, 1.0f, 1.0f), 0.7f, 15.0f}      // Cyan, smallest
    };
}

PortalSystem::~PortalSystem() {
    cleanup();
}

void PortalSystem::initialize(float roomRadius, float roomHeight) {
    portals.clear();

    std::cout << "Setting up recursive portal system..." << std::endl;

    // Create 4 portals - each leads to a room in a specific direction
    for (int i = 0; i < 4; i++) {
        Portal portal;
        portal.portalId = i;

        float angle = glm::radians(90.0f * static_cast<float>(i));

        // Portal position (at room edges)
        portal.position = glm::vec3(
            roomRadius * 0.8f * cos(angle),
            roomHeight * 0.3f,
            roomRadius * 0.8f * sin(angle)
        );

        // Portal normal (face direction)
        portal.normal = glm::vec3(-cos(angle), 0.0f, -sin(angle)); // Face inward

        // Destination position (next room center)
        float roomDistance = roomVariations[i % roomVariations.size()].roomOffset;
        portal.destinationPos = glm::vec3(
            roomDistance * cos(angle),
            0.0f,
            roomDistance * sin(angle)
        );

        // Destination normal (opposite direction)
        portal.destinationNormal = glm::vec3(cos(angle), 0.0f, sin(angle));

        // Create framebuffer for this portal
        glGenFramebuffers(1, &portal.framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);

        // Color texture
        glGenTextures(1, &portal.colorTexture);
        glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Depth texture
        glGenTextures(1, &portal.depthTexture);
        glBindTexture(GL_TEXTURE_2D, portal.depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureSize, textureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, portal.colorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, portal.depthTexture, 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Portal framebuffer " << i << " not complete!" << std::endl;
        }

        portals.push_back(portal);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    std::cout << "Portal system initialized with " << portals.size() << " portals!" << std::endl;
}

glm::mat4 PortalSystem::calculatePortalView(const Portal& portal,
    const glm::vec3& playerPos,
    const glm::vec3& playerDir,
    int recursionLevel) {
    // Calculate player position relative to portal
    glm::vec3 relativePos = playerPos - portal.position;

    // Transform player position through portal to destination space
    // This is the key to making portals work like in Portal game

    // Create transformation matrix for portal
    glm::vec3 portalRight = glm::normalize(glm::cross(portal.normal, glm::vec3(0.0f, 1.0f, 0.0f)));
    glm::vec3 portalUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Portal space to world space transformation
    glm::mat4 portalToWorld = glm::mat4(
        glm::vec4(portalRight, 0.0f),
        glm::vec4(portalUp, 0.0f),
        glm::vec4(-portal.normal, 0.0f), // Flip normal for destination
        glm::vec4(portal.destinationPos, 1.0f)
    );

    // Transform relative position through portal
    glm::vec4 transformedRelativePos = portalToWorld * glm::vec4(relativePos, 1.0f);

    // Calculate virtual camera position in destination room
    glm::vec3 virtualCameraPos = glm::vec3(transformedRelativePos) + portal.destinationPos;

    // Add offset for recursion depth to create infinite effect
    float depthOffset = recursionLevel * roomVariations[portal.portalId % roomVariations.size()].roomOffset;
    virtualCameraPos += portal.destinationNormal * depthOffset;

    // Calculate virtual camera target
    glm::vec3 virtualCameraTarget = virtualCameraPos + playerDir;

    // Create view matrix for virtual camera
    return glm::lookAt(virtualCameraPos, virtualCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

void PortalSystem::renderPortalViews(const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
    const glm::vec3& playerPos, const glm::vec3& playerDir,
    const glm::mat4& projection) {
    if (!areActive()) return;

    // Render each portal with recursion
    for (int portalIndex = 0; portalIndex < portals.size(); portalIndex++) {
        auto& portal = portals[portalIndex];

        // Skip distant portals for performance
        if (portal.distanceFromPlayer > 50.0f) continue;

        // Bind portal framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);
        glViewport(0, 0, textureSize, textureSize);

        // Clear with room variation color
        const RoomVariation& variation = roomVariations[portalIndex % roomVariations.size()];
        glm::vec3 clearColor = glm::vec3(0.01f, 0.005f, 0.02f) * variation.colorTint;
        glClearColor(clearColor.r, clearColor.g, clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render multiple recursion levels for true infinite effect
        for (int recursionLevel = 0; recursionLevel < maxRecursionDepth; recursionLevel++) {
            // Calculate virtual camera view for this recursion level
            glm::mat4 virtualView = calculatePortalView(portal, playerPos, playerDir, recursionLevel);

            // Calculate room variation for this level
            RoomVariation levelVariation = variation;
            levelVariation.scaleMultiplier *= (1.0f - recursionLevel * 0.1f); // Shrink distant rooms
            levelVariation.colorTint *= (1.0f - recursionLevel * 0.15f); // Darken distant rooms

            // Enable depth testing but allow overlapping
            if (recursionLevel > 0) {
                glDepthFunc(GL_LEQUAL);
            }
            else {
                glDepthFunc(GL_LESS);
            }

            // Render scene from virtual camera perspective
            renderScene(virtualView, projection, recursionLevel, levelVariation);
        }

        // Reset depth function
        glDepthFunc(GL_LESS);
    }

    // Restore main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PortalSystem::bindPortalTexture(int portalIndex, Shader& shader) {
    if (portalIndex >= 0 && portalIndex < portals.size() && portals[portalIndex].active) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, portals[portalIndex].colorTexture);
        shader.setInt("baseColorMap", 0);

        // Portal effect - make it glow slightly
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, 0);
        shader.setInt("metallicMap", 2);
    }
}

void PortalSystem::updateDistances(const glm::vec3& playerPos) {
    for (auto& portal : portals) {
        portal.distanceFromPlayer = glm::length(portal.position - playerPos);
    }
}

void PortalSystem::setActive(bool active) {
    for (auto& portal : portals) {
        portal.active = active;
    }
}

void PortalSystem::setQuality(int size) {
    textureSize = size;
    // Note: Would need to recreate framebuffers for full implementation
}

void PortalSystem::setRecursionDepth(int depth) {
    maxRecursionDepth = glm::clamp(depth, 1, 8); // Limit for performance
    std::cout << "Portal recursion depth set to: " << maxRecursionDepth << std::endl;
}

void PortalSystem::cleanup() {
    for (auto& portal : portals) {
        if (portal.framebuffer) glDeleteFramebuffers(1, &portal.framebuffer);
        if (portal.colorTexture) glDeleteTextures(1, &portal.colorTexture);
        if (portal.depthTexture) glDeleteTextures(1, &portal.depthTexture);
    }
    portals.clear();
    std::cout << "Portal system cleaned up." << std::endl;
}

void PortalSystem::printDebugInfo() const {
    std::cout << "\n=== PORTAL SYSTEM DEBUG ===" << std::endl;
    std::cout << "Active portals: " << portals.size() << std::endl;
    std::cout << "Texture resolution: " << textureSize << "x" << textureSize << std::endl;
    std::cout << "Max recursion depth: " << maxRecursionDepth << std::endl;
    std::cout << "Portals enabled: " << (areActive() ? "YES" : "NO") << std::endl;

    for (size_t i = 0; i < portals.size(); i++) {
        const auto& portal = portals[i];
        std::cout << "Portal " << i << ":" << std::endl;
        std::cout << "  Position: (" << portal.position.x << ", " << portal.position.y << ", " << portal.position.z << ")" << std::endl;
        std::cout << "  Destination: (" << portal.destinationPos.x << ", " << portal.destinationPos.y << ", " << portal.destinationPos.z << ")" << std::endl;
        std::cout << "  Distance: " << portal.distanceFromPlayer << std::endl;
        std::cout << "  Active: " << (portal.active ? "YES" : "NO") << std::endl;
    }
    std::cout << "==========================\n" << std::endl;
}