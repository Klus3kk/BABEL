#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>
#include <memory>
#include <cmath>

#include "shader.hpp"
#include "texture.hpp"
#include "model.hpp"
#include "scene.hpp"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

// Camera variables
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

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
    float yoffset = lastY - ypos; // Reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // Make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;
}

void processInput(GLFWwindow* window, glm::vec3& cameraPos, glm::vec3& cameraFront, glm::vec3& cameraUp, float deltaTime) {
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

    // Set up mouse capture for camera rotation
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    std::cout << "GL ready. Vendor: " << glGetString(GL_VENDOR) << std::endl;

    // Load shader
    Shader basicShader("shaders/book.vert", "shaders/book.frag");

    // Create a temporary fallback texture (blue, not pink)
    GLuint fallbackTexture;
    glGenTextures(1, &fallbackTexture);
    glBindTexture(GL_TEXTURE_2D, fallbackTexture);

    unsigned char data[] = { 0, 0, 255, 255 }; // Blue
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // Load models only once to save memory
    std::cout << "Loading models..." << std::endl;

    std::unique_ptr<Model> bookModel = std::make_unique<Model>("assets/models/book.obj");
    std::unique_ptr<Model> bookshelfModel = std::make_unique<Model>("assets/models/bookshelf.obj");
    std::unique_ptr<Model> bookshelf2Model = std::make_unique<Model>("assets/models/Bookshelf2.obj");
    std::unique_ptr<Model> columnModel = std::make_unique<Model>("assets/models/column.obj");
    std::unique_ptr<Model> floorModel = std::make_unique<Model>("assets/models/floor.obj");
    std::unique_ptr<Model> lampModel = std::make_unique<Model>("assets/models/lamb.obj");
    std::unique_ptr<Model> portalModel = std::make_unique<Model>("assets/models/portal.obj");
    std::unique_ptr<Model> ceilingModel = std::make_unique<Model>("assets/models/ceiling.obj");

    // Create scene
    Scene scene;

    // Room dimensions
    const float roomRadius = 8.0f;  // Distance from center to wall
    const float roomHeight = 6.0f;  // Height of the room
    const int numSides = 8;         // Octagonal room

    // Platform at the bottom
    scene.addObject(floorModel.get(),
        glm::vec3(0.0f, -1.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(roomRadius, 0.5f, roomRadius));

    // Ceiling at the top
    scene.addObject(ceilingModel.get(),
        glm::vec3(0.0f, roomHeight, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(roomRadius, 0.5f, roomRadius));

    // Large lamp at the center
    scene.addObject(lampModel.get(),
        glm::vec3(0.0f, roomHeight - 1.0f, 0.0f),
        glm::vec3(0.0f),
        glm::vec3(1.5f));

    // Let's rotate the lamp (index 2 in objects vector)
    scene.objects[2].setRotating(true, 0.2f);

    // Four columns at 45-degree offsets from the cardinal directions
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(45.0f + 90.0f * i);
        float x = roomRadius * 0.6f * cos(angle);
        float z = roomRadius * 0.6f * sin(angle);

        scene.addObject(columnModel.get(),
            glm::vec3(x, roomHeight / 2.0f - 1.0f, z),
            glm::vec3(0.0f),
            glm::vec3(0.5f, roomHeight / 2.0f, 0.5f));
    }

    // Bookcases around the perimeter (on each side of the octagon)
    for (int i = 0; i < numSides; i++) {
        float angle = glm::radians(360.0f * i / numSides);
        float nextAngle = glm::radians(360.0f * (i + 1) / numSides);

        // Skip portal locations (cardinal directions)
        if (i % 2 == 0) {
            continue;  // Skip this side for portal placement
        }

        // Calculate the midpoint of this wall segment
        float midAngle = (angle + nextAngle) / 2.0f;
        float x = roomRadius * 0.95f * cos(midAngle);
        float z = roomRadius * 0.95f * sin(midAngle);

        // Bookcase rotation (face inward)
        float rotY = midAngle + glm::radians(180.0f);  // Face toward center

        // Alternate bookshelf models
        Model* shelfModel = (i % 4 == 1) ? bookshelf2Model.get() : bookshelfModel.get();

        scene.addObject(shelfModel,
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, rotY, 0.0f),
            glm::vec3(1.0f, 1.0f, 1.0f));

        // Add some books on top of the bookshelf
        for (int j = 0; j < 3; j++) {
            float offsetX = 0.3f * (j - 1);
            size_t index = scene.objects.size();

            scene.addObject(bookModel.get(),
                glm::vec3(x + offsetX * cos(rotY), 2.0f, z + offsetX * sin(rotY)),
                glm::vec3(0.0f, rotY + glm::radians(90.0f * j), 0.0f),
                glm::vec3(0.3f));

            // Rotate some books
            if (j == 1) {
                scene.objects[index].setRotating(true, 0.5f);
            }
        }
    }

    // Four portals at cardinal directions
    for (int i = 0; i < 4; i++) {
        float angle = glm::radians(90.0f * i);
        float x = roomRadius * 0.95f * cos(angle);
        float z = roomRadius * 0.95f * sin(angle);

        // Rotate to face inward
        float rotY = angle + glm::radians(270.0f);

        scene.addObject(portalModel.get(),
            glm::vec3(x, 0.0f, z),
            glm::vec3(0.0f, rotY, 0.0f),
            glm::vec3(1.0f, 1.2f, 1.0f));
    }

    // Floating books in the center for added effect
    for (int i = 0; i < 5; i++) {
        float angle = glm::radians(72.0f * i);
        float radius = 2.0f + i * 0.3f;
        float height = 1.0f + 0.5f * sin(glm::radians(i * 60.0f));

        size_t index = scene.objects.size();
        scene.addObject(bookModel.get(),
            glm::vec3(radius * cos(angle), height, radius * sin(angle)),
            glm::vec3(glm::radians(20.0f), angle, 0.0f),
            glm::vec3(0.4f));

        // Make floating books rotate
        scene.objects[index].setRotating(true, 0.2f + (i * 0.1f));
    }

    // Camera setup
    glm::vec3 cameraPos = glm::vec3(0.0f, 1.5f, 0.0f);
    glm::vec3 cameraFront = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // Light position
    glm::vec3 lightPos = glm::vec3(0.0f, roomHeight - 1.0f, 0.0f);

    // Timing variables
    float deltaTime = 0.0f;
    float lastFrame = 0.0f;

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Timing
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window, cameraPos, cameraFront, cameraUp, deltaTime);

        // Update camera direction based on mouse input
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        // Update scene animations
        scene.update(deltaTime);

        // Clear buffer
        glClearColor(0.02f, 0.01f, 0.05f, 1.0f);  // Very dark blue/black background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Camera/View transformation
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        // Projection
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

        // Set up shader
        basicShader.use();
        basicShader.setMat4("view", &view[0][0]);
        basicShader.setMat4("projection", &projection[0][0]);
        basicShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
        basicShader.setVec3("viewPos", cameraPos.x, cameraPos.y, cameraPos.z);

        // Bind our fallback textures for all objects
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
        basicShader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
        basicShader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, fallbackTexture);
        basicShader.setInt("metallicMap", 2);

        // Draw all scene objects
        scene.draw(basicShader);

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up
    glDeleteTextures(1, &fallbackTexture);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}