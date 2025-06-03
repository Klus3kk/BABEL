#include "TextureManager.hpp"
#include "texture.hpp"
#include <iostream>

// Static member definition - stores all loaded textures in memory
std::unordered_map<std::string, GLuint> TextureManager::textures;

GLuint TextureManager::loadTexture(const std::string& name, const std::string& filePath) {
    // Check if texture is already loaded to avoid duplicates
    if (textures.find(name) != textures.end()) {
        return textures[name];
    }

    // Load texture from file and store in map
    GLuint textureID = Texture::load(filePath);
    textures[name] = textureID;
    std::cout << "Loaded texture: " << name << " from " << filePath << std::endl;
    return textureID;
}

GLuint TextureManager::getTexture(const std::string& name) {
    auto it = textures.find(name);
    if (it != textures.end()) {
        return it->second;  // Return OpenGL texture ID
    }

    std::cerr << "Warning: Texture '" << name << "' not found!" << std::endl;
    return 0;  // Return 0 (invalid texture ID) if not found
}

void TextureManager::loadAllTextures() {
    std::cout << "Loading all textures..." << std::endl;

    // Load book textures for PBR rendering
    loadTexture("book_basecolor", "assets/textures/book-textures/book_basecolor.png");
    loadTexture("book_roughness", "assets/textures/book-textures/book_roughness.png");
    loadTexture("book_metallic", "assets/textures/book-textures/book_metallic.png");

    // Load ceiling texture
    loadTexture("ceiling_basecolor", "assets/textures/ceiling-textures/plafondbleu.jpeg");

    // Load column textures (stone/marble)
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
        // Bind full PBR texture set for books
        glActiveTexture(GL_TEXTURE0);  // Texture unit 0
        glBindTexture(GL_TEXTURE_2D, getTexture("book_basecolor"));
        shader.setInt("baseColorMap", 0);  // Tell shader to use texture unit 0

        glActiveTexture(GL_TEXTURE1);  // Texture unit 1
        glBindTexture(GL_TEXTURE_2D, getTexture("book_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);  // Texture unit 2
        glBindTexture(GL_TEXTURE_2D, getTexture("book_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "bookshelf") {
        // Wood textures for bookshelves
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("wood_basecolor"));
        shader.setInt("baseColorMap", 0);

        // Reuse column roughness/metallic for wood (low metallic values)
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "column") {
        // Full PBR texture set for stone columns
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
        // Floor-specific texture with reused roughness/metallic
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("floor_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "wall") {
        // Rock tile texture for walls
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("wall_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "doorframe") {
        // Gray rocks texture for doorframes
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("doorframe_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "ceiling") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("ceiling_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "lamp") {
        // Metal texture for lamp
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("metal_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
    else if (objectType == "torch") {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, getTexture("torch_basecolor"));
        shader.setInt("baseColorMap", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_roughness"));
        shader.setInt("roughnessMap", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, getTexture("column_metallic"));
        shader.setInt("metallicMap", 2);
    }
}

void TextureManager::cleanup() {
    // Free all OpenGL texture objects to prevent memory leaks
    for (auto& pair : textures) {
        glDeleteTextures(1, &pair.second);
    }
    textures.clear();
    std::cout << "Textures cleaned up." << std::endl;
}