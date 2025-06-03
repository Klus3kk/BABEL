#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <functional>
#include "shader.hpp"

// Individual portal structure
struct Portal {
    glm::vec3 position;      // Portal location in world
    glm::vec3 normal;        // Direction portal faces
    glm::vec3 up, right;     // Portal coordinate system

    // Portal dimensions
    float width = 6.0f;
    float height = 6.0f;

    // Connection to other portals
    int destinationPortalId = -1;  // Which portal this connects to (-1 = none)

    // Render targets for portal view
    GLuint framebuffer = 0;    // Framebuffer for render-to-texture
    GLuint colorTexture = 0;   // What you see through portal
    GLuint depthTexture = 0;   // Depth buffer for proper 3D rendering

    // Portal state
    bool active = true;
    float distanceFromPlayer = 0.0f; // For culling distant portals
    int portalId = 0;               // Unique identifier

    // Portal geometry for rendering
    GLuint portalVAO = 0;    // Vertex array for portal quad
    GLuint portalVBO = 0;    // Vertex buffer
    GLuint portalEBO = 0;    // Element buffer

    Portal() = default;
    Portal(const glm::vec3& pos, const glm::vec3& norm, int id)
        : position(pos), normal(glm::normalize(norm)), portalId(id) {
        // Calculate portal coordinate system from normal
        up = glm::vec3(0, 1, 0);
        right = glm::normalize(glm::cross(normal, up));
        up = glm::normalize(glm::cross(right, normal));
    }
};

class PortalSystem {
private:
    std::vector<Portal> portals;     // All portals in the system
    int textureSize = 4096;           // Resolution of portal view textures
    int maxRecursionDepth = 3;       // How many portal layers to render
    bool enabled = true;             // Global portal enable/disable

    // Internal methods
    void generatePortalGeometry(Portal& portal);      // Create quad mesh for portal
    void cleanupPortalGeometry(Portal& portal);       // Free portal geometry
    glm::mat4 calculatePortalViewMatrix(const Portal& fromPortal, const Portal& toPortal,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp) const;

public:
    ~PortalSystem();

    // Setup and management
    void initialize();                                           // Initialize system
    void cleanup();                                             // Free all resources
    void addPortal(const glm::vec3& position, const glm::vec3& normal); // Add new portal
    void connectPortals(int portal1Id, int portal2Id);         // Link two portals

    // State control
    void setEnabled(bool enable) { enabled = enable; }
    bool isEnabled() const { return enabled; }
    bool areActive() const { return enabled && !portals.empty(); }

    // Main rendering functions
    void renderPortalViews(const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
        const glm::mat4& projection);

    void renderPortalSurfaces(Shader& portalShader, const glm::mat4& view, const glm::mat4& projection,
        const glm::vec3& cameraPos, float time);

    // Recursive rendering for infinite depth effect
    void renderPortalViewsRecursive(
        const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
        const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
        const glm::mat4& projection, int recursionDepth);

    // Player interaction
    bool checkPortalCollision(const glm::vec3& oldPos, const glm::vec3& newPos, glm::vec3& teleportPos) const;

    // Utility functions
    void updateDistances(const glm::vec3& playerPos);           // Update distance for culling
    void setQuality(int textureResolution);                     // Change portal texture quality
    void setRecursionDepth(int depth);                         // Change recursion limit

    // Getters
    size_t getPortalCount() const { return portals.size(); }
    const Portal& getPortal(int index) const { return portals[index]; }
    int getRecursionDepth() const { return maxRecursionDepth; }
};
