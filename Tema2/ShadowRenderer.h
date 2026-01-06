#pragma once
#include <GL/glew.h>
#include <glm/glm.hpp>

class ShadowRenderer {
private:
    GLuint depthMapFBO;
    GLuint depthMap;
    const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048; // Rezoluția umbrelor

public:
    ShadowRenderer();
    void BindForWriting();
    void UnbindWriting(int windowWidth, int windowHeight);
    GLuint GetShadowMap() const { return depthMap; }
    void Cleanup();
};
