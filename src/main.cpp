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

// Recursive portal variables
static int maxRecursionDepth = 3;
static bool recursivePortalsEnabled = true;

// Player teleportation variables
static glm::vec3 playerOffset = glm::vec3(0.0f);
static bool playerTeleported = false;
static float teleportCooldown = 0.0f;
static const float TELEPORT_COOLDOWN_TIME = 0.5f;

// Function declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

void renderScene(const Scene& scene, Shader& shader, LightingManager& lightingManager,
    const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& doorFrameModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    bool skipDoorFrames = false);

void renderRecursivePortals(const Scene& scene, Shader& basicShader, LightingManager& lightingManager,
    const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& doorFrameModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    int recursionLevel = 0);

glm::mat4 calculatePortalView(const glm::vec3& doorFramePos, const glm::vec3& doorFrameNormal,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    int recursionLevel);

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

void renderScene(const Scene& scene, Shader& shader, LightingManager& lightingManager,
    const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& doorFrameModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    bool skipDoorFrames) {

    shader.use();
    shader.setMat4("view", &view[0][0]);
    shader.setMat4("projection", &projection[0][0]);
    shader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);
    lightingManager.bindToShader(shader);

    for (size_t i = 0; i < scene.objects.size(); i++) {
        const auto& obj = scene.objects[i];

        // Skip door frames if requested
        if (skipDoorFrames && obj.model == doorFrameModel.get()) {
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
        else if (obj.model == doorFrameModel.get()) {
            TextureManager::bindTextureForObject("doorframe", shader);
        }

        shader.setMat4("model", &obj.modelMatrix[0][0]);
        obj.model->draw();
    }
}

glm::mat4 calculatePortalView(const glm::vec3& doorFramePos, const glm::vec3& doorFrameNormal,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    int recursionLevel) {

    // Calculate the virtual camera position through the portal
    glm::vec3 relativePos = cameraPos - doorFramePos;

    // Transform through the portal (simplified - move to next room)
    float roomOffset = 30.0f + (recursionLevel * 25.0f);
    glm::vec3 virtualCameraPos = doorFramePos + doorFrameNormal * roomOffset + relativePos;

    // Calculate virtual camera target
    glm::vec3 virtualTarget = virtualCameraPos + cameraFront;

    // Apply some variation based on recursion level for visual interest
    float variation = recursionLevel * 0.1f;
    virtualCameraPos += glm::vec3(sin(variation) * 2.0f, 0.0f, cos(variation) * 2.0f);

    return glm::lookAt(virtualCameraPos, virtualTarget, cameraUp);
}

void renderRecursivePortals(const Scene& scene, Shader& basicShader, LightingManager& lightingManager,
    const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& doorFrameModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    int recursionLevel) {

    // Base case: maximum recursion reached
    if (recursionLevel >= maxRecursionDepth) {
        // Render scene normally including door frames (but they won't show portals)
        renderScene(scene, basicShader, lightingManager, view, projection, cameraPos,
            bookModel, bookshelfModel, bookshelf2Model, columnModel,
            floorModel, lampModel, doorFrameModel, ceilingModel, wallModel, torchModel, false);
        return;
    }

    // Step 1: Render the entire scene normally FIRST (including door frames)
    renderScene(scene, basicShader, lightingManager, view, projection, cameraPos,
        bookModel, bookshelfModel, bookshelf2Model, columnModel,
        floorModel, lampModel, doorFrameModel, ceilingModel, wallModel, torchModel, false);

    // Step 2: Now create portal effects for each door frame opening
    std::vector<glm::vec3> doorFramePositions;
    std::vector<glm::vec3> doorFrameNormals;

    // Collect door frame positions and normals
    for (size_t i = 0; i < scene.objects.size(); i++) {
        const auto& obj = scene.objects[i];
        if (obj.model == doorFrameModel.get()) {
            doorFramePositions.push_back(obj.position);

            // Calculate door frame normal based on rotation
            glm::vec3 normal = glm::vec3(0, 0, 1); // Default forward
            glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), obj.rotation.y, glm::vec3(0, 1, 0));
            normal = glm::vec3(rotationMatrix * glm::vec4(normal, 0.0f));
            doorFrameNormals.push_back(normal);
        }
    }

    // Extract camera direction from view matrix
    glm::mat4 invView = glm::inverse(view);
    glm::vec3 cameraFront = -glm::vec3(invView[2]);
    glm::vec3 cameraUp = glm::vec3(invView[1]);

    // Step 3: Enable stencil testing for portal rendering
    glEnable(GL_STENCIL_TEST);

    // Step 4: For each door frame, create a portal effect in its opening
    for (size_t portalIndex = 0; portalIndex < doorFramePositions.size(); portalIndex++) {
        const glm::vec3& doorFramePos = doorFramePositions[portalIndex];
        const glm::vec3& doorFrameNormal = doorFrameNormals[portalIndex];

        // Check if portal is facing the camera (basic culling)
        glm::vec3 toDoorFrame = glm::normalize(doorFramePos - cameraPos);
        float facingDot = glm::dot(toDoorFrame, -doorFrameNormal);
        if (facingDot < 0.1f) continue; // Skip portals not facing camera

        // Step 4a: Create stencil mask for the door opening
        glStencilFunc(GL_ALWAYS, recursionLevel + 2, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);

        // Disable color and depth writing - only write to stencil
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        // Render a simple quad representing the door opening
        basicShader.use();
        basicShader.setMat4("view", &view[0][0]);
        basicShader.setMat4("projection", &projection[0][0]);

        // Create a simple quad for the door opening
        static GLuint doorOpeningVAO = 0;
        if (doorOpeningVAO == 0) {
            float doorOpeningVertices[] = {
                // Simple quad for door opening
                -0.8f, -1.5f, 0.0f,
                 0.8f, -1.5f, 0.0f,
                 0.8f,  1.5f, 0.0f,
                -0.8f,  1.5f, 0.0f
            };

            unsigned int doorOpeningIndices[] = {
                0, 1, 2,
                2, 3, 0
            };

            GLuint VBO, EBO;
            glGenVertexArrays(1, &doorOpeningVAO);
            glGenBuffers(1, &VBO);
            glGenBuffers(1, &EBO);

            glBindVertexArray(doorOpeningVAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(doorOpeningVertices), doorOpeningVertices, GL_STATIC_DRAW);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(doorOpeningIndices), doorOpeningIndices, GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }

        // Transform the quad to the door frame position and orientation
        glm::mat4 doorOpeningModel = glm::mat4(1.0f);
        doorOpeningModel = glm::translate(doorOpeningModel, doorFramePos);
        doorOpeningModel = glm::rotate(doorOpeningModel, atan2(doorFrameNormal.z, doorFrameNormal.x), glm::vec3(0, 1, 0));
        doorOpeningModel = glm::scale(doorOpeningModel, glm::vec3(1.3f, 1.3f, 1.3f));

        basicShader.setMat4("model", &doorOpeningModel[0][0]);
        glBindVertexArray(doorOpeningVAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Step 4b: Re-enable color and depth writing
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);

        // Only render where stencil matches (inside the door opening)
        glStencilFunc(GL_EQUAL, recursionLevel + 2, 0xFF);
        glStencilMask(0x00); // Don't modify stencil buffer

        // Clear depth buffer for the portal view
        glClear(GL_DEPTH_BUFFER_BIT);

        // Step 4c: Calculate and render the portal view through this door opening
        glm::mat4 portalView = calculatePortalView(doorFramePos, doorFrameNormal,
            cameraPos, cameraFront, cameraUp, recursionLevel);

        // Extract virtual camera position for lighting calculations
        glm::mat4 invPortalView = glm::inverse(portalView);
        glm::vec3 virtualCameraPos = glm::vec3(invPortalView[3]);

        // Recursive call to render the next level through this door opening
        renderRecursivePortals(scene, basicShader, lightingManager,
            portalView, projection, virtualCameraPos,
            bookModel, bookshelfModel, bookshelf2Model, columnModel,
            floorModel, lampModel, doorFrameModel, ceilingModel, wallModel, torchModel,
            recursionLevel + 1);
    }

    // Step 5: Disable stencil testing
    glDisable(GL_STENCIL_TEST);
}

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp, float deltaTime,
    const Scene& scene,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& doorFrameModel,
    const std::unique_ptr<Model>& portalQuadModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel,
    const std::unique_ptr<Model>& torchModel,
    float roomRadius, float roomHeight, int numSides,
    float yaw, float pitch,
    LightingManager& lightingManager,
    PortalSystem& portalSystem) {

    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Store old camera position for portal collision detection
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

    // Update teleport cooldown
    if (teleportCooldown > 0.0f) {
        teleportCooldown -= deltaTime;
    }

    // Check for portal teleportation
    if (recursivePortalsEnabled && teleportCooldown <= 0.0f) {
        glm::vec3 teleportPos;
        if (portalSystem.checkPortalCollision(oldCameraPos, cameraPos, teleportPos)) {
            glm::vec3 teleportOffset = teleportPos - cameraPos;
            cameraPos = teleportPos;
            playerOffset += teleportOffset;
            playerTeleported = true;
            teleportCooldown = TELEPORT_COOLDOWN_TIME;

            std::cout << "Player teleported! New offset: ("
                << playerOffset.x << ", " << playerOffset.y << ", " << playerOffset.z << ")" << std::endl;
        }
    }

    // Recursive Portal controls
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !portalTogglePressed) {
        portalTogglePressed = true;
        recursivePortalsEnabled = !recursivePortalsEnabled;
        std::cout << "Recursive Portals: " << (recursivePortalsEnabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        portalTogglePressed = false;
    }

    // Debug toggle
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        showDebugInfo = !showDebugInfo;
        std::cout << "Debug mode: " << (showDebugInfo ? "ON" : "OFF") << std::endl;
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
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "BABEL - Recursive Portals", nullptr, nullptr);
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

    std::cout << "BABEL - Recursive Portal System" << std::endl;
    std::cout << "GL Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "GL Renderer: " << glGetString(GL_RENDERER) << std::endl;

    // Load shaders
    Shader basicShader("shaders/book.vert", "shaders/book.frag");
    Shader portalShader("shaders/portal.vert", "shaders/portal.frag");

    // Load all textures using TextureManager
    TextureManager::loadAllTextures();

    // Load models - CHECKING IF FILES EXIST
    std::cout << "Loading models..." << std::endl;
    auto bookModel = std::make_unique<Model>("assets/models/book.obj");
    auto bookshelfModel = std::make_unique<Model>("assets/models/bookshelf.obj");
    auto bookshelf2Model = std::make_unique<Model>("assets/models/Bookshelf2.obj");
    auto columnModel = std::make_unique<Model>("assets/models/column.obj");
    auto floorModel = std::make_unique<Model>("assets/models/floor.obj");
    auto lampModel = std::make_unique<Model>("assets/models/lamb.obj");

    // Try to load door.obj, if it fails, use column.obj as fallback
    std::unique_ptr<Model> doorFrameModel;
    try {
        doorFrameModel = std::make_unique<Model>("assets/models/door.obj");
        std::cout << "Loaded door.obj successfully" << std::endl;
    }
    catch (...) {
        std::cout << "door.obj not found, using column.obj as doorframe" << std::endl;
        doorFrameModel = std::make_unique<Model>("assets/models/column.obj");
    }

    // Try to load quad.obj, if it fails, create a simple fallback
    std::unique_ptr<Model> portalQuadModel;
    try {
        portalQuadModel = std::make_unique<Model>("assets/models/quad.obj");
        std::cout << "Loaded quad.obj successfully" << std::endl;
    }
    catch (...) {
        std::cout << "quad.obj not found, using book.obj as fallback" << std::endl;
        portalQuadModel = std::make_unique<Model>("assets/models/book.obj");
    }

    auto ceilingModel = std::make_unique<Model>("assets/models/ceiling.obj");
    auto wallModel = std::make_unique<Model>("assets/models/wall.obj");
    auto torchModel = std::make_unique<Model>("assets/models/torch.obj");

    // Create scene
    Scene scene;
    const float roomRadius = 8.0f;
    const float roomHeight = 6.0f;
    const int numSides = 8;

    // Create lighting manager
    LightingManager lightingManager;

    // Create portal system
    PortalSystem portalSystem;

    std::cout << "Setting up infinite library with recursive portals..." << std::endl;

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

    // DOOR FRAMES - Portal archways
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = roomRadius * 0.8f * cos(angle);
        float z = roomRadius * 0.8f * sin(angle);
        float rotationToFaceCenter = angle + glm::radians(90.0f);

        scene.addObject(doorFrameModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            glm::vec3(1.3f, 1.3f, 1.3f));
    }

    // PORTAL QUADS - These show the recursive portal effect
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = roomRadius * 0.8f * cos(angle);
        float z = roomRadius * 0.8f * sin(angle);
        float rotationToFaceCenter = angle + glm::radians(90.0f);

        // Position the quad deeper inside the door frame to avoid z-fighting
        glm::vec3 normal = glm::vec3(-cos(angle), 0.0f, -sin(angle));
        glm::vec3 quadPos = glm::vec3(x, roomHeight * 0.3f, z) + normal * 0.3f;

        scene.addObject(portalQuadModel.get(),
            quadPos,
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            glm::vec3(0.8f, 1.2f, 1.0f));
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

    // FLOATING BOOKS - Magical animations
    std::cout << "Setting up magical floating books..." << std::endl;

    // Central orbiting books
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

        for (int book = 0; book < 2; book++) {
            size_t bookIndex = scene.objects.size();
            glm::vec3 bookStartPos = columnPos + glm::vec3(1.5f, book * 1.0f, 0.0f);

            scene.addObject(bookModel.get(),
                bookStartPos,
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.9f, 0.9f, 0.9f));

            scene.objects[bookIndex].setOrbiting(true,
                columnPos + glm::vec3(0.0f, book * 1.0f, 0.0f),
                1.8f,
                0.3f + book * 0.15f);
            scene.objects[bookIndex].setRotating(true, 1.2f);

            if (book % 2 == 0) {
                scene.objects[bookIndex].setFloating(true, 0.2f, 1.5f);
            }
        }
    }

    // High-altitude books
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = 5.0f * cos(angle);
        float z = 5.0f * sin(angle);

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(x, 4.5f, z),
            glm::vec3(glm::radians(15.0f), angle, 0.0f),
            glm::vec3(1.1f, 1.1f, 1.1f));

        scene.objects[bookIndex].setOrbiting(true, glm::vec3(0.0f, 4.5f, 0.0f), 5.0f, 0.15f);
        scene.objects[bookIndex].setFloating(true, 0.3f, 0.8f);
        scene.objects[bookIndex].setRotating(true, 0.3f);
    }

    // SETUP LIBRARY LIGHTING
    std::cout << "Setting up dramatic library lighting..." << std::endl;
    lightingManager.setupLibraryLighting(roomRadius, roomHeight);
    lightingManager.setAmbientDarkness(0.2f);
    lightingManager.setTorchIntensity(2.5f);

    // SETUP PORTAL SYSTEM
    std::cout << "Setting up portal system..." << std::endl;
    portalSystem.initialize();

    // Add portals at door frame positions
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        glm::vec3 position = glm::vec3(
            roomRadius * 0.8f * cos(angle),
            roomHeight * 0.5f,
            roomRadius * 0.8f * sin(angle)
        );
        glm::vec3 normal = glm::vec3(-cos(angle), 0.0f, -sin(angle));
        portalSystem.addPortal(position, normal);
    }

    // Connect portals in a cycle
    for (int i = 0; i < 4; i++) {
        portalSystem.connectPortals(i, (i + 1) % 4);
    }

    std::cout << "\n=== BABEL RECURSIVE PORTAL CONTROLS ===" << std::endl;
    std::cout << "  WASD + Mouse - Move camera" << std::endl;
    std::cout << "  Space/Ctrl - Up/Down" << std::endl;
    std::cout << "  H - Debug info" << std::endl;
    std::cout << "  P - Toggle RECURSIVE PORTALS on/off" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << "✅ 5+ Independent 3D Objects" << std::endl;
    std::cout << "✅ Complex Animations" << std::endl;
    std::cout << "✅ Multiple Textures" << std::endl;
    std::cout << "✅ Multiple Light Sources" << std::endl;
    std::cout << "✅ Non-trivial 3D Models" << std::endl;
    std::cout << "✅ Recursive Portals with Stencil Buffer" << std::endl;
    std::cout << "✅ Player Teleportation" << std::endl;
    std::cout << "==============================================\n" << std::endl;

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
            floorModel, lampModel, doorFrameModel, portalQuadModel, ceilingModel, wallModel, torchModel,
            roomRadius, roomHeight, numSides, yaw, pitch, lightingManager, portalSystem);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        // Apply global animation speed to scene update
        scene.update(deltaTime * globalAnimationSpeed);

        // Update lighting animations
        lightingManager.update(deltaTime * globalAnimationSpeed);

        // Update portal distances
        portalSystem.updateDistances(cameraPos);

        // Create view and projection matrices
        glm::vec3 adjustedCameraPos = cameraPos + playerOffset;
        glm::mat4 view = glm::lookAt(adjustedCameraPos, adjustedCameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

        // RECURSIVE PORTAL RENDERING
        glClearColor(0.01f, 0.005f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if (recursivePortalsEnabled) {
            // Render with recursive portals using stencil buffer
            renderRecursivePortals(scene, basicShader, lightingManager, view, projection, adjustedCameraPos,
                bookModel, bookshelfModel, bookshelf2Model, columnModel,
                floorModel, lampModel, doorFrameModel, ceilingModel, wallModel, torchModel, 0);
        }
        else {
            // Standard rendering without portals
            renderScene(scene, basicShader, lightingManager, view, projection, adjustedCameraPos,
                bookModel, bookshelfModel, bookshelf2Model, columnModel,
                floorModel, lampModel, doorFrameModel, ceilingModel, wallModel, torchModel);
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