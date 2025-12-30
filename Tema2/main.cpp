#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Camera.h"
#include "Cube.h"
#include "loadShaders.h"
#include "World.h"
#include "SOIL.h"
// --- VARIABILE GLOBALE ---
Camera camera(glm::vec3(0.0f, 20.0f, 5.0f)); // Pornim de sus pentru a vedea relieful
Cube* myCube;
GLuint programId;
float deltaTime = 0.0f, lastFrame = 0.0f;
World gameWorld(10); // Render distance (raza de chunk-uri in jurul camerei)
GLuint crosshairVAO, crosshairVBO;
// Locatii variabile uniforme
GLint viewLoc, projLoc, fogColorLoc, fogStartLoc, fogEndLoc;

// --- INITIALIZARE ---
void Initialize(void) {
    // Seteaza culoarea de fond a ecranului (Cer albastru deschis)
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);

    // Incarca shaderele
    programId = LoadShaders("simple.vert", "simple.frag");
    glUseProgram(programId);

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
    // Generate a circle with a specific radius and number of segments
    const int numSegments = 32; // More segments = smoother circle
    const float radius = 0.02f; // Circle radius
    
    std::vector<float> crosshairVertices;
    
    for (int i = 0; i <= numSegments; i++) {
        float angle = 2.0f * 3.14159265f * i / numSegments;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        
        crosshairVertices.push_back(x);
        crosshairVertices.push_back(y);
        crosshairVertices.push_back(0.0f);
    }

    glGenVertexArrays(1, &crosshairVAO);
    glGenBuffers(1, &crosshairVBO);

    glBindVertexArray(crosshairVAO);
    glBindBuffer(GL_ARRAY_BUFFER, crosshairVBO);
    glBufferData(GL_ARRAY_BUFFER, crosshairVertices.size() * sizeof(float), crosshairVertices.data(), GL_STATIC_DRAW);

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
    glDrawArrays(GL_LINE_LOOP, 0, 33); // 32 segments + 1 to close the circle

    // Reactivăm adâncimea pentru restul scenei în cadrul următor
    glEnable(GL_DEPTH_TEST);
}
// --- RENDER ---
void RenderFunction(void) {
    // Calcul DeltaTime pentru miscare fluida indiferent de FPS
    float currentFrame = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // 1. ACTUALIZARE LUME (INFINIT)
    // Verifica pozitia camerei si genereaza chunk-uri noi/sterge pe cele vechi
    gameWorld.Update(camera.Position);

    // 2. PREGATIRE ECRAN
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(programId);
    glUniform1i(glGetUniformLocation(programId, "textureAtlas"), 0);

    // 3. MATRICE VIZUALIZARE SI PROIECTIE
    // Matricea de perspectiva cu un zFar mare pentru a vedea orizontul
    float width = (float)glutGet(GLUT_WINDOW_WIDTH);
    float height = (float)glutGet(GLUT_WINDOW_HEIGHT);

    // Folosim aspect ratio-ul real pentru proiecție
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), width / height, 0.1f, 1000.0f);
    glm::mat4 view = camera.GetViewMatrix();

    glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);

    // 4. RANDARE LUME INSTANTIATA
    // World::Render apeleaza Cube::DrawInstanced pentru eficienta maxima
    gameWorld.Render(programId, myCube);

	//Randam Crosshair-ul
    RenderCrosshair();
    glutSwapBuffers();
    glutPostRedisplay(); // Fortam rularea continua a randarii
}

// În Initialize()
void LoadTexture(const char* path) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Parametri de texturare (repetare si filtrare)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // Important: NEAREST pastreaza aspectul pixelat de Minecraft
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height;
    unsigned char* image = SOIL_load_image(path, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
}

// --- CALLBACKS ---
void KeyboardDown(unsigned char key, int x, int y) {
    camera.ProcessKeyboard(key, deltaTime);
    if (key == 27) exit(0); // Iesire pe ESC
}

void mouse_callback(int x, int y) {
    // Obținem centrul actual al ferestrei
    int centerX = glutGet(GLUT_WINDOW_WIDTH) / 2;
    int centerY = glutGet(GLUT_WINDOW_HEIGHT) / 2;

    camera.HandleMouseMovement(x, y);

    // Forțăm recentrarea exact pe centrul dinamic
    if (x < 100 || x > glutGet(GLUT_WINDOW_WIDTH) - 100 ||
        y < 100 || y > glutGet(GLUT_WINDOW_HEIGHT) - 100) {
        glutWarpPointer(centerX, centerY);
        camera.lastX = (float)centerX;
        camera.lastY = (float)centerY;
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