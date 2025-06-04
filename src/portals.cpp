#include "portals.hpp"
#include <iostream>
#include <cmath>

// Portal system constants - better this than some rmagic numbers
namespace PortalConstants {
    const int MAX_RECURSION_DEPTH = 6;
    const int RENDER_RECURSION_LIMIT = 3;  // Render portals within portals up to this depth
    const float COLLISION_DISTANCE = 20.0f;
    const float PLANE_THRESHOLD = 0.1f;
    const float VIRTUAL_CAMERA_DISTANCE = 10.0f;
    const float TELEPORT_OFFSET = 0.5f;
    const float DISTANCE_CULLING = 55.0f;
    const float PORTAL_SIZE_MULTIPLIER = 1.2f;
    const float CAMERA_INFLUENCE_FACTOR = 0.3f;  // For direction transformation
}

PortalSystem::~PortalSystem() {
    cleanup();
}

void PortalSystem::initialize() {
    cleanup();
    std::cout << "Initializing optimized portal system..." << std::endl;
}

void PortalSystem::addPortal(const glm::vec3& position, const glm::vec3& normal) {
    Portal portal(position, normal, static_cast<int>(portals.size()));

    // Create framebuffer for rendering portal's view
    glGenFramebuffers(1, &portal.framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);

    // Color texture - what you see through the portal
    glGenTextures(1, &portal.colorTexture);
    glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, textureSize, textureSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Depth texture - needed for proper 3D rendering
    glGenTextures(1, &portal.depthTexture);
    glBindTexture(GL_TEXTURE_2D, portal.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, textureSize, textureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Attach textures to framebuffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, portal.colorTexture, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, portal.depthTexture, 0);

    // Check if framebuffer is properly set up
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Portal framebuffer not complete! Status: " << status << std::endl;
    }
    else {
        std::cout << "Portal framebuffer created successfully!" << std::endl;
    }

    generatePortalGeometry(portal);

    portals.push_back(portal);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::cout << "Added portal " << portal.portalId << " at position ("
        << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
}

void PortalSystem::generatePortalGeometry(Portal& portal) { // had quad.obj but this is better
    // Create portal quad using size multiplier constant
    float halfWidth = portal.width * PortalConstants::PORTAL_SIZE_MULTIPLIER;
    float halfHeight = portal.height * PortalConstants::PORTAL_SIZE_MULTIPLIER;

    // Vertex data: position (3 floats) + texture coordinates (2 floats)
    float vertices[] = {
        // positions                    // texture coords
        -halfWidth, -halfHeight, 0.0f,  0.0f, 0.0f,  // bottom-left
         halfWidth, -halfHeight, 0.0f,  1.0f, 0.0f,  // bottom-right
         halfWidth,  halfHeight, 0.0f,  1.0f, 1.0f,  // top-right
        -halfWidth,  halfHeight, 0.0f,  0.0f, 1.0f   // top-left
    };

    // Two triangles forming a quad
    unsigned int indices[] = {
        0, 1, 2,  // first triangle
        2, 3, 0   // second triangle
    };

    // Generate OpenGL objects
    glGenVertexArrays(1, &portal.portalVAO);
    glGenBuffers(1, &portal.portalVBO);
    glGenBuffers(1, &portal.portalEBO);

    glBindVertexArray(portal.portalVAO);

    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, portal.portalVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, portal.portalEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute (location = 0 in vertex shader)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Texture coordinate attribute (location = 2 in vertex shader)
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
    // Set up bidirectional connection with bounds checking
    if (portal1Id >= 0 && portal1Id < static_cast<int>(portals.size()) &&
        portal2Id >= 0 && portal2Id < static_cast<int>(portals.size())) {
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

    // Start recursive rendering from depth 0
    renderPortalViewsRecursive(renderScene, cameraPos, cameraFront, cameraUp, projection, 0, -1);
}

void PortalSystem::renderPortalViewsRecursive(
    const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    const glm::mat4& projection, int recursionDepth, int fromPortalId) {

    // Limit recursion using constant
    if (recursionDepth >= PortalConstants::MAX_RECURSION_DEPTH) return;

    GLint viewport[4];
    GLint currentFramebuffer;
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &currentFramebuffer);

    // Render view for each portal
    for (size_t i = 0; i < portals.size(); i++) {
        auto& portal = portals[i];

        // Skip portal we came from
        if (static_cast<int>(portal.portalId) == fromPortalId) continue;

        // Check portal validity with bounds checking
        if (!portal.active || portal.destinationPortalId < 0 ||
            portal.destinationPortalId >= static_cast<int>(portals.size())) continue;

        const Portal& destPortal = portals[portal.destinationPortalId];

        // Switch to portal's framebuffer for render-to-texture
        glBindFramebuffer(GL_FRAMEBUFFER, portal.framebuffer);

        // Set viewport to texture size
        glViewport(0, 0, textureSize, textureSize);
        glClearColor(0.01f, 0.008f, 0.005f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Set proper render state
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_CULL_FACE);

        // Calculate view matrix as if camera was at destination portal
        glm::mat4 portalView = calculatePortalViewMatrix(portal, destPortal, cameraPos, cameraFront, cameraUp);
        glm::mat4 portalProjection = glm::perspective(
            glm::radians(60.0f),
            1.0f,
            0.1f, 100.0f
        );

        // RECURSIVE CALL - render portals within portals using constant
        if (recursionDepth < PortalConstants::RENDER_RECURSION_LIMIT) {
            renderPortalViewsRecursive(renderScene, cameraPos, cameraFront, cameraUp,
                portalProjection, recursionDepth + 1, portal.destinationPortalId);
        }

        // Render the actual scene from portal's perspective
        renderScene(portalView, portalProjection);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Restore OpenGL state
    glBindFramebuffer(GL_FRAMEBUFFER, currentFramebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
}

glm::mat4 PortalSystem::calculatePortalViewMatrix(const Portal& fromPortal, const Portal& toPortal,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp) const {

    // Position virtual camera behind destination portal using constant
    glm::vec3 virtualCameraPos = toPortal.position - (PortalConstants::VIRTUAL_CAMERA_DISTANCE * toPortal.normal);

    // Calculate portal coordinate systems for direction transformation
    glm::vec3 fromRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), fromPortal.normal));
    glm::vec3 fromUp = glm::normalize(glm::cross(fromPortal.normal, fromRight));
    glm::vec3 toRight = glm::normalize(glm::cross(glm::vec3(0, 1, 0), toPortal.normal));
    glm::vec3 toUp = glm::normalize(glm::cross(toPortal.normal, toRight));

    // Transform camera direction through portal with controlled influence
    float frontRight = glm::dot(cameraFront, fromRight) * PortalConstants::CAMERA_INFLUENCE_FACTOR;
    float frontUp = glm::dot(cameraFront, fromUp) * PortalConstants::CAMERA_INFLUENCE_FACTOR;
    float frontForward = glm::dot(cameraFront, fromPortal.normal);

    glm::vec3 virtualCameraFront =
        -(frontRight * toRight) +
        (frontUp * toUp) +
        (-frontForward * toPortal.normal);
    virtualCameraFront = glm::normalize(virtualCameraFront);

    glm::vec3 lookTarget = virtualCameraPos + virtualCameraFront * PortalConstants::VIRTUAL_CAMERA_DISTANCE;
    return glm::lookAt(virtualCameraPos, lookTarget, cameraUp);
}

void PortalSystem::renderPortalSurfaces(Shader& portalShader, const glm::mat4& view, const glm::mat4& projection,
    const glm::vec3& cameraPos, float time) {

    if (!areActive()) return;

    portalShader.use();
    portalShader.setMat4("view", &view[0][0]);
    portalShader.setMat4("projection", &projection[0][0]);
    portalShader.setFloat("time", time);

    // Enable alpha blending for portal transparency effects
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (size_t i = 0; i < portals.size(); i++) {
        const auto& portal = portals[i];

        // Skip portals using distance culling constant
        if (!portal.active || portal.distanceFromPlayer > PortalConstants::DISTANCE_CULLING || portal.colorTexture == 0) continue;

        // Create model matrix for portal positioning and orientation
        glm::mat4 portalMatrix = glm::mat4(1.0f);
        portalMatrix = glm::translate(portalMatrix, portal.position);

        // Calculate portal orientation from normal vector
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, portal.normal));
        up = glm::normalize(glm::cross(portal.normal, right));

        // Build rotation matrix from basis vectors
        glm::mat4 rotation = glm::mat4(1.0f);
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(portal.normal, 0.0f);
        portalMatrix = portalMatrix * rotation;

        portalShader.setMat4("model", &portalMatrix[0][0]);
        portalShader.setBool("portalActive", true);

        // Bind portal's rendered view texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, portal.colorTexture);
        portalShader.setInt("portalView", 0);

        // Render portal quad
        if (portal.portalVAO != 0) {
            glBindVertexArray(portal.portalVAO);
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glBindVertexArray(0);
        }
    }

    // Restore OpenGL state
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}

bool PortalSystem::checkPortalCollision(const glm::vec3& oldPos, const glm::vec3& newPos, glm::vec3& teleportPos) const {
    for (const auto& portal : portals) {
        // Check portal validity with bounds checking
        if (!portal.active || portal.destinationPortalId < 0 ||
            portal.destinationPortalId >= static_cast<int>(portals.size())) continue;

        // Only check nearby portals using collision distance constant
        float distToPortal = glm::length(newPos - portal.position);
        if (distToPortal > PortalConstants::COLLISION_DISTANCE) continue;

        // Define portal plane using normal and position
        glm::vec3 portalNormal = portal.normal;
        float portalPlaneD = -glm::dot(portalNormal, portal.position);

        // Calculate distance from old and new positions to portal plane
        float oldDist = glm::dot(portalNormal, oldPos) + portalPlaneD;
        float newDist = glm::dot(portalNormal, newPos) + portalPlaneD;

        // Check if player crossed the portal plane using threshold constant
        if (oldDist > PortalConstants::PLANE_THRESHOLD && newDist <= PortalConstants::PLANE_THRESHOLD) {
            // Find exact intersection point on portal plane
            float t = oldDist / (oldDist - newDist);
            glm::vec3 intersectionPoint = oldPos + t * (newPos - oldPos);

            // Check if intersection is within portal bounds
            glm::vec3 localPos = intersectionPoint - portal.position;

            // Calculate portal local coordinate system
            glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
            glm::vec3 right = glm::normalize(glm::cross(up, portal.normal));
            up = glm::normalize(glm::cross(portal.normal, right));

            // Project intersection to portal local coordinates
            float u = glm::dot(localPos, right);
            float v = glm::dot(localPos, up);

            float halfWidth = portal.width * 0.5f;
            float halfHeight = portal.height * 0.5f;

            // Check if within portal bounds
            if (abs(u) <= halfWidth && abs(v) <= halfHeight) {
                const Portal& destPortal = portals[portal.destinationPortalId];

                // Simple teleportation: maintain relative position
                glm::vec3 relativePos = newPos - portal.position;
                teleportPos = destPortal.position + relativePos;

                // Add offset using constant to prevent immediate re-collision
                teleportPos += destPortal.normal * PortalConstants::TELEPORT_OFFSET;

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