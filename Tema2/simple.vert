#version 330 core
layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in mat4 instanceMatrix;

out vec3 FragPos;
out vec3 Normal;

uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * instanceMatrix * vec4(in_Position, 1.0);
    FragPos = vec3(instanceMatrix * vec4(in_Position, 1.0));
    Normal = mat3(transpose(inverse(instanceMatrix))) * in_Normal;
}