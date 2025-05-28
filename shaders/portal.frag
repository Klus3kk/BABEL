#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D portalTexture;
uniform bool portalActive = true;

void main() {
    if (portalActive) {
        vec3 portalColor = texture(portalTexture, TexCoord).rgb;
        
        // Debug fallback
        if (length(portalColor) < 0.01) {
            portalColor = vec3(1.0, 0.0, 1.0); // Magenta for debugging
        }
        
        FragColor = vec4(portalColor, 1.0);
    } else {
        // Inactive portal - dark void
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
