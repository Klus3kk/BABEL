#version 330 core
out vec4 FragColor; // Output color of the fragment shader

in vec3 FragPos; // 3D position of the fragment in world space
in vec3 Normal; // Surface normal at the fragment (shading)
in vec2 TexCoord; // Texture coordinates for the fragment

uniform vec3 viewPos; // Camera position in world space
uniform sampler2D baseColorMap; // Base color texture for mesh
uniform vec3 ambientColor; // ambientColor light
uniform float ambientStrength; // Strength of ambient light
uniform float time; // Time variable for animations 

void main() {
    // Sample the texture
    vec3 albedo = texture(baseColorMap, TexCoord).rgb; // Base color from texture
    
    // Light objects should be mostly self-lit (they ARE the light sources)
    vec3 result = albedo;
    
    // a bit of ambient light 
    result += ambientColor * ambientStrength * 0.5f;
    
    // warm glow effect
    result *= vec3(1.1f, 0.95f, 0.8f);  
    
    // gamma correction, without it the colors can look washed out
    result = pow(result, vec3(1.0f/2.2f));
    
    FragColor = vec4(result, 1.0f);
}