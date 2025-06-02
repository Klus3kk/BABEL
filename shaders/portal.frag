#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D portalView;
uniform float time;
uniform bool portalActive = true;

// Minimal portal effects to preserve natural colors
vec3 applySubtlePortalEffects(vec3 portalColor, vec2 uv) {
    if (!portalActive) {
        // Inactive portal - very dark
        return vec3(0.02, 0.02, 0.05);
    }
    
    // Distance from center for edge effects only
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    
    // Very subtle shimmer (barely noticeable)
    float shimmer = 1.0 + sin(time * 2.0 + uv.x * 10.0) * 0.02;
    portalColor *= shimmer;
    
    // Slight edge darkening for depth
    float vignette = 1.0 - distFromCenter * 0.3;
    vignette = pow(vignette, 0.8);
    portalColor *= vignette;
    
    // Very subtle blue energy at edges only
    float edgeGlow = smoothstep(0.4, 0.5, distFromCenter);
    edgeGlow *= sin(time * 3.0) * 0.5 + 0.5;
    portalColor += vec3(0.1, 0.2, 0.4) * edgeGlow * 0.1;
    
    return portalColor;
}

void main() {
    vec2 uv = TexCoord;
    
    // Sample the portal view texture
    vec3 portalColor = texture(portalView, uv).rgb;
    
    // Apply minimal effects to preserve natural colors
    portalColor = applySubtlePortalEffects(portalColor, uv);
    
    // NO gamma correction to preserve original colors
    // portalColor = pow(portalColor, vec3(1.0/2.2));
    
    // Full alpha for complete visibility
    float alpha = 1.0;
    
    // Only fade at extreme edges
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    alpha = 1.0 - smoothstep(0.48, 0.5, distFromCenter);
    
    FragColor = vec4(portalColor, alpha);
}