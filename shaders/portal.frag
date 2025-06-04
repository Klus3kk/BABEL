#version 330 core
out vec4 FragColor;

in vec2 TexCoord;  // UV coordinates from vertex shader

uniform sampler2D portalView;      // Texture containing the rendered portal view
uniform float time;                // Current time for animations
uniform bool portalActive = true;  // Whether portal should render


void main() {
    vec2 uv = TexCoord;
    
    // Sample the portal view texture - this is what was rendered from the destination portal's perspective
    vec3 portalColor = texture(portalView, uv).rgb;
    
    // Create circular portal shape with alpha 
    float alpha = 1.0;
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    
    // Smooth circular falloff - creates clean portal edge
    alpha = 1.0 - smoothstep(0.48, 0.5, distFromCenter);
    
    FragColor = vec4(portalColor, alpha);
}