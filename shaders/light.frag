// Atmospheric light.frag - Light sources that match the dark library aesthetic
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
uniform vec3 ambientColor;
uniform float ambientStrength;
uniform float time;

void main() {
    // Sample the base texture - this is the actual material (metal, stone, etc.)
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // TREAT THESE AS REGULAR OBJECTS - same as walls, columns, etc.
    
    // Warm ambient lighting (same as other objects)
    vec3 result = ambientColor * ambientStrength * albedo;
    
    // Receive lighting from all light sources (same calculation as standard objects)
    for(int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        vec3 lightPos = pointLights[i].position;
        vec3 lightColor = pointLights[i].color;
        float lightIntensity = pointLights[i].intensity;
        
        vec3 lightDir = normalize(lightPos - FragPos);
        float distance = length(lightPos - FragPos);
        
        // Standard attenuation
        float attenuation = lightIntensity / (1.0f + 0.09f * distance + 0.032f * (distance * distance));
        
        // Standard diffuse lighting
        float diff = max(dot(norm, lightDir), 0.0f);
        
        // Enhanced diffuse for better lighting response
        float diffuseWrap = max((diff + 0.3f) / 1.3f, 0.0f);
        float finalDiff = mix(diff, diffuseWrap, 0.4f);
        
        // Add this light's contribution (same as standard shader)
        result += finalDiff * lightColor * albedo * attenuation;
        
        // Add subtle specular for metal surfaces
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(norm, halfwayDir), 0.0f), 32.0f) * 0.3f;
        result += spec * lightColor * attenuation;
    }
    
    // Apply warm atmospheric tinting (same as other objects)
    result *= vec3(1.1f, 0.95f, 0.8f);
    
    // Ensure minimum visibility in dark areas
    result = max(result, albedo * 0.04f);
    
    // Standard tone mapping
    result = result / (result + vec3(1.0f));
    
    // Gamma correction
    result = pow(result, vec3(1.0f/2.2f));
    
    FragColor = vec4(result, 1.0f);
}