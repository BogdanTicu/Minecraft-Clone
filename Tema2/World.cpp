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
    float uMin, uMax, vMin, vMax;

    if (blockType == 1) { // IARBA
        if (faceDirection == 0) { // SUS - Grass Top
            uMin = 0.25f;  uMax = 0.5f;  // coloana 2
            vMin = 0.0f;    vMax = 0.25f;  // rând 1 (sus) -> inversat
        }
        else if (faceDirection == 1) { // JOS - Dirt
            uMin = 0.0f;    uMax = 0.25f;  // coloana 1
            vMin = 0.25f;  vMax = 0.5f;  // rând 2 (mijloc) -> inversat
        }
        else { // LATERAL - Grass Side
            uMin = 0.0f;    uMax = 0.25f;  // coloana 1
            vMin = 0.0f;    vMax = 0.25f;  // rând 1 (sus) -> inversat
        }
    }
    else if (blockType == 2) { // PIATRA
        uMin = 0.25f;  uMax = 0.5f;  // coloana 2
        vMin = 0.25f;  vMax = 0.5f;  // rând 2 (mijloc) -> inversat
    }
    else if (blockType == 3) { // APA
        uMin = 0.0f;    uMax = 0.25f;  // coloana 1
        vMin = 0.5f;  vMax = 0.75f;    // rând 3 (jos) -> inversat
    }
    else if (blockType == 4) { // DIRT
        uMin = 0.0f;    uMax = 0.25f;
        vMin = 0.25f;  vMax = 0.5f;
    }
    else if (blockType == 5) { // SAND
        uMin = 0.5f;    uMax = 0.75f;
        vMin = 0.0f;  vMax = 0.25f;
    }
    else if (blockType == 6) { // Copac
        if (faceDirection == 0 || faceDirection == 1) { // SUS
            uMin = 0.25f;  uMax = 0.5f;
            vMin = 0.75f;    vMax = 1.0f;
        }
        else { // LATERAL
            uMin = 0.0f;    uMax = 0.25f;
            vMin = 0.75f;    vMax = 1.0f;
        }
    }
    else if (blockType == 7) { // Frunze
        uMin = 0.5f;  uMax = 0.75f;
        vMin = 0.75f;    vMax = 1.0f;
    }


    float s = 0.5f;
    float sy = (blockType == 3) ? 0.45f : 0.5f;
    glm::vec3 v1, v2, v3, v4;
    glm::vec2 uv1, uv2, uv3, uv4;

    // Coordonatele UV trebuie mapate corect pe fiecare față
    if (faceDirection == 0) { // SUS (Y+)
        v1 = { p.x - s, p.y + sy, p.z + s };
        v2 = { p.x + s, p.y + sy, p.z + s };
        v3 = { p.x + s, p.y + sy, p.z - s };
        v4 = { p.x - s, p.y + sy, p.z - s };
    }
    else if (faceDirection == 1) { // JOS (Y-)
        v1 = { p.x - s, p.y - sy, p.z - s };
        v2 = { p.x + s, p.y - sy, p.z - s };
        v3 = { p.x + s, p.y - sy, p.z + s };
        v4 = { p.x - s, p.y - sy, p.z + s };
    }
    else if (faceDirection == 2) { // SPATE (Z-)
        v1 = { p.x + s, p.y - sy, p.z - s };
        v2 = { p.x - s, p.y - sy, p.z - s };
        v3 = { p.x - s, p.y + sy, p.z - s };
        v4 = { p.x + s, p.y + sy, p.z - s };
    }
    else if (faceDirection == 3) { // FATA (Z+)
        v1 = { p.x - s, p.y - sy, p.z + s };
        v2 = { p.x + s, p.y - sy, p.z + s };
        v3 = { p.x + s, p.y + sy, p.z + s };
        v4 = { p.x - s, p.y + sy, p.z + s };
    }
    else if (faceDirection == 4) { // STANGA (X-)
        v1 = { p.x - s, p.y - sy, p.z - s };
        v2 = { p.x - s, p.y - sy, p.z + s };
        v3 = { p.x - s, p.y + sy, p.z + s };
        v4 = { p.x - s, p.y + sy, p.z - s };
    }
    else { // DREAPTA (X+)
        v1 = { p.x + s, p.y - sy, p.z + s };
        v2 = { p.x + s, p.y - sy, p.z - s };
        v3 = { p.x + s, p.y + sy, p.z - s };
        v4 = { p.x + s, p.y + sy, p.z + s };
    }

    // UV-urile pentru cele 4 colțuri
    uv1 = { uMin, vMin };
    uv2 = { uMax, vMin };
    uv3 = { uMax, vMax };
    uv4 = { uMin, vMax };

    // Două triunghiuri (6 vertecși)
    vertices.push_back({ v1, normal, uv1 });
    vertices.push_back({ v2, normal, uv2 });
    vertices.push_back({ v3, normal, uv3 });
    vertices.push_back({ v3, normal, uv3 });
    vertices.push_back({ v4, normal, uv4 });
    vertices.push_back({ v1, normal, uv1 });
}

void World::BuildChunkMesh(ChunkPos pos) {
    ChunkData& data = activeChunks[pos];
    std::vector<WorldVertex> grassVerts;
    std::vector<WorldVertex> stoneVerts;
    std::vector<WorldVertex> waterVerts;
    std::vector<WorldVertex> dirtVerts;
    std::vector<WorldVertex> sandVerts;
    std::vector<WorldVertex> treeVerts;
    std::vector<WorldVertex> leafVerts;

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 128; y++) {
            for (int z = 0; z < 16; z++) {
                int type = data.blocks[x][y][z];

                if (type == 0) continue;

                glm::vec3 p(pos.x * 16 + x, y, pos.z * 16 + z);
                std::vector<WorldVertex>* currentList;

                if (type == 1) currentList = &grassVerts;
                else if (type == 2) currentList = &stoneVerts;
                else if (type == 3) currentList = &waterVerts;
                else if (type == 4) currentList = &dirtVerts;
                else if (type == 5) currentList = &sandVerts;
                else if (type == 6) currentList = &treeVerts;
                else if (type == 7) currentList = &leafVerts;
                else continue;

                // Face culling (verifică dacă vecinii sunt aer sau apă transparentă)
                int up = (y < 127) ? data.blocks[x][y + 1][z] : 0;
                int down = (y > 0) ? data.blocks[x][y - 1][z] : 0;
                int back = (z > 0) ? data.blocks[x][y][z - 1] : 0;
                int front = (z < 15) ? data.blocks[x][y][z + 1] : 0;
                int left = (x > 0) ? data.blocks[x - 1][y][z] : 0;
                int right = (x < 15) ? data.blocks[x + 1][y][z] : 0;

                bool shouldRenderFace[6];

                // SUS
                shouldRenderFace[0] = (up == 0 || up == 3);

                // JOS
                shouldRenderFace[1] = (down == 0);

                // SPATE
                shouldRenderFace[2] = (back == 0 || back == 3);

                // FAȚĂ
                shouldRenderFace[3] = (front == 0 || front == 3);

                // STÂNGA
                shouldRenderFace[4] = (left == 0 || left == 3);

                // DREAPTA
                shouldRenderFace[5] = (right == 0 || right == 3);

                // EXCEPȚIE: apă peste apă → nu randa
                if (type == 3) {
                    shouldRenderFace[0] &= (up != 3);
                    shouldRenderFace[2] &= (back != 3);
                    shouldRenderFace[3] &= (front != 3);
                    shouldRenderFace[4] &= (left != 3);
                    shouldRenderFace[5] &= (right != 3);
                }

                // ⬇️ Pentru APĂ, folosește mereu normala în sus
                if (type == 3) {
                    glm::vec3 waterNormal = { 0, 1, 0 };  // ⬅️ Mereu în sus!
                    if (shouldRenderFace[0]) {
                        AddFaceToMesh(*currentList, p, { 0, 1, 0 }, 3, 0);
                    }
                }
                else {
                    // Pentru IARBĂ și PIATRĂ, normale normale
                    if (shouldRenderFace[0]) AddFaceToMesh(*currentList, p, { 0, 1, 0 }, type, 0);
                    if (shouldRenderFace[1]) AddFaceToMesh(*currentList, p, { 0, -1, 0 }, type, 1);
                    if (shouldRenderFace[2]) AddFaceToMesh(*currentList, p, { 0, 0, -1 }, type, 2);
                    if (shouldRenderFace[3]) AddFaceToMesh(*currentList, p, { 0, 0, 1 }, type, 3);
                    if (shouldRenderFace[4]) AddFaceToMesh(*currentList, p, { -1, 0, 0 }, type, 4);
                    if (shouldRenderFace[5]) AddFaceToMesh(*currentList, p, { 1, 0, 0 }, type, 5);
                }
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
    upload(data.waterVAO, data.waterVBO, data.waterVertexCount, waterVerts);
    upload(data.dirtVAO, data.dirtVBO, data.dirtVertexCount, dirtVerts);
    upload(data.sandVAO, data.sandVBO, data.sandVertexCount, sandVerts);
    upload(data.treeVAO, data.treeVBO, data.treeVertexCount, treeVerts);
    upload(data.leafVAO, data.leafVBO, data.leafVertexCount, leafVerts);
}


void World::GenerateChunk(ChunkPos pos)
{
    if (savedChunks.count(pos))
    {
        activeChunks[pos] = savedChunks[pos];
        return;
    }

    ChunkData newChunk;
    memset(newChunk.blocks, 0, sizeof(newChunk.blocks));

    const int SEA_LEVEL = 12; // nivelul apei

    for (int x = 0; x < 16; x++) {
        for (int z = 0; z < 16; z++) {
            int globalX = pos.x * 16 + x;
            int globalZ = pos.z * 16 + z;

            int surfaceY = (int)getNoiseHeight(globalX, globalZ);

            // Optional: zgomot pentru plaje/nisip
            float sandNoise = (noiseGenerator.GetNoise(globalX * 2.0f, globalZ * 2.0f) + 1.0f) * 0.5f;

            for (int y = 0; y < 128; y++)
            {
                if (y < surfaceY - 1) {
                    newChunk.blocks[x][y][z] = 2; // STONE
                }
                else if (y == surfaceY - 1) {
                    newChunk.blocks[x][y][z] = 4; // DIRT de sub stratul de sus
                }
                else if (y == surfaceY) {
                    // Dacă nivelul este sub SEA_LEVEL → DIRT la fundul depresiunii
                    if (y <= SEA_LEVEL) {
                        if (sandNoise > 0.6f) {
                            newChunk.blocks[x][y][z] = 5; // SAND pe mal
                        }
                        else {
                            if (y < SEA_LEVEL)
                                newChunk.blocks[x][y][z] = 4;
                            else
                                newChunk.blocks[x][y][z] = 1; // DIRT sub apă
                        }
                    }
                    else {
                        newChunk.blocks[x][y][z] = 1; // GRASS normal pe uscat
                    }
                }
                else if (y <= SEA_LEVEL) {
                    // Doar dacă suntem sub nivelul mării și deasupra terenului
                    if (y > surfaceY) {
                        newChunk.blocks[x][y][z] = 3; // WATER
                    }
                }
                else {
                    newChunk.blocks[x][y][z] = 0; // AIR
                }
            }
        }
    }

    for (int x = 2; x < 14; x++) {  // ⬅️ Margine mai mare pentru frunze
        for (int z = 2; z < 14; z++) {
            int globalX = pos.x * 16 + x;
            int globalZ = pos.z * 16 + z;

            int surfaceY = (int)getNoiseHeight(globalX, globalZ);

            // Verifică dacă e teren valid
            if (surfaceY <= SEA_LEVEL) continue;
            if (newChunk.blocks[x][surfaceY][z] != 1) continue;

            // Noise pentru copaci
            float treeNoise = noiseGenerator.GetNoise((float)globalX * 0.5f, (float)globalZ * 0.5f);
            treeNoise = (treeNoise + 1.0f) * 0.5f;

            // Hash pentru randomizare
            int hash = (globalX * 374761393 + globalZ * 668265263) % 1000;
            float randomFactor = hash / 1000.0f;

            // Plasare copac
            if (treeNoise > 0.6f && randomFactor > 0.9f) {
                int trunkHeight = 4 + (int)(randomFactor * 3);

                // TRUNCHI
                for (int h = 1; h <= trunkHeight; h++) {
                    if (surfaceY + h < 128) {
                        newChunk.blocks[x][surfaceY + h][z] = 6;
                    }
                }

                // FRUNZE
                int topY = surfaceY + trunkHeight;

                for (int ly = -1; ly <= 2; ly++) {
                    int radius = (ly == -1 || ly == 2) ? 1 : 2;

                    for (int lx = -radius; lx <= radius; lx++) {
                        for (int lz = -radius; lz <= radius; lz++) {
                            int ax = x + lx;
                            int az = z + lz;
                            int ay = topY + ly;

                            if (ax < 0 || ax >= 16 || az < 0 || az >= 16) continue;
                            if (ay < 0 || ay >= 128) continue;

                            // Evită colțuri
                            if (abs(lx) == radius && abs(lz) == radius) {
                                if (ly != 0) continue;
                            }

                            // Nu suprascrie trunchiul
                            if (newChunk.blocks[ax][ay][az] == 0) {
                                newChunk.blocks[ax][ay][az] = 7;
                            }
                        }
                    }
                }
            }
        }
    }

    activeChunks[pos] = newChunk;
    BuildChunkMesh(pos);

}


void World::Render(GLuint programId, Cube* blockModel, glm::vec3 camPos)
{
    glUseProgram(programId);
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(glGetUniformLocation(programId, "useTexture"), 1);

    glm::mat4 identity = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(programId, "model"), 1, GL_FALSE, &identity[0][0]);
    glUniform1i(glGetUniformLocation(programId, "instanced"), 0);

    GLint colorLoc = glGetUniformLocation(programId, "blockColor");

    glUniform3fv(glGetUniformLocation(programId, "cameraPos"), 1, &camPos[0]);


    glDisable(GL_BLEND);

    // Randează blocuri opace (iarbă, piatră)
    for (auto& [pos, data] : activeChunks) {
        if (data.grassVertexCount > 0) {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.grassVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.grassVertexCount);
        }
        if (data.stoneVertexCount > 0) {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.stoneVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.stoneVertexCount);
        }
        if (data.dirtVertexCount > 0) {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.dirtVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.dirtVertexCount);
        }
        if (data.sandVertexCount > 0) {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.sandVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.sandVertexCount);
        }
        if (data.treeVertexCount > 0)
        {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.treeVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.treeVertexCount);
        }
        if (data.leafVertexCount > 0)
        {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.leafVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.leafVertexCount);
        }
    }

    // Randează apa cu transparență (după blocurile opace)
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);  // Nu scrie în depth buffer pentru transparență

    for (auto& [pos, data] : activeChunks) {
        if (data.waterVertexCount > 0) {
            glUniform3f(colorLoc, 1.0f, 1.0f, 1.0f);
            glBindVertexArray(data.waterVAO);
            glDrawArrays(GL_TRIANGLES, 0, data.waterVertexCount);
        }
    }

    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
}


float World::getNoiseHeight(int x, int z) {
    // 1. Selectorul pentru biome
    float selector = noiseGenerator.GetNoise((float)x * 0.5f, (float)z * 0.5f);
    selector = (selector + 1.0f) * 0.5f;

    // 2. Detaliul de teren pentru sus și jos
    float detailNoise = noiseGenerator.GetNoise((float)x * 2.0f, (float)z * 2.0f);
    detailNoise = (detailNoise + 1.0f) * 0.5f;

    // 3. Componentă pentru depresiunile mici
    float pitNoise = noiseGenerator.GetNoise((float)x * 1.0f, (float)z * 1.0f);
    pitNoise = (pitNoise + 1.0f) * 0.5f; // 0..1

    float baseLevel = 15.0f; // nivelul de referință pentru câmpie
    float finalHeight = 0.0f;

    // 4. Blending pentru câmpie / munți
    if (selector < 0.4f) {
        // Câmpie cu mici variații + depresiunile pentru apă
        finalHeight = baseLevel + detailNoise * 3.0f - pitNoise * 8.0f;
        // pitNoise poate scădea nivelul până la ~7
    }
    else if (selector > 0.6f) {
        // Munte, cum era înainte
        finalHeight = baseLevel + detailNoise * 40.0f;
    }
    else {
        // Tranziție câmpie -> munte
        float t = (selector - 0.4f) / 0.2f;
        float plainsY = baseLevel + detailNoise * 3.0f - pitNoise * 8.0f;
        float mountainY = baseLevel + detailNoise * 40.0f;
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

void World::FlowWaterAt(int x, int y, int z) {
    // Limite
    if (y < 0 || y >= 128) return;

    if (IsBlockAt(x, y, z)) return; // Nu suprascrie bloc solid

    // Dacă deja e apă, nu facem nimic
    int chunkX = x >= 0 ? x / 16 : (x - 15) / 16;
    int chunkZ = z >= 0 ? z / 16 : (z - 15) / 16;
    ChunkPos cPos = { chunkX, chunkZ };
    if (!activeChunks.count(cPos)) return;

    int lx = x - chunkX * 16;
    int lz = z - chunkZ * 16;

    // Setăm blocul pe WATER
    activeChunks[cPos].blocks[lx][y][lz] = 3;
    BuildChunkMesh(cPos);


    // Propagăm apă doar în jos și lateral (nu în sus pentru simplitate)
    FlowWaterAt(x, y - 1, z); // jos
    FlowWaterAt(x + 1, y, z);
    FlowWaterAt(x - 1, y, z);
    FlowWaterAt(x, y, z + 1);
    FlowWaterAt(x, y, z - 1);
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

                // după ce setezi blocul spart pe AIR
                bool hasWaterNeighbor = GetBlockTypeAt(ix + 1, iy, iz) == 3 ||
                    GetBlockTypeAt(ix - 1, iy, iz) == 3 ||
                    GetBlockTypeAt(ix, iy, iz + 1) == 3 ||
                    GetBlockTypeAt(ix, iy, iz - 1) == 3 ||
                    GetBlockTypeAt(ix, iy - 1, iz) == 3;

                if (hasWaterNeighbor) {
                    FlowWaterAt(ix, iy, iz); // folosește coordonate globale
                }

                return; // Am spart un bloc, ieșim
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

int World::GetBlockTypeAt(int x, int y, int z) {
    int chunkX = x >= 0 ? x / 16 : (x - 15) / 16;
    int chunkZ = z >= 0 ? z / 16 : (z - 15) / 16;
    ChunkPos cPos = { chunkX, chunkZ };

    if (activeChunks.count(cPos)) {
        int lx = x - (chunkX * 16);
        int lz = z - (chunkZ * 16);
        if (y >= 0 && y < 128)
            return activeChunks[cPos].blocks[lx][y][lz];
    }
    return 0; // aer
}
