#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
flat in uint normal;
flat in uint texIdx;

uniform sampler2D textures[2];

void main()
{
    switch (normal) {
        case 0: // bottom
            FragColor = texture(textures[texIdx], TexCoord) * vec4(0.3, 0.3, 0.3, 1.0);
            break;
        case 1: // top
            FragColor = texture(textures[texIdx], TexCoord) * vec4(1.0, 1.0, 1.0, 1.0);
            break;
        case 2: // side
            FragColor = texture(textures[texIdx], TexCoord) * vec4(0.7, 0.7, 0.7, 1.0);
            break;
    }
}