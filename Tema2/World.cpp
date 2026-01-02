#include "World.h"
#include <math.h>
#include "glm/gtc/matrix_transform.hpp"
#include "FastNoiseLite.h"
World::World(int size) : MapSize(size) {
    heightMap.resize(size, std::vector<float>(size));
	// Initializam generatorul de noise
	noiseGenerator.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_OpenSimplex2);
    noiseGenerator.SetSeed(1234);
    noiseGenerator.SetFractalType(FastNoiseLite::FractalType_FBm);
    noiseGenerator.SetFractalOctaves(4);      // Mai multe octave = mai multe detalii fine
    noiseGenerator.SetFractalLacunarity(2.0f);
    noiseGenerator.SetFractalGain(0.5f);      // Determină cât de "ascuțite" sunt detaliile

}

// Dimensiunea standard a unui chunk în Minecraft
const int CHUNK_SIZE = 16;

void World::UpdateNeighborChunks(ChunkPos pos, int lx, int ly, int lz) {
    // Dacă blocul e pe marginea stângă, reconstruim chunk-ul din stânga
    if (lx == 0) BuildChunkMesh({ pos.x - 1, pos.z });
    if (lx == 15) BuildChunkMesh({ pos.x + 1, pos.z });
    if (lz == 0) BuildChunkMesh({ pos.x, pos.z - 1 });
    if (lz == 15) BuildChunkMesh({ pos.x, pos.z + 1 });
}
void World::AddFaceToMesh(std::vector<WorldVertex>& vertices, glm::vec3 p, glm::vec3 normal, int blockType, int faceDirection) {
    // Coordonate UV preluate din Cube.cpp
    float uMin = 0.0f, uMax = 0.5f, vMin = 0.5f, vMax = 1.0f; // Implicit: Pamant (Side)

    if (blockType == 1) { // IARBA
        if (faceDirection == 0) { // SUS
            uMin = 0.5f; uMax = 1.0f; vMin = 0.5f; vMax = 1.0f;
        }
        else if (faceDirection == 1) { // JOS
            uMin = 0.0f; uMax = 0.5f; vMin = 0.0f; vMax = 0.5f;
        }
    }
    else if (blockType == 2) { // PIATRA (Daca ai atlas, ajustezi aici)
        uMin = 0.5f; uMax = 1.0f; vMin = 0.0f; vMax = 0.5f;
    }

    // Offset-uri pentru colțurile feței (0.5f ca în Cube.cpp)
    float s = 0.5f;
    glm::vec3 v1, v2, v3, v4;

    // Determinăm poziția celor 4 colțuri în funcție de orientare
    if (faceDirection == 0) { // SUS (Y+)
        v1 = { p.x - s, p.y + s, p.z - s }; v2 = { p.x + s, p.y + s, p.z - s };
        v3 = { p.x + s, p.y + s, p.z + s }; v4 = { p.x - s, p.y + s, p.z + s };
    }
    else if (faceDirection == 1) { // JOS (Y-)
        v1 = { p.x - s, p.y - s, p.z - s }; v2 = { p.x + s, p.y - s, p.z - s };
        v3 = { p.x + s, p.y - s, p.z + s }; v4 = { p.x - s, p.y - s, p.z + s };
    }
    else if (faceDirection == 2) { // SPATE (Z-)
        v1 = { p.x - s, p.y - s, p.z - s }; v2 = { p.x + s, p.y - s, p.z - s };
        v3 = { p.x + s, p.y + s, p.z - s }; v4 = { p.x - s, p.y + s, p.z - s };
    }
    else if (faceDirection == 3) { // FATA (Z+)
        v1 = { p.x - s, p.y - s, p.z + s }; v2 = { p.x + s, p.y - s, p.z + s };
        v3 = { p.x + s, p.y + s, p.z + s }; v4 = { p.x - s, p.y + s, p.z + s };
    }
    else if (faceDirection == 4) { // STANGA (X-)
        v1 = { p.x - s, p.y - s, p.z + s }; v2 = { p.x - s, p.y + s, p.z + s };
        v3 = { p.x - s, p.y + s, p.z - s }; v4 = { p.x - s, p.y - s, p.z - s };
    }
    else { // DREAPTA (X+)
        v1 = { p.x + s, p.y - s, p.z + s }; v2 = { p.x + s, p.y + s, p.z + s };
        v3 = { p.x + s, p.y + s, p.z - s }; v4 = { p.x + s, p.y - s, p.z - s };
    }

    // Adăugăm cele două triunghiuri (6 vertecși)
    vertices.push_back({ v1, normal, {uMin, vMin} });
    vertices.push_back({ v2, normal, {uMax, vMin} });
    vertices.push_back({ v3, normal, {uMax, vMax} });
    vertices.push_back({ v3, normal, {uMax, vMax} });
    vertices.push_back({ v4, normal, {uMin, vMax} });
    vertices.push_back({ v1, normal, {uMin, vMin} });
}

void World::BuildChunkMesh(ChunkPos pos) {
    ChunkData& data = activeChunks[pos];
    std::vector<WorldVertex> grassVerts;
    std::vector<WorldVertex> stoneVerts;

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 128; y++) {
            for (int z = 0; z < 16; z++) {
                int type = data.blocks[x][y][z];
                if (type == 0) continue;

                glm::vec3 p(pos.x * 16 + x, y, pos.z * 16 + z);
                // Alegem lista în care punem vertecșii
                std::vector<WorldVertex>& currentList = (type == 1) ? grassVerts : stoneVerts;

                // Verificăm fețele (Face Culling)
                if (y == 127 || data.blocks[x][y + 1][z] == 0) AddFaceToMesh(currentList, p, { 0, 1, 0 }, type, 0);
                if (y == 0 || data.blocks[x][y - 1][z] == 0)   AddFaceToMesh(currentList, p, { 0, -1, 0 }, type, 1);
                if (z == 0 || data.blocks[x][y][z - 1] == 0)   AddFaceToMesh(currentList, p, { 0, 0, -1 }, type, 2);
                if (z == 15 || data.blocks[x][y][z + 1] == 0)  AddFaceToMesh(currentList, p, { 0, 0, 1 }, type, 3);
                if (x == 0 || data.blocks[x - 1][y][z] == 0)   AddFaceToMesh(currentList, p, { -1, 0, 0 }, type, 4);
                if (x == 15 || data.blocks[x + 1][y][z] == 0)  AddFaceToMesh(currentList, p, { 1, 0, 0 }, type, 5);
            }
        }
    }

    // Funcție utilitară pentru a încărca un vector într-un VAO/VBO
    auto upload = [](GLuint& vao, GLuint& vbo, int& count, std::vector<WorldVertex>& verts) {
        if (verts.empty()) { count = 0; return; }
        if (vao == 0) { glGenVertexArrays(1, &vao); glGenBuffers(1, &vbo); }
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(WorldVertex), verts.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(WorldVertex), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(WorldVertex), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(6);
        count = (int)verts.size();
        };

    upload(data.grassVAO, data.grassVBO, data.grassVertexCount, grassVerts);
    upload(data.stoneVAO, data.stoneVBO, data.stoneVertexCount, stoneVerts);
}
void World::GenerateChunk(ChunkPos pos) {
    if (savedChunks.count(pos)) {
        activeChunks[pos] = savedChunks[pos];
        return;
    }

    ChunkData newChunk;
    memset(newChunk.blocks, 0, sizeof(newChunk.blocks)); // Umplem cu AIR

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int globalX = pos.x * 16 + x;
            int globalZ = pos.z * 16 + z;
            float surfaceY = getNoiseHeight(globalX, globalZ);

            for (int y = 0; y < 128; y++) {
                if (y == (int)surfaceY)
                    newChunk.blocks[x][y][z] = 1; // GRASS
                else if (y < surfaceY && y > surfaceY - 7)
                    newChunk.blocks[x][y][z] = 2; // STONE
            }
        }
    }
    activeChunks[pos] = newChunk;
    BuildChunkMesh(pos); // Generăm geometria imediat
}

void World::Render(GLuint programId, Cube* cubeModel) {
    GLint colorLoc = glGetUniformLocation(programId, "blockColor");
    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(programId, "model"), 1, GL_FALSE, &identity[0][0]);
    glUseProgram(programId);

    for (auto& [pos, data] : activeChunks) {
        // Randăm iarba
        if (data.grassVertexCount > 0) {
            glUniform3f(colorLoc, 0.2f, 0.8f, 0.2f);
            glBindVertexArray(data.grassVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.grassVertexCount);
        }
        // Randăm piatra
        if (data.stoneVertexCount > 0) {
            glUniform3f(colorLoc, 0.5f, 0.5f, 0.5f);
            glBindVertexArray(data.stoneVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.stoneVertexCount);
        }
    }
}


float World::getNoiseHeight(int x, int z) {
    // 1. Selectorul - frecvență foarte mică (0.005) pentru a crea zone mari de biomi
    float selector = noiseGenerator.GetNoise((float)x * 0.5f, (float)z * 0.5f);
    // Notă: FastNoiseLite returnează -1 la 1. Îl mapăm la 0 - 1 pentru ușurință.
    selector = (selector + 1.0f) * 0.5f;

    // 2. Detaliul de teren (frecvență medie)
    float detailNoise = noiseGenerator.GetNoise((float)x * 2.0f, (float)z * 2.0f);
    detailNoise = (detailNoise + 1.0f) * 0.5f;

    float finalHeight = 0.0f;
    float baseLevel = 15.0f;

    // 3. Logica de Blending (Interpolare)
    if (selector < 0.4f) {
        // Câmpie: variație minimă
        finalHeight = baseLevel + detailNoise * 3.0f;
    }
    else if (selector > 0.6f) {
        // Munți: variație mare
        finalHeight = baseLevel + detailNoise * 40.0f;
    }
    else {
        // Zona de tranziție (între 0.4 și 0.6): facem o trecere lină
        // Calculăm cât de mult "munte" să punem
        float t = (selector - 0.4f) / 0.2f; // Rezultă o valoare între 0 și 1
        float plainsY = baseLevel + detailNoise * 3.0f;
        float mountainY = baseLevel + detailNoise * 40.0f;

        // Interpolare liniară (Lerp)
        finalHeight = plainsY + t * (mountainY - plainsY);
    }

    return std::floor(finalHeight);
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
    float step = 0.1f;

    for (float t = 0.0f; t < reach; t += step) {
        glm::vec3 rayPoint = cameraPos + (t * cameraFront);

        int ix = (int)std::floor(rayPoint.x);
        int iy = (int)std::floor(rayPoint.y);
        int iz = (int)std::floor(rayPoint.z);

        // Calculăm chunk-ul și coordonatele locale
        int chunkX = ix >= 0 ? ix / 16 : (ix - 15) / 16;
        int chunkZ = iz >= 0 ? iz / 16 : (iz - 15) / 16;
        ChunkPos cPos = { chunkX, chunkZ };

        if (activeChunks.count(cPos)) {
            int lx = ix - (chunkX * 16);
            int lz = iz - (chunkZ * 16);

            // Verificăm dacă am lovit un bloc care nu e AER
            if (iy >= 0 && iy < 128 && activeChunks[cPos].blocks[lx][iy][lz] != 0) {
                // 1. Modificăm datele
                activeChunks[cPos].blocks[lx][iy][lz] = 0; // Setăm pe AIR

                // 2. Re-generăm mesh-ul vizual pentru acest chunk
                BuildChunkMesh(cPos);

                // 3. Verificăm dacă trebuie să actualizăm și vecinii (dacă blocul e pe margine)
                UpdateNeighborChunks(cPos, lx, iy, lz);

                return; // Am spart blocul, ieșim
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

        // Dacă am găsit un bloc solid, punem noul bloc în poziția anterioară (AER)
        if (IsBlockAt(ix, iy, iz)) {
            glm::vec3 placePoint = cameraPos + (t - step) * cameraFront;

            int px = (int)std::floor(placePoint.x);
            int py = (int)std::floor(placePoint.y);
            int pz = (int)std::floor(placePoint.z);

            // Verificăm să nu punem blocul peste noi
            if (py == (int)std::floor(cameraPos.y) && px == (int)std::floor(cameraPos.x) && pz == (int)std::floor(cameraPos.z)) return;

            int chunkX = px >= 0 ? px / 16 : (px - 15) / 16;
            int chunkZ = pz >= 0 ? pz / 16 : (pz - 15) / 16;
            ChunkPos cPos = { chunkX, chunkZ };

            if (activeChunks.count(cPos)) {
                int lx = px - (chunkX * 16);
                int lz = pz - (chunkZ * 16);

                activeChunks[cPos].blocks[lx][py][lz] = 1; // Punem IARBĂ (1)
                BuildChunkMesh(cPos);
                UpdateNeighborChunks(cPos, lx, py, lz);
                return;
            }
        }
    }
}


bool World::IsBlockAt(int x, int y, int z) {
    int chunkX = x >= 0 ? x / 16 : (x - 15) / 16;
    int chunkZ = z >= 0 ? z / 16 : (z - 15) / 16;
    ChunkPos cPos = { chunkX, chunkZ };

    if (activeChunks.count(cPos)) {
        int lx = x - (chunkX * 16);
        int lz = z - (chunkZ * 16);
        if (y >= 0 && y < 128)
            return activeChunks[cPos].blocks[lx][y][lz] != 0;
    }
    return false;
}
