#include "portals.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>

// Add this function declaration at the top of the file or in the header
float sgn(float value);

PortalSystem::PortalSystem() {
    // Create room variations for visual diversity
    roomVariations = {
        {glm::vec3(1.0f, 1.0f, 1.0f), 1.0f, 1.0f, glm::vec3(0.0f)},        // Normal
        {glm::vec3(1.2f, 0.8f, 0.6f), 1.1f, 1.2f, glm::vec3(0.1f, 0.0f, 0.0f)}, // Warm
        {glm::vec3(0.6f, 0.8f, 1.2f), 0.9f, 0.8f, glm::vec3(0.0f, 0.0f, 0.1f)}, // Cool
        {glm::vec3(0.8f, 1.2f, 0.8f), 1.3f, 1.1f, glm::vec3(0.0f, 0.1f, 0.0f)}, // Green
        {glm::vec3(1.1f, 0.9f, 1.1f), 0.8f, 0.9f, glm::vec3(0.05f, 0.0f, 0.05f)}, // Purple
        {glm::vec3(1.0f, 1.0f, 0.7f), 1.0f, 1.3f, glm::vec3(0.1f, 0.1f, 0.0f)}, // Yellow
    };
}

PortalSystem::~PortalSystem() {
    cleanup();
}

void PortalSystem::initialize(float roomRadius, float roomHeight) {
    cleanup(); // Clean up any existing portals

    std::cout << "Initializing proper portal system..." << std::endl;

    // Create 4 portals around the room (matching the portal quad positions)
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));

        // Calculate portal quad position (matches main.cpp)
        glm::vec3 basePos = glm::vec3(
            roomRadius * 0.8f * cos(angle),
            0.0f,
            roomRadius * 0.8f * sin(angle)
        );

        glm::vec3 normal = glm::vec3(-cos(angle), 0.0f, -sin(angle)); // Face inward
        glm::vec3 position = basePos + glm::vec3(0.0f, roomHeight * 0.3f, 0.0f) + normal * 0.3f; // Match the deeper position

        addPortal(position, normal, glm::vec3(0, 0, 30 + i * 5)); // Different room offsets
    }

    // Connect portals in a cycle (0->1->2->3->0)
    for (int i = 0; i < 4; i++) {
        connectPortals(i, (i + 1) % 4);
    }

    std::cout << "Portal system initialized with " << portals.size() << " portals!" << std::endl;
}

void PortalSystem::addPortal(const glm::vec3& position, const glm::vec3& normal, const glm::vec3& destinationOffset) {
    Portal portal(position, normal, static_cast<int>(portals.size()));
    portal.destinationOffset = destinationOffset;

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
        std::cerr << "Portal framebuffer " << portal.portalId << " not complete! Status: 0x" << std::hex << status << std::endl;
    }

    // Generate portal geometry for stencil testing
    generatePortalGeometry(portal);

    portals.push_back(portal);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PortalSystem::generatePortalGeometry(Portal& portal) {
    // Create a quad representing the portal opening
    float halfWidth = portal.width * 0.5f;
    float halfHeight = portal.height * 0.5f;

    // Portal vertices in local space (centered at origin, facing +Z)
    float vertices[] = {
        -halfWidth, -halfHeight, 0.0f,  // Bottom-left
         halfWidth, -halfHeight, 0.0f,  // Bottom-right
         halfWidth,  halfHeight, 0.0f,  // Top-right
        -halfWidth,  halfHeight, 0.0f   // Top-left
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    glGenVertexArrays(1, &portal.stencilVAO);
    glGenBuffers(1, &portal.stencilVBO);
    glGenBuffers(1, &portal.stencilEBO);

    glBindVertexArray(portal.stencilVAO);

    glBindBuffer(GL_ARRAY_BUFFER, portal.stencilVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, portal.stencilEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void PortalSystem::cleanupPortalGeometry(Portal& portal) {
    if (portal.stencilVAO) {
        glDeleteVertexArrays(1, &portal.stencilVAO);
        portal.stencilVAO = 0;
    }
    if (portal.stencilVBO) {
        glDeleteBuffers(1, &portal.stencilVBO);
        portal.stencilVBO = 0;
    }
    if (portal.stencilEBO) {
        glDeleteBuffers(1, &portal.stencilEBO);
        portal.stencilEBO = 0;
    }
}

void PortalSystem::connectPortals(int portal1Id, int portal2Id) {
    if (portal1Id >= 0 && portal1Id < portals.size() &&
        portal2Id >= 0 && portal2Id < portals.size()) {
        portals[portal1Id].destinationPortalId = portal2Id;
        portals[portal2Id].destinationPortalId = portal1Id;
    }
}

glm::mat4 PortalSystem::calculatePortalTransformation(const Portal& fromPortal, const Portal& toPortal,
    const glm::mat4& originalView) const {

    // Extract camera position and orientation from view matrix
    glm::mat4 invView = glm::inverse(originalView);
    glm::vec3 cameraPos = glm::vec3(invView[3]);
    glm::vec3 cameraFront = -glm::vec3(invView[2]);
    glm::vec3 cameraUp = glm::vec3(invView[1]);
    glm::vec3 cameraRight = glm::vec3(invView[0]);

    // Calculate relative position to source portal
    glm::vec3 relativePos = cameraPos - fromPortal.position;

    // Transform relative position through portal
    // This involves rotating from source portal space to destination portal space

    // Create transformation matrices for portal orientations
    glm::mat4 fromPortalMatrix = glm::mat4(1.0f);
    fromPortalMatrix[0] = glm::vec4(fromPortal.right, 0.0f);
    fromPortalMatrix[1] = glm::vec4(fromPortal.up, 0.0f);
    fromPortalMatrix[2] = glm::vec4(-fromPortal.normal, 0.0f); // Negative for looking through
    fromPortalMatrix[3] = glm::vec4(fromPortal.position, 1.0f);

    glm::mat4 toPortalMatrix = glm::mat4(1.0f);
    toPortalMatrix[0] = glm::vec4(-toPortal.right, 0.0f); // Flip for opposite side
    toPortalMatrix[1] = glm::vec4(toPortal.up, 0.0f);
    toPortalMatrix[2] = glm::vec4(toPortal.normal, 0.0f);
    toPortalMatrix[3] = glm::vec4(toPortal.position, 1.0f);

    // Transform camera through portal
    glm::mat4 portalTransform = toPortalMatrix * glm::inverse(fromPortalMatrix);

    // Apply transformation to camera
    glm::vec4 newCameraPos4 = portalTransform * glm::vec4(cameraPos, 1.0f);
    glm::vec3 newCameraPos = glm::vec3(newCameraPos4);

    glm::vec3 newCameraFront = glm::mat3(portalTransform) * cameraFront;
    glm::vec3 newCameraUp = glm::mat3(portalTransform) * cameraUp;

    return glm::lookAt(newCameraPos, newCameraPos + newCameraFront, newCameraUp);
}

glm::vec4 PortalSystem::getPortalPlane(const Portal& portal) const {
    // Portal plane equation: normal * (point - position) = 0
    // Rearranged to: normal.x * x + normal.y * y + normal.z * z + d = 0
    float d = -glm::dot(portal.normal, portal.position);
    return glm::vec4(portal.normal, d);
}

glm::mat4 PortalSystem::createObliqueProjection(const glm::mat4& projection, const glm::vec4& clipPlane) const {
    // Oblique frustum clipping implementation
    // Based on: "Oblique View Frustum Depth Projection and Clipping" by Eric Lengyel

    glm::mat4 modifiedProjection = projection;

    // Calculate the clip-space clip plane
    glm::vec4 q;
    q.x = (sgn(clipPlane.x) + modifiedProjection[2][0]) / modifiedProjection[0][0];
    q.y = (sgn(clipPlane.y) + modifiedProjection[2][1]) / modifiedProjection[1][1];
    q.z = -1.0f;
    q.w = (1.0f + modifiedProjection[2][2]) / modifiedProjection[3][2];

    // Calculate the scaled plane vector
    glm::vec4 c = clipPlane * (2.0f / glm::dot(clipPlane, q));

    // Replace the third row of the projection matrix
    modifiedProjection[0][2] = c.x;
    modifiedProjection[1][2] = c.y;
    modifiedProjection[2][2] = c.z + 1.0f;
    modifiedProjection[3][2] = c.w;

    return modifiedProjection;
}

bool PortalSystem::isInFrontOfPortal(const Portal& portal, const glm::vec3& point) const {
    glm::vec3 toPoint = point - portal.position;
    return glm::dot(toPoint, portal.normal) > 0.0f;
}

void PortalSystem::renderPortalStencil(const Portal& portal, const glm::mat4& view,
    const glm::mat4& projection, int stencilValue) const {

    // Create transformation matrix for portal quad
    glm::mat4 portalTransform = glm::mat4(1.0f);
    portalTransform = glm::translate(portalTransform, portal.position);

    // Create rotation matrix from portal orientation
    glm::mat4 rotation = glm::mat4(1.0f);
    rotation[0] = glm::vec4(portal.right, 0.0f);
    rotation[1] = glm::vec4(portal.up, 0.0f);
    rotation[2] = glm::vec4(portal.normal, 0.0f);

    portalTransform = portalTransform * rotation;

    // Set up stencil testing
    glStencilFunc(GL_ALWAYS, stencilValue, 0xFF);
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    glStencilMask(0xFF);

    // Disable color and depth writing
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);

    // Render portal quad to stencil buffer
    // Note: This assumes you have a basic shader set up for rendering
    // You'll need to set up the shader uniforms here

    glBindVertexArray(portal.stencilVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Re-enable color and depth writing
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
}

void PortalSystem::renderRecursivePortals(
    const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    const glm::mat4& view, const glm::mat4& projection, int recursionLevel) {

    // Base case: maximum recursion depth reached
    if (recursionLevel >= maxRecursionDepth) {
        RoomVariation variation = roomVariations[recursionLevel % roomVariations.size()];
        renderScene(view, projection, recursionLevel, variation);
        return;
    }

    // Step 1: Render the main scene first
    RoomVariation currentVariation = roomVariations[recursionLevel % roomVariations.size()];
    renderScene(view, projection, recursionLevel, currentVariation);

    // Step 2: Enable stencil testing
    glEnable(GL_STENCIL_TEST);
    glClear(GL_STENCIL_BUFFER_BIT);

    // Step 3: Render each portal recursively
    for (const auto& portal : portals) {
        if (!portal.active) continue;

        // Skip portals that are behind the camera
        if (!isInFrontOfPortal(portal, cameraPos)) continue;

        // Skip portals that are too far away
        if (portal.distanceFromPlayer > 50.0f) continue;

        // Step 3a: Render portal stencil mask
        renderPortalStencil(portal, view, projection, recursionLevel + 1);

        // Step 3b: Set stencil test to only render where portal is
        glStencilFunc(GL_EQUAL, recursionLevel + 1, 0xFF);
        glStencilMask(0x00);

        // Step 3c: Clear depth buffer for portal rendering
        glClear(GL_DEPTH_BUFFER_BIT);

        // Step 3d: Calculate portal view transformation
        glm::mat4 portalView;
        if (portal.destinationPortalId >= 0 && portal.destinationPortalId < portals.size()) {
            // Use proper portal-to-portal transformation
            const Portal& destPortal = portals[portal.destinationPortalId];
            portalView = calculatePortalTransformation(portal, destPortal, view);
        }
        else {
            // Use simple offset transformation
            glm::vec3 virtualCameraPos = cameraPos + portal.destinationOffset;
            portalView = glm::lookAt(virtualCameraPos, virtualCameraPos + cameraFront, cameraUp);
        }

        // Step 3e: Create oblique projection matrix
        glm::vec4 portalPlane = getPortalPlane(portal);
        glm::mat4 obliqueProjection = createObliqueProjection(projection, portalPlane);

        // Step 3f: Recursive call
        glm::mat4 invPortalView = glm::inverse(portalView);
        glm::vec3 virtualCameraPos = glm::vec3(invPortalView[3]);
        glm::vec3 virtualCameraFront = -glm::vec3(invPortalView[2]);
        glm::vec3 virtualCameraUp = glm::vec3(invPortalView[1]);

        renderRecursivePortals(renderScene, virtualCameraPos, virtualCameraFront, virtualCameraUp,
            portalView, obliqueProjection, recursionLevel + 1);
    }

    // Step 4: Disable stencil testing
    glDisable(GL_STENCIL_TEST);
}

void PortalSystem::renderPortalViews(
    const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
    const glm::vec3& playerPos, const glm::vec3& playerDir,
    const glm::mat4& projection) {

    if (!areActive()) return;

    // Save current viewport and framebuffer
    GLint viewport[4];
    GLint currentFramebuffer;
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);

    // Render portal views to textures
    for (auto& portal : portals) {
        if (!portal.active) continue;
        if (portal.distanceFromPlayer > 80.0f) continue;

        // Bind portal framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);
        glViewport(0, 0, textureSize, textureSize);

        // Clear with a distinctive color for debugging
        glClearColor(0.1f, 0.05f, 0.2f, 1.0f); // Dark purple
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Enable depth testing
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Calculate portal view
        glm::mat4 portalView;
        if (portal.destinationPortalId >= 0 && portal.destinationPortalId < portals.size()) {
            const Portal& destPortal = portals[portal.destinationPortalId];
            glm::mat4 currentView = glm::lookAt(playerPos, playerPos + playerDir, glm::vec3(0, 1, 0));
            portalView = calculatePortalTransformation(portal, destPortal, currentView);
        }
        else {
            glm::vec3 virtualPos = playerPos + portal.destinationOffset;
            portalView = glm::lookAt(virtualPos, virtualPos + playerDir, glm::vec3(0, 1, 0));
        }

        // Render scene from portal view
        RoomVariation variation = roomVariations[portal.portalId % roomVariations.size()];
        renderScene(portalView, projection, 0, variation);

        // Debug: Check if anything was actually rendered
        glFinish(); // Ensure rendering is complete

        std::cout << "Rendered portal view " << portal.portalId
            << " to texture " << portal.colorTexture << std::endl;
    }

    // Restore main framebuffer and viewport
    glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

void PortalSystem::bindPortalTexture(int portalIndex, Shader& shader) const {
    if (portalIndex >= 0 && portalIndex < portals.size() && portals[portalIndex].active) {
        glBindTexture(GL_TEXTURE_2D, portals[portalIndex].colorTexture);
    }
    else {
        // Bind fallback texture
        static GLuint fallbackTexture = 0;
        if (fallbackTexture == 0) {
            glGenTextures(1, &fallbackTexture);
            glBindTexture(GL_TEXTURE_2D, fallbackTexture);
            unsigned char data[] = { 20, 20, 40, 255 }; // Dark blue
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        }
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
    }
}

void PortalSystem::updateDistances(const glm::vec3& playerPos) {
    for (auto& portal : portals) {
        portal.distanceFromPlayer = glm::length(portal.position - playerPos);
    }
}

bool PortalSystem::checkPortalCollision(const glm::vec3& oldPos, const glm::vec3& newPos, glm::vec3& teleportPos) const {
    for (const auto& portal : portals) {
        if (!portal.active) continue;

        // Calculate distance from portal center to player trajectory
        glm::vec3 toOldPos = oldPos - portal.position;
        glm::vec3 toNewPos = newPos - portal.position;

        // Check if player crossed the portal plane
        float oldDist = glm::dot(toOldPos, portal.normal);
        float newDist = glm::dot(toNewPos, portal.normal);

        // Player crossed from front to back of portal
        if (oldDist > 0.0f && newDist <= 0.0f) {
            // Check if crossing point is within portal bounds
            glm::vec3 crossingPoint = oldPos + (newPos - oldPos) * (oldDist / (oldDist - newDist));
            glm::vec3 localPos = crossingPoint - portal.position;

            // Project onto portal's local coordinate system
            float u = glm::dot(localPos, portal.right);
            float v = glm::dot(localPos, portal.up);

            // Check if within portal bounds (considering quad size)
            if (abs(u) <= portal.width * 0.5f && abs(v) <= portal.height * 0.5f) {
                // Player went through this portal!
                if (portal.destinationPortalId >= 0 && portal.destinationPortalId < portals.size()) {
                    const Portal& destPortal = portals[portal.destinationPortalId];
                    teleportPos = transformThroughPortal(portal, destPortal, newPos);
                }
                else {
                    // Simple offset teleportation
                    teleportPos = newPos + portal.destinationOffset;
                }

                std::cout << "Portal collision detected! Teleporting from portal "
                    << portal.portalId << " to destination." << std::endl;
                return true;
            }
        }
    }
    return false;
}

glm::vec3 PortalSystem::transformThroughPortal(const Portal& fromPortal, const Portal& toPortal, const glm::vec3& position) const {
    // Transform position through portal using the same logic as camera transformation
    glm::vec3 relativePos = position - fromPortal.position;

    // Project onto portal's local coordinate system
    float u = glm::dot(relativePos, fromPortal.right);
    float v = glm::dot(relativePos, fromPortal.up);
    float w = glm::dot(relativePos, fromPortal.normal);

    // Transform to destination portal's coordinate system (flip w for opposite side)
    glm::vec3 transformedPos = toPortal.position +
        u * (-toPortal.right) +
        v * toPortal.up +
        (-w) * toPortal.normal;

    return transformedPos;
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
    maxRecursionDepth = glm::clamp(depth, 1, 8);
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
        cleanupPortalGeometry(portal);
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
        std::cout << "  Normal: (" << portal.normal.x << ", " << portal.normal.y << ", " << portal.normal.z << ")" << std::endl;
        std::cout << "  Destination Portal ID: " << portal.destinationPortalId << std::endl;
        std::cout << "  Distance: " << portal.distanceFromPlayer << std::endl;
        std::cout << "  Active: " << (portal.active ? "YES" : "NO") << std::endl;
        std::cout << "  Framebuffer ID: " << portal.framebuffer << std::endl;
    }
    std::cout << "==========================\n" << std::endl;
}

// Helper function for oblique projection (missing sgn function)
float sgn(float value) {
    if (value > 0.0f) return 1.0f;
    if (value < 0.0f) return -1.0f;
    return 0.0f;
}

