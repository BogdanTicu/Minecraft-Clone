#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Camera {
public:
    // Vectori de stare
    glm::vec3 Position, Front, Up, Right, WorldUp;
    // Unghiuri Euler (echivalente cu alpha si beta din lab)
    float Yaw, Pitch;
    float MovementSpeed, MouseSensitivity;
    float lastX = 400.0f, lastY = 300.0f;
    bool firstMouse = true;

    Camera(glm::vec3 startPos = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);

    glm::mat4 GetViewMatrix();
    void ProcessKeyboard(unsigned char key, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void HandleMouseMovement(int x, int y);

private:
    void updateCameraVectors();
};