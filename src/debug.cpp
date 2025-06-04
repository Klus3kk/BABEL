#include "debug.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <GLFW/glfw3.h>

// Static member definitions - debug system state
bool DebugSystem::debugMode = false;
bool DebugSystem::showPerformanceStats = true;
bool DebugSystem::showPortalInfo = true;
bool DebugSystem::showLightingInfo = false;
bool DebugSystem::showSceneInfo = false;
bool DebugSystem::showCameraInfo = true;

// Performance tracking variables
float DebugSystem::frameTime = 0.0f;
float DebugSystem::fps = 0.0f;
int DebugSystem::frameCount = 0;
float DebugSystem::lastTime = 0.0f;

void DebugSystem::initialize() {
    std::cout << "=== BABEL DEBUG SYSTEM INITIALIZED ===" << std::endl;
    std::cout << "Press F10 to toggle debug mode" << std::endl;
    std::cout << "Debug categories:" << std::endl;
    std::cout << "  F1 - Performance stats" << std::endl;
    std::cout << "  F2 - Portal information" << std::endl;
    std::cout << "  F3 - Lighting information" << std::endl;
    std::cout << "  F4 - Scene information" << std::endl;
    std::cout << "  F5 - Camera information" << std::endl;
    std::cout << "=======================================" << std::endl;
}

void DebugSystem::toggleDebugMode() {
    debugMode = !debugMode;
    if (debugMode) {
        std::cout << "\n=== DEBUG MODE ACTIVATED ===" << std::endl;
    }
    else {
        std::cout << "\n=== DEBUG MODE DEACTIVATED ===" << std::endl;
    }
}

void DebugSystem::updatePerformanceStats(float deltaTime) {
    frameTime = deltaTime;  // Store current frame time
    frameCount++;           // Increment frame counter

    float currentTime = static_cast<float>(glfwGetTime());

    // Calculate FPS every second
    if (currentTime - lastTime >= 1.0f) {
        fps = frameCount / (currentTime - lastTime);  // Frames per second
        frameCount = 0;      // Reset counter
        lastTime = currentTime;

        // Print performance stats if enabled and debug mode is on
        if (showPerformanceStats && debugMode) {
            std::cout << "FPS: " << fps << " | Frame Time: " << frameTime * 1000.0f << "ms" << std::endl;
        }
    }
}

void DebugSystem::printCameraInfo(const glm::vec3& pos, const glm::vec3& front, float yaw, float pitch) {
    if (!showCameraInfo || !debugMode) return;

    std::cout << "\n=== CAMERA INFO ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);  // Format floats to 2 decimal places
    std::cout << "Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    std::cout << "Front: (" << front.x << ", " << front.y << ", " << front.z << ")" << std::endl;
    std::cout << "Yaw: " << yaw << " degrees" << std::endl;
    std::cout << "Pitch: " << pitch << " degrees" << std::endl;
    std::cout << "===================" << std::endl;
}

void DebugSystem::printLightingInfo(const LightingManager& lightingManager) {
    if (!showLightingInfo || !debugMode) return;

    std::cout << "\n=== LIGHTING INFO ===" << std::endl;
    std::cout << "Point lights: " << lightingManager.pointLights.size() << std::endl;
    std::cout << "Ambient strength: " << lightingManager.ambientStrength << std::endl;
    std::cout << "======================" << std::endl;
}

void DebugSystem::printSceneInfo(const Scene& scene,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& portalModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel) {

    if (!showSceneInfo || !debugMode) return;

    std::cout << "\n=== SCENE INFO ===" << std::endl;
    std::cout << "Total objects: " << scene.objects.size() << std::endl;

    // Count different object types by comparing model pointers
    int books = 0, shelves = 0, torches = 0, animated = 0;
    for (const auto& obj : scene.objects) {
        if (obj.model == bookModel.get()) books++;
        else if (obj.model == bookshelfModel.get() || obj.model == bookshelf2Model.get()) shelves++;
        else if (obj.model == torchModel.get()) torches++;

        // Count objects with any animation enabled
        if (obj.rotating || obj.floating || obj.orbiting || obj.pulsing) animated++;
    }

    std::cout << "Object breakdown:" << std::endl;
    std::cout << "  - Books: " << books << std::endl;
    std::cout << "  - Bookshelves: " << shelves << std::endl;
    std::cout << "  - Torches: " << torches << std::endl;
    std::cout << "  - Other: " << (scene.objects.size() - books - shelves - torches) << std::endl;
    std::cout << "Animated objects: " << animated << std::endl;

    std::cout << "==================" << std::endl;
}

// Toggle functions for different debug categories
void DebugSystem::togglePerformanceStats() {
    showPerformanceStats = !showPerformanceStats;
    std::cout << "Performance stats: " << (showPerformanceStats ? "ON" : "OFF") << std::endl;
}

void DebugSystem::togglePortalInfo() {
    showPortalInfo = !showPortalInfo;
    std::cout << "Portal info: " << (showPortalInfo ? "ON" : "OFF") << std::endl;
}

void DebugSystem::toggleLightingInfo() {
    showLightingInfo = !showLightingInfo;
    std::cout << "Lighting info: " << (showLightingInfo ? "ON" : "OFF") << std::endl;
}

void DebugSystem::toggleSceneInfo() {
    showSceneInfo = !showSceneInfo;
    std::cout << "Scene info: " << (showSceneInfo ? "ON" : "OFF") << std::endl;
}

void DebugSystem::toggleCameraInfo() {
    showCameraInfo = !showCameraInfo;
    std::cout << "Camera info: " << (showCameraInfo ? "ON" : "OFF") << std::endl;
}