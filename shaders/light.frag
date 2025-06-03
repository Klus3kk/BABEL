#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform sampler2D baseColorMap;
uniform vec3 ambientColor;
uniform float ambientStrength;
uniform float time;

void main() {
    // Sample the texture
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    
    // Light objects should be mostly self-lit (they ARE the light sources)
    vec3 result = albedo;
    
    // Add just a tiny bit of ambient so they're not completely flat
    result += ambientColor * ambientStrength * 0.5f;
    
    // Optional: Add a warm glow effect
    result *= vec3(1.1f, 0.95f, 0.8f);  // Warm tint
    
    // Optional: Add subtle pulsing for torches
    float pulse = 1.0f + sin(time * 3.0f) * 0.1f;  // Faster pulse for flame effect
    result *= pulse;
    
    // Simple gamma correction
    result = pow(result, vec3(1.0f/2.2f));
    
    FragColor = vec4(result, 1.0f);
}