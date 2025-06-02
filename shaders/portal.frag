#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D portalView;
uniform float time;
uniform bool portalActive = true;

// Enhanced portal effects
vec3 applyPortalEffects(vec3 portalColor, vec2 uv) {
    if (!portalActive) {
        // Inactive portal - dark mystical blue
        return vec3(0.05, 0.05, 0.15) * (0.5 + 0.5 * sin(time * 2.0));
    }
    
    // Distance from center for radial effects
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    
    // Shimmering energy effect
    float shimmer = sin(time * 4.0 + uv.x * 15.0 + uv.y * 12.0) * 0.03 + 1.0;
    shimmer += sin(time * 6.0 + distFromCenter * 20.0) * 0.02;
    portalColor *= shimmer;
    
    // Swirling distortion effect
    float swirl = time * 1.5;
    vec2 swirlUV = uv;
    swirlUV.x += sin(swirl + uv.y * 8.0) * 0.01;
    swirlUV.y += cos(swirl + uv.x * 8.0) * 0.01;
    
    // Re-sample with distorted coordinates
    vec3 distortedColor = texture(portalView, swirlUV).rgb;
    portalColor = mix(portalColor, distortedColor, 0.3);
    
    // Portal energy glow - blue/purple tint
    vec3 energyColor = vec3(0.2, 0.4, 1.0);
    float energyIntensity = 0.15 + 0.1 * sin(time * 3.0);
    portalColor += energyColor * energyIntensity;
    
    // Vignette effect - darker edges
    float vignette = 1.0 - distFromCenter * 0.8;
    vignette = pow(vignette, 1.5);
    portalColor *= vignette;
    
    // Edge glow effect
    float edgeGlow = 1.0 - smoothstep(0.3, 0.5, distFromCenter);
    edgeGlow *= sin(time * 5.0 + distFromCenter * 10.0) * 0.5 + 0.5;
    portalColor += vec3(0.4, 0.6, 1.0) * edgeGlow * 0.2;
    
    // Depth effect - make center brighter
    float depthEffect = 1.0 - distFromCenter * 0.5;
    portalColor *= depthEffect;
    
    // Magical sparkle effect
    float sparkle = sin(time * 8.0 + uv.x * 25.0) * sin(time * 12.0 + uv.y * 20.0);
    sparkle = pow(max(sparkle, 0.0), 8.0);
    portalColor += vec3(1.0, 0.8, 0.6) * sparkle * 0.1;
    
    return portalColor;
}

void main() {
    vec2 uv = TexCoord;
    
    // Sample the portal view texture
    vec3 portalColor = texture(portalView, uv).rgb;
    
    // Apply all portal effects
    portalColor = applyPortalEffects(portalColor, uv);
    
    // Ensure minimum visibility for debugging
    portalColor = max(portalColor, vec3(0.02, 0.02, 0.05));
    
    // Gamma correction for better visual quality
    portalColor = pow(portalColor, vec3(1.0/2.2));
    
    // Calculate alpha for proper blending
    vec2 center = vec2(0.5, 0.5);
    float distFromCenter = length(uv - center);
    float alpha = 1.0 - smoothstep(0.45, 0.5, distFromCenter);
    alpha *= 0.9; // Slight transparency
    
    FragColor = vec4(portalColor, alpha);
}