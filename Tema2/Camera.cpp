#include "Camera.h"

Camera::Camera(glm::vec3 startPos, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(30.5f), MouseSensitivity(0.1f) {
    Position = startPos;
    WorldUp = up;
    Yaw = yaw;
    Pitch = pitch;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() {
    // Folosim lookAt la fel ca in laboratoarele 10 si 11
    return glm::lookAt(Position, Position + Front, Up);
}

void Camera::ProcessKeyboard(unsigned char key, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    if (key == 'w' || key == 'W') Position += Front * velocity;
    if (key == 's' || key == 'S') Position -= Front * velocity;
    if (key == 'a' || key == 'A') Position -= Right * velocity;
    if (key == 'd' || key == 'D') Position += Right * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // Limitare pentru a nu da peste cap camera (ca in lab 08_03)
    if (constrainPitch) {
        if (Pitch > 89.0f) Pitch = 89.0f;
        if (Pitch < -89.0f) Pitch = -89.0f;
    }
    updateCameraVectors();
}

void Camera::updateCameraVectors() {
    // Calculam noul vector Front folosind unghiurile Euler
    glm::vec3 front;
    front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    front.y = sin(glm::radians(Pitch));
    front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    Front = glm::normalize(front);

    // Recalculam vectorii Right si Up
    Right = glm::normalize(glm::cross(Front, WorldUp));
    Up = glm::normalize(glm::cross(Right, Front));
}

void Camera::HandleMouseMovement(int x, int y) {
    if (firstMouse) {
        lastX = (float)x;
        lastY = (float)y;
        firstMouse = false;
    }

    float xoffset = (float)x - lastX;
    float yoffset = lastY - (float)y; // Inversat pentru coordonatele ecranului

    lastX = (float)x;
    lastY = (float)y;

    // Apelăm metoda de procesare a unghiurilor pe care o avem deja
    this->ProcessMouseMovement(xoffset, yoffset);
}