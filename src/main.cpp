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

        // Enhanced debug info for new animation types
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
    LightingManager& lightingManager) {

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

    // Debug toggle (H key)
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        showDebugInfo = !showDebugInfo;

        if (showDebugInfo) {
            std::cout << "\n*** DEBUG MODE ACTIVATED ***" << std::endl;
            printRoomDebugInfo(roomRadius, roomHeight, numSides);
            printCameraDebugInfo(cameraPos, cameraFront, yaw, pitch);
            printLightingDebugInfo(lightingManager);
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "BABEL - Dark Library of Infinite Knowledge", nullptr, nullptr);
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
    srand(static_cast<unsigned int>(time(nullptr)));

    std::cout << "GL ready. Vendor: " << glGetString(GL_VENDOR) << std::endl;

    // Load shader
    Shader basicShader("shaders/book.vert", "shaders/book.frag");

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

    std::cout << "Setting up dark library room..." << std::endl;

    // FLOOR
    scene.addObject(floorModel.get(),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
        glm::vec3(5.0f, 1.0f, 5.0f));

    // CEILING
    scene.addObject(ceilingModel.get(),
        glm::vec3(0.0f, roomHeight + 0.4f, 0.0f),
        glm::vec3(0.0f, glm::radians(90.0f), 0.0f),
        glm::vec3(3.0f, 2.0f, 3.0f));

    // LAMP - Enhanced with pulsing (dimmed for dark atmosphere)
    scene.addObject(lampModel.get(),
        glm::vec3(0.0f, roomHeight + 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 3.0f, 3.0f));
    scene.objects[2].setRotating(true, 0.2f);
    scene.objects[2].setPulsing(true, 0.05f, 3.0f);

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
            glm::vec3(x, roomHeight * 0.05f, z),
            glm::vec3(0.0f, wallRotation, 0.0f),
            glm::vec3(0.05f, 0.05f, 0.05f));
    }

    // COLUMNS
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = 3.0f * cos(angle);
        float z = 3.0f * sin(angle);

        scene.addObject(columnModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(1.8f, 3.0f, 1.8f));
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
            glm::vec3(1.3f, 1.4f, 1.3f));
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
            scale = glm::vec3(2.0f, 5.0f, 3.0f);
        }
        else {
            rotationToFaceCenter = angle + glm::radians(270.0f);
            scale = glm::vec3(1.2f, 5.0f, 1.4f);
        }

        scene.addObject(shelfModel,
            glm::vec3(x, 1.0f, z),
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            scale);
    }

    // TORCH MODELS - Place torches at light positions
    std::cout << "Placing torch models..." << std::endl;

    // Wall torches - match the lighting positions
    for (int i = 0; i < 8; i++) {
        float angle = glm::radians(45.0f * static_cast<float>(i));

        // Position torches closer to the wall surface
        glm::vec3 torchPos = glm::vec3(
            roomRadius * 0.92f * cos(angle), // Very close to wall
            2.2f, // Wall-mounted height
            roomRadius * 0.92f * sin(angle)
        );

        scene.addObject(torchModel.get(),
            torchPos,
            glm::vec3(0.0f, angle + glm::radians(180.0f), 0.0f), // Face inward toward room
            glm::vec3(0.8f, 0.8f, 0.8f)); // Smaller size for wall mounting
    }

    // Column torches
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));

        // Calculate column center position
        glm::vec3 columnCenter = glm::vec3(3.0f * cos(angle), 0.0f, 3.0f * sin(angle));

        // Position torch to hang from the column at shoulder height
        // Offset slightly outward from column surface
        float offsetDistance = 1.0f; // Distance from column center
        glm::vec3 torchPos = glm::vec3(
            columnCenter.x + offsetDistance * cos(angle), // Offset outward from column
            2.8f, // Hanging height on column
            columnCenter.z + offsetDistance * sin(angle)
        );

        scene.addObject(torchModel.get(),
            torchPos,
            glm::vec3(0.0f, angle + glm::radians(90.0f), 0.0f), // Perpendicular to column face
            glm::vec3(1.0f, 1.0f, 1.0f)); // Normal size for column torches

        // Add a second torch on the opposite side of each column for more light
        glm::vec3 torchPos2 = glm::vec3(
            columnCenter.x - offsetDistance * cos(angle), // Opposite side
            2.8f,
            columnCenter.z - offsetDistance * sin(angle)
        );

        scene.addObject(torchModel.get(),
            torchPos2,
            glm::vec3(0.0f, angle + glm::radians(270.0f), 0.0f), // Face opposite direction
            glm::vec3(1.0f, 1.0f, 1.0f));
    }

    // ENHANCED FLOATING BOOKS
    std::cout << "Setting up enhanced animated books..." << std::endl;

    // Central orbiting and floating books
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * static_cast<float>(i));
        float x = 2.0f * cos(angle);
        float z = 2.0f * sin(angle);

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(x, 2.0f, z),
            glm::vec3(0.0f, angle, 0.0f),
            glm::vec3(1.3f, 1.3f, 1.3f));

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

    // SETUP DARK LIBRARY LIGHTING
    std::cout << "Setting up dark library lighting with dramatic torches..." << std::endl;
    lightingManager.setupLibraryLighting(roomRadius, roomHeight);

    // Set initial dark atmosphere
    lightingManager.setAmbientDarkness(0.2f); // Start very dark
    lightingManager.setTorchIntensity(2.5f);   // Bright torches for contrast

    std::cout << "Dark library setup complete!" << std::endl;
    std::cout << "\n=== DARK BABEL CONTROLS ===" << std::endl;
    std::cout << "  WASD + Mouse - Move camera" << std::endl;
    std::cout << "  Space/Ctrl - Up/Down" << std::endl;
    std::cout << "  H - Debug info" << std::endl;
    std::cout << "  1/2/3 - Animation speed" << std::endl;
    std::cout << "  L - Cycle lighting modes" << std::endl;
    std::cout << "  T - Torch intensity (Dim/Medium/Bright/Very Bright)" << std::endl;
    std::cout << "  B - Darkness level (Pitch Black/Very Dark/Dark/Medium)" << std::endl;
    std::cout << "  M - Toggle dramatic mode" << std::endl;
    std::cout << "  +/- - Increase/Decrease torch flicker" << std::endl;
    std::cout << "============================\n" << std::endl;

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

        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime, scene,
            bookModel, bookshelfModel, bookshelf2Model, columnModel,
            floorModel, lampModel, portalModel, ceilingModel, wallModel, torchModel,
            roomRadius, roomHeight, numSides, yaw, pitch, lightingManager);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        // Apply global animation speed to scene update
        scene.update(deltaTime * globalAnimationSpeed);

        // Update lighting animations
        lightingManager.update(deltaTime * globalAnimationSpeed);

        glClearColor(0.01f, 0.005f, 0.02f, 1.0f); // Darker background for atmosphere
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

        basicShader.use();
        basicShader.setMat4("view", &view[0][0]);
        basicShader.setMat4("projection", &projection[0][0]);
        basicShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Bind all lighting to shader
        lightingManager.bindToShader(basicShader);

        // Draw objects with appropriate textures using TextureManager
        for (size_t i = 0; i < scene.objects.size(); i++) {
            const auto& obj = scene.objects[i];

            // Determine object type and bind appropriate texture
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
            else if (obj.model == portalModel.get()) {
                TextureManager::bindTextureForObject("portal", basicShader);
            }
            else if (obj.model == torchModel.get()) {
                TextureManager::bindTextureForObject("torch", basicShader);
            }

            // Set model matrix and draw
            basicShader.setMat4("model", &obj.modelMatrix[0][0]);
            obj.model->draw();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    TextureManager::cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}