#include "portals.hpp"
#include <iostream>
#include <cmath>

PortalSystem::~PortalSystem() {
    cleanup();
}

void PortalSystem::initialize() {
    cleanup();
    std::cout << "Initializing optimized portal system..." << std::endl;
}

void PortalSystem::addPortal(const glm::vec3& position, const glm::vec3& normal) {
    Portal portal(position, normal, static_cast<int>(portals.size()));

    // Create framebuffer for portal view
    glGenFramebuffers(1, &portal.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);

    // Color texture with better format
    glGenTextures(1, &portal.colorTexture);
    glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
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

    // Check framebuffer completeness
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Portal framebuffer not complete! Status: " << status << std::endl;

        // Fallback: create a simple test texture
        unsigned char testData[4] = { 255, 0, 255, 255 }; // Magenta for debugging
        glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, testData);
    }
    else {
        std::cout << "Portal framebuffer created successfully!" << std::endl;
    }

    // Generate portal geometry
    generatePortalGeometry(portal);

    portals.push_back(portal);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "Added portal " << portal.portalId << " at position ("
        << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void PortalSystem::generatePortalGeometry(Portal& portal) {
    // Make portal larger and ensure proper UV mapping
    float halfWidth = portal.width * 0.75f;  // Increased for better visibility
    float halfHeight = portal.height * 0.75f; // Increased for better visibility

    float vertices[] = {
        // positions                    // texture coords
        -halfWidth, -halfHeight, 0.0f,  0.0f, 0.0f,  // bottom-left
         halfWidth, -halfHeight, 0.0f,  1.0f, 0.0f,  // bottom-right
         halfWidth,  halfHeight, 0.0f,  1.0f, 1.0f,  // top-right
        -halfWidth,  halfHeight, 0.0f,  0.0f, 1.0f   // top-left
    };

    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    glGenVertexArrays(1, &portal.portalVAO);
    glGenBuffers(1, &portal.portalVBO);
    glGenBuffers(1, &portal.portalEBO);

    glBindVertexArray(portal.portalVAO);

    glBindBuffer(GL_ARRAY_BUFFER, portal.portalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, portal.portalEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute (location = 0)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (location = 2, matching vertex shader)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void PortalSystem::cleanupPortalGeometry(Portal& portal) {
    if (portal.portalVAO) {
        glDeleteVertexArrays(1, &portal.portalVAO);
        portal.portalVAO = 0;
    }
    if (portal.portalVBO) {
        glDeleteBuffers(1, &portal.portalVBO);
        portal.portalVBO = 0;
    }
    if (portal.portalEBO) {
        glDeleteBuffers(1, &portal.portalEBO);
        portal.portalEBO = 0;
    }
}

void PortalSystem::connectPortals(int portal1Id, int portal2Id) {
    if (portal1Id >= 0 && portal1Id < portals.size() &&
        portal2Id >= 0 && portal2Id < portals.size()) {
        portals[portal1Id].destinationPortalId = portal2Id;
        portals[portal2Id].destinationPortalId = portal1Id;
        std::cout << "Connected portal " << portal1Id << " to portal " << portal2Id << std::endl;
    }
}


void PortalSystem::renderPortalViews(
    const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    const glm::mat4& projection) {

    if (!areActive() || portals.empty()) return;

    // Render recursively starting from depth 0
    renderPortalViewsRecursive(renderScene, cameraPos, cameraFront, cameraUp, projection, 0);
}



// src/portals.cpp - EMERGENCY FIX: Prevent brightness accumulation

void PortalSystem::renderPortalViewsRecursive(
    const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    const glm::mat4& projection, int recursionDepth) {

    // CHANGE THIS to allow more infinite layers
    if (recursionDepth >= 3) return; // INCREASED from 1 to 5 for deep infinity effect

    // Save OpenGL state
    GLint viewport[4];
    GLint currentFramebuffer;
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);

    GLboolean depthTest, blend, cullFace;
    glGetBooleanv(GL_DEPTH_TEST, &depthTest);
    glGetBooleanv(GL_BLEND, &blend);
    glGetBooleanv(GL_CULL_FACE, &cullFace);

    // Render each portal view
    for (size_t i = 0; i < portals.size(); i++) {
        auto& portal = portals[i];
        if (!portal.active || portal.destinationPortalId < 0) continue;

        const Portal& destPortal = portals[portal.destinationPortalId];

        // Bind portal framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);

        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Portal framebuffer incomplete!" << std::endl;
            continue;
        }

        glViewport(0, 0, textureSize, textureSize);
        glClearColor(0.01f, 0.008f, 0.005f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        // Portal view matrix and projection
        glm::mat4 portalView = calculatePortalViewMatrix(portal, destPortal, cameraPos, cameraFront, cameraUp);
        glm::mat4 portalProjection = glm::perspective(
            glm::radians(60.0f),
            1.0f, // Wider aspect for your preference
            0.1f, 100.0f
        );

        // RECURSIVE CALL - this creates the infinite effect!
        if (recursionDepth < 2) { // Render portals within portals
            renderPortalViewsRecursive(renderScene, cameraPos, cameraFront, cameraUp, portalProjection, recursionDepth + 1);
        }

        // Render the main scene
        renderScene(portalView, portalProjection);

        glFlush();
        glFinish();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    if (depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
    if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
    if (cullFace) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
}


glm::mat4 PortalSystem::calculatePortalViewMatrix(const Portal& fromPortal, const Portal& toPortal,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp) const {

    // BETTER positioning for infinite tunnel effect
    glm::vec3 virtualCameraPos = toPortal.position - (5.0f * toPortal.normal); // MOVED from 3.0f to 5.0f for better depth

    // Only slight rotation influence
    glm::vec3 fromRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), fromPortal.normal));
    glm::vec3 fromUp = glm::normalize(glm::cross(fromPortal.normal, fromRight));
    glm::vec3 toRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), toPortal.normal));
    glm::vec3 toUp = glm::normalize(glm::cross(toPortal.normal, toRight));

    float frontRight = glm::dot(cameraFront, fromRight) * 0.1f;  // Minimal rotation
    float frontUp = glm::dot(cameraFront, fromUp) * 0.1f;
    float frontForward = glm::dot(cameraFront, fromPortal.normal);

    glm::vec3 virtualCameraFront =
        -(frontRight * toRight) +
        (frontUp * toUp) +
        (-frontForward * toPortal.normal);
    virtualCameraFront = glm::normalize(virtualCameraFront);

    glm::vec3 lookTarget = virtualCameraPos + virtualCameraFront * 5.0f;
    return glm::lookAt(virtualCameraPos, lookTarget, cameraUp);
}

void PortalSystem::renderPortalSurfaces(Shader& portalShader, const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos, float time) {

    if (!areActive()) return;

    portalShader.use();
    portalShader.setMat4("view", &view[0][0]);
    portalShader.setMat4("projection", &projection[0][0]);
    portalShader.setFloat("time", time);

    // MINIMAL STATE CHANGES
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (size_t i = 0; i < portals.size(); i++) {
        const auto& portal = portals[i];

        if (!portal.active || portal.distanceFromPlayer > 50.0f || portal.colorTexture == 0) continue;

        glm::mat4 portalMatrix = glm::mat4(1.0f);
        portalMatrix = glm::translate(portalMatrix, portal.position);

        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, portal.normal));
        up = glm::normalize(glm::cross(portal.normal, right));

        glm::mat4 rotation = glm::mat4(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(portal.normal, 0.0f);
        portalMatrix = portalMatrix * rotation;

        portalShader.setMat4("model", &portalMatrix[0][0]);
        portalShader.setBool("portalActive", true);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
        portalShader.setInt("portalView", 0);

        if (portal.portalVAO != 0) {
            glBindVertexArray(portal.portalVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    // RESTORE STATE
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

bool PortalSystem::checkPortalCollision(const glm::vec3& oldPos, const glm::vec3& newPos, glm::vec3& teleportPos) const {
    for (const auto& portal : portals) {
        if (!portal.active || portal.destinationPortalId < 0) continue;

        // Check if player is close enough to portal
        float distToPortal = glm::length(newPos - portal.position);
        if (distToPortal > 3.0f) continue;

        // Calculate portal plane
        glm::vec3 portalNormal = portal.normal;
        float portalPlaneD = -glm::dot(portalNormal, portal.position);

        // Distance from old and new positions to portal plane
        float oldDist = glm::dot(portalNormal, oldPos) + portalPlaneD;
        float newDist = glm::dot(portalNormal, newPos) + portalPlaneD;

        // Check if crossed the portal plane
        if (oldDist > 0.1f && newDist <= 0.1f) {
            // Find intersection point
            float t = oldDist / (oldDist - newDist);
            glm::vec3 intersectionPoint = oldPos + t * (newPos - oldPos);

            // Check if intersection is within portal bounds
            glm::vec3 localPos = intersectionPoint - portal.position;

            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(up, portal.normal));
            up = glm::normalize(glm::cross(portal.normal, right));

            float u = glm::dot(localPos, right);
            float v = glm::dot(localPos, up);

            float halfWidth = portal.width * 0.5f;
            float halfHeight = portal.height * 0.5f;

            if (abs(u) <= halfWidth && abs(v) <= halfHeight) {
                // Valid portal collision! Calculate teleport position
                const Portal& destPortal = portals[portal.destinationPortalId];

                // Simple teleportation: maintain relative position
                glm::vec3 relativePos = newPos - portal.position;
                teleportPos = destPortal.position + relativePos;

                // Add small offset to prevent re-collision
                teleportPos += destPortal.normal * 0.5f;

                std::cout << "Portal teleport! From " << portal.portalId
                    << " to " << portal.destinationPortalId << std::endl;

                return true;
            }
        }
    }
    return false;
}

void PortalSystem::updateDistances(const glm::vec3& playerPos) {
    for (auto& portal : portals) {
        portal.distanceFromPlayer = glm::length(portal.position - playerPos);
    }
}

void PortalSystem::setQuality(int textureResolution) {
    textureSize = textureResolution;
}

void PortalSystem::setRecursionDepth(int depth) {
    maxRecursionDepth = glm::clamp(depth, 1, 20);
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
}

void PortalSystem::printDebugInfo() const {
    std::cout << "\n=== PORTAL SYSTEM DEBUG ===" << std::endl;
    std::cout << "Portals: " << portals.size() << std::endl;
    std::cout << "Texture size: " << textureSize << "x" << textureSize << std::endl;
    std::cout << "Active: " << (areActive() ? "YES" : "NO") << std::endl;

    for (size_t i = 0; i < portals.size(); i++) {
        const auto& portal = portals[i];
        std::cout << "Portal " << i << ": pos("
            << portal.position.x << ", " << portal.position.y << ", " << portal.position.z
            << ") -> " << portal.destinationPortalId << std::endl;
    }
    std::cout << "==========================\n" << std::endl;
}