#pragma once

#include <vector>
#include <tuple>

#include "FastNoiseLite.h"

#include "Cube.hpp"
#include "ShaderClass.hpp"

class Chunk {
public:
	Chunk(int chunkX, int chunkZ, FastNoiseLite noise);

	void draw(Shader sh, glm::vec3 playerPosition, glm::vec3 playerFront);

	std::vector<std::vector<std::vector<Cube>>> cubes;
	std::vector<float> vertices;

	static const int chunkSize = 10;
	static const int chunkHeight = 20;
private:
	std::tuple<int, Cube> getBlockAt(int x, int y, int z, int axis);
	void addQuadToMesh(int x, int y, int z, int width, int height, int axis, glm::vec2 materia);
	bool compareQuads(int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1);
	void greedyMeshing(std::vector<std::vector<std::tuple<int, Cube>>>& mask, int d, int axis, int back, int chunkSizeX, int chunkSizeY, int chunkSizeZ);

	unsigned int VBO, VAO;

	glm::vec3 position;
};

class World {
public:
	World();

	void draw(Shader sh, glm::vec3 playerPosition, glm::vec3 playerFront);

	std::vector<Chunk> chunks;
private:
	int worldSize = 10;
};

float vecAngle(glm::vec3 v1, glm::vec3 v2);
float vecLength(glm::vec3 v1);