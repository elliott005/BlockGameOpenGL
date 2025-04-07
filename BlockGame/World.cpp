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

Chunk::Chunk(int chunkX, int chunkZ, const FastNoiseLite& noise) {
    position.x = chunkX;
    position.z = chunkZ;
    position.y = 0;
    center.x = chunkX + chunkSize / 2;
    center.z = chunkZ + chunkSize / 2;
    center.y = 0;

    for (int i = 0; i < chunkSize + 2; i++) {
        for (int j = 0; j < chunkSize + 2; j++) {
            noiseArray[i][j] = -1;
        }
    }
    //std::fill(noiseArray, noiseArray + chunkSize * chunkSize, -1);

    //printf("%i, %i\n", noiseArray[0][0], noiseArray[1][1]);

    for (int blockX = 0; blockX < chunkSize; blockX++) {
        for (int y = 0; y < chunkHeight; y++) {
            for (int blockZ = 0; blockZ < chunkSize; blockZ++) {
                float noiseValue = getHeightAtPoint(noise, (float)(chunkX + blockX), (float)(chunkZ + blockZ));
                float averageHeightDiff = 0;
                for (int i = 0; i < 4; i++) {
                    float heightDiff = noiseValue - getHeightAtPoint(noise, chunkX + blockX + sides[i][0], chunkZ + blockZ + sides[i][1]);
                    if (heightDiff < 0)
                        averageHeightDiff += -heightDiff;
                    else
                        averageHeightDiff += heightDiff;
                }
                averageHeightDiff /= 4.0;

                int blockType = BlockTypes::dirt;
                if (averageHeightDiff > 0.75) {
                    blockType = BlockTypes::stone;
                }

                if (y < noiseValue) {
                    cubes[blockX][y][blockZ] = Cube(blockX, y, blockZ, blockType, false);
                }
                else {
                    cubes[blockX][y][blockZ] = Cube(blockX, y, blockZ, blockType, true);
                }
            }
        }
    }
    
    //printf("values: %i, %i, %i\n", cubes.size(), cubes[0].size(), cubes[0][0].size());

    // Iterate over each axis (X, Y, Z)
    for (int axis = 0; axis < 3; axis++) {
        //int u = (axis + 1) % 3; // Other two axes
        //int v = (axis + 2) % 3;

        int chunkSizeX = chunkSize;
        int chunkSizeY = chunkHeight;
        int chunkSizeZ = chunkSize;
        if (axis == 1) {
            int chunkSizeY = chunkSize;
            chunkSizeZ = chunkHeight;
        }
        //printf("axis: %i\n", axis);

        // Temporary 2D mask array
        //std::vector<std::vector<Cube>> mask(chunkSizeX, std::vector<Cube>(chunkSizeY, Cube(0, 0, 0, 0, true)));
        Cube* mask = new Cube[chunkSizeX * chunkSizeY];
        /*for (int i = 0; i < chunkSizeX * chunkSizeY; i++) {
            mask[i] = Cube();
        }*/
        // Iterate through the chunk along the main axis
        for (int d = 0; d < chunkSizeZ; d++) {
            //printf("d: %i\n", d);
            // Reset mask
            for (int i = 0; i < chunkSizeX * chunkSizeY; i++) {
                mask[i].isProcessed = true;
            }
            // Fill mask with visible faces
            for (int i = 0; i < chunkSizeX; i++) {
                for (int j = 0; j < chunkSizeY; j++) {
                    //printf("%i, %i, %i, %i\n", i, j, d, axis);
                    Cube* current = getBlockAt(i, j, d, axis);
                    //printf("z: %i\n", chunkSizeZ);
                    if (d < chunkSizeZ - 1) {
                        Cube* next = getBlockAt(i, j, d + 1, axis);
                        // Only store if the face is visible
                        if (not current->isAir != not next->isAir) {
                            mask[i % chunkSizeX * chunkSizeY + j] = not current->isAir ? *current : *next;
                            mask[i % chunkSizeX * chunkSizeY + j].isProcessed = false;
                        }
                    }
                    else if (not current->isAir) {
                        mask[i % chunkSizeX * chunkSizeY + j] = *current;
                        mask[i % chunkSizeX * chunkSizeY + j].isProcessed = false;
                    }
                }
            }
            
            greedyMeshing(mask, d, axis, 1, chunkSizeX, chunkSizeY, chunkSizeZ);

            // --------------------------------------------------------------------------------------------

            for (int i = 0; i < chunkSizeX * chunkSizeY; i++) {
                mask[i].isProcessed = true;
            }

            // Fill mask with visible faces
            for (int i = 0; i < chunkSizeX; i++) {
                for (int j = 0; j < chunkSizeY; j++) {
                    Cube* current = getBlockAt(i, j, d, axis);
                    if (d > 0) {
                        Cube* next = getBlockAt(i, j, d - 1, axis);
                        // Only store if the face is visible
                        if (not current->isAir != not next->isAir) {
                            mask[i % chunkSizeX * chunkSizeY + j] = not current->isAir ? *current : *next;
                            mask[i % chunkSizeX * chunkSizeY + j].isProcessed = false;
                        }
                    }
                    else if (not current->isAir) {
                        mask[i % chunkSizeX * chunkSizeY + j] = *current;
                        mask[i % chunkSizeX * chunkSizeY + j].isProcessed = false;
                    }
                }
            }

            greedyMeshing(mask, d, axis, 0, chunkSizeX, chunkSizeY, chunkSizeZ);
        }
        delete[] mask;
    }
    //printf("size: %i\n", vertices.size());
    /*for (float vert : vertices) {
        printf("%f\n", vert);
    }*/

    glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 + sizeof(uint8_t) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
    std::vector<float> posData = { position.x, position.z };
    /*glm::mat4 model(1.0f);
    glm::translate(model, position);*/
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(uint8_t) * vertices.size(), sizeof(float) * 2, posData.data());
	// position attribute
	glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 7 * sizeof(uint8_t), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_UNSIGNED_BYTE, GL_FALSE, 7 * sizeof(uint8_t), (void*)(3 * sizeof(uint8_t)));
	glEnableVertexAttribArray(1);
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 7 * sizeof(uint8_t), (void*)(5 * sizeof(uint8_t)));
    glEnableVertexAttribArray(2);
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, 7 * sizeof(uint8_t), (void*)(6 * sizeof(uint8_t)));
    glEnableVertexAttribArray(3);

    glVertexAttribPointer(4, 2, GL_FLOAT, false, sizeof(float) * 2, (void*)(sizeof(uint8_t) * vertices.size()));
    glEnableVertexAttribArray(4);
    glVertexAttribDivisor(4, 1);
	

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
        
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 6);

    /*glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);*/
}

Cube* Chunk::getBlockAt(int x, int y, int z, int axis) {
    // Convert axis-based indexing into 3D coordinates
    if (axis == 0) { // X-axis
        if (0 <= x and x < chunkSize and 0 <= y and y < chunkHeight and 0 <= z and z < chunkSize)
            return &cubes[x][y][z];
        return &airCube;
    }
    else if (axis == 1) { // Y-axis
        if (0 <= x and x < chunkSize and 0 <= z and z < chunkHeight and 0 <= y and y < chunkSize)
            return &cubes[x][z][y];
        return &airCube;
    }
    else { // Z-axis
        if (0 <= z and z < chunkSize and 0 <= y and y < chunkHeight and 0 <= x and x < chunkSize)
            return &cubes[z][y][x];
        return &airCube;
    }
}

void Chunk::addQuadToMesh(int x, int y, int z, int width, int height, int axis, int back, int material) {
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

    //printf("%i\n", (uint8_t)normal

    // Push the quad vertices
    vertices.insert(vertices.end(), {
        (uint8_t)pos[0].x,
        (uint8_t)pos[0].y,
        (uint8_t)pos[0].z,
        (uint8_t)texPos[0].x,
        (uint8_t)texPos[0].y,
        (uint8_t)normal,
        (uint8_t)material,
        (uint8_t)pos[1].x,
        (uint8_t)pos[1].y,
        (uint8_t)pos[1].z,
        (uint8_t)texPos[1].x,
        (uint8_t)texPos[1].y,
        (uint8_t)normal,
        (uint8_t)material,
        (uint8_t)pos[2].x,
        (uint8_t)pos[2].y,
        (uint8_t)pos[2].z,
        (uint8_t)texPos[2].x,
        (uint8_t)texPos[2].y,
        (uint8_t)normal,
        (uint8_t)material,

        (uint8_t)pos[0].x,
        (uint8_t)pos[0].y,
        (uint8_t)pos[0].z,
        (uint8_t)texPos[0].x,
        (uint8_t)texPos[0].y,
        (uint8_t)normal,
        (uint8_t)material,
        (uint8_t)pos[2].x,
        (uint8_t)pos[2].y,
        (uint8_t)pos[2].z,
        (uint8_t)texPos[2].x,
        (uint8_t)texPos[2].y,
        (uint8_t)normal,
        (uint8_t)material,
        (uint8_t)pos[3].x,
        (uint8_t)pos[3].y,
        (uint8_t)pos[3].z,
        (uint8_t)texPos[3].x,
        (uint8_t)texPos[3].y,
        (uint8_t)normal,
        (uint8_t)material
    });
}

void Chunk::greedyMeshing(Cube* mask, int d, int axis, int back, int chunkSizeX, int chunkSizeY, int chunkSizeZ) {
    // Apply greedy merging on the mask
    for (int i = 0; i < chunkSizeX; i++) {
        for (int j = 0; j < chunkSizeY; j++) {
            Cube* currentCube = &mask[i % chunkSizeX * chunkSizeY + j];
            if (currentCube->isProcessed or currentCube->isAir) continue;

            int material = currentCube->type;
            int width = 1, height = 1;

            // Expand width
            while (i + width < chunkSizeX and not mask[(i + width) % chunkSizeX * chunkSizeY + j].isProcessed and mask[(i + width) % chunkSizeX * chunkSizeY + j].type == material and not mask[(i + width) % chunkSizeX * chunkSizeY + j].isAir) width++;

            // Expand height
            bool canExpandHeight = true;
            while (j + height < chunkSizeY && canExpandHeight) {
                for (int k = 0; k < width; k++) {
                    if (mask[(i + k) % chunkSizeX * chunkSizeY + j + height].type != material or mask[(i + k) % chunkSizeX * chunkSizeY + j + height].isProcessed or mask[(i + k) % chunkSizeX * chunkSizeY + j + height].isAir) {
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
                    mask[(i + x) % chunkSizeX * chunkSizeY + j + y].isProcessed = true;

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

float Chunk::getHeightAtPoint(const FastNoiseLite& noise, int x, int z) {
    float noiseValue;
    int localX = x - (int)position.x;
    int localZ = z - (int)position.z;
    if (noiseArray[localX + 1][localZ + 1] == -1) {
        noiseValue = noise.GetNoise((float)x, (float)z);
        noiseValue = noiseValue * noiseValue;
        noiseValue *= chunkHeight - 5;
        noiseValue += 5;
        noiseArray[localX + 1][localZ + 1] = noiseValue;
    }
    else {
        noiseValue = noiseArray[localX + 1][localZ + 1];
    }
    
    /* noiseValue = (noiseValue + 1) / 2.0f;
     noiseValue *= chunkHeight / 8.0f;
     noiseValue += 2;
     noiseValue = std::pow(noiseValue, 5);*/
    //noiseValue = 1.f + -1 * std::abs(noiseValue);
    return noiseValue;
}

float vecAngle(glm::vec3 v1, glm::vec3 v2) {
    if (vecLength(v1) * vecLength(v2) == 0.0f) {
        return 0.0;
    }
    return glm::acos(glm::dot(v1, v2) / (vecLength(v1) * vecLength(v2)));
}
float vecLength(const glm::vec3 &v1) {
    return (std::sqrt(v1.x * v1.x + v1.y * v1.y + v1.z * v1.z));
}