// Force discrete GPU on laptops with hybrid graphics
extern "C" {
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

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
#include "debug.hpp"

// Application constants
namespace Config {
    const unsigned int WIDTH = 1280;
    const unsigned int HEIGHT = 720;
    const float ROOM_RADIUS = 8.0f;
    const float ROOM_HEIGHT = 6.0f;
    const int NUM_SIDES = 8;
    const float CAMERA_SPEED = 2.5f;
    const float MOUSE_SENSITIVITY = 0.2f;
}

// Camera controls
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = Config::WIDTH / 2.0f;
float lastY = Config::HEIGHT / 2.0f;
bool firstMouse = true;

// Input state tracking
static bool portalTogglePressed = false;
static bool dramaModePressed = false;
static bool helpPressed = false;
static bool debugTogglePressed = false;
static bool f1Pressed = false;
static bool f2Pressed = false;
static bool f3Pressed = false;
static bool f4Pressed = false;
static bool f5Pressed = false;
static bool recursivePortalsEnabled = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront,
    glm::vec3& cameraUp, float deltaTime, LightingManager& lightingManager,
    PortalSystem& portalSystem);
void setupScene(Scene& scene, const std::vector<std::unique_ptr<Model>>& models,
    std::vector<size_t>& torchIndices);

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

    xoffset *= Config::MOUSE_SENSITIVITY;
    yoffset *= Config::MOUSE_SENSITIVITY;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront,
    glm::vec3& cameraUp, float deltaTime, LightingManager& lightingManager,
    PortalSystem& portalSystem) {

    const float cameraSpeed = Config::CAMERA_SPEED * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Store old position for portal collision detection
    glm::vec3 oldCameraPos = cameraPos;

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

    // Check for portal teleportation
    glm::vec3 teleportPos;
    if (recursivePortalsEnabled && portalSystem.checkPortalCollision(oldCameraPos, cameraPos, teleportPos)) {
        cameraPos = teleportPos;
    }

    // Portal toggle
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !portalTogglePressed) {
        portalTogglePressed = true;
        recursivePortalsEnabled = !recursivePortalsEnabled;
        portalSystem.setEnabled(recursivePortalsEnabled);
        std::cout << "Portals " << (recursivePortalsEnabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        portalTogglePressed = false;
    }

    // Drama mode toggle
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !dramaModePressed) {
        dramaModePressed = true;
        static bool dramaticMode = false;
        dramaticMode = !dramaticMode;
        lightingManager.setDramaticMode(dramaticMode);
        std::cout << "Drama Mode: " << (dramaticMode ? "WARM & BRIGHT" : "NORMAL") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        dramaModePressed = false;
    }

    // Torch intensity control
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            lightingManager.setTorchIntensity(3.5f);
            std::cout << "Torches: BRIGHT & WARM" << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            lightingManager.setTorchIntensity(1.5f);
            std::cout << "Torches: DIM & COZY" << std::endl;
        }
    }

    // Help display (H key - as originally intended)
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !helpPressed) {
        helpPressed = true;
        std::cout << "\n===== BABEL CONTROLS =====" << std::endl;
        std::cout << "MOVEMENT:" << std::endl;
        std::cout << "  WASD + Mouse - Move camera" << std::endl;
        std::cout << "  Space/Ctrl - Up/Down" << std::endl;
        std::cout << "  P - Toggle portals" << std::endl;
        std::cout << "\nLIGHTING:" << std::endl;
        std::cout << "  M - Drama Mode (warmer & brighter)" << std::endl;
        std::cout << "  L + up key - Bright warm torches" << std::endl;
        std::cout << "  L + down key - Dim cozy torches" << std::endl;
        std::cout << "\nDEBUG:" << std::endl;
        std::cout << "  F10 - Toggle debug mode" << std::endl;
        std::cout << "  F1 - Performance stats" << std::endl;
        std::cout << "  F2 - Portal information" << std::endl;
        std::cout << "  F3 - Lighting information" << std::endl;
        std::cout << "  F4 - Scene information" << std::endl;
        std::cout << "  F5 - Camera information" << std::endl;
        std::cout << "  H - Show this help" << std::endl;
        std::cout << "==============================\n" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        helpPressed = false;
    }

    // DEBUG SYSTEM CONTROLS

    // Debug mode toggle (F10)
    if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_PRESS && !debugTogglePressed) {
        debugTogglePressed = true;
        DebugSystem::toggleDebugMode();
    }
    if (glfwGetKey(window, GLFW_KEY_F10) == GLFW_RELEASE) {
        debugTogglePressed = false;
    }

    // Performance stats toggle (F1)
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS && !f1Pressed) {
        f1Pressed = true;
        DebugSystem::togglePerformanceStats();
    }
    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_RELEASE) {
        f1Pressed = false;
    }

    // Portal info toggle (F2)
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_PRESS && !f2Pressed) {
        f2Pressed = true;
        DebugSystem::togglePortalInfo();
    }
    if (glfwGetKey(window, GLFW_KEY_F2) == GLFW_RELEASE) {
        f2Pressed = false;
    }

    // Lighting info toggle (F3)
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !f3Pressed) {
        f3Pressed = true;
        DebugSystem::toggleLightingInfo();
    }
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE) {
        f3Pressed = false;
    }

    // Scene info toggle (F4)
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS && !f4Pressed) {
        f4Pressed = true;
        DebugSystem::toggleSceneInfo();
    }
    if (glfwGetKey(window, GLFW_KEY_F4) == GLFW_RELEASE) {
        f4Pressed = false;
    }

    // Camera info toggle (F5)
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !f5Pressed) {
        f5Pressed = true;
        DebugSystem::toggleCameraInfo();
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE) {
        f5Pressed = false;
    }
}

void setupScene(Scene& scene, const std::vector<std::unique_ptr<Model>>& models,
    std::vector<size_t>& torchIndices) {

    // Extract models from vector for easier access
    const auto& bookModel = models[0];
    const auto& bookshelfModel = models[1];
    const auto& bookshelf2Model = models[2];
    const auto& columnModel = models[3];
    const auto& floorModel = models[4];
    const auto& ceilingModel = models[5];
    const auto& wallModel = models[6];
    const auto& torchModel = models[7];
    const auto& lampModel = models[8];
    const auto& doorFrameModel = models[9];

    std::cout << "Building the library..." << std::endl;

    // Floor
    scene.addObject(floorModel.get(),
        glm::vec3(0.0f, 0.0f, 0.0f), // p
        glm::vec3(0.0f, glm::radians(90.0f), 0.0f), // r
        glm::vec3(3.4f, 1.0f, 3.4f)); // s

    // Ceiling
    scene.addObject(ceilingModel.get(),
        glm::vec3(0.0f, Config::ROOM_HEIGHT + 1.2f, 0.0f),
        glm::vec3(0.0f, glm::radians(105.0f), 0.0f),
        glm::vec3(3.5f, 2.0f, 3.5f));

    // Walls
    for (int i = 0; i < Config::NUM_SIDES; i++) {
        float angle = glm::radians(360.0f * static_cast<float>(i) / static_cast<float>(Config::NUM_SIDES));
        float x = Config::ROOM_RADIUS * cos(angle);
        float z = Config::ROOM_RADIUS * sin(angle);
        float wallRotation = angle + (i % 2 == 0 ? glm::radians(90.0f) : 0.0f);

        if (i != 2 && i != 3 && i != 6 && i != 7) {
            wallRotation += glm::radians(180.0f);
        }

        scene.addObject(wallModel.get(),
            glm::vec3(x, 0.1f, z),
            glm::vec3(0.0f, wallRotation, 0.0f),
            glm::vec3(0.015f, 0.05f, 0.015f));
    }

    // Columns
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        float x = 3.2f * cos(angle);
        float z = 3.2f * sin(angle);

        scene.addObject(columnModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.8f, 3.5f, 1.8f));
    }

    // Door frames
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = Config::ROOM_RADIUS * 0.85f * cos(angle);
        float z = Config::ROOM_RADIUS * 0.85f * sin(angle);
        float rotationToCenter = angle + glm::radians(90.0f);

        scene.addObject(doorFrameModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, rotationToCenter, 0.0f),
            glm::vec3(1.5f, 1.5f, 1.5f));
    }

    // Bookshelves
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        float x = Config::ROOM_RADIUS * 0.90f * cos(angle);
        float z = Config::ROOM_RADIUS * 0.90f * sin(angle);

        Model* shelfModel = (i % 2 == 0) ? bookshelfModel.get() : bookshelf2Model.get();
        float rotationToCenter = angle + glm::radians(90.0f) + (i % 2 == 0 ? glm::radians(360.0f) : 135.0f);
        glm::vec3 scale = (i % 2 == 0) ? glm::vec3(2.0f, 4.3f, 3.0f) : glm::vec3(1.4f, 4.0f, 1.6f);

        scene.addObject(shelfModel,
            glm::vec3(x, 1.2f, z),
            glm::vec3(0.0f, rotationToCenter, 0.0f),
            scale);
    }

    // Central lamp with rotation
    size_t lampIndex = scene.objects.size();
    scene.addObject(lampModel.get(),
        glm::vec3(0.0f, 8.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 2.0f, 2.0f));
    scene.objects[lampIndex].setRotating(true, 0.5f);

    // Orbiting torches
    for (int i = 0; i < 4; i++) {
        float columnAngle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        glm::vec3 columnCenter = glm::vec3(3.2f * cos(columnAngle), 0.0f, 3.2f * sin(columnAngle));

        float torchDistance = 1.2f;
        glm::vec3 torchPos = columnCenter + glm::vec3(
            torchDistance * cos(columnAngle + glm::radians(90.0f)),
            3.0f,
            torchDistance * sin(columnAngle + glm::radians(90.0f))
        );

        size_t torchIndex = scene.objects.size();
        torchIndices.push_back(torchIndex);

        scene.addObject(torchModel.get(),
            torchPos,
            glm::vec3(0.0f, columnAngle + glm::radians(90.0f), 0.0f),
            glm::vec3(0.8f, 0.8f, 0.8f));

        scene.objects[torchIndex].setOrbiting(true, columnCenter + glm::vec3(0.0f, 3.0f, 0.0f), 1.2f, 0.5f + (i * 0.2f));
        scene.objects[torchIndex].setRotating(true, 1.0f);
    }

    // Floating books with varied animations
    for (int i = 0; i < 20; i++) {
        float angle = glm::radians(18.0f * static_cast<float>(i));
        float radius = 1.5f + (i % 4) * 0.7f;
        float height = 2.0f + sin(angle * 3.0f) * 1.0f;

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(radius * cos(angle), height, radius * sin(angle)),
            glm::vec3(glm::radians(15.0f), angle, glm::radians(10.0f)),
            glm::vec3(1.2f, 1.2f, 1.2f));

        // Different animation patterns for variety
        switch (i % 4) {
        case 0: // Orbital + rotation
            scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, height, 0.0f), radius, 0.4f);
            scene.objects[bookIndex].setRotating(true, 0.8f);
            break;
        case 1: // Floating + rotation
            scene.objects[bookIndex].setFloating(true, 0.5f, 1.0f);
            scene.objects[bookIndex].setRotating(true, 0.6f);
            break;
        case 2: // Rotation
            scene.objects[bookIndex].setRotating(true, 0.4f);
            break;
        case 3: // Combined: orbital + floating + rotation
            scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, height, 0.0f), radius, 0.3f);
            scene.objects[bookIndex].setFloating(true, 0.3f, 1.5f);
            scene.objects[bookIndex].setRotating(true, 0.7f);
            break;
        }
    }
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(Config::WIDTH, Config::HEIGHT,
        "BABEL - Infinite Library", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    srand(static_cast<unsigned int>(time(nullptr)));

    // INITIALIZE DEBUG SYSTEM
    DebugSystem::initialize();

    std::cout << "\n===== BABEL LIBRARY =====" << std::endl;
    std::cout << "Loading atmospheric lighting..." << std::endl;

    Shader standardShader("shaders/standard.vert", "shaders/standard.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");
    Shader portalShader("shaders/portal.vert", "shaders/portal.frag");

    TextureManager::loadAllTextures();

    std::vector<std::unique_ptr<Model>> models;
    models.push_back(std::make_unique<Model>("assets/models/book.obj"));
    models.push_back(std::make_unique<Model>("assets/models/bookshelf.obj"));
    models.push_back(std::make_unique<Model>("assets/models/Bookshelf2.obj"));
    models.push_back(std::make_unique<Model>("assets/models/column.obj"));
    models.push_back(std::make_unique<Model>("assets/models/floor.obj"));
    models.push_back(std::make_unique<Model>("assets/models/ceiling.obj"));
    models.push_back(std::make_unique<Model>("assets/models/wall.obj"));
    models.push_back(std::make_unique<Model>("assets/models/torch.obj"));
    models.push_back(std::make_unique<Model>("assets/models/lamb.obj"));
    models.push_back(std::make_unique<Model>("assets/models/door.obj"));

    Scene scene;
    std::vector<size_t> torchIndices;
    setupScene(scene, models, torchIndices);

    LightingManager lightingManager;
    lightingManager.setupLibraryLighting(Config::ROOM_RADIUS, Config::ROOM_HEIGHT);

    PortalSystem portalSystem;
    portalSystem.initialize();

    // Add portals at door positions
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 position = glm::vec3(
            Config::ROOM_RADIUS * 0.85f * cos(angle),
            2.8f,
            Config::ROOM_RADIUS * 0.85f * sin(angle)
        );
        glm::vec3 normal = glm::vec3(-cos(angle), 0.0f, -sin(angle));
        portalSystem.addPortal(position, normal);
    }

    // Connect portals for infinite effect
    portalSystem.connectPortals(0, 2); // North <-> South
    portalSystem.connectPortals(1, 3); // East <-> West

    glm::vec3 cameraPos = glm::vec3(0.0f, 2.5f, 4.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Lambda function for rendering
    auto renderSceneFunc = [&](const glm::mat4& view, const glm::mat4& projection) {
        glm::mat4 invView = glm::inverse(view);
        glm::vec3 currentCameraPos = glm::vec3(invView[3]);
        float currentFrame = static_cast<float>(glfwGetTime());

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        // Render standard objects with lighting
        standardShader.use();
        standardShader.setMat4("view", &view[0][0]);
        standardShader.setMat4("projection", &projection[0][0]);
        standardShader.setVec3("viewPos", currentCameraPos.x, currentCameraPos.y, currentCameraPos.z);
        standardShader.setFloat("time", currentFrame);
        lightingManager.bindToShader(standardShader);

        for (const auto& obj : scene.objects) {
            if (obj.model == models[7].get() || obj.model == models[8].get()) continue;

            if (obj.model == models[0].get()) {
                TextureManager::bindTextureForObject("book", standardShader);
            }
            else if (obj.model == models[1].get() || obj.model == models[2].get()) {
                TextureManager::bindTextureForObject("bookshelf", standardShader);
            }
            else if (obj.model == models[3].get()) {
                TextureManager::bindTextureForObject("column", standardShader);
            }
            else if (obj.model == models[4].get()) {
                TextureManager::bindTextureForObject("floor", standardShader);
            }
            else if (obj.model == models[6].get()) {
                TextureManager::bindTextureForObject("wall", standardShader);
            }
            else if (obj.model == models[5].get()) {
                TextureManager::bindTextureForObject("ceiling", standardShader);
            }
            else if (obj.model == models[9].get()) {
                TextureManager::bindTextureForObject("doorframe", standardShader);
            }

            standardShader.setMat4("model", &obj.modelMatrix[0][0]);
            obj.model->draw();
        }

        // Render light sources
        lightShader.use();
        lightShader.setMat4("view", &view[0][0]);
        lightShader.setMat4("projection", &projection[0][0]);
        lightShader.setVec3("viewPos", currentCameraPos.x, currentCameraPos.y, currentCameraPos.z);
        lightShader.setFloat("time", currentFrame);
        lightingManager.bindToShader(lightShader);

        for (const auto& obj : scene.objects) {
            if (obj.model == models[7].get()) { // Torch
                TextureManager::bindTextureForObject("torch", lightShader);
                lightShader.setMat4("model", &obj.modelMatrix[0][0]);
                obj.model->draw();
            }
            else if (obj.model == models[8].get()) { // Lamp
                TextureManager::bindTextureForObject("lamp", lightShader);
                lightShader.setMat4("model", &obj.modelMatrix[0][0]);
                obj.model->draw();
            }
        }

        if (recursivePortalsEnabled) {
            portalSystem.renderPortalSurfaces(portalShader, view, projection, currentCameraPos, currentFrame);
        }
        };

    // Debug info counter
    static int debugFrameCounter = 0;

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // UPDATE DEBUG SYSTEM PERFORMANCE STATS
        DebugSystem::updatePerformanceStats(deltaTime);

        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime, lightingManager, portalSystem);

        // Update camera direction
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        scene.update(deltaTime);

        // Update light positions based on torch objects
        std::vector<glm::vec3> currentTorchPositions;
        for (const auto& obj : scene.objects) {
            if (obj.model == models[7].get()) { // Torch model
                currentTorchPositions.push_back(glm::vec3(obj.modelMatrix[3]));
            }
        }

        if (!currentTorchPositions.empty()) {
            lightingManager.updateTorchPositions(currentTorchPositions);
        }

        portalSystem.updateDistances(cameraPos);

		// It's for debug info printing every 2 seconds
        debugFrameCounter++;
        if (debugFrameCounter % 60 == 0) { // Every 2 seconds at 60fps
            DebugSystem::printCameraInfo(cameraPos, cameraFront, yaw, pitch);
            DebugSystem::printLightingInfo(lightingManager);
            DebugSystem::printSceneInfo(scene, models[0], models[1], models[2],
                models[3], models[4], models[8], nullptr,
                models[5], models[6], models[7]);
        }

        // Setup matrices
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(60.0f),
            static_cast<float>(Config::WIDTH) / static_cast<float>(Config::HEIGHT), 0.1f, 100.0f);

        // Clear screen with dark atmosphere
        glClearColor(0.01f, 0.008f, 0.005f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render portal views first for infinite effect
        if (recursivePortalsEnabled) {
            portalSystem.renderPortalViews(renderSceneFunc, cameraPos, cameraFront, cameraUp, projection);
        }

        // Render main scene
        renderSceneFunc(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    portalSystem.cleanup();
    TextureManager::cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}