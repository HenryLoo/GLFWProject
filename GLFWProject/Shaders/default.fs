#version 330 core

out vec4 FragColor;

in vec3 colour;
in vec2 texCoord;

uniform sampler2D thisTexture;

void main()
{
    FragColor = texture(thisTexture, texCoord) * vec4(colour, 1.0);
}
