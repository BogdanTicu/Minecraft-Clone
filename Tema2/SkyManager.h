#pragma once

#include <GL/glew.h>
#include "glm/glm.hpp"
#include "Cube.h"

class SkyManager {
private:
    float worldTime;
    float dayDuration;

    // Culori pentru diferite momente ale zilei
    glm::vec3 dayColor;
    glm::vec3 sunsetColor;
    glm::vec3 nightColor;

    // Starea curentă calculată la fiecare Update
    glm::vec3 currentSkyColor;
    glm::vec3 lightDirection;
    float ambientStrength;

    glm::vec3 lerp(glm::vec3 a, glm::vec3 b, float t);

public:
    SkyManager(float duration = 60.0f);

    void Update(float deltaTime);
    void ApplyToShader(GLuint programId, GLint fogColorLoc, GLint fogStartLoc, GLint fogEndLoc);
    void Render(GLuint programId, Cube* cubeModel, glm::vec3 cameraPos);
    glm::mat4 GetLightSpaceMatrix(glm::vec3 cameraPos);
    glm::vec3 GetSkyColor() const { return currentSkyColor; }
};