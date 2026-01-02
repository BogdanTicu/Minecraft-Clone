#version 330 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in mat4 instanceMatrix;

out vec3 FragPos;
out vec3 Normal;
out float viewDistance; // Distance from camera to fragment

uniform mat4 view;
uniform mat4 projection;

void main() {
    vec4 worldPos = instanceMatrix * vec4(in_Position, 1.0);
    vec4 viewPos = view * worldPos;
    
    gl_Position = projection * viewPos;
    FragPos = vec3(worldPos);
    Normal = mat3(transpose(inverse(instanceMatrix))) * in_Normal;
    
    // Calculate distance from camera in view space
    viewDistance = length(viewPos.xyz);
}