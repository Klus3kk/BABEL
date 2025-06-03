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

#define MAX_POINT_LIGHTS 32

uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform vec3 viewPos;
uniform sampler2D baseColorMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;
uniform vec3 ambientColor;
uniform float ambientStrength;
uniform float time;

void main() {
    // Sample textures
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    float roughness = texture(roughnessMap, TexCoord).r;
    float metallic = texture(metallicMap, TexCoord).r;
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Simple warm ambient
    vec3 result = ambientColor * ambientStrength * albedo;
    
    // Add point lights
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos);
        float distance = length(pointLights[i].position - FragPos);
        
        // Simple attenuation
        float attenuation = pointLights[i].intensity / (pointLights[i].constant + 
            pointLights[i].linear * distance + pointLights[i].quadratic * distance * distance);
        
        // Simple diffuse
        float diff = max(dot(norm, lightDir), 0.0);
        
        // Simple specular
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0), mix(32.0, 128.0, 1.0 - roughness));
        spec *= (1.0 - roughness) * 0.5;
        
        // Add light contribution
        vec3 lightContrib = (diff * albedo + spec) * pointLights[i].color * attenuation;
        result += lightContrib;
    }
    
    // Simple warm tint
    result *= vec3(1.1, 0.95, 0.8);
    
    // Simple tone mapping and gamma
    result = result / (result + vec3(1.0));
    result = pow(result, vec3(1.0/2.2));
    
    FragColor = vec4(result, 1.0);
}