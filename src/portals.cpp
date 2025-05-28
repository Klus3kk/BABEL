#include "portals.hpp"
#include <iostream>
#include <cmath>

// NEW: Implement clipped projection matrix
glm::mat4 Portal::clippedProjMat(const glm::mat4& viewMat, const glm::mat4& projMat) const {
    // Calculate the clipping plane in view space
    glm::vec4 clipPlane = viewMat * glm::vec4(normal, -glm::dot(normal, position));

    // Modify the projection matrix to clip at the portal plane
    glm::mat4 clippedProj = projMat;

    // Oblique view frustum clipping
    glm::vec4 q = glm::inverse(clippedProj) * glm::vec4(
        (clipPlane.x < 0 ? -1 : (clipPlane.x > 0 ? 1 : 0)),
        (clipPlane.y < 0 ? -1 : (clipPlane.y > 0 ? 1 : 0)),
        1.0f,
        1.0f
    );

    glm::vec4 c = clipPlane * (2.0f / glm::dot(clipPlane, q));

    // Replace the third row of the projection matrix
    clippedProj[0][2] = c.x;
    clippedProj[1][2] = c.y;
    clippedProj[2][2] = c.z + 1.0f;
    clippedProj[3][2] = c.w;

    return clippedProj;
}

PortalSystem::PortalSystem() {
    // Create room variations for infinite depth
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

    std::cout << "Setting up enhanced recursive portal system..." << std::endl;

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

        portals.push_back(portal);
    }

    // Link portals to each other (circular linking)
    for (size_t i = 0; i < portals.size(); i++) {
        portals[i].destination = &portals[(i + 1) % portals.size()];
    }

    std::cout << "Enhanced portal system initialized with " << portals.size() << " portals!" << std::endl;
}

// MODIFIED: Your existing renderPortalViews now uses stencil buffer approach
void PortalSystem::renderPortalViews(
    const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
    const glm::vec3& playerPos,
    const glm::vec3& playerDir,
    const glm::mat4& projection) {

    if (!areActive()) return;

    // Create simple render function for stencil system
    auto simpleRenderScene = [&renderScene](const glm::mat4& view, const glm::mat4& proj) {
        // Use default room variation for main scene
        RoomVariation defaultVariation;
        renderScene(view, proj, 0, defaultVariation);
        };

    // Calculate view matrix
    glm::vec3 target = playerPos + playerDir;
    glm::mat4 view = glm::lookAt(playerPos, target, glm::vec3(0.0f, 1.0f, 0.0f));

    // Use stencil buffer recursive rendering
    drawRecursivePortals(simpleRenderScene, view, projection);
}

// NEW: Stencil buffer recursive rendering
void PortalSystem::drawRecursivePortals(
    const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
    const glm::mat4& viewMat,
    const glm::mat4& projMat,
    size_t recursionLevel) {

    if (!areActive()) {
        drawNonPortals(renderScene, viewMat, projMat);
        return;
    }

    for (auto& portal : portals) {
        if (!portal.active || !portal.destination) continue;

        // Skip very distant portals for performance  
        if (portal.distanceFromPlayer > 100.0f) continue;

        // STEP 1: Draw portal into stencil buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_STENCIL_TEST);

        // Fail stencil test when inside of outer portal
        glStencilFunc(GL_NOTEQUAL, recursionLevel, 0xFF);
        glStencilOp(GL_INCR, GL_KEEP, GL_KEEP);
        glStencilMask(0xFF);

        // This would draw portal geometry - we'll handle this in main.cpp
        // drawPortalGeometry(portal, viewMat, projMat, portalShader, drawPortalModel);

        // STEP 2: Calculate destination view matrix
        glm::mat4 destView = viewMat;

        // Simple transform - move camera to destination
        glm::vec3 currentPos = glm::vec3(glm::inverse(viewMat)[3]);
        glm::vec3 relativePos = currentPos - portal.position;
        glm::vec3 newPos = portal.destination->position + relativePos;

        // Reconstruct view matrix at new position
        glm::vec3 front = glm::vec3(viewMat[0][2], viewMat[1][2], viewMat[2][2]) * -1.0f;
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        destView = glm::lookAt(newPos, newPos + front, up);

        // STEP 3: Render scene through portal
        if (recursionLevel >= maxRecursionDepth) {
            // Base case - render final scene
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glDepthMask(GL_TRUE);
            glClear(GL_DEPTH_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_STENCIL_TEST);
            glStencilMask(0x00);
            glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);

            // Render with clipped projection
            renderScene(destView, portal.clippedProjMat(destView, projMat));
        }
        else {
            // Recursive case
            drawRecursivePortals(renderScene, destView,
                portal.clippedProjMat(destView, projMat),
                recursionLevel + 1);
        }

        // STEP 4: Clean up stencil buffer
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);
        glEnable(GL_STENCIL_TEST);
        glStencilMask(0xFF);
        glStencilFunc(GL_NOTEQUAL, recursionLevel + 1, 0xFF);
        glStencilOp(GL_DECR, GL_KEEP, GL_KEEP);

        // Draw portal geometry again for cleanup - handle in main.cpp
    }

    // STEP 5: Prepare depth buffer for current level
    glDisable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_ALWAYS);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Draw portals into depth buffer - handle in main.cpp

    glDepthFunc(GL_LESS);

    // STEP 6: Render current level scene
    glEnable(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_LEQUAL, recursionLevel, 0xFF);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);

    drawNonPortals(renderScene, viewMat, projMat);
}

void PortalSystem::drawNonPortals(
    const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
    const glm::mat4& viewMat,
    const glm::mat4& projMat) {

    renderScene(viewMat, projMat);
}

void PortalSystem::drawPortalGeometry(const Portal& portal,
    const glm::mat4& viewMat,
    const glm::mat4& projMat,
    Shader& portalShader,
    const std::function<void()>& drawPortalModel) {
    // This will be called from main.cpp with your portal shader and model
    portalShader.use();
    portalShader.setMat4("view", &viewMat[0][0]);
    portalShader.setMat4("projection", &projMat[0][0]);

    // Calculate model matrix for this portal
    float angle = atan2(portal.normal.z, portal.normal.x) + glm::radians(90.0f);
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, portal.position);
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0.0f, 1.0f, 0.0f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.3f, 1.3f, 1.3f));

    portalShader.setMat4("model", &modelMatrix[0][0]);
    drawPortalModel();
}

void PortalSystem::handlePlayerTeleport(glm::vec3& playerPos) {
    for (auto& portal : portals) {
        if (!portal.active || !portal.destination) continue;

        float distance = glm::length(playerPos - portal.position);
        if (distance < teleportThreshold) {
            // Calculate relative position to portal
            glm::vec3 relativePos = playerPos - portal.position;

            // Teleport player to destination
            playerPos = portal.destination->position + relativePos;

            std::cout << "Player teleported through portal " << portal.portalId << std::endl;
            break;
        }
    }
}

// EXISTING: Keep your existing functions unchanged
glm::mat4 PortalSystem::calculatePortalView(const Portal& portal, const glm::vec3& playerPos,
    const glm::vec3& playerDir, int recursionLevel) {

    glm::vec3 relativeToPortal = playerPos - portal.position;
    glm::vec3 virtualCameraPos = portal.destinationPos + relativeToPortal;

    float depthOffset = recursionLevel * roomVariations[portal.portalId % roomVariations.size()].roomOffset;
    virtualCameraPos += glm::vec3(0.0f, 0.0f, depthOffset);

    glm::vec3 virtualTarget = virtualCameraPos + playerDir;

    return glm::lookAt(virtualCameraPos, virtualTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

void PortalSystem::bindPortalTexture(int portalIndex, Shader& shader) {
    // Keep existing implementation for compatibility
    // This might not be used in stencil buffer approach but keep for fallback
    static GLuint fallbackTexture = 0;
    if (fallbackTexture == 0) {
        glGenTextures(1, &fallbackTexture);
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
        unsigned char data[] = { 50, 50, 100, 255 };
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    glBindTexture(GL_TEXTURE_2D, fallbackTexture);
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
    std::cout << "Enhanced portals " << (active ? "ENABLED" : "DISABLED") << std::endl;
}

void PortalSystem::setQuality(int size) {
    // Keep for compatibility - not used in stencil approach
    std::cout << "Portal quality setting not applicable to stencil buffer rendering" << std::endl;
}

void PortalSystem::setRecursionDepth(int depth) {
    maxRecursionDepth = std::max(1, std::min(depth, 8));
    std::cout << "Portal recursion depth set to: " << maxRecursionDepth << std::endl;
}

void PortalSystem::printDebugInfo() const {
    std::cout << "\n=== ENHANCED PORTAL SYSTEM DEBUG ===" << std::endl;
    std::cout << "Active portals: " << portals.size() << std::endl;
    std::cout << "Max recursion depth: " << maxRecursionDepth << std::endl;
    std::cout << "Teleport threshold: " << teleportThreshold << std::endl;
    std::cout << "Portals enabled: " << (areActive() ? "YES" : "NO") << std::endl;

    for (size_t i = 0; i < portals.size(); i++) {
        const auto& portal = portals[i];
        std::cout << "Portal " << i << ":" << std::endl;
        std::cout << "  Position: (" << portal.position.x << ", " << portal.position.y << ", " << portal.position.z << ")" << std::endl;
        std::cout << "  Destination: (" << portal.destinationPos.x << ", " << portal.destinationPos.y << ", " << portal.destinationPos.z << ")" << std::endl;
        std::cout << "  Distance: " << portal.distanceFromPlayer << std::endl;
        std::cout << "  Active: " << (portal.active ? "YES" : "NO") << std::endl;
        std::cout << "  Has destination: " << (portal.destination ? "YES" : "NO") << std::endl;
    }
    std::cout << "=======================================\n" << std::endl;
}

void PortalSystem::cleanup() {
    portals.clear();
    std::cout << "Enhanced portal system cleaned up." << std::endl;
}