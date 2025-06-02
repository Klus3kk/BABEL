// Replace shaders/standard.frag - Simple, clean lighting:
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

// Simple point light calculation
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 albedo) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // Simple attenuation
    float attenuation = light.intensity / (light.constant + light.linear * distance + 
                                         light.quadratic * (distance * distance));
    
    // Simple diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * albedo;
    
    return diffuse * attenuation;
}

// Simple directional light
vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    
    // Simple diffuse
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * light.intensity * albedo;
    
    return diffuse;
}

void main() {
    // Sample base color only (ignore roughness/metallic for simplicity)
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    
    // Start with ambient
    vec3 result = ambientColor * ambientStrength * albedo;
    
    // Add point lights
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, albedo);
    }
    
    // Add directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcDirLight(dirLights[i], norm, albedo);
    }
    
    // Simple tone mapping
    result = result / (result + vec3(1.0));
    
    // Standard gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}