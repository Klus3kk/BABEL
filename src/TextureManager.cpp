#include "TextureManager.hpp"
#include "texture.hpp"
#include <iostream>

// Static member definition
std::unordered_map<std::string, GLuint> TextureManager::textures;

GLuint TextureManager::loadTexture(const std::string& name, const std::string& filePath) {
    if (textures.find(name) != textures.end()) {
        return textures[name]; // Already loaded
    }

    GLuint textureID = Texture::load(filePath);
    textures[name] = textureID;
    std::cout << "Loaded texture: " << name << " from " << filePath << std::endl;
    return textureID;
}

GLuint TextureManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second;
    }

    std::cerr << "Warning: Texture '" << name << "' not found!" << std::endl;
}

void TextureManager::loadAllTextures() {
    std::cout << "Loading all textures..." << std::endl;

    // Load book textures 
    loadTexture("book_basecolor", "assets/textures/book-textures/book_basecolor.png");
    loadTexture("book_roughness", "assets/textures/book-textures/book_roughness.png");
    loadTexture("book_metallic", "assets/textures/book-textures/book_metallic.png");

    // Load ceiling texture 
    loadTexture("ceiling_basecolor", "assets/textures/ceiling-textures/plafondbleu.jpeg");

    // Load column textures 
    loadTexture("column_basecolor", "assets/textures/column-textures/pillar_skfb_col.png");
    loadTexture("column_roughness", "assets/textures/column-textures/pillar_skfb_r.png");
    loadTexture("column_metallic", "assets/textures/column-textures/pillar_skfb_m.png");

    // Load floor texture 
    loadTexture("floor_basecolor", "assets/textures/floor-textures/1.jpg");

    // Load stone textures for walls 
    loadTexture("wall_basecolor", "assets/textures/stone-textures/rock_tile_floor_diff_1k.jpg");

    // Load stone textures for doorframes 
    loadTexture("doorframe_basecolor", "assets/textures/stone-textures/gray_rocks_diff_1k.jpg");

    // Load wood textures for bookshelves
    loadTexture("wood_basecolor", "assets/textures/wood-textures/oak_veneer_01_diff_1k.jpg");

    // Load metal textures for lamp
    loadTexture("metal_basecolor", "assets/textures/lamp-textures/Lamp_AlbedoTransparency.png");

    // Load torch texture
    loadTexture("torch_basecolor", "assets/textures/torch-textures/Torch_texture.png");

    std::cout << "All textures loaded!" << std::endl;
}


void TextureManager::bindTextureForObject(const std::string& objectType, Shader& shader) {
    if (objectType == "book") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("book_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("book_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("book_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "bookshelf") {
        // Use wood textures for bookshelves
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("wood_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse column roughness
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Low metallic for wood
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "column") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "floor") {
        // Use the specific floor texture 
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("floor_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Reuse
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "wall") {
        // Use rock tile texture for walls
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("wall_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Reuse
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "doorframe") {
        // Use gray rocks texture for doorframes
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("doorframe_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Reuse
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "ceiling") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("ceiling_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Reuse
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "lamp") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("metal_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Reuse
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "torch") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("torch_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness")); // Reuse for roughness
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic")); // Reuse for metallic
        shader.setInt("metallicMap", 2);
    }
}

void TextureManager::cleanup() {
    for (auto& pair : textures) {
        glDeleteTextures(1, &pair.second);
    }
    textures.clear();
    std::cout << "Textures cleaned up." << std::endl;
}