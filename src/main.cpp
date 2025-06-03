// Complete main.cpp for BABEL - Warm Atmospheric Lighting with Moving Torch Sync
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

// Constants for window size
const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

// Camera controls
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// Portal controls
static bool portalTogglePressed = false;
static bool recursivePortalsEnabled = true;

// Warm lighting controls
static bool dramaModePressed = false;
static bool warmModePressed = false;
static bool helpPressed = false;

// Global variables for recursive rendering
float currentFrame = 0.0f;
glm::vec3 globalCameraPos;
glm::vec3 globalCameraFront;

// Function for handling window resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// Mouse callback for camera rotation
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

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
}

// Enhanced input processing with warm lighting controls
void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp,
    float deltaTime, LightingManager& lightingManager, PortalSystem& portalSystem) {

    const float cameraSpeed = 3.0f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement with teleportation check
    glm::vec3 oldCameraPos = cameraPos;

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
        std::cout << "🌀 Portals " << (recursivePortalsEnabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        portalTogglePressed = false;
    }

    // WARM LIGHTING CONTROLS

    // DRAMA MODE - M key (warmer, brighter lighting)
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !dramaModePressed) {
        dramaModePressed = true;
        static bool dramaticMode = false;
        dramaticMode = !dramaticMode;
        lightingManager.setDramaticMode(dramaticMode);
        std::cout << "🔥 Drama Mode: " << (dramaticMode ? "WARM & BRIGHT" : "NORMAL") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE) {
        dramaModePressed = false;
    }

    // TORCH INTENSITY - L key (make torches warmer/cooler)
    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            lightingManager.setTorchIntensity(3.5f);
            std::cout << "🔥 Torches: BRIGHT & WARM" << std::endl;
        }
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            lightingManager.setTorchIntensity(1.5f);
            std::cout << "🔥 Torches: DIM & COZY" << std::endl;
        }
    }

    // HELP - H key
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !helpPressed) {
        helpPressed = true;
        std::cout << "\n🔥 ===== BABEL WARM LIBRARY CONTROLS ===== 🔥" << std::endl;
        std::cout << "MOVEMENT:" << std::endl;
        std::cout << "  WASD + Mouse - Move camera" << std::endl;
        std::cout << "  Space/Ctrl - Up/Down" << std::endl;
        std::cout << "  P - Toggle portals" << std::endl;
        std::cout << "\nWARM LIGHTING:" << std::endl;
        std::cout << "  M - Drama Mode (warmer & brighter)" << std::endl;
        std::cout << "  L + ↑ - Bright warm torches" << std::endl;
        std::cout << "  L + ↓ - Dim cozy torches" << std::endl;
        std::cout << "  H - Show this help" << std::endl;
        std::cout << "\nYou should see:" << std::endl;
        std::cout << "  🔥 WARM TORCH FLAMES (amber, moving with torches)" << std::endl;
        std::cout << "  💡 GOLDEN LAMP GLOW (warm white, steady)" << std::endl;
        std::cout << "  🏛️ WARM STONE ATMOSPHERE (golden ambient)" << std::endl;
        std::cout << "==========================================\n" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_RELEASE) {
        helpPressed = false;
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set GLFW options
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "BABEL - Warm Atmospheric Library", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize GLEW
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Seed random number generator
    srand(static_cast<unsigned int>(time(nullptr)));

    // Welcome message
    std::cout << "\n🔥 ===== BABEL WARM LIBRARY ===== 🔥" << std::endl;
    std::cout << "Loading warm atmospheric lighting..." << std::endl;

    // Load shaders
    Shader standardShader("shaders/standard.vert", "shaders/standard.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");
    Shader portalShader("shaders/portal.vert", "shaders/portal.frag");

    // Load textures and models
    TextureManager::loadAllTextures();
    auto bookModel = std::make_unique<Model>("assets/models/book.obj");
    auto bookshelfModel = std::make_unique<Model>("assets/models/bookshelf.obj");
    auto bookshelf2Model = std::make_unique<Model>("assets/models/Bookshelf2.obj");
    auto columnModel = std::make_unique<Model>("assets/models/column.obj");
    auto floorModel = std::make_unique<Model>("assets/models/floor.obj");
    auto ceilingModel = std::make_unique<Model>("assets/models/ceiling.obj");
    auto wallModel = std::make_unique<Model>("assets/models/wall.obj");
    auto torchModel = std::make_unique<Model>("assets/models/torch.obj");
    auto lampModel = std::make_unique<Model>("assets/models/lamb.obj");
    auto doorFrameModel = std::make_unique<Model>("assets/models/door.obj");

    // Create scene
    Scene scene;
    const float roomRadius = 8.0f;
    const float roomHeight = 6.0f;
    const int numSides = 8;

    std::cout << "🏗️ Building warm atmospheric library..." << std::endl;

    // Floor
    scene.addObject(floorModel.get(),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
        glm::vec3(2.8f, 1.0f, 2.8f));

    // Ceiling
    scene.addObject(ceilingModel.get(),
        glm::vec3(0.0f, roomHeight + 1.2f, 0.0f),
        glm::vec3(0.0f, glm::radians(105.0f), 0.0f),
        glm::vec3(3.5f, 2.0f, 3.5f));

    // Walls
    for (int i = 0; i < numSides; i++) {
        float angle = glm::radians(360.0f * static_cast<float>(i) / static_cast<float>(numSides));
        float x = roomRadius * cos(angle);
        float z = roomRadius * sin(angle);
        float wallRotation = angle + (i % 2 == 0 ? glm::radians(90.0f) : 0.0f);

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
        float x = roomRadius * 0.85f * cos(angle);
        float z = roomRadius * 0.85f * sin(angle);
        float rotationToCenter = angle + glm::radians(90.0f);

        scene.addObject(doorFrameModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, rotationToCenter, 0.0f),
            glm::vec3(1.5f, 1.5f, 1.5f));
    }

    // Bookshelves
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * static_cast<float>(i));
        float x = roomRadius * 0.90f * cos(angle);
        float z = roomRadius * 0.90f * sin(angle);

        Model* shelfModel = (i % 2 == 0) ? bookshelfModel.get() : bookshelf2Model.get();
        float rotationToCenter = angle + glm::radians(90.0f) + (i % 2 == 0 ? glm::radians(360.0f) : 135.0f);
        glm::vec3 scale = (i % 2 == 0) ? glm::vec3(2.0f, 4.3f, 3.0f) : glm::vec3(1.4f, 4.0f, 1.6f);

        scene.addObject(shelfModel,
            glm::vec3(x, 1.2f, z),
            glm::vec3(0.0f, rotationToCenter, 0.0f),
            scale);
    }

    // THE CENTRAL ROTATING LAMP - Golden warm glow
    size_t lampIndex = scene.objects.size();
    scene.addObject(lampModel.get(),
        glm::vec3(0.0f, 7.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(2.0f, 2.0f, 2.0f));

    // Make the lamp rotate slowly
    scene.objects[lampIndex].setRotating(true, 0.5f);

    // Track torch indices for light position updates
    std::vector<size_t> torchIndices;

    // ORBITING TORCHES around columns - Warm amber flames
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

    // Floating books (animated with warm atmosphere)
    for (int i = 0; i < 20; i++) {
        float angle = glm::radians(18.0f * static_cast<float>(i));
        float radius = 1.5f + (i % 4) * 0.7f;
        float height = 2.0f + sin(angle * 3.0f) * 1.0f;

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(radius * cos(angle), height, radius * sin(angle)),
            glm::vec3(glm::radians(15.0f), angle, glm::radians(10.0f)),
            glm::vec3(1.2f, 1.2f, 1.2f));

        switch (i % 4) {
        case 0:
            scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, height, 0.0f), radius, 0.4f);
            scene.objects[bookIndex].setRotating(true, 0.8f);
            break;
        case 1:
            scene.objects[bookIndex].setFloating(true, 0.5f, 1.0f);
            scene.objects[bookIndex].setRotating(true, 0.6f);
            break;
        case 2:
            scene.objects[bookIndex].setPulsing(true, 0.1f, 2.0f);
            scene.objects[bookIndex].setRotating(true, 0.4f);
            break;
        case 3:
            scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, height, 0.0f), radius, 0.3f);
            scene.objects[bookIndex].setFloating(true, 0.3f, 1.5f);
            scene.objects[bookIndex].setRotating(true, 0.7f);
            break;
        }
    }

    // Setup WARM lighting system
    LightingManager lightingManager;
    lightingManager.setupLibraryLighting(roomRadius, roomHeight);

    // Setup portals
    PortalSystem portalSystem;
    portalSystem.initialize();

    // Add portals at door positions 
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 position = glm::vec3(
            roomRadius * 0.85f * cos(angle),
            2.8f,
            roomRadius * 0.85f * sin(angle)
        );
        glm::vec3 normal = glm::vec3(-cos(angle), 0.0f, -sin(angle));
        portalSystem.addPortal(position, normal);
    }

    // Connect portals
    portalSystem.connectPortals(0, 2); // North <-> South
    portalSystem.connectPortals(1, 3); // East <-> West

    portalSystem.setQuality(512);
    portalSystem.setRecursionDepth(2);

    std::cout << "WARM SETUP COMPLETE:" << std::endl;
    std::cout << "- 1 GOLDEN LAMP light at (0, 7, 0) - rotating" << std::endl;
    std::cout << "- 4 AMBER TORCH lights - moving with orbiting torches" << std::endl;
    std::cout << "- WARM golden ambient atmosphere" << std::endl;
    std::cout << "- Light positions sync automatically with torch movement" << std::endl;
    std::cout << "=====================================\n" << std::endl;

    // Camera setup
    glm::vec3 cameraPos = glm::vec3(0.0f, 2.5f, 4.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Initialize time variables
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Render function
    auto renderSceneFunc = [&](const glm::mat4& view, const glm::mat4& projection) {
        glm::mat4 invView = glm::inverse(view);
        glm::vec3 currentCameraPos = glm::vec3(invView[3]);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDepthMask(GL_TRUE);

        // RENDER STANDARD OBJECTS WITH WARM LIGHTING
        standardShader.use();
        standardShader.setMat4("view", &view[0][0]);
        standardShader.setMat4("projection", &projection[0][0]);
        standardShader.setVec3("viewPos", currentCameraPos.x, currentCameraPos.y, currentCameraPos.z);
        standardShader.setFloat("time", currentFrame);

        lightingManager.bindToShader(standardShader);

        for (const auto& obj : scene.objects) {
            // Skip light sources - they use light shader
            if (obj.model == torchModel.get() || obj.model == lampModel.get()) continue;

            if (obj.model == bookModel.get()) {
                TextureManager::bindTextureForObject("book", standardShader);
            }
            else if (obj.model == bookshelfModel.get() || obj.model == bookshelf2Model.get()) {
                TextureManager::bindTextureForObject("bookshelf", standardShader);
            }
            else if (obj.model == columnModel.get()) {
                TextureManager::bindTextureForObject("column", standardShader);
            }
            else if (obj.model == floorModel.get()) {
                TextureManager::bindTextureForObject("floor", standardShader);
            }
            else if (obj.model == wallModel.get()) {
                TextureManager::bindTextureForObject("wall", standardShader);
            }
            else if (obj.model == ceilingModel.get()) {
                TextureManager::bindTextureForObject("ceiling", standardShader);
            }
            else if (obj.model == doorFrameModel.get()) {
                TextureManager::bindTextureForObject("doorframe", standardShader);
            }

            standardShader.setMat4("model", &obj.modelMatrix[0][0]);
            obj.model->draw();
        }

        // RENDER WARM LIGHT SOURCES (TORCHES AND LAMP) - Now affected by lighting
        lightShader.use();
        lightShader.setMat4("view", &view[0][0]);
        lightShader.setMat4("projection", &projection[0][0]);
        lightShader.setVec3("viewPos", currentCameraPos.x, currentCameraPos.y, currentCameraPos.z);
        lightShader.setFloat("time", currentFrame);

        // IMPORTANT: Bind lighting to light shader so they receive warm illumination
        lightingManager.bindToShader(lightShader);

        for (const auto& obj : scene.objects) {
            if (obj.model == torchModel.get()) {
                TextureManager::bindTextureForObject("torch", lightShader);
                lightShader.setMat4("model", &obj.modelMatrix[0][0]);
                obj.model->draw();
            }
            else if (obj.model == lampModel.get()) {
                TextureManager::bindTextureForObject("lamp", lightShader);
                lightShader.setMat4("model", &obj.modelMatrix[0][0]);
                obj.model->draw();
            }
        }

        // RENDER PORTALS
        if (recursivePortalsEnabled) {
            portalSystem.renderPortalSurfaces(portalShader, view, projection, currentCameraPos, currentFrame);
        }
        };

    // Main render loop
    while (!glfwWindowShouldClose(window)) {
        currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime, lightingManager, portalSystem);

        globalCameraPos = cameraPos;
        globalCameraFront = cameraFront;

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        scene.update(deltaTime);

        // *** CRITICAL: UPDATE TORCH LIGHT POSITIONS TO FOLLOW MOVING TORCHES ***
        std::vector<glm::vec3> currentTorchPositions;

        // Extract current torch positions from scene objects
        for (const auto& obj : scene.objects) {
            if (obj.model == torchModel.get()) {
                // Extract position from the model matrix
                glm::vec3 torchPos = glm::vec3(obj.modelMatrix[3]);
                currentTorchPositions.push_back(torchPos);
            }
        }

        // Update lighting manager with current torch positions
        if (!currentTorchPositions.empty()) {
            lightingManager.updateTorchPositions(currentTorchPositions);
        }

        portalSystem.updateDistances(cameraPos);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(60.0f),
            static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

        // Clear screen with warm dark atmosphere
        glClearColor(0.01f, 0.008f, 0.005f, 1.0f); // Very dark warm brown
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (recursivePortalsEnabled) {
            portalSystem.renderPortalViews(renderSceneFunc, cameraPos, cameraFront, cameraUp, projection);
        }

        renderSceneFunc(view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    std::cout << "\n🌟 Cleaning up warm library..." << std::endl;
    portalSystem.cleanup();
    TextureManager::cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();

    std::cout << "✨ BABEL warm library closed" << std::endl;
    return 0;
}