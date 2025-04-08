#pragma once

#include <vector>
#include <tuple>
#include <thread>
#include <mutex>

#include "FastNoiseLite.h"

#include "Cube.hpp"
#include "ShaderClass.hpp"

class Chunk {
public:
	Chunk(int chunkX, int chunkZ, const FastNoiseLite& noise);

	void draw(Shader* sh, glm::vec3 playerPosition, glm::vec3 playerFront);

	static const uint16_t chunkSize = 16;
	static const uint16_t chunkHeight = 50;

	Cube cubes[chunkSize][chunkHeight][chunkSize];
	//Cube cubes[chunkSize * chunkHeight * chunkSize];
	std::vector<uint8_t> vertices;

	glm::vec3 position;
	glm::vec3 center;

	void passVerticesToGPU();

	Cube* getBlockAt(int x, int y, int z, int axis);
private:
	void addQuadToMesh(int x, int y, int z, int width, int height, int axis, int back, int material);
	bool compareQuads(int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1);
	void greedyMeshing(Cube* mask, int d, int axis, int back, int chunkSizeX, int chunkSizeY, int chunkSizeZ);
	float getHeightAtPoint(const FastNoiseLite& noise, int x, int z);

	float noiseArray[chunkSize + 2][chunkSize + 2];

	int sides[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

	unsigned int VBO, VAO;

	Cube airCube{};
};

class World {
public:
	World();

	void draw(Shader* sh, glm::vec3 playerPosition, glm::vec3 playerFront);

	std::vector<Chunk> chunks;

	void genSuperChunk(int superChunkX, int superChunkZ, const FastNoiseLite& noise);
private:
	static const int worldSize = 110;

	static const int numSuperChunks = 2;
	static const int superChunkSize = worldSize / numSuperChunks;
	std::thread threads[numSuperChunks * numSuperChunks];
	std::mutex chunksMutex; // Protects access to 'chunks'
};

void genSuperChunkStatic(int superChunkX, int superChunkZ, const FastNoiseLite& noise, World* thisPrime);

float vecAngle(glm::vec3 v1, glm::vec3 v2);
float vecLength(const glm::vec3 &v1);