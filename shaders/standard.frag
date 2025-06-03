// shaders/standard.frag - Enhanced lighting for consistent wall surfaces
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

// Enhanced point light calculation with better falloff
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 albedo) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    
    // Improved attenuation with minimum light guarantee
    float attenuation = light.intensity / (light.constant + light.linear * distance + 
                                         light.quadratic * (distance * distance));
    
    // Enhanced diffuse with minimum lighting for walls
    float diff = max(dot(normal, lightDir), 0.2); // Minimum 0.2 prevents complete darkness
    
    // Add distance-based minimum lighting for walls
    float minLighting = 0.3 * (1.0 / (1.0 + distance * 0.1));
    diff = max(diff, minLighting);
    
    vec3 diffuse = diff * light.color * albedo;
    
    return diffuse * attenuation;
}

// Enhanced directional light with better coverage
vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 albedo) {
    vec3 lightDir = normalize(-light.direction);
    
    // Enhanced diffuse with minimum lighting
    float diff = max(dot(normal, lightDir), 0.3); // Higher minimum for better wall visibility
    vec3 diffuse = diff * light.color * light.intensity * albedo;
    
    return diffuse;
}

void main() {
    // Sample base color
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    vec3 norm = normalize(Normal);
    
    // Enhanced ambient lighting
    vec3 result = ambientColor * ambientStrength * albedo;
    
    // Add enhanced minimum ambient for walls specifically
    result += albedo * 0.15; // Additional base lighting
    
    // Add point lights with enhanced calculation
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += calcPointLight(pointLights[i], norm, FragPos, albedo);
    }
    
    // Add directional lights
    for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
        result += calcDirLight(dirLights[i], norm, albedo);
    }
    
    // Ensure minimum brightness for all surfaces
    result = max(result, albedo * 0.2);
    
    // Improved tone mapping
    result = result / (result + vec3(1.0));
    
    // Standard gamma correction
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}