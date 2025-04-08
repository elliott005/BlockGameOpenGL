#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "ShaderClass.hpp"

class BlockTypes {
public:
    static const int stone;
    static const int dirt;
};

class Cube {
public:
    //Cube(int x, int y, int z, int p_type, bool air);
    Cube(uint8_t p_type, bool air);
    Cube();

    //void draw(Shader sh);

    //glm::vec3 position;

    //static const float vertices[];
    
    uint8_t type;
    bool isAir;
    bool isProcessed = false;
private:
};