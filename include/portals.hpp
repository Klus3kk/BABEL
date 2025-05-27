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

    // Render targets for recursive rendering
    GLuint framebuffer = 0;
    GLuint colorTexture = 0;
    GLuint depthTexture = 0;

    bool active = true;
    float distanceFromPlayer = 0.0f;
    int portalId = 0;
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
    int textureSize = 512;
    int maxRecursionDepth = 4; // How many levels deep to render



    // Infinite library additions
    bool infiniteMode = true;
    glm::ivec3 lastPlayerRoom = glm::ivec3(INT_MAX);
    float roomGridSize = 30.0f;

public:
    PortalSystem();
    ~PortalSystem();
    std::vector<RoomVariation> roomVariations;
    void initialize(float roomRadius, float roomHeight);
    void renderPortalViews(const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
        const glm::vec3& playerPos, const glm::vec3& playerDir,
        const glm::mat4& projection);

    // Enhanced portal rendering with optimization
    void renderPortalViewsOptimized(const std::function<void(const glm::mat4&, const glm::mat4&, int, const RoomVariation&)>& renderScene,
        const glm::vec3& playerPos, const glm::vec3& playerDir,
        const glm::mat4& projection);

    void bindPortalTexture(int portalIndex, Shader& shader);
    void updateDistances(const glm::vec3& playerPos);

    // Portal transformation math
    glm::mat4 calculatePortalView(const Portal& portal, const glm::vec3& playerPos,
        const glm::vec3& playerDir, int recursionLevel);

    void setActive(bool active);
    void setQuality(int size);
    void setRecursionDepth(int depth);
    void cleanup();

    // Infinite library methods
    void forceGenerateRooms(const glm::vec3& playerPos);
    void generateInfiniteRooms(const glm::vec3& playerPos);
    void updateRoomGeneration(const glm::vec3& playerPos);
    void setInfiniteMode(bool enabled) { infiniteMode = enabled; }

    size_t getPortalCount() const { return portals.size(); }
    bool areActive() const { return !portals.empty() && portals[0].active; }
    void printDebugInfo() const;

private:
    // Helper methods for infinite library
    glm::ivec3 worldToRoomGrid(const glm::vec3& worldPos);
    RoomVariation generateRoomVariation(int x, int y, int z);
};