#pragma once

#include <vector>
#include <tuple>

#include "FastNoiseLite.h"

#include "Cube.hpp"
#include "ShaderClass.hpp"

class Chunk {
public:
	Chunk(int chunkX, int chunkZ, const FastNoiseLite& noise);

	void draw(Shader* sh, glm::vec3 playerPosition, glm::vec3 playerFront);

	std::vector<std::vector<std::vector<Cube>>> cubes;
	std::vector<std::vector<uint8_t>> vertices;

	static const int chunkSize = 16;
	static const int chunkHeight = 50;

	glm::vec3 position;
	glm::vec3 center;

	std::tuple<int, Cube> getBlockAt(int x, int y, int z, int axis);
private:
	void addQuadToMesh(int x, int y, int z, int width, int height, int axis, int back, int material);
	bool compareQuads(int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1);
	void greedyMeshing(std::vector<std::vector<std::tuple<int, Cube>>>& mask, int d, int axis, int back, int chunkSizeX, int chunkSizeY, int chunkSizeZ);
	float getHeightAtPoint(const FastNoiseLite& noise, int x, int z);

	float noiseArray[chunkSize][chunkSize];

	int sides[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };

	unsigned int VBOs[6], VAOs[6];
};

class World {
public:
	World();

	void draw(Shader* sh, glm::vec3 playerPosition, glm::vec3 playerFront);

	std::vector<Chunk> chunks;
private:
	int worldSize = 25;
	unsigned int SSBO;
};

float vecAngle(glm::vec3 v1, glm::vec3 v2);
float vecLength(glm::vec3 v1);