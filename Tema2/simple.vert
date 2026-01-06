#version 330 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 6) in vec2 in_UV;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;
out vec4 FragPosLightSpace; // Output nou

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix; // Uniformă nouă

void main() {
    vec4 worldPos = model * vec4(in_Position, 1.0);
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * in_Normal;
    TexCoord = in_UV;
    
    // Calculăm poziția fragmentului relativ la "camera" soarelui
    FragPosLightSpace = lightSpaceMatrix * worldPos;

    gl_Position = projection * view * worldPos;
}