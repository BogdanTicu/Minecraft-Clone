#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera.h"
#include "Cube.h"
#include "loadShaders.h"
#include "World.h"
#include "SOIL.h"
#include "SkyManager.h"
#include "ShadowRenderer.h"
// --- VARIABILE GLOBALE ---
Camera camera(glm::vec3(0.0f, 20.0f, 5.0f)); // Pornim de sus pentru a vedea relieful
Cube* myCube;
GLuint programId;
float deltaTime = 0.0f, lastFrame = 0.0f;
World gameWorld(10); // Render distance (raza de chunk-uri in jurul camerei)
GLuint crosshairVAO, crosshairVBO;
// Locatii variabile uniforme
GLint viewLoc, projLoc, fogColorLoc, fogStartLoc, fogEndLoc;


GLuint textureAtlas[3];  // Array de 3 texturi pentru animație
int currentWaterFrame = 0;
float waterAnimationTimer = 0.0f;
const float WATER_FRAME_DURATION = 0.3f; // 300ms per frame

SkyManager* skyManager = new SkyManager(120.0f); // O zi durează 120 de secunde

ShadowRenderer* shadowRenderer;
GLuint shadowProgram;
void LoadTextures() {
    const char* atlasFiles[3] = {
        "atlas1.png",
        "atlas2.png",
        "atlas3.png"
    };

    for (int i = 0; i < 3; i++) {
        glGenTextures(1, &textureAtlas[i]);
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, textureAtlas[i]);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        int width, height, channels;
        unsigned char* image = SOIL_load_image(atlasFiles[i], &width, &height, &channels, SOIL_LOAD_RGBA);
        if (image) {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
            SOIL_free_image_data(image);
            printf("Textura %s incarcata: %dx%d\n", atlasFiles[i], width, height);
        }
        else {
            printf("Eroare la incarcarea texturii: %s\n", atlasFiles[i]);
        }
    }
}

// --- INITIALIZARE ---
void Initialize(void) {

    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    programId = LoadShaders("simple.vert", "simple.frag");
    shadowProgram = LoadShaders("shadow.vert", "shadow.frag");
    glUseProgram(programId);

    skyManager = new SkyManager(120.0f);
    shadowRenderer = new ShadowRenderer(); // Acesta face apeluri GL
    LoadTextures();  // Încarcă toate cele 3 atlas-uri
    glUniform1i(glGetUniformLocation(programId, "textureAtlas"), 0);
    glUniform1i(glGetUniformLocation(programId, "useTexture"), 1);


    // Initializare obiecte geometrice
    myCube = new Cube();

    // Activare setari 3D si performanta
    glEnable(GL_DEPTH_TEST);  // Testul de adancime pentru randare 3D corecta
    glEnable(GL_CULL_FACE);   // Nu randeaza fetele cuburilor care nu se vad (interiorul)
    glCullFace(GL_BACK);
    glDisable(GL_CULL_FACE);


    // Preluare locatii uniforme din shader
    viewLoc = glGetUniformLocation(programId, "view");
    projLoc = glGetUniformLocation(programId, "projection");
    fogColorLoc = glGetUniformLocation(programId, "fogColor");
    fogStartLoc = glGetUniformLocation(programId, "fogStart");
    fogEndLoc = glGetUniformLocation(programId, "fogEnd");

    // Configurare initiala Ceata (Fog)
    glUniform3f(fogColorLoc, 0.5f, 0.8f, 1.0f); // Aceeasi culoare cu glClearColor
    glUniform1f(fogStartLoc, 64.0f);
    glUniform1f(fogEndLoc, 128.0f);

    // Prima generare a lumii in jurul camerei
    gameWorld.Update(camera.Position);
}

void InitCrosshair() {
    // Două linii: una orizontală și una verticală care se intersectează la (0,0)
    float crosshairVertices[] = {
        // Poziții (x, y, z)
        -0.03f,  0.0f, 0.0f,   0.03f, 0.0f, 0.0f, // Linia orizontală
         0.0f, -0.04f, 0.0f,   0.0f,  0.04f, 0.0f  // Linia verticală
    };

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshairVertices), crosshairVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}
void RenderCrosshair() {
    glUseProgram(programId); // Folosim același shader

    // Dezactivăm adâncimea pentru ca vizorul să fie mereu deasupra blocurilor
    glDisable(GL_DEPTH_TEST);

    // Resetăm matricile la identitate pentru a randa în coordonate 2D de ecran (-1 la 1)
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &identity[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &identity[0][0]);

    // Matricea model trebuie să fie și ea identitate (vizorul stă la 0,0,0)
    // Dacă shaderul tău folosește instanceMatrix, va trebui să trimiți o matrice identitate și acolo
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(programId, "instanceMatrix"), 1, GL_FALSE, &model[0][0]);

    // Setăm o culoare contrastantă (ex: Alb sau Negru)
    glUniform3f(glGetUniformLocation(programId, "blockColor"), 1.0f, 1.0f, 1.0f);

    glBindVertexArray(crosshairVAO);
    glLineWidth(2.0f); // Facem liniile puțin mai groase
    glDrawArrays(GL_LINES, 0, 4);

    // Reactivăm adâncimea pentru restul scenei în cadrul următor
    glEnable(GL_DEPTH_TEST);
}

void RenderFunction(void) {
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // 1. Update logica
    waterAnimationTimer += deltaTime;
    if (waterAnimationTimer >= WATER_FRAME_DURATION) {
        waterAnimationTimer = 0.0f;
        currentWaterFrame = (currentWaterFrame + 1) % 3;
    }
    skyManager->Update(deltaTime);
    gameWorld.Update(camera.Position);

    // Calculăm matricea luminii o singură dată pe cadru
    glm::mat4 lightSpaceMatrix = skyManager->GetLightSpaceMatrix(camera.Position);

    // --- PASS 1: RANDARE UMBRE (Depth Map) ---
    glUseProgram(shadowProgram);
    glUniformMatrix4fv(glGetUniformLocation(shadowProgram, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    shadowRenderer->BindForWriting();
    // Dezactivăm culling-ul pentru a evita „holes” în umbrele obiectelor subțiri
    glDisable(GL_CULL_FACE);
    gameWorld.Render(shadowProgram, myCube, camera.Position);
    glEnable(GL_CULL_FACE);
    shadowRenderer->UnbindWriting(800, 600);

    // --- PASS 2: RANDARE FINALĂ ---
    glm::vec3 skyColor = skyManager->GetSkyColor();
    glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(programId);

    // Matricele camerei
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    // Trimitere date umbre către shader
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowRenderer->GetShadowMap());
    glUniform1i(glGetUniformLocation(programId, "shadowMap"), 1);
    glUniformMatrix4fv(glGetUniformLocation(programId, "lightSpaceMatrix"), 1, GL_FALSE, &lightSpaceMatrix[0][0]);

    // Randare cer (fundal)
    skyManager->Render(programId, myCube, camera.Position);

    // Randare lume cu texturi
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureAtlas[currentWaterFrame]);
    glUniform1i(glGetUniformLocation(programId, "textureAtlas"), 0);

    skyManager->ApplyToShader(programId, fogColorLoc, fogStartLoc, fogEndLoc);
    glUniform3fv(glGetUniformLocation(programId, "cameraPos"), 1, &camera.Position[0]);

    gameWorld.Render(programId, myCube, camera.Position);

    RenderCrosshair();

    glutSwapBuffers();
    glutPostRedisplay();
}
// --- CALLBACKS ---
void KeyboardDown(unsigned char key, int x, int y) {
    camera.ProcessKeyboard(key, deltaTime);
    if (key == 27) exit(0); // Iesire pe ESC
}

void mouse_callback(int x, int y) {
    // Transmite coordonatele mousepad-ului catre clasa Camera
    camera.HandleMouseMovement(x, y);

    // Recentrare cursor (Warp) pentru control de tip FPS
    if (x < 100 || x > 700 || y < 100 || y > 500) {
        glutWarpPointer(400, 300);
        camera.lastX = 400;
        camera.lastY = 300;
    }
}

void MouseClick(int button, int state, int x, int y) {
    if (state == GLUT_DOWN) {
        if (button == GLUT_LEFT_BUTTON) {
            // Click stanga -> Sparge
            gameWorld.BreakBlock(camera.Position, camera.Front);
        }
        else if (button == GLUT_RIGHT_BUTTON) {
            // Click dreapta -> Pune (Logica similara cu Break, dar adauga la rayPoint - 0.1f)
            gameWorld.PlaceBlock(camera.Position, camera.Front);
        }
    }
}


void Cleanup(void) {
    delete myCube;
    glDeleteProgram(programId);
}



int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Minecraft Clone");

    glewInit();

    Initialize();
    InitCrosshair();
    glutDisplayFunc(RenderFunction);
    glutKeyboardFunc(KeyboardDown);
    glutPassiveMotionFunc(mouse_callback);
    glutCloseFunc(Cleanup);

    glutSetCursor(GLUT_CURSOR_NONE); // Ascundem cursorul ca intr-un joc real
    glutMouseFunc(MouseClick); // Inregistreaza functia de click
    glutMainLoop();
    return 0;
}