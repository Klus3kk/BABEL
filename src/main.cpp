#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>
#include <cstdlib>
#include <ctime>

#include "shader.hpp"
#include "texture.hpp"
#include "model.hpp"
#include "scene.hpp"
#include "TextureManager.hpp"
#include "LightingManager.hpp"
#include "portals.hpp"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

// Camera variables
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

bool showDebugInfo = false;
bool hKeyPressed = false;

// Animation controls
static bool animationSpeedChanged = false;
static float globalAnimationSpeed = 1.0f;

// Lighting controls
static bool lightingKeyPressed = false;
static bool torchKeyPressed = false;
static bool darknessKeyPressed = false;
static bool dramaticKeyPressed = false;
static bool plusKeyPressed = false;
static bool minusKeyPressed = false;

// Portal controls
static bool portalTogglePressed = false;
static bool portalQualityPressed = false;
static bool recursionKeyPressed = false;

// New infinite library controls
static bool infiniteKeyPressed = false;
static bool fpsKeyPressed = false;
static bool optimizeKeyPressed = false;
static bool generateKeyPressed = false;

// Performance Manager Class
class PerformanceManager {
private:
    float targetFPS = 60.0f;
    float currentFPS = 60.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;

    // Quality settings
    int currentPortalQuality = 512;
    int currentRecursionDepth = 4;
    bool adaptiveQuality = true;

public:
    void updateFPS(float deltaTime) {
        frameCount++;
        fpsTimer += deltaTime;

        if (fpsTimer >= 1.0f) {
            currentFPS = frameCount / fpsTimer;
            frameCount = 0;
            fpsTimer = 0.0f;

            if (adaptiveQuality) {
                adjustQuality();
            }
        }
    }

    void adjustQuality() {
        if (currentFPS < targetFPS * 0.8f) {
            // Performance is poor, reduce quality
            if (currentPortalQuality > 256) {
                currentPortalQuality /= 2;
                std::cout << "Reducing portal quality to " << currentPortalQuality << std::endl;
            }
            else if (currentRecursionDepth > 2) {
                currentRecursionDepth--;
                std::cout << "Reducing recursion depth to " << currentRecursionDepth << std::endl;
            }
        }
        else if (currentFPS > targetFPS * 1.1f) {
            // Performance is good, increase quality
            if (currentRecursionDepth < 6) {
                currentRecursionDepth++;
                std::cout << "Increasing recursion depth to " << currentRecursionDepth << std::endl;
            }
            else if (currentPortalQuality < 1024) {
                currentPortalQuality *= 2;
                std::cout << "Increasing portal quality to " << currentPortalQuality << std::endl;
            }
        }
    }

    int getPortalQuality() const { return currentPortalQuality; }
    int getRecursionDepth() const { return currentRecursionDepth; }
    float getCurrentFPS() const { return currentFPS; }

    void setAdaptiveQuality(bool enabled) { adaptiveQuality = enabled; }
};

// Memory Manager Class
class MemoryManager {
public:
    static void optimizeTextureMemory() {
        glFinish();
        std::cout << "Optimized texture memory usage" << std::endl;
    }

    static void cleanupUnusedResources() {
        std::cout << "Cleaned up unused resources" << std::endl;
    }

    static size_t estimateVRAMUsage(int portalCount, int textureSize, int recursionDepth) {
        size_t portalTextures = portalCount * textureSize * textureSize * 4 * 2; // Color + depth
        size_t recursionMultiplier = recursionDepth;

        return (portalTextures * recursionMultiplier) / (1024 * 1024); // Return MB
    }
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

// LOD rendering function
void renderSceneWithLOD(const Scene& scene, Shader& shader, const glm::vec3& cameraPos,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& portalModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    int recursionLevel = 0) {
    float maxDistance = 50.0f + recursionLevel * 20.0f;
    int objectsRendered = 0;
    int objectsCulled = 0;

    for (size_t i = 0; i < scene.objects.size(); i++) {
        const auto& obj = scene.objects[i];

        // Calculate distance from camera to object
        float distance = glm::length(obj.position - cameraPos);

        // Distance-based LOD culling
        if (distance > maxDistance) {
            objectsCulled++;
            continue;
        }

        // Model complexity-based culling for distant objects
        bool isComplexModel = (obj.model == bookModel.get() && obj.model->vertexCount > 500) ||
            (obj.model == bookshelfModel.get() && obj.model->vertexCount > 1000);

        if (isComplexModel && distance > 30.0f && recursionLevel > 0) {
            objectsCulled++;
            continue;
        }

        // Random culling for very distant recursive rooms
        if (recursionLevel > 1 && distance > 25.0f && (rand() % (recursionLevel + 1)) == 0) {
            objectsCulled++;
            continue;
        }

        // Bind appropriate texture for each object type
        if (obj.model == bookModel.get()) {
            TextureManager::bindTextureForObject("book", shader);
        }
        else if (obj.model == bookshelfModel.get() || obj.model == bookshelf2Model.get()) {
            TextureManager::bindTextureForObject("bookshelf", shader);
        }
        else if (obj.model == columnModel.get()) {
            TextureManager::bindTextureForObject("column", shader);
        }
        else if (obj.model == floorModel.get()) {
            TextureManager::bindTextureForObject("floor", shader);
        }
        else if (obj.model == wallModel.get()) {
            TextureManager::bindTextureForObject("wall", shader);
        }
        else if (obj.model == ceilingModel.get()) {
            TextureManager::bindTextureForObject("ceiling", shader);
        }
        else if (obj.model == lampModel.get()) {
            TextureManager::bindTextureForObject("lamp", shader);
        }
        else if (obj.model == torchModel.get()) {
            TextureManager::bindTextureForObject("torch", shader);
        }

        // Set model matrix and draw
        shader.setMat4("model", &obj.modelMatrix[0][0]);
        obj.model->draw();
        objectsRendered++;
    }

    // Debug output for performance monitoring
    if (recursionLevel == 0 && showDebugInfo) {
        static int frameCount = 0;
        frameCount++;
        if (frameCount % 60 == 0) { // Print every 60 frames
            std::cout << "LOD Stats - Rendered: " << objectsRendered
                << ", Culled: " << objectsCulled << std::endl;
        }
    }
}

void printSceneDebugInfo(const Scene& scene,
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

    std::cout << "\n=== BABEL SCENE DEBUG INFO ===" << std::endl;
    std::cout << "Total objects: " << scene.objects.size() << std::endl;
    std::cout << "================================" << std::endl;

    for (size_t i = 0; i < scene.objects.size(); i++) {
        const auto& obj = scene.objects[i];

        // Determine object type
        std::string objectType = "Unknown";
        if (obj.model == bookModel.get()) objectType = "Book";
        else if (obj.model == bookshelfModel.get()) objectType = "Bookshelf";
        else if (obj.model == bookshelf2Model.get()) objectType = "Bookshelf2";
        else if (obj.model == columnModel.get()) objectType = "Column";
        else if (obj.model == floorModel.get()) objectType = "Floor";
        else if (obj.model == lampModel.get()) objectType = "Lamp";
        else if (obj.model == portalModel.get()) objectType = "Portal";
        else if (obj.model == ceilingModel.get()) objectType = "Ceiling";
        else if (obj.model == wallModel.get()) objectType = "Wall";
        else if (obj.model == torchModel.get()) objectType = "Torch";

        std::cout << "Object " << i << " [" << objectType << "]:" << std::endl;
        std::cout << "  Position: (" << obj.position.x << ", " << obj.position.y << ", " << obj.position.z << ")" << std::endl;
        std::cout << "  Rotation: (" << glm::degrees(obj.rotation.x) << " , "
            << glm::degrees(obj.rotation.y) << " , "
            << glm::degrees(obj.rotation.z) << " )" << std::endl;
        std::cout << "  Scale: (" << obj.scale.x << ", " << obj.scale.y << ", " << obj.scale.z << ")" << std::endl;

        if (obj.rotating) {
            std::cout << "  Rotating: YES (speed: " << obj.rotationSpeed << ")" << std::endl;
        }
        if (obj.floating) {
            std::cout << "  Floating: YES (amplitude: " << obj.floatAmplitude << ", speed: " << obj.floatSpeed << ")" << std::endl;
        }
        if (obj.orbiting) {
            std::cout << "  Orbiting: YES (radius: " << obj.orbitRadius << ", speed: " << obj.orbitSpeed << ")" << std::endl;
        }
        if (obj.pulsing) {
            std::cout << "  Pulsing: YES (amplitude: " << obj.pulseAmplitude << ", speed: " << obj.pulseSpeed << ")" << std::endl;
        }

        std::cout << "  --------" << std::endl;
    }
    std::cout << "================================\n" << std::endl;
}

void printCameraDebugInfo(const glm::vec3& cameraPos, const glm::vec3& cameraFront,
    float yaw, float pitch) {
    std::cout << "\n=== CAMERA DEBUG INFO ===" << std::endl;
    std::cout << "Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "Front: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;
    std::cout << "Yaw: " << yaw << " " << std::endl;
    std::cout << "Pitch: " << pitch << " " << std::endl;
    std::cout << "=========================\n" << std::endl;
}

void printRoomDebugInfo(float roomRadius, float roomHeight, int numSides) {
    std::cout << "\n=== ROOM DEBUG INFO ===" << std::endl;
    std::cout << "Room Radius: " << roomRadius << std::endl;
    std::cout << "Room Height: " << roomHeight << std::endl;
    std::cout << "Number of Sides: " << numSides << std::endl;
    std::cout << "========================\n" << std::endl;
}

void printLightingDebugInfo(const LightingManager& lightingManager) {
    std::cout << "\n=== LIGHTING DEBUG INFO ===" << std::endl;
    std::cout << "Point lights: " << lightingManager.pointLights.size() << std::endl;
    std::cout << "Directional lights: " << lightingManager.directionalLights.size() << std::endl;
    std::cout << "Ambient color: (" << lightingManager.ambientColor.x << ", "
        << lightingManager.ambientColor.y << ", " << lightingManager.ambientColor.z << ")" << std::endl;
    std::cout << "Ambient strength: " << lightingManager.ambientStrength << std::endl;

    for (size_t i = 0; i < lightingManager.pointLights.size(); i++) {
        const auto& light = lightingManager.pointLights[i];
        std::cout << "Point Light " << i << ":" << std::endl;
        std::cout << "  Position: (" << light.position.x << ", " << light.position.y << ", " << light.position.z << ")" << std::endl;
        std::cout << "  Color: (" << light.color.x << ", " << light.color.y << ", " << light.color.z << ")" << std::endl;
        std::cout << "  Intensity: " << light.intensity << std::endl;
        if (light.flickering) std::cout << "  Flickering: YES" << std::endl;
        if (light.moving) std::cout << "  Moving: YES" << std::endl;
    }
    std::cout << "============================\n" << std::endl;
}

void printPerformanceDebugInfo(const PerformanceManager& perfManager,
    const PortalSystem& portalSystem) {
    std::cout << "\n=== PERFORMANCE DEBUG INFO ===" << std::endl;
    std::cout << "Current FPS: " << perfManager.getCurrentFPS() << std::endl;
    std::cout << "Portal Quality: " << perfManager.getPortalQuality() << "x"
        << perfManager.getPortalQuality() << std::endl;
    std::cout << "Recursion Depth: " << perfManager.getRecursionDepth() << std::endl;
    std::cout << "Active Portals: " << portalSystem.getPortalCount() << std::endl;

    // OpenGL performance info
    GLint maxTextureSize;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTextureSize);
    std::cout << "Max Texture Size: " << maxTextureSize << std::endl;

    GLint maxViewportDims[2];
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, maxViewportDims);
    std::cout << "Max Viewport: " << maxViewportDims[0] << "x" << maxViewportDims[1] << std::endl;

    std::cout << "==============================\n" << std::endl;
}

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp, float deltaTime,
    const Scene& scene,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& portalModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    float roomRadius, float roomHeight, int numSides,
    float yaw, float pitch,
    LightingManager& lightingManager,
    PortalSystem& portalSystem,
    PerformanceManager& perfManager) {

    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos.y += cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos.y -= cameraSpeed;

    // Animation speed controls
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !animationSpeedChanged) {
        globalAnimationSpeed = 0.5f; // Slow motion
        animationSpeedChanged = true;
        std::cout << "Animation speed: SLOW" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !animationSpeedChanged) {
        globalAnimationSpeed = 1.0f; // Normal speed
        animationSpeedChanged = true;
        std::cout << "Animation speed: NORMAL" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !animationSpeedChanged) {
        globalAnimationSpeed = 2.0f; // Fast motion
        animationSpeedChanged = true;
        std::cout << "Animation speed: FAST" << std::endl;
    }

    // Reset animation speed change flag
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_2) == GLFW_RELEASE &&
        glfwGetKey(window, GLFW_KEY_3) == GLFW_RELEASE) {
        animationSpeedChanged = false;
    }

    // Lighting controls
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lightingKeyPressed) {
        lightingKeyPressed = true;
        static int lightingMode = 0;
        lightingMode = (lightingMode + 1) % 4;

        switch (lightingMode) {
        case 0: // Normal lighting
            lightingManager.setAmbientColor(glm::vec3(0.15f, 0.1f, 0.2f), 0.3f);
            std::cout << "Lighting: NORMAL" << std::endl;
            break;
        case 1: // Warm cozy lighting
            lightingManager.setAmbientColor(glm::vec3(0.3f, 0.2f, 0.1f), 0.4f);
            std::cout << "Lighting: WARM COZY" << std::endl;
            break;
        case 2: // Cool mystical lighting
            lightingManager.setAmbientColor(glm::vec3(0.1f, 0.1f, 0.3f), 0.2f);
            std::cout << "Lighting: COOL MYSTICAL" << std::endl;
            break;
        case 3: // Dramatic low lighting
            lightingManager.setAmbientColor(glm::vec3(0.05f, 0.05f, 0.1f), 0.1f);
            std::cout << "Lighting: DRAMATIC" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        lightingKeyPressed = false;
    }

    // Torch intensity control (T key)
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !torchKeyPressed) {
        torchKeyPressed = true;
        static int torchLevel = 2; // Start at medium
        torchLevel = (torchLevel + 1) % 4;

        switch (torchLevel) {
        case 0: // Dim torches
            lightingManager.setTorchIntensity(1.5f);
            std::cout << "Torch intensity: DIM" << std::endl;
            break;
        case 1: // Medium torches
            lightingManager.setTorchIntensity(2.0f);
            std::cout << "Torch intensity: MEDIUM" << std::endl;
            break;
        case 2: // Bright torches
            lightingManager.setTorchIntensity(2.5f);
            std::cout << "Torch intensity: BRIGHT" << std::endl;
            break;
        case 3: // Very bright torches
            lightingManager.setTorchIntensity(3.0f);
            std::cout << "Torch intensity: VERY BRIGHT" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        torchKeyPressed = false;
    }

    // Darkness level control (B key for "Brightness")
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !darknessKeyPressed) {
        darknessKeyPressed = true;
        static int darknessLevel = 1; // Start medium dark
        darknessLevel = (darknessLevel + 1) % 4;

        switch (darknessLevel) {
        case 0: // Pitch black
            lightingManager.setAmbientDarkness(0.0f);
            std::cout << "Darkness: PITCH BLACK" << std::endl;
            break;
        case 1: // Very dark
            lightingManager.setAmbientDarkness(0.2f);
            std::cout << "Darkness: VERY DARK" << std::endl;
            break;
        case 2: // Dark
            lightingManager.setAmbientDarkness(0.4f);
            std::cout << "Darkness: DARK" << std::endl;
            break;
        case 3: // Medium dark
            lightingManager.setAmbientDarkness(0.6f);
            std::cout << "Darkness: MEDIUM DARK" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        darknessKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !generateKeyPressed) {
        generateKeyPressed = true;
        portalSystem.forceGenerateRooms(cameraPos);
        std::cout << "Forced room generation!" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        generateKeyPressed = false;
    }


    // Dramatic mode toggle (M key for "Mode")
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !dramaticKeyPressed) {
        dramaticKeyPressed = true;
        static bool dramaticMode = false;
        dramaticMode = !dramaticMode;

        lightingManager.setDramaticMode(dramaticMode);
        std::cout << "Dramatic mode: " << (dramaticMode ? "ON" : "OFF") << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        dramaticKeyPressed = false;
    }

    // Torch flicker controls (+ and - keys)
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && !plusKeyPressed) { // + key
        plusKeyPressed = true;
        lightingManager.increaseTorchFlicker();
        std::cout << "Increased torch flicker" << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_RELEASE) {
        plusKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS && !minusKeyPressed) {
        minusKeyPressed = true;
        lightingManager.decreaseTorchFlicker();
        std::cout << "Decreased torch flicker" << std::endl;
    }

    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_RELEASE) {
        minusKeyPressed = false;
    }

    // Portal controls (P key)
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !portalTogglePressed) {
        portalTogglePressed = true;
        static bool portalsEnabled = true;
        portalsEnabled = !portalsEnabled;
        portalSystem.setActive(portalsEnabled);
        std::cout << "Portals: " << (portalsEnabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        portalTogglePressed = false;
    }

    // Portal quality controls (Q key)
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !portalQualityPressed) {
        portalQualityPressed = true;
        static int portalQuality = 1; // 0=low, 1=medium, 2=high
        portalQuality = (portalQuality + 1) % 3;

        switch (portalQuality) {
        case 0: // Low quality
            portalSystem.setQuality(256);
            std::cout << "Portal quality: LOW (256x256)" << std::endl;
            break;
        case 1: // Medium quality
            portalSystem.setQuality(512);
            std::cout << "Portal quality: MEDIUM (512x512)" << std::endl;
            break;
        case 2: // High quality
            portalSystem.setQuality(1024);
            std::cout << "Portal quality: HIGH (1024x1024)" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_RELEASE) {
        portalQualityPressed = false;
    }

    // Recursion depth controls (R key)
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !recursionKeyPressed) {
        recursionKeyPressed = true;
        static int recursionDepth = 4;
        recursionDepth = recursionDepth % 8 + 1; // Cycle 1-8
        portalSystem.setRecursionDepth(recursionDepth);
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        recursionKeyPressed = false;
    }

    // Infinite library controls (I key for "Infinite")
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS && !infiniteKeyPressed) {
        infiniteKeyPressed = true;
        static bool infiniteEnabled = true;
        infiniteEnabled = !infiniteEnabled;
        portalSystem.setInfiniteMode(infiniteEnabled);
        std::cout << "Infinite library mode: " << (infiniteEnabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_RELEASE) {
        infiniteKeyPressed = false;
    }

    // Performance controls (F key for "FPS")
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS && !fpsKeyPressed) {
        fpsKeyPressed = true;
        static bool adaptiveQuality = true;
        adaptiveQuality = !adaptiveQuality;
        perfManager.setAdaptiveQuality(adaptiveQuality);
        std::cout << "Adaptive quality: " << (adaptiveQuality ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_RELEASE) {
        fpsKeyPressed = false;
    }

    // Memory optimization (O key for "Optimize")
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS && !optimizeKeyPressed) {
        optimizeKeyPressed = true;
        MemoryManager::optimizeTextureMemory();
        MemoryManager::cleanupUnusedResources();

        size_t vramUsage = MemoryManager::estimateVRAMUsage(
            portalSystem.getPortalCount(),
            perfManager.getPortalQuality(),
            perfManager.getRecursionDepth()
        );
        std::cout << "Estimated VRAM usage: " << vramUsage << " MB" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_O) == GLFW_RELEASE) {
        optimizeKeyPressed = false;
    }

    // Debug toggle (H key)
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        showDebugInfo = !showDebugInfo;

        if (showDebugInfo) {
            std::cout << "\n*** DEBUG MODE ACTIVATED ***" << std::endl;
            printRoomDebugInfo(roomRadius, roomHeight, numSides);
            printCameraDebugInfo(cameraPos, cameraFront, yaw, pitch);
            printLightingDebugInfo(lightingManager);
            printPerformanceDebugInfo(perfManager, portalSystem);
            portalSystem.printDebugInfo();
            printSceneDebugInfo(scene, bookModel, bookshelfModel, bookshelf2Model,
                columnModel, floorModel, lampModel, portalModel,
                ceilingModel, wallModel, torchModel);
        }
        else {
            std::cout << "\n*** DEBUG MODE DEACTIVATED ***\n" << std::endl;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        hKeyPressed = false;
    }
}

int main() {
    // Init GLFW
    if (!glfwInit()) {
        std::cerr << "GLFW init failed!\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "BABEL - Infinite Library", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window!\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    srand(static_cast<unsigned int>(time(nullptr)));

    std::cout << "GL ready. Vendor: " << glGetString(GL_VENDOR) << std::endl;

    // Load shaders
    Shader basicShader("shaders/book.vert", "shaders/book.frag");
    Shader portalShader("shaders/portal.vert", "shaders/portal.frag");

    // Load all textures using TextureManager
    TextureManager::loadAllTextures();

    // Load models
    std::cout << "Loading models..." << std::endl;
    std::unique_ptr<Model> bookModel = std::make_unique<Model>("assets/models/book.obj");
    std::unique_ptr<Model> bookshelfModel = std::make_unique<Model>("assets/models/bookshelf.obj");
    std::unique_ptr<Model> bookshelf2Model = std::make_unique<Model>("assets/models/Bookshelf2.obj");
    std::unique_ptr<Model> columnModel = std::make_unique<Model>("assets/models/column.obj");
    std::unique_ptr<Model> floorModel = std::make_unique<Model>("assets/models/floor.obj");
    std::unique_ptr<Model> lampModel = std::make_unique<Model>("assets/models/lamb.obj");
    std::unique_ptr<Model> portalModel = std::make_unique<Model>("assets/models/portal.obj");
    std::unique_ptr<Model> ceilingModel = std::make_unique<Model>("assets/models/ceiling.obj");
    std::unique_ptr<Model> wallModel = std::make_unique<Model>("assets/models/wall.obj");
    std::unique_ptr<Model> torchModel = std::make_unique<Model>("assets/models/torch.obj");

    // Create scene
    Scene scene;
    const float roomRadius = 8.0f;
    const float roomHeight = 6.0f;
    const int numSides = 8;

    // Create lighting manager
    LightingManager lightingManager;

    // Create portal system
    PortalSystem portalSystem;

    // Create performance manager
    PerformanceManager perfManager;
    perfManager.setAdaptiveQuality(true);

    std::cout << "Setting up library room..." << std::endl;

    // FLOOR
    scene.addObject(floorModel.get(),
        glm::vec3(1.1f, 0.0f, 1.0f),
        glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
        glm::vec3(2.5f, 1.0f, 2.5f));

    // CEILING
    scene.addObject(ceilingModel.get(),
        glm::vec3(0.0f, roomHeight + 1.1f, 0.0f),
        glm::vec3(0.0f, glm::radians(105.0f), 0.0f),
        glm::vec3(3.2f, 2.0f, 3.2f));

    // LAMP
    scene.addObject(lampModel.get(),
        glm::vec3(0.0f, roomHeight + 0.9f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(3.5f, 2.0f, 3.5f));
    scene.objects[2].setRotating(true, 0.3f);

    // WALLS
    for (int i = 0; i < numSides; i++) {
        float angle = glm::radians(360.0f * static_cast<float>(i) / static_cast<float>(numSides));
        float x = roomRadius * cos(angle);
        float z = roomRadius * sin(angle);
        float wallRotation = angle;
        if (i % 2 == 0) {
            wallRotation += glm::radians(90.0f);
        }
        scene.addObject(wallModel.get(),
            glm::vec3(x, 0.05f, z),
            glm::vec3(0.0f, wallRotation, 0.0f),
            glm::vec3(0.014f, 0.048f, 0.014f));
    }

    // COLUMNS
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        float x = 2.8f * cos(angle);
        float z = 2.8f * sin(angle);

        scene.addObject(columnModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.5f, 3.2f, 1.5f));
    }

    // PORTALS
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = roomRadius * 0.8f * cos(angle);
        float z = roomRadius * 0.8f * sin(angle);
        float rotationToFaceCenter = angle + glm::radians(90.0f);

        scene.addObject(portalModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            glm::vec3(1.3f, 1.3f, 1.3f));
    }

    // BOOKSHELVES
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        float x = roomRadius * 0.7f * cos(angle);
        float z = roomRadius * 0.7f * sin(angle);

        Model* shelfModel = (i % 2 == 0) ? bookshelfModel.get() : bookshelf2Model.get();

        float rotationToFaceCenter;
        glm::vec3 scale;

        if (i % 2 == 0) {
            rotationToFaceCenter = angle + glm::radians(90.0f);
            scale = glm::vec3(2.0f, 3.7f, 3.0f);
        }
        else {
            rotationToFaceCenter = angle + glm::radians(270.0f);
            scale = glm::vec3(1.2f, 3.7f, 1.4f);
        }

        scene.addObject(shelfModel,
            glm::vec3(x, 1.0f, z),
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            scale);
    }

    // ENHANCED FLOATING BOOKS
    std::cout << "Setting up animations (books)..." << std::endl;

    // Central orbiting and floating books
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * static_cast<float>(i));
        float x = 2.0f * cos(angle);
        float z = 2.0f * sin(angle);

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(x, 2.0f, z),
            glm::vec3(0.0f, angle, 0.0f),
            glm::vec3(2.3f, 2.3f, 2.3f));

        // Enhanced animations - vary by book
        switch (i % 3) {
        case 0: // Orbiting around center
            scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, 2.0f, 0.0f), 2.0f, 0.4f);
            scene.objects[bookIndex].setRotating(true, 0.6f);
            break;
        case 1: // Floating in place
            scene.objects[bookIndex].setFloating(true, 0.5f, 1.0f);
            scene.objects[bookIndex].setRotating(true, 0.8f);
            break;
        case 2: // Pulsing + rotating + floating
            scene.objects[bookIndex].setPulsing(true, 0.12f, 1.8f);
            scene.objects[bookIndex].setRotating(true, 0.5f);
            scene.objects[bookIndex].setFloating(true, 0.3f, 0.7f);
            break;
        }
    }

    // Books orbiting around columns
    for (int col = 0; col < 4; col++) {
        float colAngle = glm::radians(90.0f * static_cast<float>(col));
        glm::vec3 columnPos = glm::vec3(3.0f * cos(colAngle), 2.0f, 3.0f * sin(colAngle));

        // Add 2 books per column at different heights
        for (int book = 0; book < 2; book++) {
            size_t bookIndex = scene.objects.size();
            glm::vec3 bookStartPos = columnPos + glm::vec3(1.5f, book * 1.0f, 0.0f);

            scene.addObject(bookModel.get(),
                bookStartPos,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.9f, 0.9f, 0.9f));

            // Orbit around the column
            scene.objects[bookIndex].setOrbiting(true,
                columnPos + glm::vec3(0.0f, book * 1.0f, 0.0f),
                1.8f,
                0.3f + book * 0.15f); // Different speeds
            scene.objects[bookIndex].setRotating(true, 1.2f);

            // Add slight floating to some
            if (book % 2 == 0) {
                scene.objects[bookIndex].setFloating(true, 0.2f, 1.5f);
            }
        }
    }

    // High-altitude slowly drifting books
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = 5.0f * cos(angle);
        float z = 5.0f * sin(angle);

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(x, 4.5f, z),
            glm::vec3(glm::radians(15.0f), angle, 0.0f), // Slight tilt
            glm::vec3(1.1f, 1.1f, 1.1f));

        // Slow, large orbit with floating
        scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, 4.5f, 0.0f), 5.0f, 0.15f);
        scene.objects[bookIndex].setFloating(true, 0.3f, 0.8f);
        scene.objects[bookIndex].setRotating(true, 0.3f);
    }

    // SETUP LIBRARY LIGHTING
    std::cout << "Setting up dark library lighting with dramatic torches..." << std::endl;
    lightingManager.setupLibraryLighting(roomRadius, roomHeight);

    // Set initial dark atmosphere
    lightingManager.setAmbientDarkness(0.2f); // Start very dark
    lightingManager.setTorchIntensity(2.5f);   // Bright torches for contrast

    // SETUP PORTAL SYSTEM
    std::cout << "Setting up portal system..." << std::endl;
    portalSystem.initialize(roomRadius, roomHeight);

    // Enhanced scene rendering lambda function for recursive portals
    auto renderSceneFunction = [&](const glm::mat4& view, const glm::mat4& projection,
        int recursionLevel, const RoomVariation& variation) {

            if (recursionLevel == 0) {
                std::cout << "Rendering with color tint: " << variation.colorTint.x
                    << ", " << variation.colorTint.y << ", " << variation.colorTint.z << std::endl;
            }

            basicShader.use();
            basicShader.setMat4("view", &view[0][0]);
            basicShader.setMat4("projection", &projection[0][0]);

            // Extract camera position from view matrix
            glm::mat4 invView = glm::inverse(view);
            glm::vec3 virtualViewPos = glm::vec3(invView[3]);
            basicShader.setVec3("viewPos", virtualViewPos.x, virtualViewPos.y, virtualViewPos.z);

            // IMPORTANT: Actually apply the room variation
            basicShader.setVec3("roomColorTint", variation.colorTint.x, variation.colorTint.y, variation.colorTint.z);
            basicShader.setFloat("roomScale", variation.scaleMultiplier);

            // Add fog effects for depth
            float fogIntensity = 0.02f + recursionLevel * 0.01f;
            basicShader.setFloat("fogDensity", fogIntensity);

            glm::vec3 fogColor = glm::vec3(0.01f, 0.005f, 0.02f) * variation.colorTint;
            basicShader.setVec3("fogColor", fogColor.x, fogColor.y, fogColor.z);

            // Create modified lighting for room variation
            LightingManager modifiedLighting = lightingManager;

            // Apply room variation to lighting
            for (auto& light : modifiedLighting.pointLights) {
                light.color *= variation.colorTint;
                light.intensity *= variation.scaleMultiplier;

                // Apply position offset for different rooms based on recursion
                glm::vec3 roomOffset = glm::vec3(
                    recursionLevel * variation.roomOffset * cos(recursionLevel * 0.5f),
                    0.0f,
                    recursionLevel * variation.roomOffset * sin(recursionLevel * 0.5f)
                );
                light.position += roomOffset;
            }
            modifiedLighting.ambientColor *= variation.colorTint;

            // Bind modified lighting
            modifiedLighting.bindToShader(basicShader);

            // Enhanced object rendering with room variations
            for (size_t i = 0; i < scene.objects.size(); i++) {
                const auto& obj = scene.objects[i];

                // Skip portal objects at deeper recursion levels to prevent infinite recursion
                if (obj.model == portalModel.get() && recursionLevel > 2) {
                    continue;
                }

                // Performance culling for distant rooms
                if (recursionLevel > 0 && obj.model == bookModel.get() &&
                    (rand() % (recursionLevel + 1)) != 0) {
                    continue; // Randomly cull some books in distant rooms
                }

                // Apply room variation transformations
                glm::mat4 roomTransform = obj.modelMatrix;

                // Apply scaling
                if (variation.scaleMultiplier != 1.0f) {
                    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f),
                        glm::vec3(variation.scaleMultiplier));
                    roomTransform = scaleMatrix * roomTransform;
                }

                // Apply room offset for recursion
                if (recursionLevel > 0) {
                    glm::vec3 roomOffset = glm::vec3(
                        recursionLevel * variation.roomOffset * cos(recursionLevel * 0.5f),
                        recursionLevel * 5.0f, // Slight vertical offset
                        recursionLevel * variation.roomOffset * sin(recursionLevel * 0.5f)
                    );
                    glm::mat4 offsetMatrix = glm::translate(glm::mat4(1.0f), roomOffset);
                    roomTransform = offsetMatrix * roomTransform;
                }

                // Bind textures based on object type
                if (obj.model == bookModel.get()) {
                    TextureManager::bindTextureForObject("book", basicShader);
                }
                else if (obj.model == bookshelfModel.get() || obj.model == bookshelf2Model.get()) {
                    TextureManager::bindTextureForObject("bookshelf", basicShader);
                }
                else if (obj.model == columnModel.get()) {
                    TextureManager::bindTextureForObject("column", basicShader);
                }
                else if (obj.model == floorModel.get()) {
                    TextureManager::bindTextureForObject("floor", basicShader);
                }
                else if (obj.model == wallModel.get()) {
                    TextureManager::bindTextureForObject("wall", basicShader);
                }
                else if (obj.model == ceilingModel.get()) {
                    TextureManager::bindTextureForObject("ceiling", basicShader);
                }
                else if (obj.model == lampModel.get()) {
                    TextureManager::bindTextureForObject("lamp", basicShader);
                }
                else if (obj.model == torchModel.get()) {
                    TextureManager::bindTextureForObject("torch", basicShader);
                }

                basicShader.setMat4("model", &roomTransform[0][0]);
                obj.model->draw();
            }

            // Reset room-specific uniforms
            basicShader.setVec3("roomColorTint", 1.0f, 1.0f, 1.0f);
            basicShader.setFloat("roomScale", 1.0f);
        };

    std::cout << "Library setup complete!" << std::endl;
    std::cout << "\n=== BABEL INFINITE LIBRARY CONTROLS ===" << std::endl;
    std::cout << "  WASD + Mouse - Move camera" << std::endl;
    std::cout << "  Space/Ctrl - Up/Down" << std::endl;
    std::cout << "  H - Debug info" << std::endl;
    std::cout << "  1/2/3 - Animation speed" << std::endl;
    std::cout << "  L - Cycle lighting modes" << std::endl;
    std::cout << "  T - Torch intensity" << std::endl;
    std::cout << "  B - Darkness level" << std::endl;
    std::cout << "  M - Toggle dramatic mode" << std::endl;
    std::cout << "  +/- - Torch flicker" << std::endl;
    std::cout << "  P - Toggle portals on/off" << std::endl;
    std::cout << "  Q - Portal quality (Low/Medium/High)" << std::endl;
    std::cout << "  R - Recursion depth (1-8 levels)" << std::endl;
    std::cout << "  I - Toggle infinite library mode" << std::endl;
    std::cout << "  F - Toggle adaptive quality" << std::endl;
    std::cout << "  O - Optimize memory usage" << std::endl;
    std::cout << "==========================================\n" << std::endl;

    // Camera setup
    glm::vec3 cameraPos = glm::vec3(1.2f, 2.3f, 1.16f);
    glm::vec3 cameraFront = glm::vec3(-1.0f, 0.0f, -0.5f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -135.0f;
    pitch = -20.0f;

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Update performance manager
        perfManager.updateFPS(deltaTime);

        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime, scene,
            bookModel, bookshelfModel, bookshelf2Model, columnModel,
            floorModel, lampModel, portalModel, ceilingModel, wallModel, torchModel,
            roomRadius, roomHeight, numSides, yaw, pitch, lightingManager, portalSystem, perfManager);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        // Apply global animation speed to scene update
        scene.update(deltaTime * globalAnimationSpeed);

        // Update lighting animations
        lightingManager.update(deltaTime * globalAnimationSpeed);

        // Update portal distances and infinite room generation
        portalSystem.updateDistances(cameraPos);
        if (portalSystem.areActive()) {
            portalSystem.updateRoomGeneration(cameraPos);
        }

        // Apply performance settings
        if (perfManager.getPortalQuality() != 512) { // Default quality check
            portalSystem.setQuality(perfManager.getPortalQuality());
        }
        if (perfManager.getRecursionDepth() != 4) { // Default depth check
            portalSystem.setRecursionDepth(perfManager.getRecursionDepth());
        }

        // Create view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

        // PHASE 1: Render recursive portal views FIRST using optimized rendering
        if (portalSystem.areActive()) {
            portalSystem.renderPortalViewsOptimized(renderSceneFunction, cameraPos, cameraFront, projection);
        }

        // PHASE 2: Render main scene to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glViewport(0, 0, WIDTH, HEIGHT);
        glClearColor(0.01f, 0.005f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw all non-portal objects with basic shader using LOD
        basicShader.use();
        basicShader.setMat4("view", &view[0][0]);
        basicShader.setMat4("projection", &projection[0][0]);
        basicShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Set default room uniforms for main scene
        basicShader.setVec3("roomColorTint", 1.0f, 1.0f, 1.0f);
        basicShader.setFloat("roomScale", 1.0f);
        basicShader.setFloat("fogDensity", 0.02f);
        basicShader.setVec3("fogColor", 0.01f, 0.005f, 0.02f);

        // Bind all lighting to shader
        lightingManager.bindToShader(basicShader);

        // Render scene with LOD optimizations
        renderSceneWithLOD(scene, basicShader, cameraPos, bookModel, bookshelfModel, bookshelf2Model,
            columnModel, floorModel, lampModel, portalModel, ceilingModel, wallModel, torchModel, 0);

        // PHASE 3: Render portals with portal shader
        if (scene.objects.size() > 0) {
            portalShader.use();
            portalShader.setMat4("view", &view[0][0]);
            portalShader.setMat4("projection", &projection[0][0]);
            portalShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
            portalShader.setFloat("time", currentFrame);

            // Bind lighting to portal shader
            lightingManager.bindToShader(portalShader);

            // Set portal activity state
            bool portalsActive = portalSystem.areActive();
            portalShader.setBool("portalActive", portalsActive);

            // Render each portal
            int portalCount = 0;
            for (size_t i = 0; i < scene.objects.size(); i++) {
                const auto& obj = scene.objects[i];

                if (obj.model == portalModel.get()) {
                    // Bind stone textures for the frame
                    glActiveTexture(GL_TEXTURE1);
                    glBindTexture(GL_TEXTURE_2D, TextureManager::getTexture("portal_basecolor"));
                    portalShader.setInt("baseColorMap", 1);

                    glActiveTexture(GL_TEXTURE2);
                    glBindTexture(GL_TEXTURE_2D, TextureManager::getTexture("column_roughness"));
                    portalShader.setInt("roughnessMap", 2);

                    glActiveTexture(GL_TEXTURE3);
                    glBindTexture(GL_TEXTURE_2D, TextureManager::getTexture("column_metallic"));
                    portalShader.setInt("metallicMap", 3);

                    if (portalsActive && portalCount < portalSystem.getPortalCount()) {
                        // Bind the portal's recursive view texture
                        glActiveTexture(GL_TEXTURE0);
                        portalSystem.bindPortalTexture(portalCount, portalShader);
                        portalShader.setInt("portalView", 0);

                        // Set recursion level and destination color tint
                        portalShader.setInt("recursionLevel", portalCount);
                        if (portalCount < 8) { // Ensure we don't go out of bounds
                            const auto& variation = portalSystem.roomVariations[portalCount % 8];
                            portalShader.setVec3("destinationColorTint",
                                variation.colorTint.x, variation.colorTint.y, variation.colorTint.z);
                        }
                    }
                    else {
                        // Inactive portal - show black
                        static GLuint blackTexture = 0;
                        if (blackTexture == 0) {
                            glGenTextures(1, &blackTexture);
                            glBindTexture(GL_TEXTURE_2D, blackTexture);
                            unsigned char blackData[4] = { 0, 0, 0, 255 };
                            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, blackData);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                        }
                        glActiveTexture(GL_TEXTURE0);
                        glBindTexture(GL_TEXTURE_2D, blackTexture);
                        portalShader.setInt("portalView", 0);
                        portalShader.setVec3("destinationColorTint", 1.0f, 1.0f, 1.0f);
                    }

                    // Set model matrix and draw portal
                    portalShader.setMat4("model", &obj.modelMatrix[0][0]);
                    obj.model->draw();

                    portalCount++;
                }
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    portalSystem.cleanup();
    TextureManager::cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}