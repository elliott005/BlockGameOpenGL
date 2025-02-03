#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

#include "ShaderClass.hpp"

class BlockTypes {
public:
    static const glm::vec2 stone;
    static const glm::vec2 dirt;
};

class Cube {
public:
    Cube(int x, int y, int z, glm::vec2 p_type, bool air);

    void draw(Shader sh);

    glm::vec3 position;

    static const float vertices[];
    
    glm::vec2 type;
    bool isAir;
private:
};