#version 330 core
// light objects use same vertex processing
layout (location = 0) in vec3 aPos; // vertex 
layout (location = 1) in vec3 aNormal; // normal
layout (location = 2) in vec2 aTexCoord; // UV coordinates 

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model; // local -> world 
uniform mat4 view; // world -> view (camera)
uniform mat4 projection; // view -> clip (pespective)

void main() {
    // Identical transformation to standard.vert
    // Light objects are rendered as normal objects that receive lighting
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;
    gl_Position = projection * view * vec4(FragPos, 1.0);
}
