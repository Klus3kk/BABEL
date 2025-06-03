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
    static bool debugMode;
    static bool showPerformanceStats;
    static bool showPortalInfo;
    static bool showLightingInfo;
    static bool showSceneInfo;
    static bool showCameraInfo;

    // Performance tracking
    static float frameTime;
    static float fps;
    static int frameCount;
    static float lastTime;

public:
    // Initialize debug system
    static void initialize();

    // Toggle debug mode
    static void toggleDebugMode();
    static bool isDebugMode() { return debugMode; }

    // Performance monitoring
    static void updatePerformanceStats(float deltaTime);

    // Debug info printing
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