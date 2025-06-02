// shaders/light.frag - For emissive light sources like torches
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 viewPos;
uniform sampler2D baseColorMap;
uniform float time;

void main() {
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    
    // Make light sources glow
    float glowIntensity = 1.5 + sin(time * 5.0) * 0.3; // Flickering glow
    vec3 emissive = albedo * glowIntensity;
    
    // Add warm flame color to torches
    vec3 flameColor = vec3(1.0, 0.6, 0.2);
    emissive += flameColor * 0.3 * glowIntensity;
    
    // Simple ambient for base visibility
    vec3 ambient = albedo * 0.2;
    
    vec3 result = ambient + emissive;
    
    // Tone mapping
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}