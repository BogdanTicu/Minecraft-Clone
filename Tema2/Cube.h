#pragma once
#include <GL/glew.h>
#include "glm/glm.hpp"
#include <vector>

class Cube {
public:
    GLuint VAO, VBO, instanceVBO;
    Cube();
    void Draw(GLuint programId, glm::mat4 model);
    void DrawInstanced(GLuint programId, const std::vector<glm::mat4>& instanceMatrices);
    void Cleanup();
};