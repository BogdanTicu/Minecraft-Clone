#include "SkyManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

SkyManager::SkyManager(float duration) : worldTime(0.0f), dayDuration(duration) {
    dayColor = glm::vec3(0.5f, 0.8f, 1.0f); //[cite_start]// Albastru deschis [cite: 12]
    sunsetColor = glm::vec3(1.0f, 0.4f, 0.2f); // Portocaliu apus
    nightColor = glm::vec3(0.02f, 0.02f, 0.05f); // Negru/Albastru închis
}

glm::vec3 SkyManager::lerp(glm::vec3 a, glm::vec3 b, float t) {
    return a + t * (b - a);
}

void SkyManager::Update(float deltaTime) {
    worldTime += deltaTime;
    if (worldTime > dayDuration) worldTime -= dayDuration;

    float dayProgress = worldTime / dayDuration;
    float angle = dayProgress * 2.0f * 3.14159f;

    // Calculăm direcția luminii (Soarele se rotește pe axa XY)
    lightDirection = glm::normalize(glm::vec3(cos(angle), sin(angle), 0.2f));

    float sunHeight = lightDirection.y;

    // Calculăm culoarea cerului și intensitatea ambientală
    if (sunHeight > 0.2f) {
        currentSkyColor = dayColor;
        ambientStrength = 0.5f;
    }
    else if (sunHeight > -0.2f) {
        float t = (sunHeight + 0.2f) / 0.4f;
        currentSkyColor = lerp(sunsetColor, dayColor, t);
        ambientStrength = 0.2f + t * 0.3f;
    }
    else {
        currentSkyColor = nightColor;
        ambientStrength = 0.1f;
    }
}

void SkyManager::ApplyToShader(GLuint programId, GLint fogColorLoc, GLint fogStartLoc, GLint fogEndLoc) {
    glUniform3fv(fogColorLoc, 1, glm::value_ptr(currentSkyColor));
    glUniform3fv(glGetUniformLocation(programId, "lightDir"), 1, glm::value_ptr(lightDirection));
    glUniform1f(glGetUniformLocation(programId, "ambientStrength"), ambientStrength);

    // Putem ajusta și distanța ceții în funcție de zi/noapte dacă dorim
    glUniform1f(fogStartLoc, 64.0f);
        glUniform1f(fogEndLoc, 128.0f);
}

void SkyManager::Render(GLuint programId, Cube* cubeModel, glm::vec3 cameraPos) {
    float dayProgress = worldTime / dayDuration;
    float angle = dayProgress * 2.0f * 3.14159f;
    float distance = 400.0f;

    // Dezactivăm Depth Test pentru ca Soarele/Luna să fie fundalul infinit
    glDisable(GL_DEPTH_TEST);
    glUniform1i(glGetUniformLocation(programId, "useTexture"), 0);

    // Poziția Soarelui relativă la cameră
    glm::vec3 sunPos = cameraPos + glm::vec3(cos(angle) * distance, sin(angle) * distance, 0.0f);
    glm::mat4 model = glm::translate(glm::mat4(1.0f), sunPos);
    model = glm::scale(model, glm::vec3(20.0f));
    glUniform3f(glGetUniformLocation(programId, "blockColor"), 1.0f, 0.9f, 0.0f); // Galben
    cubeModel->Draw(programId, model);

    // Poziția Lunii (opusă Soarelui)
    glm::vec3 moonPos = cameraPos + glm::vec3(cos(angle + 3.1415f) * distance, sin(angle + 3.1415f) * distance, 0.0f);
    model = glm::translate(glm::mat4(1.0f), moonPos);
    model = glm::scale(model, glm::vec3(15.0f));
    glUniform3f(glGetUniformLocation(programId, "blockColor"), 0.8f, 0.8f, 0.9f); // Albastru deschis/Gri
    cubeModel->Draw(programId, model);

    // IMPORTANT: Reactivăm Depth Test pentru ca blocurile să se poată desena corect după
    glEnable(GL_DEPTH_TEST);
    glUniform1i(glGetUniformLocation(programId, "useTexture"), 1);
}

glm::mat4 SkyManager::GetLightSpaceMatrix(glm::vec3 cameraPos) {
    // Definim o zonă în jurul jucătorului unde se calculează umbrele
    float orthoSize = 50.0f;
    glm::mat4 lightProjection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 1.0f, 500.0f);

    // Camera soarelui „privește” spre centrul unde se află jucătorul
    // lightDirection este deja calculat în Update() bazat pe worldTime
    glm::mat4 lightView = glm::lookAt(cameraPos + lightDirection * 100.0f, cameraPos, glm::vec3(0.0f, 1.0f, 0.0f));

    return lightProjection * lightView;
}