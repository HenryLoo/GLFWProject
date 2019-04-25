#version 330 core

out vec4 FragColor;

in vec3 normal;
in vec2 texCoord;

uniform sampler2D thisTexture;

void main()
{
    FragColor = texture(thisTexture, texCoord);
}
