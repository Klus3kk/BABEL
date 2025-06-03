#version 330 core
// Simplified vertex shader for portal quads
layout (location = 0) in vec3 aPos;      // Portal quad vertices
layout (location = 2) in vec2 aTexCoord; // UV coordinates for portal texture

out vec2 TexCoord; // Pass UV to fragment shader

// Transformation matrices
uniform mat4 model;      // Portal positioning and orientation
uniform mat4 view;       // Camera transformation
uniform mat4 projection; // Projection to screen

void main() {
    // Simple pass-through of texture coordinates
    TexCoord = aTexCoord;
    
    // Transform portal quad to screen space
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}