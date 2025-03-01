#version 460 core

out vec4 FragColor;

in vec2 TexCoord;
flat in uint normal;

uniform sampler2D texture1;

void main()
{
    switch (normal) {
        case 0: // bottom
            FragColor = texture(texture1, TexCoord) * vec4(0.3, 0.3, 0.3, 1.0);
            break;
        case 1: // top
            FragColor = texture(texture1, TexCoord) * vec4(1.0, 1.0, 1.0, 1.0);
            break;
        case 2: // side
            FragColor = texture(texture1, TexCoord) * vec4(0.7, 0.7, 0.7, 1.0);
            break;
    }
}