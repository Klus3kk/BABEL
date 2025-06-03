#version 330 core
// Input vertex attributes (from Model::draw())
layout (location = 0) in vec3 aPos;      // Vertex position
layout (location = 1) in vec3 aNormal;   // Surface normal
layout (location = 2) in vec2 aTexCoord; // Texture coordinates

// Output to fragment shader
out vec3 FragPos;  // World position of vertex
out vec3 Normal;   // Transformed normal
out vec2 TexCoord; // Pass-through texture coordinates

// Transformation matrices (set by application)
uniform mat4 model;      // Object-to-world transformation
uniform mat4 view;       // World-to-camera transformation  
uniform mat4 projection; // Camera-to-screen projection

void main() {
    // Transform vertex position to world space
    FragPos = vec3(model * vec4(aPos, 1.0));
    
    // Transform normal to world space (using normal matrix to handle non-uniform scaling)
    // mat3(transpose(inverse(model))) is the proper normal transformation matrix
    Normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass texture coordinates unchanged
    TexCoord = aTexCoord;
    
    // Transform vertex to screen space for rasterization
    gl_Position = projection * view * vec4(FragPos, 1.0);
}