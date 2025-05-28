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
    glm::vec3 normal;        // Portal face direction
    glm::vec3 up;            // Portal up direction
    glm::vec3 right;         // Portal right direction

    // Portal dimensions
    float width = 2.0f;
    float height = 3.0f;

    // Destination portal connection
    int destinationPortalId = -1;
    glm::vec3 destinationOffset = glm::vec3(0, 0, 30); // Default offset if no destination

    // Render targets for recursive rendering
    GLuint framebuffer = 0;
    GLuint colorTexture = 0;
    GLuint depthTexture = 0;

    // Portal state
    bool active = true;
    float distanceFromPlayer = 0.0f;
    int portalId = 0;

    // Portal geometry for stencil testing
    GLuint stencilVAO = 0;
    GLuint stencilVBO = 0;
    GLuint stencilEBO = 0;

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
    int textureSize = 512;
    int maxRecursionDepth = 4;

    std::vector<RoomVariation> roomVariations;

    // Portal geometry generation
    void generatePortalGeometry(Portal& portal);
    void cleanupPortalGeometry(Portal& portal);

    // Portal transformation math
    glm::mat4 calculatePortalTransformation(const Portal& fromPortal, const Portal& toPortal,
        const glm::mat4& originalView) const;

    // Oblique frustum clipping (from CS148 tutorial)
    glm::mat4 createObliqueProjection(const glm::mat4& projection, const glm::vec4& clipPlane) const;

    // Portal plane equation
    glm::vec4 getPortalPlane(const Portal& portal) const;

    // Check if point is in front of portal
    bool isInFrontOfPortal(const Portal& portal, const glm::vec3& point) const;

    // Render portal stencil mask
    void renderPortalStencil(const Portal& portal, const glm::mat4& view,
        const glm::mat4& projection, int stencilValue) const;

public:
    PortalSystem();
    ~PortalSystem();

    // Initialization
    void initialize(float roomRadius, float roomHeight);
    void cleanup();

    // Portal management
    void addPortal(const glm::vec3& position, const glm::vec3& normal,
        const glm::vec3& destinationOffset = glm::vec3(0, 0, 30));
    void connectPortals(int portal1Id, int portal2Id);

    // Rendering
    void renderPortalViews(const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
        const glm::vec3& playerPos, const glm::vec3& playerDir,
        const glm::mat4& projection);

    void renderRecursivePortals(const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
        const glm::mat4& view, const glm::mat4& projection, int recursionLevel = 0);

    // Portal texture binding for shader
    void bindPortalTexture(int portalIndex, Shader& shader) const;

    // Utility functions
    void updateDistances(const glm::vec3& playerPos);
    void setActive(bool active);
    void setQuality(int size);
    void setRecursionDepth(int depth);

    // Getters
    size_t getPortalCount() const { return portals.size(); }
    bool areActive() const { return !portals.empty() && portals[0].active; }
    const Portal& getPortal(int index) const { return portals[index]; }

    // Debug
    void printDebugInfo() const;

    // Player interaction
    bool checkPortalCollision(const glm::vec3& oldPos, const glm::vec3& newPos, glm::vec3& teleportPos) const;
    glm::vec3 transformThroughPortal(const Portal& fromPortal, const Portal& toPortal, const glm::vec3& position) const;
};