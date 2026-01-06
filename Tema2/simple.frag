#version 330 core
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
in vec4 FragPosLightSpace;

out vec4 FragColor;

uniform sampler2D textureAtlas;
uniform sampler2D shadowMap; // Textura de adâncime
uniform int useTexture;
uniform vec3 blockColor;
uniform vec3 fogColor;
uniform float fogStart;
uniform float fogEnd;
uniform vec3 cameraPos;
uniform vec3 lightDir;
uniform float ambientStrength;

float ShadowCalculation(vec4 fragPosLightSpace) {
    // Transformare în coordonate [0,1]
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;
    
    if(projCoords.z > 1.0) return 0.0;

    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    float currentDepth = projCoords.z;

    // Bias pentru a preveni "shadow acne"
    float bias = max(0.005 * (1.0 - dot(Normal, lightDir)), 0.0005);
    
    return currentDepth - bias > closestDepth ? 0.5 : 0.0;
}

void main() {
    vec4 texColor = (useTexture == 1) ? texture(textureAtlas, TexCoord) : vec4(blockColor, 1.0);
    if (texColor.a < 0.1) discard;

    vec3 baseColor = texColor.rgb;
    bool isWater = (baseColor.b > 0.7 && baseColor.r < 0.3 && baseColor.g < 0.5);
    
    vec3 finalColor;
    float alpha = texColor.a;

    if (isWater) {
        finalColor = baseColor;
        alpha = 0.6;
    } else {
        vec3 norm = normalize(Normal);
        vec3 effectiveLightDir = (lightDir.y > 0) ? normalize(lightDir) : normalize(-lightDir);
        float diff = max(dot(norm, effectiveLightDir), 0.0);
        
        // Calculăm factorul de umbră (0.0 = lumină, 0.5 = umbră)
        float shadow = ShadowCalculation(FragPosLightSpace);
        if (lightDir.y < 0) diff *= 0.2; 

        // Umbra afectează doar componenta difuză
        vec3 lighting = (ambientStrength + (1.0 - shadow) * diff) * baseColor;

        float dist = distance(FragPos, cameraPos);
        float fogFactor = clamp((fogEnd - dist) / (fogEnd - fogStart), 0.0, 1.0);
        finalColor = mix(fogColor, lighting, fogFactor);
    }

    FragColor = vec4(finalColor, alpha);
}