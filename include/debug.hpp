#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include "scene.hpp"
#include "model.hpp"
#include "LightingManager.hpp"
#include "portals.hpp"

class DebugSystem {
private:
    // Debug state flags
    static bool debugMode;
    static bool showPerformanceStats;
    static bool showPortalInfo;
    static bool showLightingInfo;
    static bool showSceneInfo;
    static bool showCameraInfo;

    // Performance tracking
    static float frameTime;    // Time for last frame
    static float fps;          // Frames per second
    static int frameCount;     // Frame counter
    static float lastTime;     // Last time FPS was calculated

public:
    // System control
    static void initialize();                              // Setup debug system
    static void toggleDebugMode();                        // Toggle debug on/off
    static bool isDebugMode() { return debugMode; }

    // Performance monitoring
    static void updatePerformanceStats(float deltaTime);  // Update FPS counter

    // Debug info printing (only prints if debug mode is on and specific category is enabled)
    static void printCameraInfo(const glm::vec3& pos, const glm::vec3& front, float yaw, float pitch);
    static void printLightingInfo(const LightingManager& lightingManager);
    static void printSceneInfo(const Scene& scene,
        const std::unique_ptr<Model>& bookModel,
        const std::unique_ptr<Model>& bookshelfModel,
        const std::unique_ptr<Model>& bookshelf2Model,
        const std::unique_ptr<Model>& columnModel,
        const std::unique_ptr<Model>& floorModel,
        const std::unique_ptr<Model>& lampModel,
        const std::unique_ptr<Model>& portalModel,
        const std::unique_ptr<Model>& ceilingModel,
        const std::unique_ptr<Model>& wallModel,
        const std::unique_ptr<Model>& torchModel);

    // Toggle specific debug categories
    static void togglePerformanceStats();
    static void togglePortalInfo();
    static void toggleLightingInfo();
    static void toggleSceneInfo();
    static void toggleCameraInfo();
};