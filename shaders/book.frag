#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Multiple light sources
struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

struct DirectionalLight {
    vec3 direction;
    vec3 color;
    float intensity;
};

// Maximum number of lights (increased for better lighting)
#define MAX_POINT_LIGHTS 32
#define MAX_DIR_LIGHTS 4

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight dirLights[MAX_DIR_LIGHTS];
uniform int numPointLights;
uniform int numDirLights;

uniform vec3 viewPos;

uniform sampler2D baseColorMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;

// Enhanced ambient lighting
uniform vec3 ambientColor;
uniform float ambientStrength;

// Enhanced infinite library uniforms
uniform vec3 roomColorTint = vec3(1.0, 1.0, 1.0);
uniform float roomScale = 1.0;
uniform vec3 fogColor = vec3(0.02, 0.015, 0.04);
uniform float fogDensity = 0.015;
uniform float time = 0.0;

// Enhanced fog calculation with depth and height
float calculateAdvancedFog(vec3 fragPos, vec3 viewPos) {
    float distance = length(fragPos - viewPos);
    
    // Distance-based fog
    float distanceFog = exp(-fogDensity * distance);
    
    // Height-based fog (thicker at lower elevations)
    float heightFactor = max(0.0, (fragPos.y + 2.0) / 8.0);
    float heightFog = mix(0.3, 1.0, heightFactor);
    
    // Combine both fog effects
    float combinedFog = distanceFog * heightFog;
    
    return clamp(combinedFog, 0.1, 1.0);
}

// Enhanced point light calculation with improved attenuation
vec3 calcEnhancedPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, 
                           vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // Enhanced attenuation with smoother falloff
    float attenuation = light.intensity / (light.constant + light.linear * distance + 
                                         light.quadratic * (distance * distance));
    
    // Improved diffuse shading (Lambert)
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = NdotL * light.color;
    
    // Enhanced specular shading (Blinn-Phong with roughness)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float specularPower = mix(8.0, 128.0, 1.0 - roughness);
    float spec = pow(NdotH, specularPower);
    
    // Fresnel approximation for more realistic metallic reflections
    float fresnel = metallic + (1.0 - metallic) * pow(1.0 - max(dot(halfwayDir, lightDir), 0.0), 5.0);
    vec3 specular = spec * light.color * fresnel;
    
    // Energy conservation
    vec3 kS = specular;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
    return (kD * diffuse + specular) * attenuation * albedo;
}

// Enhanced directional light calculation
vec3 calcEnhancedDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, 
                         vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse shading
    float NdotL = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = NdotL * light.color * light.intensity;
    
    // Specular shading with roughness
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float specularPower = mix(8.0, 128.0, 1.0 - roughness);
    float spec = pow(NdotH, specularPower);
    
    // Fresnel for metallics
    float fresnel = metallic + (1.0 - metallic) * pow(1.0 - max(dot(halfwayDir, lightDir), 0.0), 5.0);
    vec3 specular = spec * light.color * light.intensity * fresnel;
    
    // Energy conservation
    vec3 kS = specular;
    vec3 kD = (vec3(1.0) - kS) * (1.0 - metallic);
    
    return (kD * diffuse + specular) * albedo;
}

// Add magical glow effect for floating books
vec3 addMagicalAura(vec3 color, vec3 fragPos, vec3 viewPos) {
    float distanceToViewer = length(fragPos - viewPos);
    
    // Subtle magical glow for objects close to camera
    if (distanceToViewer < 15.0) {
        float glowIntensity = (15.0 - distanceToViewer) / 15.0;
        glowIntensity *= 0.1; // Keep it subtle
        
        // Magical color shift based on time and position
        vec3 magicalColor = vec3(
            0.8 + 0.2 * sin(time * 2.0 + fragPos.x),
            0.6 + 0.4 * sin(time * 1.5 + fragPos.y),
            1.0 + 0.2 * sin(time * 3.0 + fragPos.z)
        );
        
        color += magicalColor * glowIntensity;
    }
    
    return color;
}

void main() {
    // Sample material textures with enhanced filtering
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    float roughness = texture(roughnessMap, TexCoord).r;
    float metallic = texture(metallicMap, TexCoord).r;
    
    // Apply room color tint for variety
    albedo *= roomColorTint;
    
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Enhanced ambient lighting with color variation
    vec3 ambient = ambientColor * ambientStrength * albedo;
    
    // Add subtle ambient occlusion effect
    float ao = 0.8 + 0.2 * max(dot(norm, vec3(0.0, 1.0, 0.0)), 0.0);
    ambient *= ao;
    
    // Start with ambient contribution
    vec3 result = ambient;
    
    // Add contribution from all point lights
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcEnhancedPointLight(pointLights[i], norm, FragPos, viewDir, albedo, roughness, metallic);
    }
    
    // Add contribution from all directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcEnhancedDirLight(dirLights[i], norm, viewDir, albedo, roughness, metallic);
    }
    
    // Add magical aura for floating objects
    result = addMagicalAura(result, FragPos, viewPos);
    
    // Apply enhanced fog for infinite depth effect
    float fogFactor = calculateAdvancedFog(FragPos, viewPos);
    result = mix(fogColor, result, fogFactor);
    
    // Enhanced tone mapping with better color preservation
    result = result / (result + vec3(1.2));
    
    // Improved gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    // Subtle color grading for warmer library atmosphere
    result.r *= 1.05; // Slightly warmer reds
    result.g *= 0.98; // Slightly reduce greens
    result.b *= 1.02; // Slightly enhance blues
    
    FragColor = vec4(result, 1.0);
}