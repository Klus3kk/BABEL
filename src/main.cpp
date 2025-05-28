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
static glm::vec3 playerOffset = glm::vec3(0.0f); // Accumulated teleportation offset
static bool playerTeleported = false;
static float teleportCooldown = 0.0f;
static const float TELEPORT_COOLDOWN_TIME = 0.5f; // Prevent rapid teleportation

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
            TextureManager::bindTextureForObject("portal", shader); // Use portal texture for door frames
        }

        shader.setMat4("model", &obj.modelMatrix[0][0]);
        obj.model->draw();
    }
}

glm::mat4 calculatePortalView(const glm::vec3& doorFramePos, const glm::vec3& doorFrameNormal,
    const glm::vec3& cameraPos, const glm::vec3& cameraFront, const glm::vec3& cameraUp,
    int recursionLevel) {

    // Calculate the virtual camera position through the portal
    // This implements the portal transformation from the tutorial

    // Distance from camera to portal
    glm::vec3 toDoorFrame = doorFramePos - cameraPos;
    float distanceToPortal = glm::length(toDoorFrame);

    // Calculate the position relative to the portal
    glm::vec3 relativePos = cameraPos - doorFramePos;

    // Transform through the portal (simplified - move to next room)
    float roomOffset = 30.0f + (recursionLevel * 25.0f); // Rooms get further apart
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

        // Step 4a: Create stencil mask for the door opening (not the frame!)
        glStencilFunc(GL_ALWAYS, recursionLevel + 2, 0xFF);
        glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
        glStencilMask(0xFF);

        // Disable color and depth writing - only write to stencil
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDepthMask(GL_FALSE);

        // Render a simple quad representing the door opening (NOT the full door frame)
        // This creates the stencil mask for where the portal effect should appear
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
        doorOpeningModel = glm::scale(doorOpeningModel, glm::vec3(1.3f, 1.3f, 1.3f)); // Match door frame scale

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

void printSceneDebugInfo(const Scene& scene,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& doorFrameModel,
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
        else if (obj.model == doorFrameModel.get()) objectType = "DoorFrame";
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
    std::cout << "==========================" << std::endl;
    std::cout << "Recursive Portal Info:" << std::endl;
    std::cout << "Max Recursion Depth: " << maxRecursionDepth << std::endl;
    std::cout << "Portals Enabled: " << (recursivePortalsEnabled ? "YES" : "NO") << std::endl;
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
            // Player walked through a portal!
            glm::vec3 teleportOffset = teleportPos - cameraPos;

            // Apply the teleportation
            cameraPos = teleportPos;
            playerOffset += teleportOffset;
            playerTeleported = true;
            teleportCooldown = TELEPORT_COOLDOWN_TIME;

            std::cout << "Player teleported! New offset: ("
                << playerOffset.x << ", " << playerOffset.y << ", " << playerOffset.z << ")" << std::endl;
        }
    }

    // Animation speed controls
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && !animationSpeedChanged) {
        globalAnimationSpeed = 0.5f;
        animationSpeedChanged = true;
        std::cout << "Animation speed: SLOW" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && !animationSpeedChanged) {
        globalAnimationSpeed = 1.0f;
        animationSpeedChanged = true;
        std::cout << "Animation speed: NORMAL" << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && !animationSpeedChanged) {
        globalAnimationSpeed = 2.0f;
        animationSpeedChanged = true;
        std::cout << "Animation speed: FAST" << std::endl;
    }

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
        case 0:
            lightingManager.setAmbientColor(glm::vec3(0.15f, 0.1f, 0.2f), 0.3f);
            std::cout << "Lighting: NORMAL" << std::endl;
            break;
        case 1:
            lightingManager.setAmbientColor(glm::vec3(0.3f, 0.2f, 0.1f), 0.4f);
            std::cout << "Lighting: WARM COZY" << std::endl;
            break;
        case 2:
            lightingManager.setAmbientColor(glm::vec3(0.1f, 0.1f, 0.3f), 0.2f);
            std::cout << "Lighting: COOL MYSTICAL" << std::endl;
            break;
        case 3:
            lightingManager.setAmbientColor(glm::vec3(0.05f, 0.05f, 0.1f), 0.1f);
            std::cout << "Lighting: DRAMATIC" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE) {
        lightingKeyPressed = false;
    }

    // Torch intensity control
    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS && !torchKeyPressed) {
        torchKeyPressed = true;
        static int torchLevel = 2;
        torchLevel = (torchLevel + 1) % 4;

        switch (torchLevel) {
        case 0:
            lightingManager.setTorchIntensity(1.5f);
            std::cout << "Torch intensity: DIM" << std::endl;
            break;
        case 1:
            lightingManager.setTorchIntensity(2.0f);
            std::cout << "Torch intensity: MEDIUM" << std::endl;
            break;
        case 2:
            lightingManager.setTorchIntensity(2.5f);
            std::cout << "Torch intensity: BRIGHT" << std::endl;
            break;
        case 3:
            lightingManager.setTorchIntensity(3.0f);
            std::cout << "Torch intensity: VERY BRIGHT" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_RELEASE) {
        torchKeyPressed = false;
    }

    // Darkness level control
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !darknessKeyPressed) {
        darknessKeyPressed = true;
        static int darknessLevel = 1;
        darknessLevel = (darknessLevel + 1) % 4;

        switch (darknessLevel) {
        case 0:
            lightingManager.setAmbientDarkness(0.0f);
            std::cout << "Darkness: PITCH BLACK" << std::endl;
            break;
        case 1:
            lightingManager.setAmbientDarkness(0.2f);
            std::cout << "Darkness: VERY DARK" << std::endl;
            break;
        case 2:
            lightingManager.setAmbientDarkness(0.4f);
            std::cout << "Darkness: DARK" << std::endl;
            break;
        case 3:
            lightingManager.setAmbientDarkness(0.6f);
            std::cout << "Darkness: MEDIUM DARK" << std::endl;
            break;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        darknessKeyPressed = false;
    }

    // Dramatic mode toggle
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

    // Torch flicker controls
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS && !plusKeyPressed) {
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

    // Recursive Portal controls
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !portalTogglePressed) {
        portalTogglePressed = true;
        recursivePortalsEnabled = !recursivePortalsEnabled;
        std::cout << "Recursive Portals: " << (recursivePortalsEnabled ? "ENABLED" : "DISABLED") << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE) {
        portalTogglePressed = false;
    }

    // Recursion depth controls
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !recursionKeyPressed) {
        recursionKeyPressed = true;
        maxRecursionDepth = maxRecursionDepth % 6 + 1; // Cycle 1-6
        std::cout << "Portal recursion depth: " << maxRecursionDepth << std::endl;
    }
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
        recursionKeyPressed = false;
    }

    // Debug toggle
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        showDebugInfo = !showDebugInfo;

        if (showDebugInfo) {
            std::cout << "\n*** DEBUG MODE ACTIVATED ***" << std::endl;
            printRoomDebugInfo(roomRadius, roomHeight, numSides);
            printCameraDebugInfo(cameraPos, cameraFront, yaw, pitch);
            printLightingDebugInfo(lightingManager);
            printSceneDebugInfo(scene, bookModel, bookshelfModel, bookshelf2Model,
                columnModel, floorModel, lampModel, doorFrameModel,
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

    // CRITICAL: Enable stencil buffer for recursive portals
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

    // Load models
    std::cout << "Loading models..." << std::endl;
    std::unique_ptr<Model> bookModel = std::make_unique<Model>("assets/models/book.obj");
    std::unique_ptr<Model> bookshelfModel = std::make_unique<Model>("assets/models/bookshelf.obj");
    std::unique_ptr<Model> bookshelf2Model = std::make_unique<Model>("assets/models/Bookshelf2.obj");
    std::unique_ptr<Model> columnModel = std::make_unique<Model>("assets/models/column.obj");
    std::unique_ptr<Model> floorModel = std::make_unique<Model>("assets/models/floor.obj");
    std::unique_ptr<Model> lampModel = std::make_unique<Model>("assets/models/lamb.obj");
    std::unique_ptr<Model> doorFrameModel = std::make_unique<Model>("assets/models/door.obj");
    std::unique_ptr<Model> portalQuadModel = std::make_unique<Model>("assets/models/quad.obj");  // NEW: Portal quad
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

    // Create portal system (still needed for some functionality)
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

    // DOOR FRAMES - These are the stone archways
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

    // PORTAL QUADS - These show the recursive portal effect (positioned in door openings)
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * static_cast<float>(i));
        float x = roomRadius * 0.8f * cos(angle);
        float z = roomRadius * 0.8f * sin(angle);
        float rotationToFaceCenter = angle + glm::radians(90.0f);

        // Position the quad deeper inside the door frame to avoid z-fighting
        glm::vec3 normal = glm::vec3(-cos(angle), 0.0f, -sin(angle));
        glm::vec3 quadPos = glm::vec3(x, roomHeight * 0.01f, z) + normal * 0.3f; // Move deeper inward

        scene.addObject(portalQuadModel.get(),
            quadPos,
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            glm::vec3(0.8f, 1.2f, 1.0f)); // Slightly smaller to fit better
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

    // FLOATING BOOKS - Enhanced magical animations
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

    // SETUP PORTAL SYSTEM (for compatibility)
    std::cout << "Setting up portal system..." << std::endl;
    portalSystem.initialize(roomRadius, roomHeight);

    // Create scene rendering lambda function for recursive portals
    auto renderSceneFunction = [&](const glm::mat4& view, const glm::mat4& projection,
        int recursionLevel, const RoomVariation& variation) {
            basicShader.use();
            basicShader.setMat4("view", &view[0][0]);
            basicShader.setMat4("projection", &projection[0][0]);

            // Extract camera position from view matrix
            glm::mat4 invView = glm::inverse(view);
            glm::vec3 virtualViewPos = glm::vec3(invView[3]);
            basicShader.setVec3("viewPos", virtualViewPos.x, virtualViewPos.y, virtualViewPos.z);

            // Create modified lighting for room variation
            LightingManager modifiedLighting = lightingManager;

            // Apply room variation to lighting
            for (auto& light : modifiedLighting.pointLights) {
                light.color *= variation.colorTint;
                light.intensity *= variation.lightIntensityMultiplier;
            }
            modifiedLighting.ambientColor *= variation.colorTint;
            modifiedLighting.ambientColor += variation.ambientColorShift;

            // Bind modified lighting
            modifiedLighting.bindToShader(basicShader);

            // Render all objects except portal quads in recursive views
            for (size_t i = 0; i < scene.objects.size(); i++) {
                const auto& obj = scene.objects[i];

                // Skip portal quads in recursive views to prevent infinite recursion
                if (obj.model == portalQuadModel.get() && recursionLevel > 0) {
                    continue;
                }

                // Apply room variation scaling
                glm::mat4 scaledModel = obj.modelMatrix;
                if (variation.scaleMultiplier != 1.0f) {
                    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(variation.scaleMultiplier));
                    scaledModel = scaleMatrix * scaledModel;
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
                else if (obj.model == doorFrameModel.get()) {
                    TextureManager::bindTextureForObject("portal", basicShader);
                }
                // Skip portal quads in scene function - they're rendered separately
                else if (obj.model == portalQuadModel.get()) {
                    continue;
                }

                basicShader.setMat4("model", &scaledModel[0][0]);
                obj.model->draw();
            }
        };
    std::cout << "\n=== BABEL RECURSIVE PORTAL CONTROLS ===" << std::endl;
    std::cout << "  WASD + Mouse - Move camera" << std::endl;
    std::cout << "  Space/Ctrl - Up/Down" << std::endl;
    std::cout << "  H - Debug info" << std::endl;
    std::cout << "  1/2/3 - Animation speed" << std::endl;
    std::cout << "  L - Cycle lighting modes" << std::endl;
    std::cout << "  T - Torch intensity" << std::endl;
    std::cout << "  B - Darkness level" << std::endl;
    std::cout << "  M - Toggle dramatic mode" << std::endl;
    std::cout << "  +/- - Torch flicker" << std::endl;
    std::cout << "  P - Toggle RECURSIVE PORTALS on/off" << std::endl;
    std::cout << "  R - Recursion depth (1-6 levels)" << std::endl;
    std::cout << "==============================================" << std::endl;
    std::cout << "RECURSIVE PORTAL FEATURES:" << std::endl;
    std::cout << "- True recursive rendering using stencil buffer" << std::endl;
    std::cout << "- Walk through door frames to see infinite rooms" << std::endl;
    std::cout << "- Adjustable recursion depth for performance" << std::endl;
    std::cout << "- Based on https://th0mas.nl/2013/05/19/rendering-recursive-portals-with-opengl/" << std::endl;
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

        // Create view and projection matrices (apply player offset for seamless teleportation)
        glm::vec3 adjustedCameraPos = cameraPos + playerOffset;
        glm::mat4 view = glm::lookAt(adjustedCameraPos, adjustedCameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

        // RECURSIVE PORTAL RENDERING
        glClearColor(0.01f, 0.005f, 0.02f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        if (recursivePortalsEnabled) {
            // PHASE 1: Render portal views to textures first
            portalSystem.renderPortalViews(renderSceneFunction, adjustedCameraPos, cameraFront, projection);

            // PHASE 2: Render normal scene first (including door frames and torches)
            renderScene(scene, basicShader, lightingManager, view, projection, adjustedCameraPos,
                bookModel, bookshelfModel, bookshelf2Model, columnModel,
                floorModel, lampModel, doorFrameModel, ceilingModel, wallModel, torchModel, false);

            // PHASE 3: Render portal quads with portal shader (enable depth testing for proper layering)
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LEQUAL); // Allow portal quads to render at same depth as door frames

            portalShader.use();
            portalShader.setMat4("view", &view[0][0]);
            portalShader.setMat4("projection", &projection[0][0]);
            portalShader.setVec3("viewPos", adjustedCameraPos.x, adjustedCameraPos.y, adjustedCameraPos.z);
            portalShader.setFloat("time", currentFrame);
            portalShader.setBool("portalActive", true);

            // Render each portal quad with portal effect
            int portalCount = 0;
            for (size_t i = 0; i < scene.objects.size(); i++) {
                const auto& obj = scene.objects[i];

                if (obj.model == portalQuadModel.get()) {
                    // Bind portal view texture
                    if (portalCount < portalSystem.getPortalCount()) {
                        glActiveTexture(GL_TEXTURE0);
                        portalSystem.bindPortalTexture(portalCount, portalShader);
                        portalShader.setInt("portalView", 0);

                        // Debug: Check if texture is valid
                        GLint textureId;
                        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureId);
                        if (showDebugInfo && textureId == 0) {
                            std::cout << "Warning: Portal " << portalCount << " has no texture bound!" << std::endl;
                        }
                    }

                    // Render portal quad
                    portalShader.setMat4("model", &obj.modelMatrix[0][0]);
                    obj.model->draw();

                    portalCount++;
                }
            }

            // Restore normal depth function
            glDepthFunc(GL_LESS);
        }
        else {
            // Standard rendering without recursive portals
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