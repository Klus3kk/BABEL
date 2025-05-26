#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;

uniform sampler2D portalTexture;
uniform float time;
uniform vec3 viewPos;

// Portal effect parameters
uniform float portalRipple = 0.02;
uniform float portalDistortion = 0.1;
uniform vec3 portalTint = vec3(0.8, 0.9, 1.2);

void main() {
    vec2 uv = TexCoord;
    
    // Calculate distance from center for ripple effect
    vec2 center = vec2(0.5, 0.5);
    float dist = length(uv - center);
    
    // Create ripple distortion
    float ripple = sin(dist * 20.0 - time * 8.0) * portalRipple * (1.0 - dist);
    
    // Add some mystical distortion
    float distortX = sin(uv.y * 10.0 + time * 2.0) * portalDistortion * (1.0 - dist);
    float distortY = sin(uv.x * 8.0 + time * 1.5) * portalDistortion * (1.0 - dist);
    
    // Apply distortions
    uv.x += ripple + distortX;
    uv.y += ripple + distortY;
    
    // Sample the portal texture
    vec3 portalColor = texture(portalTexture, uv).rgb;
    
    // Apply mystical tinting
    portalColor *= portalTint;
    
    // Add edge glow effect
    float edgeGlow = 1.0 - smoothstep(0.3, 0.5, dist);
    portalColor += vec3(0.1, 0.2, 0.4) * edgeGlow;
    
    // Add some shimmer
    float shimmer = sin(time * 4.0 + dist * 15.0) * 0.1 + 0.9;
    portalColor *= shimmer;
    
    // Fade edges to black for portal frame effect
    float vignette = smoothstep(0.5, 0.3, dist);
    
    FragColor = vec4(portalColor * vignette, 1.0);
}