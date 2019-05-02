#version 330 core

in vec2 texCoords;
in vec4 colour;

out vec4 fragColour;

uniform sampler2D textureSampler;

void main(){
	fragColour = texture(textureSampler, texCoords) * colour;
}
