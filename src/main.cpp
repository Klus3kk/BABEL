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

// Add this function before main()
void printSceneDebugInfo(const Scene& scene,
    const std::unique_ptr<Model>& bookModel,
    const std::unique_ptr<Model>& bookshelfModel,
    const std::unique_ptr<Model>& bookshelf2Model,
    const std::unique_ptr<Model>& columnModel,
    const std::unique_ptr<Model>& floorModel,
    const std::unique_ptr<Model>& lampModel,
    const std::unique_ptr<Model>& portalModel,
    const std::unique_ptr<Model>& ceilingModel,
    const std::unique_ptr<Model>& wallModel) {

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

        std::cout << "Object " << i << " [" << objectType << "]:" << std::endl;
        std::cout << "  Position: (" << obj.position.x << ", " << obj.position.y << ", " << obj.position.z << ")" << std::endl;
        std::cout << "  Rotation: (" << glm::degrees(obj.rotation.x) << " , "
            << glm::degrees(obj.rotation.y) << " , "
            << glm::degrees(obj.rotation.z) << " )" << std::endl;
        std::cout << "  Scale: (" << obj.scale.x << ", " << obj.scale.y << ", " << obj.scale.z << ")" << std::endl;

        if (obj.rotating) {
            std::cout << "  Rotating: YES (speed: " << obj.rotationSpeed << ")" << std::endl;
        }
        std::cout << "  --------" << std::endl;
    }
    std::cout << "================================\n" << std::endl;
}

// Add this function to get camera debug info
void printCameraDebugInfo(const glm::vec3& cameraPos, const glm::vec3& cameraFront,
    float yaw, float pitch) {
    std::cout << "\n=== CAMERA DEBUG INFO ===" << std::endl;
    std::cout << "Position: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ")" << std::endl;
    std::cout << "Front: (" << cameraFront.x << ", " << cameraFront.y << ", " << cameraFront.z << ")" << std::endl;
    std::cout << "Yaw: " << yaw << " " << std::endl;
    std::cout << "Pitch: " << pitch << " " << std::endl;
    std::cout << "=========================\n" << std::endl;
}

// Add this function to display room parameters
void printRoomDebugInfo(float roomRadius, float roomHeight, int numSides) {
    std::cout << "\n=== ROOM DEBUG INFO ===" << std::endl;
    std::cout << "Room Radius: " << roomRadius << std::endl;
    std::cout << "Room Height: " << roomHeight << std::endl;
    std::cout << "Number of Sides: " << numSides << std::endl;
    std::cout << "========================\n" << std::endl;
}

// Update your processInput function to include this:
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
    float roomRadius, float roomHeight, int numSides,
    float yaw, float pitch) {

    const float cameraSpeed = 2.5f * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement (keep existing code)
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

    // Debug toggle (H key)
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS && !hKeyPressed) {
        hKeyPressed = true;
        showDebugInfo = !showDebugInfo;

        if (showDebugInfo) {
            std::cout << "\n*** DEBUG MODE ACTIVATED ***" << std::endl;
            printRoomDebugInfo(roomRadius, roomHeight, numSides);
            printCameraDebugInfo(cameraPos, cameraFront, yaw, pitch);
            printSceneDebugInfo(scene, bookModel, bookshelfModel, bookshelf2Model,
                columnModel, floorModel, lampModel, portalModel,
                ceilingModel, wallModel);
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "BABEL", nullptr, nullptr);
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

    // Create scene
    Scene scene;
    const float roomRadius = 8.0f;
    const float roomHeight = 6.0f;
    const int numSides = 8;

    std::cout << "Setting up library room..." << std::endl;

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

    // LAMP
    scene.addObject(lampModel.get(),
        glm::vec3(0.0f, roomHeight + 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(3.0f, 3.0f, 3.0f));
    scene.objects[2].setRotating(true, 0.2f);

    // WALLS
    for (int i = 0; i < numSides; i++) {
        float angle = glm::radians(360.0f * static_cast<float>(i) / static_cast<float>(numSides));
        float x = roomRadius * cos(angle);
        float z = roomRadius * sin(angle);
        float wallRotation = angle;
		if (i % 2 == 0) {
			wallRotation += glm::radians(90.0f); // Adjust rotation for even walls
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
            glm::vec3(0.8f, 3.0f, 0.8f));
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
            // bookshelfModel (even indices: 0, 2)
            rotationToFaceCenter = angle + glm::radians(90.0f);
            scale = glm::vec3(2.0f, 5.0f, 3.0f);  // Different size for bookshelfModel
        }
        else {
            // bookshelf2Model (odd indices: 1, 3) 
            rotationToFaceCenter = angle + glm::radians(270.0f);  // Different rotation
            scale = glm::vec3(1.2f, 5.0f, 1.4f);  // Different size for bookshelf2Model
        }

        scene.addObject(shelfModel,
            glm::vec3(x, 1.0f, z),
            glm::vec3(0.0f, rotationToFaceCenter, 0.0f),
            scale);
    }

    // FLOATING BOOKS
    for (int i = 0; i < 6; i++) {
        float angle = glm::radians(60.0f * static_cast<float>(i));
        float x = 2.0f * cos(angle);
        float z = 2.0f * sin(angle);

        size_t bookIndex = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(x, 2.0f, z),
            glm::vec3(0.0f, angle, 0.0f),
            glm::vec3(1.3f, 1.3f, 1.3f));
        scene.objects[bookIndex].setRotating(true, 0.5f);
    }

    std::cout << "Library setup complete!" << std::endl;

    // Camera setup
    glm::vec3 cameraPos = glm::vec3(1.2f, 2.3f, 1.16f);
    glm::vec3 cameraFront = glm::vec3(-1.0f, 0.0f, -0.5f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -135.0f;
    pitch = -20.0f;

    glm::vec3 lightPos = glm::vec3(0.0f, roomHeight - 2.5f, 0.0f);

    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime, scene,
            bookModel, bookshelfModel, bookshelf2Model, columnModel,
            floorModel, lampModel, portalModel, ceilingModel, wallModel,
            roomRadius, roomHeight, numSides, yaw, pitch);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        scene.update(deltaTime);

        glClearColor(0.02f, 0.01f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(WIDTH) / static_cast<float>(HEIGHT), 0.1f, 100.0f);

        basicShader.use();
        basicShader.setMat4("view", &view[0][0]);
        basicShader.setMat4("projection", &projection[0][0]);
        basicShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
        basicShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

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
                TextureManager::bindTextureForObject("floor", basicShader);  // Use floor texture (1.jpg)
            }
            else if (obj.model == wallModel.get()) {
                TextureManager::bindTextureForObject("wall", basicShader);   // Use stone texture
            }
            else if (obj.model == ceilingModel.get()) {
                TextureManager::bindTextureForObject("ceiling", basicShader);
            }
            else if (obj.model == lampModel.get()) {
                TextureManager::bindTextureForObject("lamp", basicShader);
            }
            else if (obj.model == portalModel.get()) {
                TextureManager::bindTextureForObject("portal", basicShader); // Use stone texture
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