#version 330 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 6) in vec2 in_UV; // Locația 6 pe care am setat-o în C++

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;   // Trimitem UV-ul către Fragment Shader
out float viewDistance;

uniform mat4 model; // Acum primim matricea ca uniformă, nu ca atribut
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Calculăm poziția în lumea 3D folosind matricea model (care va fi identitate)
    vec4 worldPos = model * vec4(in_Position, 1.0);
    vec4 viewPos = view * worldPos;

    gl_Position = projection * viewPos;
    
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(model))) * in_Normal; // Normală corectă
    TexCoord = in_UV;
    
    // Calculăm distanța față de cameră pentru ceață [cite: 13]
    viewDistance = length(viewPos.xyz);
}