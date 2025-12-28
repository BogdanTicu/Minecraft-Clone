#include "World.h"
#include <math.h>
#include "glm/gtc/matrix_transform.hpp"

World::World(int size) : MapSize(size) {
    heightMap.resize(size, std::vector<float>(size));
}

// Dimensiunea standard a unui chunk în Minecraft
const int CHUNK_SIZE = 16;

void World::GenerateChunk(ChunkPos pos) {
    ChunkData newChunk;

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int globalX = pos.x * 16 + x;
            int globalZ = pos.z * 16 + z;
            float y = getNoiseHeight(globalX, globalZ);

            // Matricea pentru blocul de sus (Iarbă)
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3((float)globalX, y, (float)globalZ));
            newChunk.grassMatrices.push_back(model);

            // Matricele pentru blocurile de dedesubt (Piatră/Gri)
            for (int d = 1; d <= 2; d++) {
                glm::mat4 stoneModel = glm::translate(glm::mat4(1.0f), glm::vec3((float)globalX, y - d, (float)globalZ));
                newChunk.stoneMatrices.push_back(stoneModel);
            }
        }
    }
    activeChunks[pos] = newChunk;
}

void World::Render(GLuint programId, Cube* cubeModel) {
    GLint colorLoc = glGetUniformLocation(programId, "blockColor");

    for (auto const& [pos, data] : activeChunks) {
        // 1. Randăm iarba (Verde)
        glUniform3f(colorLoc, 0.2f, 0.8f, 0.2f);
        cubeModel->DrawInstanced(programId, data.grassMatrices);

        // 2. Randăm piatra (Gri)
        glUniform3f(colorLoc, 0.5f, 0.5f, 0.5f);
        cubeModel->DrawInstanced(programId, data.stoneMatrices);
    }
}
float World::getNoiseHeight(int x, int z) {
    // Frecvența și amplitudinea controlează cât de "deluros" este terenul
    float freq = 0.05f;
    float amp = 6.0f;

    // Pseudo-noise folosind suprapuneri de funcții trigonometrice (FBM simplificat)
    float h = sin(x * freq) * cos(z * freq);
    h += sin(x * freq * 0.5f) * 2.0f;
    h += cos(z * freq * 0.2f) * 1.5f;

    return floor(h * amp); // Rotunjim la cel mai apropiat întreg pentru aspectul de blocuri
}

void World::GenerateMap() {
    for (int x = 0; x < MapSize; x++) {
        for (int z = 0; z < MapSize; z++) {
            heightMap[x][z] = getNoiseHeight(x, z);
        }
    }
}
void World::Update(glm::vec3 cameraPos) {
    // 1. Transformăm coordonatele camerei în indici de Chunk (snap to grid)
    int playerChunkX = (int)std::floor(cameraPos.x / 16.0f);
    int playerChunkZ = (int)std::floor(cameraPos.z / 16.0f);

    // 2. ÎNCĂRCARE: Verificăm zona din jurul nostru
    // Toate chunk-urile din acest careu trebuie să fie în memorie
    for (int x = playerChunkX - RenderDistance; x <= playerChunkX + RenderDistance; x++) {
        for (int z = playerChunkZ - RenderDistance; z <= playerChunkZ + RenderDistance; z++) {
            ChunkPos pos = { x, z };
            // Dacă nu este în mapă, îl generăm acum
            if (activeChunks.find(pos) == activeChunks.end()) {
                GenerateChunk(pos);
            }
        }
    }

    // 3. DESCARCARE (UNLOAD): Curățăm ce a rămas prea departe
    // Păstrăm o marjă de 2 chunk-uri extra pentru a preveni încărcarea/ștergerea repetitivă
    for (auto it = activeChunks.begin(); it != activeChunks.end(); ) {
        ChunkPos currentPos = it->first;

        // Calculăm distanța față de jucător
        if (std::abs(currentPos.x - playerChunkX) > RenderDistance + 2 ||
            std::abs(currentPos.z - playerChunkZ) > RenderDistance + 2) {

            // Aceasta este ștergerea PE CHUNK
            it = activeChunks.erase(it);
        }
        else {
            ++it;
        }
    }
}

void World::BreakBlock(glm::vec3 cameraPos, glm::vec3 cameraFront) {
    float reach = 5.0f;
    float step = 0.02f; // Reducem pasul de la 0.1 la 0.02 pentru precizie chirurgicală

    for (float t = 0.0f; t < reach; t += step) {
        // Adăugăm un mic epsilon (0.001) în direcția razei pentru a evita erorile la marginea fețelor
        glm::vec3 rayPoint = cameraPos + (t * cameraFront);

        int ix = (int)std::floor(rayPoint.x);
        int iy = (int)std::floor(rayPoint.y);
        int iz = (int)std::floor(rayPoint.z);

        ChunkPos cPos = { (int)std::floor(ix / 16.0f), (int)std::floor(iz / 16.0f) };

        if (activeChunks.count(cPos)) {
            // Verificăm iarba
            auto& grass = activeChunks[cPos].grassMatrices;
            for (auto it = grass.begin(); it != grass.end(); ++it) {
                glm::vec3 bPos = glm::vec3((*it)[3]); // Poziția stocată în matrice

                // Comparăm direct valorile întregi, nu distanța float
                if ((int)std::floor(bPos.x) == ix &&
                    (int)std::floor(bPos.y) == iy &&
                    (int)std::floor(bPos.z) == iz) {
                    grass.erase(it);
                    return;
                }
            }
            // Repetă la fel pentru stoneMatrices...
        }
    }
}

void World::PlaceBlock(glm::vec3 cameraPos, glm::vec3 cameraFront) {
    float reach = 5.0f;
    float step = 0.01f; // Pas foarte mic pentru precizie maximă

    for (float t = 0.0f; t < reach; t += step) {
        glm::vec3 rayPoint = cameraPos + (t * cameraFront);

        int ix = (int)std::floor(rayPoint.x);
        int iy = (int)std::floor(rayPoint.y);
        int iz = (int)std::floor(rayPoint.z);

        if (IsBlockAt(ix, iy, iz)) {
            // Când lovim un bloc, ne întoarcem exact un pas înapoi pentru a găsi spațiul gol
            glm::vec3 placePoint = cameraPos + (t - step) * cameraFront;

            int px = (int)std::floor(placePoint.x);
            int py = (int)std::floor(placePoint.y);
            int pz = (int)std::floor(placePoint.z);

            ChunkPos cPos = { (int)std::floor(px / 16.0f), (int)std::floor(pz / 16.0f) };

            if (activeChunks.count(cPos)) {
                glm::mat4 newModel = glm::translate(glm::mat4(1.0f), glm::vec3(px, py, pz));
                activeChunks[cPos].grassMatrices.push_back(newModel); // Adăugăm blocul
            }
            return;
        }
    }
}

// 2. Modifică IsBlockAt să compare INTREGII, nu distanța FLOAT
bool World::IsBlockAt(int x, int y, int z) {
    ChunkPos cPos = { (int)std::floor(x / 16.0f), (int)std::floor(z / 16.0f) };
    if (activeChunks.count(cPos)) {
        // Verificăm în listele de instanțe ale chunk-ului
        for (const auto& m : activeChunks[cPos].grassMatrices) {
            glm::vec3 p = glm::vec3(m[3]); // Coloana 4 din matrice este poziția
            if ((int)std::floor(p.x) == x && (int)std::floor(p.y) == y && (int)std::floor(p.z) == z)
                return true;
        }
        for (const auto& m : activeChunks[cPos].stoneMatrices) {
            glm::vec3 p = glm::vec3(m[3]);
            if ((int)std::floor(p.x) == x && (int)std::floor(p.y) == y && (int)std::floor(p.z) == z)
                return true;
        }
    }
    return false;
}