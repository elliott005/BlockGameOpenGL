#include "World.hpp"

World::World() {
	FastNoiseLite noise;
	noise.SetFrequency(0.005f);
	noise.SetFractalType(FastNoiseLite::FractalType_FBm);

	float heightPower = 2.0;

    //std::vector<glm::vec4> chunkData;

	for (int chunkX = 0; chunkX < worldSize; chunkX++) {
		for (int chunkZ = 0; chunkZ < worldSize; chunkZ++) {
            //chunkData.push_back(glm::vec4(chunkX * Chunk::chunkSize, 0.0f, chunkZ * Chunk::chunkSize, 0.0f));
            chunks.push_back(Chunk(chunkX * Chunk::chunkSize, chunkZ * Chunk::chunkSize, noise));
		}
	}

    /*glGenBuffers(1, &SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, chunkData.size() * sizeof(glm::vec4), &chunkData[0], GL_DYNAMIC_DRAW);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, SSBO);*/

    //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void World::draw(Shader* sh, glm::vec3 playerPosition, glm::vec3 playerFront) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    for (int i = 0; i < chunks.size(); i++) {
		chunks[i].draw(sh, playerPosition, playerFront);
	}
}

Chunk::Chunk(int chunkX, int chunkZ, FastNoiseLite noise) {
    position.x = chunkX;
    position.z = chunkZ;
    position.y = 0;
    center.x = chunkX + chunkSize / 2;
    center.z = chunkZ + chunkSize / 2;
    center.y = 0;

    for (int blockX = 0; blockX < chunkSize; blockX++) {
        cubes.push_back({});
        for (int y = 0; y < chunkHeight; y++) {
            cubes[blockX].push_back({});
            for (int blockZ = 0; blockZ < chunkSize; blockZ++) {
                float noiseValue = noise.GetNoise((float)(chunkX + blockX), (float)(chunkZ + blockZ));
               /* noiseValue = (noiseValue + 1) / 2.0f;
                noiseValue *= chunkHeight / 8.0f;
                noiseValue += 2;
                noiseValue = std::pow(noiseValue, 5);*/
                noiseValue = 1.f + -1 * std::abs(noiseValue);
                noiseValue = std::pow(noiseValue, 2);
                noiseValue *= chunkHeight;
                if (y < noiseValue) {
                    cubes[blockX][y].push_back(Cube(blockX, y, blockZ, BlockTypes::dirt, false));
                }
                else {
                    cubes[blockX][y].push_back(Cube(blockX, y, blockZ, BlockTypes::dirt, true));
                }
            }
        }
    }
    
    //printf("values: %i, %i, %i\n", cubes.size(), cubes[0].size(), cubes[0][0].size());

    // Iterate over each axis (X, Y, Z)
    for (int axis = 0; axis < 3; axis++) {
        //int u = (axis + 1) % 3; // Other two axes
        //int v = (axis + 2) % 3;

        vertices.push_back({});
        vertices.push_back({});

        int chunkSizeX = chunkSize;
        int chunkSizeY = chunkHeight;
        int chunkSizeZ = chunkSize;
        if (axis == 1) {
            chunkSizeY = chunkSize;
            chunkSizeZ = chunkHeight;
        }
        //printf("axis: %i\n", axis);

        // Temporary 2D mask array
        std::vector<std::vector<std::tuple<int, Cube>>> mask(chunkSizeX, std::vector<std::tuple<int, Cube>>(chunkSizeY, std::make_tuple(-1, Cube(0, 0, 0, glm::vec2(0, 0), true))));
        // Iterate through the chunk along the main axis
        for (int d = 0; d < chunkSizeZ; d++) {
            //printf("d: %i\n", d);
            // Reset mask
            for (int i = 0; i < chunkSizeX; i++)
                for (int j = 0; j < chunkSizeY; j++)
                    std::get<0>(mask[i][j]) = -1;
            // Fill mask with visible faces
            for (int i = 0; i < chunkSizeX; i++) {
                for (int j = 0; j < chunkSizeY; j++) {
                    //printf("%i, %i, %i, %i\n", i, j, d, axis);
                    std::tuple<int, Cube> current = getBlockAt(i, j, d, axis);
                    //printf("z: %i\n", chunkSizeZ);
                    if (d < chunkSizeZ - 1) {
                        std::tuple<int, Cube> next = getBlockAt(i, j, d + 1, axis);
                        // Only store if the face is visible
                        if ((std::get<0>(current) != 0 and not std::get<1>(current).isAir) != (std::get<0>(next) != 0 and not std::get<1>(next).isAir)) {
                            mask[i][j] = std::get<0>(current) or not std::get<1>(current).isAir ? current : next;
                        }
                    }
                    else if (not std::get<1>(current).isAir) {
                        mask[i][j] = current;
                    }
                }
            }
            
            greedyMeshing(mask, d, axis, 1, chunkSizeX, chunkSizeY, chunkSizeZ);

            // --------------------------------------------------------------------------------------------

            for (int i = 0; i < chunkSizeX; i++)
                for (int j = 0; j < chunkSizeY; j++)
                    std::get<0>(mask[i][j]) = -1;
            // Fill mask with visible faces
            for (int i = 0; i < chunkSizeX; i++) {
                for (int j = 0; j < chunkSizeY; j++) {
                    std::tuple<int, Cube> current = getBlockAt(i, j, d, axis);
                   if (d > 0) {
                        std::tuple<int, Cube> next = getBlockAt(i, j, d - 1, axis);
                        // Only store if the face is visible
                        if ((std::get<0>(current) != 0 and not std::get<1>(current).isAir) != (std::get<0>(next) != 0 and not std::get<1>(next).isAir)) {
                            mask[i][j] = std::get<0>(current) or not std::get<1>(current).isAir ? current : next;
                        }
                    }
                    else if (not std::get<1>(current).isAir) {
                        mask[i][j] = current;
                    }
                }
            }

            greedyMeshing(mask, d, axis, 0, chunkSizeX, chunkSizeY, chunkSizeZ);
        }
    }
    //printf("size: %i\n", vertices.size());
    /*for (float vert : vertices) {
        printf("%f\n", vert);
    }*/

    int idx = 0;
    for (std::vector<uint8_t> sideVertices : vertices) {
        glGenVertexArrays(1, &VAOs[idx]);
	    glGenBuffers(1, &VBOs[idx]);
	    glBindVertexArray(VAOs[idx]);
	    glBindBuffer(GL_ARRAY_BUFFER, VBOs[idx]);
        glBufferData(GL_ARRAY_BUFFER, sizeof(uint8_t) * sideVertices.size(), sideVertices.data(), GL_STATIC_DRAW);
	    // position attribute
	    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(uint8_t), (void*)0);
	    glEnableVertexAttribArray(0);
	    // texture coord attribute
	    glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(uint8_t), (void*)(3 * sizeof(uint8_t)));
	    glEnableVertexAttribArray(1);
        glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 6 * sizeof(uint8_t), (void*)(5 * sizeof(uint8_t)));
        glEnableVertexAttribArray(2);

        idx++;
    }
	

    /*glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_STATIC_DRAW);*/
}

void Chunk::draw(Shader* sh, glm::vec3 playerPosition, glm::vec3 playerFront) {
    //printf("%f, %f, %f, v2: %f, %f, %f, angle: %f\n", position.x, position.y, position.z, playerPosition.x, playerPosition.y, playerPosition.z, vecAngle(position - playerPosition, playerPosition + playerFront));
    glm::vec3 playerDir = glm::normalize(position - playerPosition);
    /*bool isInChunk = vecLength(position - playerPosition) < chunkSize * 2;
    if (std::abs(vecAngle(playerDir , playerFront)) > glm::radians(90.0f) and not isInChunk)
        return;*/

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, position);
    (*sh).setMat4("model", model);

    glm::vec3 playerDirHorizontal = glm::vec3(playerDir.x, 0, playerDir.z);
    int toCull = -1;
    /*if (not isInChunk) {
        if (vecAngle(playerDirHorizontal, glm::vec3(-1, 0, 0)) < glm::radians(45.0f))
            toCull = 0;
        if (vecAngle(playerDirHorizontal, glm::vec3(1, 0, 0)) < glm::radians(45.0f))
            toCull = 1;
        if (vecAngle(playerDirHorizontal, glm::vec3(0, 0, -1)) < glm::radians(45.0f))
            toCull = 4;
        if (vecAngle(playerDirHorizontal, glm::vec3(0, 0, 1)) < glm::radians(45.0f))
            toCull = 5;
    }*/

    for (int i = 0; i < 6; i++) {
        if (i == toCull) {
            continue;
        }
        
        glBindVertexArray(VAOs[i]);
        glDrawArrays(GL_TRIANGLES, 0, vertices[i].size() / 6);
    }
    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);*/
}

std::tuple<int, Cube> Chunk::getBlockAt(int x, int y, int z, int axis) {
    // Convert axis-based indexing into 3D coordinates
    if (axis == 0) { // X-axis
        if (0 <= x and x < cubes.size() and 0 <= y and y < cubes[x].size() and 0 <= z and z < cubes[x][y].size())
            return std::make_tuple(1, cubes[x][y][z]);
        return std::make_tuple(0, Cube(0, 0, 0, glm::vec2(0, 0), true));
    }
    else if (axis == 1) { // Y-axis
        if (0 <= x and x < cubes.size() and 0 <= z and z < cubes[x].size() and 0 <= y and y < cubes[x][z].size())
            return std::make_tuple(1, cubes[x][z][y]);
        return std::make_tuple(0, Cube(0, 0, 0, glm::vec2(0, 0), true));
    }
    else { // Z-axis
        if (0 <= z and z < cubes.size() and 0 <= y and y < cubes[z].size() and 0 <= x and x < cubes[z][y].size())
            return std::make_tuple(1, cubes[z][y][x]);
        return std::make_tuple(0, Cube(0, 0, 0, glm::vec2(0, 0), true));
    }
}

void Chunk::addQuadToMesh(int x, int y, int z, int width, int height, int axis, int back, glm::vec2 material) {
    // Generate the four corner vertices of the quad
    glm::vec3 pos[4];
    glm::vec2 texPos[4];
    int normal = 2;
    if (axis == 0) { // X-axis
        /*pos[0] = glm::vec3(z, x, y);
        pos[1] = glm::vec3(z, x + width, y);
        pos[2] = glm::vec3(z, x + width, y + height);
        pos[3] = glm::vec3(z, x, y + height);*/
        pos[0] = glm::vec3(x, y, z);
        pos[1] = glm::vec3(x + width, y, z);
        pos[2] = glm::vec3(x + width, y + height, z);
        pos[3] = glm::vec3(x, y + height, z);

        texPos[0] = glm::vec2(0, 0);
        texPos[1] = glm::vec2(width, 0);
        texPos[2] = glm::vec2(width, height);
        texPos[3] = glm::vec2(0, height);
    }
    else if (axis == 1) { // Y-axis
        normal = back;

        pos[0] = glm::vec3(x, z, y);
        pos[1] = glm::vec3(x + width, z, y);
        pos[2] = glm::vec3(x + width, z, y + height);
        pos[3] = glm::vec3(x, z, y + height);

        texPos[0] = glm::vec2(0, 0);
        texPos[1] = glm::vec2(width, 0);
        texPos[2] = glm::vec2(width, height);
        texPos[3] = glm::vec2(0, height);
    }
    else { // Z-axis
        pos[0] = glm::vec3(z, y, x);
        pos[1] = glm::vec3(z, y + height, x);
        pos[2] = glm::vec3(z, y + height, x + width);
        pos[3] = glm::vec3(z, y, x + width);
        
        texPos[0] = glm::vec2(0, 0);
        texPos[1] = glm::vec2(height, 0);
        texPos[2] = glm::vec2(height, width);
        texPos[3] = glm::vec2(0, width);
    }

    /*for (int i = 0; i < 4; i++) {
        texPos[i] += material;
    }*/

    /*int indicesSizeStart = indices.size();
    indices.push_back(indicesSizeStart + 0);
    indices.push_back(indicesSizeStart + 1);
    indices.push_back(indicesSizeStart + 2);
    indices.push_back(indicesSizeStart + 0);
    indices.push_back(indicesSizeStart + 2);
    indices.push_back(indicesSizeStart + 3);*/

    //printf("%i\n", (uint8_t)normal);

    // Push the quad vertices
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[0].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[0].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[0].z);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[0].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[0].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)normal);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[1].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[1].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[1].z);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[1].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[1].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)normal);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[2].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[2].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[2].z);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[2].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[2].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)normal);
    /*vertices.push_back((float)pos[3].x);
    vertices.push_back((float)pos[3].y);
    vertices.push_back((float)pos[3].z);
    vertices.push_back((float)texPos[3].x);
    vertices.push_back((float)texPos[3].y);*/

    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[0].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[0].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[0].z);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[0].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[0].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)normal);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[2].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[2].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[2].z);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[2].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[2].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)normal);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[3].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[3].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)pos[3].z);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[3].x);
    vertices[axis * 2 - back + 1].push_back((uint8_t)texPos[3].y);
    vertices[axis * 2 - back + 1].push_back((uint8_t)normal);
}

void Chunk::greedyMeshing(std::vector<std::vector<std::tuple<int, Cube>>>& mask, int d, int axis, int back, int chunkSizeX, int chunkSizeY, int chunkSizeZ) {
    // Apply greedy merging on the mask
    for (int i = 0; i < chunkSizeX; i++) {
        for (int j = 0; j < chunkSizeY; j++) {
            if (std::get<0>(mask[i][j]) == -1 or std::get<1>(mask[i][j]).isAir) continue;

            glm::vec2 material = std::get<1>(mask[i][j]).type;
            int width = 1, height = 1;

            // Expand width
            while (i + width < chunkSizeX and std::get<0>(mask[i + width][j]) != -1 and std::get<1>(mask[i + width][j]).type == material and not std::get<1>(mask[i + width][j]).isAir) width++;

            // Expand height
            bool canExpandHeight = true;
            while (j + height < chunkSizeY && canExpandHeight) {
                for (int k = 0; k < width; k++) {
                    if (std::get<1>(mask[i + k][j + height]).type != material or std::get<0>(mask[i + k][j + height]) == -1 or std::get<1>(mask[i + k][j + height]).isAir) {
                        canExpandHeight = false;
                        break;
                    }
                }
                if (canExpandHeight) height++;
            }

            //printf("%i, %i, %i, %i\n", i, j, width, height);

            // Mark these as processed
            for (int x = 0; x < width; x++)
                for (int y = 0; y < height; y++)
                    std::get<0>(mask[i + x][j + y]) = -1;

            // Add quad to mesh
            addQuadToMesh(i, j, d + back, width, height, axis, back, material);
        }
    }
}

bool Chunk::compareQuads(int x0, int y0, int w0, int h0, int x1, int y1, int w1, int h1) {
	if (y0 != y1)
		return y0 < y1;
	if (x0 != x1)
		return x0 < x1;
	if (w0 != w1)
		return w0 > w1;
	return h0 >= h1;
}

float vecAngle(glm::vec3 v1, glm::vec3 v2) {
    if (vecLength(v1) * vecLength(v2) == 0.0f) {
        return 0.0;
    }
    return glm::acos(glm::dot(v1, v2) / (vecLength(v1) * vecLength(v2)));
}
float vecLength(glm::vec3 v1) {
    return (std::sqrt(std::pow(v1.x, 2) + std::pow(v1.y, 2) + std::pow(v1.z, 2)));
}