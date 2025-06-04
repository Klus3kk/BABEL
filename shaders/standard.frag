#version 330 core

out vec4 FragColor;
// Final output color written to framebuffer

in vec3 FragPos;    // Fragment position in world space (from vertex shader)
in vec3 Normal;     // Fragment normal vector (interpolated)
in vec2 TexCoord;   // Texture coordinates for sampling material maps

// Struct defining a physically accurate point light
struct PointLight {
    vec3 position;   // Light's world-space position
    vec3 color;      // RGB color of the light
    float intensity; // Light power multiplier
    float constant;  // Attenuation: constant term
    float linear;    // Attenuation: linear term
    float quadratic; // Attenuation: quadratic term (realistic falloff)
};

#define MAX_POINT_LIGHTS 16 // Max number of lights supported in shader (compile-time)

// Scene-wide uniforms
uniform PointLight pointLights[MAX_POINT_LIGHTS]; // Array of all active point lights
uniform int numPointLights;                       // Actual count of active lights
uniform vec3 viewPos;                             // Camera position in world space
uniform sampler2D baseColorMap;                   // Albedo (diffuse) texture
uniform sampler2D roughnessMap;                   // Roughness texture (R channel)
uniform sampler2D metallicMap;                    // Metallic texture (R channel)
uniform vec3 ambientColor;                        // Ambient light color
uniform float ambientStrength;                    // Ambient light intensity
uniform float time;                               // Time uniform (for future animation use)

void main() {
    // Sample PBR material properties from textures
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;            // Surface base color
    float roughness = texture(roughnessMap, TexCoord).r;          // Roughness controls highlight sharpness
    float metallic = texture(metallicMap, TexCoord).r;            // (Not used directly here)

    vec3 norm = normalize(Normal);                                // Ensure normal is unit length
    vec3 viewDir = normalize(viewPos - FragPos);                  // Direction to the camera (for specular reflection)

    // Start with ambient lighting contribution (soft fill light)
    vec3 result = ambientColor * ambientStrength * albedo;

    // Loop through all point lights and accumulate their contributions
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        vec3 lightDir = normalize(pointLights[i].position - FragPos); // Direction from fragment to light
        float distance = length(pointLights[i].position - FragPos);   // Distance to light

        // Attenuation factor: energy falloff over distance
        float attenuation = pointLights[i].intensity / (
            pointLights[i].constant +
            pointLights[i].linear * distance +
            pointLights[i].quadratic * distance * distance);

        // Diffuse term (Lambertian reflection): how much the surface faces the light
        float diff = max(dot(norm, lightDir), 0.0);

        // Specular term (Blinn-Phong with roughness-based shininess)
        vec3 halfwayDir = normalize(lightDir + viewDir); // Halfway vector between light and view
        float spec = pow(max(dot(norm, halfwayDir), 0.0), 
                         mix(32.0, 128.0, 1.0 - roughness)); // Shininess depends on surface smoothness
        spec *= (1.0 - roughness) * 0.5; // Weaken specular on rough surfaces

        // Combine diffuse and specular lighting, scale by light color and attenuation
        vec3 lightContrib = (diff * albedo + spec) * pointLights[i].color * attenuation;
        result += lightContrib; // Accumulate into total result
    }

    // Apply a warm color tint (artistic tone)
    result *= vec3(1.1, 0.95, 0.8); // Slight red/orange boost for firelight look

    // Apply simple tone mapping (compress HDR to LDR)
    result = result / (result + vec3(1.0));

    // Gamma correction (convert from linear to sRGB for display)
    result = pow(result, vec3(1.0 / 2.2));

    // Output final color with full opacity
    FragColor = vec4(result, 1.0);
}
