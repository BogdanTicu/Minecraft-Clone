#pragma once
#include <vector>
#include <GL/glew.h>
#include "Cube.h"
#include "glm/glm.hpp"
#include <map>

struct ChunkPos {
    int x, z;
    // Operator necesar pentru a folosi structura ca cheie in std::map
    bool operator<(const ChunkPos& other) const {
        if (x != other.x) return x < other.x;
        return z < other.z;
    }
};

struct ChunkData {
    std::vector<glm::mat4> grassMatrices;
    std::vector<glm::mat4> stoneMatrices;
};

class World {
public:
    int MapSize;
    // Stocam inaltimile intr-o matrice 2D (vector de vectori)
    std::vector<std::vector<float>> heightMap;
    std::map<ChunkPos, ChunkData> activeChunks;
	std::map<ChunkPos, ChunkData> savedChunks; //salvam chunkurile cu modificari(blocuri plasate/sparte) pt a pastra persistenta
    int RenderDistance = 4; // Cate chunk-uri vedem in jurul nostru
    World(int size);
    // Genereaza datele folosind algoritmul de noise
    void GenerateMap();
    void Update(glm::vec3 cameraPos);
    // Parcurge harta si apeleaza Cube::Draw
    void Render(GLuint programId, Cube* blockModel);
    void GenerateChunk(ChunkPos pos);
    void BreakBlock(glm::vec3 cameraPos, glm::vec3 cameraFront);
    void PlaceBlock(glm::vec3 cameraPos, glm::vec3 cameraFront);
    bool IsBlockAt(int x, int y, int z);

private:
    // Functia matematica de zgomot (noise)
    float getNoiseHeight(int x, int z);
};