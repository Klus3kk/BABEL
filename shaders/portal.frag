#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

// Portal texture
uniform sampler2D portalView;      // The recursive portal view

uniform vec3 viewPos;
uniform bool portalActive = true;
uniform float time = 0.0;

// Add some portal effects
vec3 applyPortalEffects(vec3 portalColor, vec2 uv) {
    if (!portalActive) return vec3(0.0, 0.0, 0.1); // Dark blue if inactive
    
    // Subtle shimmer effect
    float shimmer = sin(time * 3.0 + uv.x * 10.0 + uv.y * 8.0) * 0.02 + 1.0;
    portalColor *= shimmer;
    
    // Slight blue tint to indicate it's a portal
    portalColor.b += 0.05;
    
    // Darken edges slightly for depth
    vec2 center = vec2(0.5, 0.5);
    float edgeFade = 1.0 - length(uv - center) * 0.3;
    portalColor *= edgeFade;
    
    return portalColor;
}

void main() {
    vec2 uv = TexCoord;
    
    // Since this is a dedicated portal quad, always show the portal effect
    vec3 portalColor = texture(portalView, uv).rgb;
    
    // Apply portal effects
    portalColor = applyPortalEffects(portalColor, uv);
    
    // Ensure portal view is never completely black (for debugging)
    portalColor = max(portalColor, vec3(0.01, 0.01, 0.02));
    
    FragColor = vec4(portalColor, 1.0);
}