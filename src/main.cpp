#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "shader.hpp"
#include "texture.hpp"
#include "model.hpp"

const unsigned int WIDTH = 1280;
const unsigned int HEIGHT = 720;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

int main() {
    // Init
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

    if (glewInit() != GLEW_OK) {
        std::cerr << "GLEW init failed!\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    std::cout << "GL ready. Vendor: " << glGetString(GL_VENDOR) << std::endl;

    // Load shader
    Shader bookShader("shaders/book.vert", "shaders/book.frag");

    // Load textures
    GLuint baseColorMap = Texture::load("assets/textures/book_basecolor.png");
    GLuint roughnessMap = Texture::load("assets/textures/book_roughness.png");
    GLuint metallicMap = Texture::load("assets/textures/book_metallic.png");

    // Load model
    Model book("assets/models/book.obj");

    // Camera + projection
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 1.5f, 3.0f),  // camera position
        glm::vec3(0.0f, 1.0f, 0.0f),  // looking at
        glm::vec3(0.0f, 1.0f, 0.0f)   // up
    );

    // Light position
    glm::vec3 lightPos = glm::vec3(2.0f, 3.0f, 2.0f);

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.05f, 0.02f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Time-based rotation
        float angle = (float)glfwGetTime();
        // Try centering manually
        glm::vec3 pivotOffset = glm::vec3(0.05f, 0.0f, 0.0f); // try Y axis


        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f));      // position in the world
        model = glm::translate(model, pivotOffset);          // bring pivot to origin
        model = glm::rotate(model, angle, glm::vec3(0.0f, 0.5f, 0.0f)); // rotate in place
        model = glm::translate(model, -pivotOffset);         // push back
        model = glm::scale(model, glm::vec3(1.0f));          // scale if needed

        // Activate shader
        bookShader.use();
        bookShader.setMat4("model", &model[0][0]);
        bookShader.setMat4("view", &view[0][0]);
        bookShader.setMat4("projection", &projection[0][0]);
        bookShader.setVec3("lightPos", lightPos.x, lightPos.y, lightPos.z);
        bookShader.setVec3("viewPos", 0.0f, 1.5f, 3.0f);

        // Bind PBR textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, baseColorMap);
        bookShader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, roughnessMap);
        bookShader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, metallicMap);
        bookShader.setInt("metallicMap", 2);

        // Draw model
        book.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
