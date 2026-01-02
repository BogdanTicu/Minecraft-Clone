#version 330 core
in vec3 FragPos;
in vec3 Normal;
in float viewDistance; // Distance from camera

out vec4 FragColor;

uniform vec3 blockColor; // Primită din World::Render
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;

void main() {
    // Calcul simplu de lumină (opțional, pentru a vedea fețele cubului)
    float ambient = 0.4;
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.3));
    float diff = max(dot(Normal, lightDir), 0.0);
    vec3 lighting = (ambient + diff) * blockColor;

    // Calcul ceață (Fog) bazat pe distanța de la cameră
    float fogFactor = clamp((fogEnd - viewDistance) / (fogEnd - fogStart), 0.0, 1.0);
    
    // Mix între culoarea blocului și culoarea ceții
    vec3 finalColor = mix(fogColor, lighting, fogFactor);
    
    FragColor = vec4(finalColor, 1.0);
}