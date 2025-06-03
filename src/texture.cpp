#define STB_IMAGE_IMPLEMENTATION  // Include implementation of stb_image
#include "texture.hpp"
#include "stb_image.h"
#include <iostream>

GLuint Texture::load(const std::string& path, bool flip) {
    GLuint textureID;
    glGenTextures(1, &textureID);  // Generate OpenGL texture object

    // Set vertical flip option (some image formats are upside down)
    if (flip) stbi_set_flip_vertically_on_load(true);

    // Load image data from file
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

    // Determine OpenGL format based on number of channels
    GLenum format = GL_RGB;
    if (nrChannels == 1) format = GL_RED;        // Grayscale
    else if (nrChannels == 3) format = GL_RGB;   // RGB
    else if (nrChannels == 4) format = GL_RGBA;  // RGBA with alpha

    // Upload texture data to GPU
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);  // Generate mipmap chain for better quality at distance

    // Set texture filtering and wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);     // Horizontal wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);     // Vertical wrapping
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); // Minification filter
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // Magnification filter

    // Free CPU image data (it's now on the GPU)
    stbi_image_free(data);
    return textureID;
}