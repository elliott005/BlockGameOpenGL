#version 460 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in int aNormal;
layout (location = 3) in uint aTex;

out vec2 TexCoord;
flat out uint normal;
flat out uint texIdx;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

//uniform int dummy;

//layout(binding = 3) readonly buffer ssbo1 {
//    mat4 modelMatrices[];
//};

//uniform float numTilesX;
//uniform float numTilesY;

void main()
{
    gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0f);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    normal = aNormal;
    texIdx = aTex;
}