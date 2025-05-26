// shaders/portal.frag - FIXED UV detection for tiny center opening only
#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform sampler2D portalView;      
uniform sampler2D baseColorMap;    
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;

uniform vec3 viewPos;
uniform bool portalActive = true;

// Lighting uniforms
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

#define MAX_POINT_LIGHTS 8
#define MAX_DIR_LIGHTS 2

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform DirectionalLight dirLights[MAX_DIR_LIGHTS];
uniform int numPointLights;
uniform int numDirLights;
uniform vec3 ambientColor;
uniform float ambientStrength;

vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(light.position - fragPos);
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * light.intensity;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0 * (1.0 - roughness));
    vec3 specular = spec * light.color * light.intensity * metallic;
    return (diffuse + specular) * attenuation * albedo;
}

vec3 calcDirLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 albedo, float roughness, float metallic) {
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * light.color * light.intensity;
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0 * (1.0 - roughness));
    vec3 specular = spec * light.color * light.intensity * metallic;
    return (diffuse + specular) * albedo;
}

bool isInsidePortalOpening(vec2 uv) {
    // MUCH SMALLER opening - only the very center
    vec2 center = vec2(0.5, 0.5);
    float distance = length(uv - center);
    
    // TINY opening radius - adjust this value to make opening smaller/larger
    float openingRadius = 0.1;  // Very small opening
    
    return distance < openingRadius;
}

void main() {
    vec2 uv = TexCoord;
    
    // Check if we're in the TINY center opening
    if (portalActive && isInsidePortalOpening(uv)) {
        // PORTAL CENTER - Show scene view
        vec2 center = vec2(0.5, 0.5);
        vec2 portalUV = (uv - center) / 0.1 + center; // Map to full texture
        portalUV = clamp(portalUV, 0.0, 1.0);
        
        vec3 portalColor = texture(portalView, portalUV).rgb;
        FragColor = vec4(portalColor, 1.0);
        
    } else {
        // STONE FRAME - Normal stone rendering (99% of the portal model)
        vec3 albedo = texture(baseColorMap, uv).rgb;
        float roughness = texture(roughnessMap, uv).r;
        float metallic = texture(metallicMap, uv).r;
        
        vec3 norm = normalize(Normal);
        vec3 viewDir = normalize(viewPos - FragPos);
        
        // Normal lighting calculation
        vec3 result = ambientColor * ambientStrength * albedo;
        
        for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
            result += calcPointLight(pointLights[i], norm, FragPos, viewDir, albedo, roughness, metallic);
        }
        
        for(int i = 0; i < numDirLights && i < MAX_DIR_LIGHTS; i++) {
            result += calcDirLight(dirLights[i], norm, viewDir, albedo, roughness, metallic);
        }
        
        // Tone mapping and gamma correction
        result = result / (result + vec3(1.0));
        result = pow(result, vec3(1.0/2.2));
        
        FragColor = vec4(result, 1.0);
    }
}