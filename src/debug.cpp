#include "debug.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>

// Static member definitions
bool DebugSystem::debugMode = false;
bool DebugSystem::showPerformanceStats = true;
bool DebugSystem::showPortalInfo = true;
bool DebugSystem::showLightingInfo = false;
bool DebugSystem::showSceneInfo = false;
bool DebugSystem::showCameraInfo = true;

float DebugSystem::frameTime = 0.0f;
float DebugSystem::fps = 0.0f;
int DebugSystem::frameCount = 0;
float DebugSystem::lastTime = 0.0f;

void DebugSystem::initialize() {
    std::cout << "=== BABEL DEBUG SYSTEM INITIALIZED ===" << std::endl;
    std::cout << "Press H to toggle debug mode" << std::endl;
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
        std::cout << "\n*** DEBUG MODE ACTIVATED ***" << std::endl;
    }
    else {
        std::cout << "\n*** DEBUG MODE DEACTIVATED ***" << std::endl;
    }
}

void DebugSystem::updatePerformanceStats(float deltaTime) {
    frameTime = deltaTime;
    frameCount++;

    float currentTime = glfwGetTime();
    if (currentTime - lastTime >= 1.0f) {
        fps = frameCount / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
    }
}

void DebugSystem::printCameraInfo(const glm::vec3& pos, const glm::vec3& front, float yaw, float pitch) {
    if (!showCameraInfo) return;

    std::cout << "\n=== CAMERA INFO ===" << std::endl;
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Position: (" << pos.x << ", " << pos.y << ", " << pos.z << ")" << std::endl;
    std::cout << "Front: (" << front.x << ", " << front.y << ", " << front.z << ")" << std::endl;
    std::cout << "Yaw: " << yaw << "°" << std::endl;
    std::cout << "Pitch: " << pitch << "°" << std::endl;
    std::cout << "===================" << std::endl;
}

void DebugSystem::printLightingInfo(const LightingManager& lightingManager) {
    if (!showLightingInfo) return;

    std::cout << "\n=== LIGHTING INFO ===" << std::endl;
    std::cout << "Point lights: " << lightingManager.pointLights.size() << std::endl;
    std::cout << "Directional lights: " << lightingManager.directionalLights.size() << std::endl;
    std::cout << "Ambient strength: " << lightingManager.ambientStrength << std::endl;

    int flickeringLights = 0;
    int movingLights = 0;
    for (const auto& light : lightingManager.pointLights) {
        if (light.flickering) flickeringLights++;
        if (light.moving) movingLights++;
    }

    std::cout << "Animated lights: " << (flickeringLights + movingLights) << std::endl;
    std::cout << "  - Flickering: " << flickeringLights << std::endl;
    std::cout << "  - Moving: " << movingLights << std::endl;

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
    if (!showSceneInfo) return;

    std::cout << "\n=== SCENE INFO ===" << std::endl;
    std::cout << "Total objects: " << scene.objects.size() << std::endl;

    // Count object types
    int books = 0, shelves = 0, portals = 0, torches = 0, animated = 0;
    for (const auto& obj : scene.objects) {
        if (obj.model == bookModel.get()) books++;
        else if (obj.model == bookshelfModel.get() || obj.model == bookshelf2Model.get()) shelves++;
        else if (obj.model == portalModel.get()) portals++;
        else if (obj.model == torchModel.get()) torches++;

        if (obj.rotating || obj.floating || obj.orbiting || obj.pulsing) animated++;
    }

    std::cout << "Object breakdown:" << std::endl;
    std::cout << "  - Books: " << books << std::endl;
    std::cout << "  - Bookshelves: " << shelves << std::endl;
    std::cout << "  - Portals: " << portals << std::endl;
    std::cout << "  - Torches: " << torches << std::endl;
    std::cout << "  - Other: " << (scene.objects.size() - books - shelves - portals - torches) << std::endl;
    std::cout << "Animated objects: " << animated << std::endl;

    std::cout << "==================" << std::endl;
}

void DebugSystem::printOpenGLInfo() {
    std::cout << "\n=== OPENGL INFO ===" << std::endl;
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    GLint maxTextureUnits, maxLights, maxVertexAttribs;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextureUnits);
    glGetIntegerv(GL_MAX_LIGHTS, &maxLights);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);

    std::cout << "Max texture units: " << maxTextureUnits << std::endl;
    std::cout << "Max vertex attributes: " << maxVertexAttribs << std::endl;
    std::cout << "===================" << std::endl;
}

void DebugSystem::checkOpenGLErrors(const std::string& operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cout << "OpenGL Error in " << operation << ": ";
        switch (error) {
        case GL_INVALID_ENUM: std::cout << "GL_INVALID_ENUM"; break;
        case GL_INVALID_VALUE: std::cout << "GL_INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: std::cout << "GL_INVALID_OPERATION"; break;
        case GL_OUT_OF_MEMORY: std::cout << "GL_OUT_OF_MEMORY"; break;
        default: std::cout << "Unknown error " << error; break;
        }
        std::cout << std::endl;
    }
}

void DebugSystem::togglePerformanceStats() { showPerformanceStats = !showPerformanceStats; }
void DebugSystem::togglePortalInfo() { showPortalInfo = !showPortalInfo; }
void DebugSystem::toggleLightingInfo() { showLightingInfo = !showLightingInfo; }
void DebugSystem::toggleSceneInfo() { showSceneInfo = !showSceneInfo; }
void DebugSystem::toggleCameraInfo() { showCameraInfo = !showCameraInfo; }

void DebugSystem::printAllDebugInfo(const glm::vec3& cameraPos, const glm::vec3& cameraFront,
    float yaw, float pitch, float roomRadius, float roomHeight, int numSides,
    const Scene& scene, const PortalSystem& portalSystem,
    const LightingManager& lightingManager,
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

    std::cout << "\n" << std::string(50, '=') << std::endl;
    std::cout << "         BABEL DEBUG INFORMATION" << std::endl;
    std::cout << std::string(50, '=') << std::endl;

    // Room info
    std::cout << "\n=== ROOM CONFIGURATION ===" << std::endl;
    std::cout << "Radius: " << roomRadius << ", Height: " << roomHeight << ", Sides: " << numSides << std::endl;

    // Debug info based on toggle states
    printCameraInfo(cameraPos, cameraFront, yaw, pitch);
    printLightingInfo(lightingManager);
    printSceneInfo(scene, bookModel, bookshelfModel, bookshelf2Model, columnModel,
        floorModel, lampModel, portalModel, ceilingModel, wallModel, torchModel);

    std::cout << "\n" << std::string(50, '=') << std::endl;
}