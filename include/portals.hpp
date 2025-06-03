#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <functional>
#include "shader.hpp"

struct Portal {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 up;
    glm::vec3 right;

    // Portal dimensions
    float width = 6.0f;
    float height = 6.0f;

    // Destination portal connection
    int destinationPortalId = -1;

    // Render targets for portal views
    GLuint framebuffer = 0;
    GLuint colorTexture = 0;
    GLuint depthTexture = 0;

    // Portal state
    bool active = true;
    float distanceFromPlayer = 0.0f;
    int portalId = 0;

    // Portal geometry for rendering
    GLuint portalVAO = 0;
    GLuint portalVBO = 0;
    GLuint portalEBO = 0;

    Portal() = default;
    Portal(const glm::vec3& pos, const glm::vec3& norm, int id)
        : position(pos), normal(glm::normalize(norm)), portalId(id) {
        // Calculate up and right vectors for portal orientation
        up = glm::vec3(0, 1, 0);
        right = glm::normalize(glm::cross(normal, up));
        up = glm::normalize(glm::cross(right, normal));
    }
};

// Room generation parameters for infinite variety
struct RoomVariation {
    glm::vec3 colorTint = glm::vec3(1.0f);
    float scaleMultiplier = 1.0f;
    float lightIntensityMultiplier = 1.0f;
    glm::vec3 ambientColorShift = glm::vec3(0.0f);
};

class PortalSystem {
private:
    std::vector<Portal> portals;
    int textureSize = 512;  // Optimized for performance
    int maxRecursionDepth = 3;
    bool enabled = true;

    std::vector<RoomVariation> roomVariations;

    // Portal geometry generation
    void generatePortalGeometry(Portal& portal);
    void cleanupPortalGeometry(Portal& portal);

    // Portal transformation math
    glm::mat4 calculatePortalViewMatrix(const Portal& fromPortal, const Portal& toPortal,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp) const;

public:
    ~PortalSystem();

    // Initialization
    void initialize();
    void cleanup();

    // Portal management
    void addPortal(const glm::vec3& position, const glm::vec3& normal);
    void connectPortals(int portal1Id, int portal2Id);

    // State management - THESE WERE MISSING
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() const { return enabled; }
    bool areActive() const { return enabled && !portals.empty(); }

    // Main rendering functions
    void renderPortalViews(const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
        const glm::mat4& projection);

    void renderPortalSurfaces(Shader& portalShader, const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos, float time);

    // Add this method to the header file (portals.hpp)
    void renderPortalViewsRecursive(
        const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
        const glm::mat4& projection, int recursionDepth);

    // Player interaction
    bool checkPortalCollision(const glm::vec3& oldPos, const glm::vec3& newPos, glm::vec3& teleportPos) const;

    // Utility functions
    void updateDistances(const glm::vec3& playerPos);
    void setQuality(int textureResolution);
    void setRecursionDepth(int depth);

    // Getters
    size_t getPortalCount() const { return portals.size(); }
    const Portal& getPortal(int index) const { return portals[index]; }
    int getRecursionDepth() const { return maxRecursionDepth; }
    // Debug
    void printDebugInfo() const;
};