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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, textureSize, textureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, portal.colorTexture, 0);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, portal.depthTexture, 0);

        // Check framebuffer status
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Portal framebuffer " << i << " not complete! Status: 0x" << std::hex << status << std::endl;
        }
        else {
            std::cout << "Portal " << i << " framebuffer created successfully!" << std::endl;
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

    // Simple approach: just move the camera to the destination room
    // and offset it based on the player's relative position to the portal

    glm::vec3 relativeToPortal = playerPos - portal.position;

    // Transform relative position to destination space
    glm::vec3 virtualCameraPos = portal.destinationPos + relativeToPortal;

    // Add depth offset for recursion levels
    float depthOffset = recursionLevel * roomVariations[portal.portalId % roomVariations.size()].roomOffset;
    virtualCameraPos += glm::vec3(0.0f, 0.0f, depthOffset);

    // Use same camera direction
    glm::vec3 virtualTarget = virtualCameraPos + playerDir;

    return glm::lookAt(virtualCameraPos, virtualTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}


void PortalSystem::renderPortalViews(
    const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
    const glm::vec3& playerPos,
    const glm::vec3& playerDir,
    const glm::mat4& projection) {

    if (!areActive()) return;

    // Save current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Render each portal view
    for (int portalIndex = 0; portalIndex < portals.size(); portalIndex++) {
        auto& portal = portals[portalIndex];

        // Skip very distant portals
        if (portal.distanceFromPlayer > 80.0f) continue;

        // Bind portal framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);
        glViewport(0, 0, textureSize, textureSize);

        // Clear with the same background as main scene
        glClearColor(0.01f, 0.005f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Calculate virtual camera view
        glm::mat4 virtualView = calculatePortalView(portal, playerPos, playerDir, 0);

        // Create room variation for this portal
        const RoomVariation& variation = roomVariations[portalIndex % roomVariations.size()];

        // Render the scene from the virtual camera
        // This should render ALL objects (books, shelves, torches, etc.)
        renderScene(virtualView, projection, 0, variation);

        // Optional: Render additional recursion levels for infinite effect
        for (int recursionLevel = 1; recursionLevel < maxRecursionDepth; recursionLevel++) {
            glDepthFunc(GL_LEQUAL); // Allow some overlap

            glm::mat4 deeperView = calculatePortalView(portal, playerPos, playerDir, recursionLevel);
            RoomVariation deeperVariation = variation;
            deeperVariation.scaleMultiplier *= (1.0f - recursionLevel * 0.1f);
            deeperVariation.colorTint *= (1.0f - recursionLevel * 0.2f);

            renderScene(deeperView, projection, recursionLevel, deeperVariation);
        }

        glDepthFunc(GL_LESS);
    }

    // Restore main framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}
void PortalSystem::bindPortalTexture(int portalIndex, Shader& shader) {
    if (portalIndex >= 0 && portalIndex < portals.size() && portals[portalIndex].active) {
        // Bind the portal's rendered view texture
        glBindTexture(GL_TEXTURE_2D, portals[portalIndex].colorTexture);

        // Check if texture is valid
        GLint textureWidth;
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &textureWidth);
        if (textureWidth != textureSize) {
            std::cerr << "Warning: Portal " << portalIndex << " texture may not be properly initialized!" << std::endl;
        }
    }
    else {
        // Bind a fallback texture or create a simple colored texture
        static GLuint fallbackTexture = 0;
        if (fallbackTexture == 0) {
            glGenTextures(1, &fallbackTexture);
            glBindTexture(GL_TEXTURE_2D, fallbackTexture);

            // Create a simple blue swirling pattern
            unsigned char data[64 * 64 * 3];
            for (int y = 0; y < 64; y++) {
                for (int x = 0; x < 64; x++) {
                    int index = (y * 64 + x) * 3;
                    float fx = (x - 32) / 32.0f;
                    float fy = (y - 32) / 32.0f;
                    float dist = sqrt(fx * fx + fy * fy);
                    float angle = atan2(fy, fx);

                    data[index + 0] = (unsigned char)(128 + 127 * sin(angle * 4 + dist * 8)); // Red
                    data[index + 1] = (unsigned char)(64 + 64 * cos(dist * 6)); // Green
                    data[index + 2] = (unsigned char)(200 + 55 * sin(dist * 10)); // Blue
                }
            }

            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 64, 64, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        }
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
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
    std::cout << "Portals " << (active ? "ENABLED" : "DISABLED") << std::endl;
}

void PortalSystem::setQuality(int size) {
    if (size != textureSize) {
        textureSize = size;

        // Recreate framebuffers with new size
        for (auto& portal : portals) {
            if (portal.framebuffer != 0) {
                // Delete old textures
                if (portal.colorTexture != 0) glDeleteTextures(1, &portal.colorTexture);
                if (portal.depthTexture != 0) glDeleteTextures(1, &portal.depthTexture);

                // Recreate color texture
                glGenTextures(1, &portal.colorTexture);
                glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                // Recreate depth texture
                glGenTextures(1, &portal.depthTexture);
                glBindTexture(GL_TEXTURE_2D, portal.depthTexture);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, textureSize, textureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                // Reattach to framebuffer
                glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, portal.colorTexture, 0);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, portal.depthTexture, 0);

                if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                    std::cerr << "Portal framebuffer recreation failed!" << std::endl;
                }
            }
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        std::cout << "Portal quality updated to " << textureSize << "x" << textureSize << std::endl;
    }
}

void PortalSystem::setRecursionDepth(int depth) {
    maxRecursionDepth = glm::clamp(depth, 1, 8); // Limit for performance
    std::cout << "Portal recursion depth set to: " << maxRecursionDepth << std::endl;
}

void PortalSystem::cleanup() {
    for (auto& portal : portals) {
        if (portal.framebuffer) {
            glDeleteFramebuffers(1, &portal.framebuffer);
            portal.framebuffer = 0;
        }
        if (portal.colorTexture) {
            glDeleteTextures(1, &portal.colorTexture);
            portal.colorTexture = 0;
        }
        if (portal.depthTexture) {
            glDeleteTextures(1, &portal.depthTexture);
            portal.depthTexture = 0;
        }
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
        std::cout << "  Framebuffer ID: " << portal.framebuffer << std::endl;
        std::cout << "  Color Texture ID: " << portal.colorTexture << std::endl;
        std::cout << "  Depth Texture ID: " << portal.depthTexture << std::endl;
    }
    std::cout << "==========================\n" << std::endl;
}