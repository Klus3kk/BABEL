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

// Maximum number of lights
#define MAX_POINT_LIGHTS 8
#define MAX_DIR_LIGHTS 2

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight dirLights[MAX_DIR_LIGHTS];
uniform int numPointLights;
uniform int numDirLights;

uniform vec3 viewPos;

uniform sampler2D baseColorMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;

// Ambient lighting
uniform vec3 ambientColor;
uniform float ambientStrength;

// Calculate point light contribution
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(light.position - fragPos);
    
    // Distance attenuation
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * light.intensity;
    
    // Specular component (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0 * (1.0 - roughness));
    vec3 specular = spec * light.color * light.intensity * metallic;
    
    return (diffuse + specular) * attenuation * albedo;
}

// Calculate directional light contribution
vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * light.intensity;
    
    // Specular component
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0 * (1.0 - roughness));
    vec3 specular = spec * light.color * light.intensity * metallic;
    
    return (diffuse + specular) * albedo;
}

void main() {
    // Sample material textures
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    float roughness = texture(roughnessMap, TexCoord).r;
    float metallic = texture(metallicMap, TexCoord).r;
    
    // Normalize vectors
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Start with ambient lighting
    vec3 result = ambientColor * ambientStrength * albedo;
    
    // Add contribution from all point lights
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, viewDir, albedo, roughness, metallic);
    }
    
    // Add contribution from all directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcDirLight(dirLights[i], norm, viewDir, albedo, roughness, metallic);
    }
    
    // Tone mapping and gamma correction
    result = result / (result + vec3(1.0)); // Simple tone mapping
    result = pow(result, vec3(1.0/2.2)); // Gamma correction
    
    FragColor = vec4(result, 1.0);
}