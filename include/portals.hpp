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
    glm::vec3 destinationPos; // Where this portal leads
    glm::vec3 destinationNormal;

    // NEW: Add these for stencil buffer rendering
    Portal* destination = nullptr;  // Link to destination portal

    bool active = true;
    float distanceFromPlayer = 0.0f;
    int portalId = 0;

    // Calculate clipped projection matrix for proper portal rendering
    glm::mat4 clippedProjMat(const glm::mat4& viewMat, const glm::mat4& projMat) const;
};

// Room generation parameters
struct RoomVariation {
    glm::vec3 colorTint = glm::vec3(1.0f);
    float scaleMultiplier = 1.0f;
    float roomOffset = 30.0f; // Distance between rooms
};

class PortalSystem {
private:
    std::vector<Portal> portals;
    std::vector<RoomVariation> roomVariations;

    // NEW: Stencil buffer settings
    size_t maxRecursionDepth = 5;
    float teleportThreshold = 2.0f;

public:
    PortalSystem();
    ~PortalSystem();

    void initialize(float roomRadius, float roomHeight);

    // EXISTING: Keep your existing function signature but change implementation
    void renderPortalViews(const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
        const glm::vec3& playerPos, const glm::vec3& playerDir,
        const glm::mat4& projection);

    // EXISTING: Keep existing functions
    void bindPortalTexture(int portalIndex, Shader& shader);
    void updateDistances(const glm::vec3& playerPos);

    // Portal transformation math
    glm::mat4 calculatePortalView(const Portal& portal, const glm::vec3& playerPos,
        const glm::vec3& playerDir, int recursionLevel);

    void setActive(bool active);
    void setQuality(int size);
    void setRecursionDepth(int depth);
    void cleanup();

    size_t getPortalCount() const { return portals.size(); }
    bool areActive() const { return !portals.empty() && portals[0].active; }
    void printDebugInfo() const;

    // NEW: Add stencil buffer rendering functions
    void drawRecursivePortals(const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
        const glm::mat4& viewMat,
        const glm::mat4& projMat,
        size_t recursionLevel = 0);

    void drawNonPortals(const std::function<void(const glm::mat4&, const glm::mat4&)>& renderScene,
        const glm::mat4& viewMat,
        const glm::mat4& projMat);

    void drawPortalGeometry(const Portal& portal, const glm::mat4& viewMat, const glm::mat4& projMat, Shader& portalShader, const std::function<void()>& drawPortalModel);

    // NEW: Handle player teleportation
    void handlePlayerTeleport(glm::vec3& playerPos);
};