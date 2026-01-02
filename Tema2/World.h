#pragma once
#include <vector>
#include <GL/glew.h>
#include "Cube.h"
#include "glm/glm.hpp"
#include <map>
#include "FastNoiseLite.h"
struct ChunkPos {
    int x, z;
    // Operator necesar pentru a folosi structura ca cheie in std::map
    bool operator<(const ChunkPos& other) const {
        if (x != other.x) return x < other.x;
        return z < other.z;
    }
};

// În World.h
struct WorldVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

struct ChunkData {
    unsigned char blocks[16][128][16];

    // Două perechi de VAO/VBO pentru a păstra culorile separate
    GLuint grassVAO = 0, grassVBO = 0;
    int grassVertexCount = 0;

    GLuint stoneVAO = 0, stoneVBO = 0;
    int stoneVertexCount = 0;

    void Cleanup() {
        if (grassVAO != 0) { glDeleteVertexArrays(1, &grassVAO); glDeleteBuffers(1, &grassVBO); }
        if (stoneVAO != 0) { glDeleteVertexArrays(1, &stoneVAO); glDeleteBuffers(1, &stoneVBO); }
    }
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
    void AddFaceToMesh(std::vector<WorldVertex>& vertices, glm::vec3 p, glm::vec3 normal, int blockType, int faceDirection);
    void BuildChunkMesh(ChunkPos pos);
	void UpdateNeighborChunks(ChunkPos pos, int lx, int ly, int lz);
	FastNoiseLite noiseGenerator;
};