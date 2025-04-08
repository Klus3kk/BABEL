#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform vec3 lightPos;
uniform vec3 viewPos;

uniform sampler2D baseColorMap;
uniform sampler2D normalMap;
uniform sampler2D roughnessMap;
uniform sampler2D metallicMap;

void main() {
    vec3 albedo = texture(baseColorMap, TexCoord).rgb;
    float roughness = texture(roughnessMap, TexCoord).r;
    float metallic = texture(metallicMap, TexCoord).r;

    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 viewDir = normalize(viewPos - FragPos);

    // Basic diffuse + specular lighting
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(norm, halfwayDir), 0.0), 64.0 * (1.0 - roughness));

    vec3 lighting = (0.3 + diff + spec * metallic) * albedo;
    FragColor = vec4(lighting, 1.0);
}
