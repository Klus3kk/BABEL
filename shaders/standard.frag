// Enhanced standard.frag - Atmospheric Mystical Library Lighting
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

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
uniform vec3 ambientColor;
uniform float ambientStrength;
uniform float time;

// Enhanced atmospheric parameters
const float ATMOSPHERE_DENSITY = 0.8;
const float LIGHT_SCATTERING = 0.3;
const float MYSTICAL_GLOW = 0.4;

// Enhanced point light calculation with atmospheric effects
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // Enhanced attenuation with atmospheric scattering
    float attenuation = light.intensity / (light.constant + light.linear * distance + 
                                         light.quadratic * (distance * distance));
    
    // Add atmospheric scattering for mystical effect
    float scattering = exp(-distance * LIGHT_SCATTERING) * ATMOSPHERE_DENSITY;
    attenuation *= (1.0 + scattering);
    
    // Enhanced diffuse lighting with wrap-around for softer shadows
    float NdotL = dot(normal, lightDir);
    float diffuseWrap = max((NdotL + 0.3) / 1.3, 0.0); // Wrap lighting for mystical softness
    float diffuse = mix(max(NdotL, 0.0), diffuseWrap, 0.4);
    
    // Add distance-based ambient boost for atmospheric depth
    float ambientBoost = 1.0 / (1.0 + distance * 0.05);
    diffuse = max(diffuse, 0.15 * ambientBoost);
    
    // Enhanced specular with mystical properties
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float specularStrength = mix(0.1, 1.0, 1.0 - roughness);
    float shininess = mix(32.0, 128.0, 1.0 - roughness);
    float spec = pow(NdotH, shininess) * specularStrength;
    
    // Fresnel effect for mystical glow
    float VdotH = max(dot(viewDir, halfwayDir), 0.0);
    float fresnel = pow(1.0 - VdotH, 5.0);
    spec *= mix(spec, 1.0, fresnel * metallic);
    
    // Color contribution with atmospheric tinting
    vec3 lightColor = light.color;
    
    // Add mystical color shifting based on distance
    if (distance > 5.0) {
        float mysticalShift = sin(time * 2.0 + distance * 0.1) * 0.1;
        lightColor.b += mysticalShift * MYSTICAL_GLOW;
        lightColor.r += mysticalShift * 0.5 * MYSTICAL_GLOW;
    }
    
    // Calculate final contribution
    vec3 diffuseContrib = diffuse * lightColor * albedo;
    vec3 specularContrib = spec * lightColor * mix(vec3(0.04), albedo, metallic);
    
    return (diffuseContrib + specularContrib) * attenuation;
}

// Enhanced directional light with atmospheric perspective
vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(-light.direction);
    
    // Enhanced diffuse with atmospheric wrap
    float NdotL = dot(normal, lightDir);
    float diffuseWrap = max((NdotL + 0.4) / 1.4, 0.0);
    float diffuse = mix(max(NdotL, 0.0), diffuseWrap, 0.6); // More wrap for ambient feel
    
    // Ensure minimum lighting for atmospheric visibility
    diffuse = max(diffuse, 0.2);
    
    // Soft specular for ambient light
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0);
    float spec = pow(NdotH, mix(16.0, 64.0, 1.0 - roughness)) * (1.0 - roughness) * 0.3;
    
    // Atmospheric color contribution
    vec3 lightColor = light.color * light.intensity;
    
    vec3 diffuseContrib = diffuse * lightColor * albedo;
    vec3 specularContrib = spec * lightColor * mix(vec3(0.04), albedo, metallic);
    
    return diffuseContrib + specularContrib;
}

// Atmospheric fog calculation for depth and mystery
vec3 applyAtmosphericFog(vec3 color, vec3 fragPos, vec3 viewPos) {
    float distance = length(fragPos - viewPos);
    float fogFactor = exp(-distance * 0.08); // Subtle fog for depth
    
    // Mystical fog color that shifts with time
    vec3 fogColor = ambientColor * 2.0;
    fogColor.b += sin(time * 0.5) * 0.1; // Subtle blue pulsing
    
    return mix(fogColor, color, fogFactor);
}

// Enhanced rim lighting for mystical atmosphere
vec3 calcRimLighting(vec3 normal, vec3 viewDir, vec3 baseColor) {
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, 3.0);
    
    // Mystical rim color that pulses
    vec3 rimColor = vec3(0.3, 0.5, 1.0) * (0.8 + sin(time * 3.0) * 0.2);
    
    return rimColor * rim * 0.4;
}

void main() {
    // Sample material properties
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    float roughness = texture(roughnessMap, TexCoord).r;
    float metallic = texture(metallicMap, TexCoord).r;
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Enhanced ambient lighting with mystical properties
    vec3 ambient = ambientColor * ambientStrength * albedo;
    
    // Add mystical base glow that pulses subtly
    float mysticalPulse = 0.8 + sin(time * 1.5) * 0.2;
    ambient += albedo * 0.1 * mysticalPulse;
    
    vec3 result = ambient;
    
    // Add point lights with enhanced atmospheric effects
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, albedo, roughness, metallic);
    }
    
    // Add directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcDirLight(dirLights[i], norm, viewDir, albedo, roughness, metallic);
    }
    
    // Add mystical rim lighting
    result += calcRimLighting(norm, viewDir, albedo);
    
    // Apply atmospheric fog for depth and mystery
    result = applyAtmosphericFog(result, FragPos, viewPos);
    
    // Ensure minimum visibility - never completely black
    result = max(result, albedo * 0.05);
    
    // Enhanced tone mapping for mystical atmosphere
    // Use Reinhard tone mapping with slight color grading
    result = result / (result + vec3(1.0));
    
    // Add slight mystical color grading
    result.r = pow(result.r, 0.9);  // Slightly reduce red
    result.g = pow(result.g, 0.95); // Slightly reduce green  
    result.b = pow(result.b, 1.1);  // Enhance blue for mystical feel
    
    // Gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}