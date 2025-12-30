#include "World.h"
#include <math.h>
#include "glm/gtc/matrix_transform.hpp"

World::World(int size) : MapSize(size) {
    heightMap.resize(size, std::vector<float>(size));
}

// Dimensiunea standard a unui chunk în Minecraft
const int CHUNK_SIZE = 16;

void World::GenerateChunk(ChunkPos pos) {
	//daca exista chunk salvat, il incarcam
    if (savedChunks.count(pos)) {
        activeChunks[pos] = savedChunks[pos];
        return;
    }
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
// Funcție helper pentru a genera un număr pseudo-aleatoriu bazat pe coordonate
float hash(float n) {
    return glm::fract(sin(n) * 43758.5453123f);
}

// O funcție de zgomot 2D simplificată (Noise)
float noise(float x, float z) {
    float n = x + z * 57.0f;
    return hash(n);
}

float World::getNoiseHeight(int x, int z) {
    float freq = 0.02f; // Scara munților
    float amp = 10.0f;  // Înălțimea maximă

    float h = 0;
    float currentFreq = freq;
    float currentAmp = amp;

    // Aplicăm 4 octave pentru detalii
    for (int i = 0; i < 4; i++) {
        // Folosim sin/cos dar cu frecvențe care nu se aliniază perfect pentru a simula noise-ul
        h += (sin(x * currentFreq) + cos(z * currentFreq)) * currentAmp;

        currentFreq *= 2.1f; // Creștem frecvența (detalii mai fine)
        currentAmp *= 0.5f;  // Scădem amplitudinea (impact mai mic)
    }

    // Nivelul de bază al mării/terenului
    float baseHeight = 5.0f;
    return std::floor(h + baseHeight);
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
            
			// Salvăm chunk-ul înainte de a-l șterge
			savedChunks[currentPos] = it->second;
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
    float step = 0.1f; // Larger step for better performance

    for (float t = 0.0f; t < reach; t += step) {
        glm::vec3 rayPoint = cameraPos + (t * cameraFront);

        // Determinăm coordonatele întregi ale punctului de pe rază
        int ix = (int)std::floor(rayPoint.x);
        int iy = (int)std::floor(rayPoint.y);
        int iz = (int)std::floor(rayPoint.z);

        // Handle negative chunk coordinates properly
        int chunkX = ix >= 0 ? ix / 16 : (ix - 15) / 16;
        int chunkZ = iz >= 0 ? iz / 16 : (iz - 15) / 16;
        ChunkPos cPos = { chunkX, chunkZ };

        if (activeChunks.count(cPos)) {
            // VERIFICARE IARBĂ
            auto& grass = activeChunks[cPos].grassMatrices;
            for (auto it = grass.begin(); it != grass.end(); ++it) {
                glm::vec3 bPos = glm::vec3((*it)[3]);
                if ((int)std::floor(bPos.x) == ix && (int)std::floor(bPos.y) == iy && (int)std::floor(bPos.z) == iz) {
                    grass.erase(it);
                    return;
                }
            }
            // VERIFICARE PIATRĂ
            auto& stone = activeChunks[cPos].stoneMatrices;
            for (auto it = stone.begin(); it != stone.end(); ++it) {
                glm::vec3 bPos = glm::vec3((*it)[3]);
                if ((int)std::floor(bPos.x) == ix && (int)std::floor(bPos.y) == iy && (int)std::floor(bPos.z) == iz) {
                    stone.erase(it);
                    return;
                }
            }
        }
    }
}

void World::PlaceBlock(glm::vec3 cameraPos, glm::vec3 cameraFront) {
    float reach = 5.0f;
    float step = 0.1f;

    for (float t = 0.0f; t < reach; t += step) {
        glm::vec3 rayPoint = cameraPos + (t * cameraFront);

        int ix = (int)std::floor(rayPoint.x);
        int iy = (int)std::floor(rayPoint.y);
        int iz = (int)std::floor(rayPoint.z);

        if (IsBlockAt(ix, iy, iz)) {
            // Mergem înapoi pe rază exact un pas pentru a găsi aerul
            glm::vec3 placePoint = cameraPos + (t - step) * cameraFront;

            int px = (int)std::floor(placePoint.x);
            int py = (int)std::floor(placePoint.y);
            int pz = (int)std::floor(placePoint.z);

            // Verificăm să nu punem blocul peste noi înșine
            int playerX = (int)std::floor(cameraPos.x);
            int playerY = (int)std::floor(cameraPos.y);
            int playerZ = (int)std::floor(cameraPos.z);
            
            if ((px == playerX && py == playerY && pz == playerZ) ||
                (px == playerX && py == playerY - 1 && pz == playerZ)) {
                return;
            }

            // Handle negative chunk coordinates properly
            int chunkX = px >= 0 ? px / 16 : (px - 15) / 16;
            int chunkZ = pz >= 0 ? pz / 16 : (pz - 15) / 16;
            ChunkPos cPos = { chunkX, chunkZ };

            if (activeChunks.count(cPos)) {
                glm::mat4 newModel = glm::translate(glm::mat4(1.0f), glm::vec3(px, py, pz));
                activeChunks[cPos].grassMatrices.push_back(newModel);
            }
            return;
        }
    }
}

// 2. Modifică IsBlockAt să compare INTREGII, nu distanța FLOAT
bool World::IsBlockAt(int x, int y, int z) {
    // Handle negative chunk coordinates properly
    int chunkX = x >= 0 ? x / 16 : (x - 15) / 16;
    int chunkZ = z >= 0 ? z / 16 : (z - 15) / 16;
    
    ChunkPos cPos = { chunkX, chunkZ };
    
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
