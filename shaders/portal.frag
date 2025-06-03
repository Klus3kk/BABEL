#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D portalView;
uniform float time;
uniform bool portalActive = true;

// Minimal portal effect shader
vec3 applyMinimalPortalEffects(vec3 portalColor, vec2 uv) {
    if (!portalActive) {
        return vec3(0.02, 0.02, 0.05);
    }
    
    // Distance from center
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    
    // REMOVED: All brightness multiplications that were causing the issue
    // Keep original color as-is
    
    // Only apply very subtle edge darkening
    float vignette = 1.0 - distFromCenter * 0.1; // Reduced effect
    portalColor *= vignette;

    return portalColor;
}

void main() {
    vec2 uv = TexCoord;
    
    // Sample the portal view texture directly
    vec3 portalColor = texture(portalView, uv).rgb;
    
    portalColor = applyMinimalPortalEffects(portalColor, uv);
    
    // Simple alpha
    float alpha = 1.0;
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    alpha = 1.0 - smoothstep(0.48, 0.5, distFromCenter);
    
    FragColor = vec4(portalColor, alpha);
}