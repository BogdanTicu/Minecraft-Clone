#include "Cube.h"
#include "glm/gtc/type_ptr.hpp"

Cube::Cube() {
    // Format: Pozitie (x,y,z), Normale (nx,ny,nz), Coordonate UV (u,v)
    float vertices[] = {
        // Pozitii             // Normale           // UV (Atlas mapping)

        // Fata Spate (Oz-) - Pamant
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f, 0.5f,
         0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.5f, 0.5f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.5f, 1.0f,
         0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.5f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,   0.0f, 0.5f,

        // Fata Fata (Oz+) - Pamant
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.0f, 0.5f,
         0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.5f, 0.5f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.5f, 1.0f,
         0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.5f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,   0.0f, 0.5f,

        // Fata Stanga (Ox-) - Pamant
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,   0.5f, 1.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,   0.0f, 0.5f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,   0.0f, 0.5f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,   0.5f, 0.5f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,   0.5f, 1.0f,

        // Fata Dreapta (Ox+) - Pamant
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.5f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.5f, 0.5f,
         0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,   0.5f, 0.5f,
         0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 0.5f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,   0.0f, 1.0f,

         // Fata Jos (Oy-) - Pamant (fundul blocului)
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.0f, 0.5f,
          0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.5f, 0.5f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   0.5f, 0.0f,
          0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   0.5f, 0.0f,
         -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,   0.0f, 0.0f,
         -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,   0.0f, 0.5f,

         // Fata Sus (Oy+) - Iarba (partea de sus)
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   0.5f, 0.5f,
          0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 0.5f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
          0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   1.0f, 1.0f,
         -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,   0.5f, 1.0f,
         -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,   0.5f, 0.5f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &instanceVBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Stride-ul este acum de 8 float-uri (3 pos + 3 normale + 2 UV)
    GLsizei stride = 8 * sizeof(float);

    // Atributul 0: Pozitie
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    // Atributul 1: Normale
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Atributul 6: Coordonate UV (Atlas)
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(6);
}

void Cube::Draw(GLuint programId, glm::mat4 model) {
    glUniformMatrix4fv(glGetUniformLocation(programId, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
}

void Cube::DrawInstanced(GLuint programId, const std::vector<glm::mat4>& instanceMatrices) {
    if (instanceMatrices.empty()) return;

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
    glBufferData(GL_ARRAY_BUFFER, instanceMatrices.size() * sizeof(glm::mat4), &instanceMatrices[0], GL_DYNAMIC_DRAW);

    // Matricea de instantiere (locatiile 2, 3, 4, 5)
    for (int i = 0; i < 4; i++) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4) * i));
        glVertexAttribDivisor(2 + i, 1);
    }

    glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)instanceMatrices.size());
}