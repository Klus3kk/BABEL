// Warm standard.frag - Atmospheric lighting with warm tones
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

// Warm atmospheric parameters
const float ATMOSPHERE_DENSITY = 0.7f;
const float LIGHT_SCATTERING = 0.15f;
const vec3 WARM_TINT = vec3(1.1f, 0.95f, 0.8f); // Golden warm tint

// Warm point light calculation
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // Enhanced attenuation with warm atmospheric scattering
    float attenuation = light.intensity / (light.constant + light.linear * distance + 
                                         light.quadratic * (distance * distance));
    
    // Warm atmospheric scattering
    float scattering = exp(-distance * LIGHT_SCATTERING) * ATMOSPHERE_DENSITY;
    attenuation *= (1.0f + scattering * 0.6f);
    
    // Enhanced diffuse lighting with warm wrap
    float NdotL = dot(normal, lightDir);
    float diffuseWrap = max((NdotL + 0.3f) / 1.3f, 0.0f);
    float diffuse = mix(max(NdotL, 0.0f), diffuseWrap, 0.4f);
    
    // Warm ambient boost for cozy atmosphere
    float ambientBoost = 1.0f / (1.0f + distance * 0.08f);
    diffuse = max(diffuse, 0.12f * ambientBoost);
    
    // Warm specular calculation
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0f);
    float specularStrength = mix(0.1f, 0.9f, 1.0f - roughness);
    float shininess = mix(20.0f, 120.0f, 1.0f - roughness);
    float spec = pow(NdotH, shininess) * specularStrength;
    
    // Warm Fresnel effect
    float VdotH = max(dot(viewDir, halfwayDir), 0.0f);
    float fresnel = pow(1.0f - VdotH, 4.0f);
    spec *= mix(spec, 1.0f, fresnel * metallic * 0.6f);
    
    // Apply warm tint to light color
    vec3 lightColor = light.color * WARM_TINT;
    
    // Add distance-based warm color shifting
    if (distance > 3.0f) {
        float warmShift = min(distance / 15.0f, 0.3f);
        lightColor.r += warmShift * 0.2f; // More red at distance
        lightColor.g += warmShift * 0.1f; // Slight green
        lightColor.b -= warmShift * 0.1f; // Less blue for warmth
    }
    
    // Calculate final contribution with warm bias
    vec3 diffuseContrib = diffuse * lightColor * albedo;
    vec3 specularContrib = spec * lightColor * mix(vec3(0.06f), albedo, metallic);
    
    return (diffuseContrib + specularContrib) * attenuation;
}

// Warm directional light calculation
vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(-light.direction);
    
    float NdotL = dot(normal, lightDir);
    float diffuseWrap = max((NdotL + 0.4f) / 1.4f, 0.0f);
    float diffuse = mix(max(NdotL, 0.0f), diffuseWrap, 0.6f);
    diffuse = max(diffuse, 0.18f); // Higher minimum for warmth
    
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float NdotH = max(dot(normal, halfwayDir), 0.0f);
    float spec = pow(NdotH, mix(18.0f, 68.0f, 1.0f - roughness)) * (1.0f - roughness) * 0.25f;
    
    // Apply warm tint to directional light
    vec3 lightColor = light.color * light.intensity * WARM_TINT;
    
    vec3 diffuseContrib = diffuse * lightColor * albedo;
    vec3 specularContrib = spec * lightColor * mix(vec3(0.06f), albedo, metallic);
    
    return diffuseContrib + specularContrib;
}

// Warm atmospheric fog
vec3 applyWarmAtmosphericFog(vec3 color, vec3 fragPos, vec3 viewPos) {
    float distance = length(fragPos - viewPos);
    float fogFactor = exp(-distance * 0.04f);
    
    // Warm golden fog color
    vec3 fogColor = ambientColor * 2.0f * WARM_TINT;
    fogColor += vec3(0.15f, 0.1f, 0.05f); // Add golden warmth
    
    return mix(fogColor, color, fogFactor);
}

// Warm rim lighting
vec3 calcWarmRimLighting(vec3 normal, vec3 viewDir, vec3 baseColor) {
    float rim = 1.0f - max(dot(viewDir, normal), 0.0f);
    rim = pow(rim, 2.0f);
    
    // Warm rim color with golden tint
    vec3 rimColor = vec3(0.8f, 0.6f, 0.3f); // Golden rim
    
    return rimColor * rim * 0.25f;
}

void main() {
    // Sample material properties
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    float roughness = texture(roughnessMap, TexCoord).r;
    float metallic = texture(metallicMap, TexCoord).r;
    
    // Apply warm tint to albedo for overall warmth
    albedo *= WARM_TINT;
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Warm ambient lighting
    vec3 ambient = ambientColor * ambientStrength * albedo * WARM_TINT;
    
    // Subtle warm pulsing for magical atmosphere
    float warmPulse = 1.0f + sin(time * 0.2f) * 0.04f;
    ambient *= warmPulse;
    
    vec3 result = ambient;
    
    // Add point lights with warm enhancement
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, albedo, roughness, metallic);
    }
    
    // Add directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcDirLight(dirLights[i], norm, viewDir, albedo, roughness, metallic);
    }
    
    // Add warm rim lighting
    result += calcWarmRimLighting(norm, viewDir, albedo);
    
    // Apply warm atmospheric fog
    result = applyWarmAtmosphericFog(result, FragPos, viewPos);
    
    // Ensure minimum warm visibility
    result = max(result, albedo * 0.04f);
    
    // Enhanced warm tone mapping
    result = result / (result + vec3(0.8f)); // Slightly softer tone mapping
    
    // Warm color grading
    result.r = pow(result.r, 0.95f); // Enhance red slightly
    result.g = pow(result.g, 0.98f); // Keep green natural
    result.b = pow(result.b, 1.05f); // Reduce blue slightly for warmth
    
    // Gamma correction
    result = pow(result, vec3(1.0f/2.2f));
    
    FragColor = vec4(result, 1.0f);
}