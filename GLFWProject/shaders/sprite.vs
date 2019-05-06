#version 330 core

layout(location = 0) in vec3 aVertices;
layout(location = 1) in vec4 aPositionScale;
layout(location = 2) in vec4 aColour;
layout(location = 3) in vec4 aTexCoords;

out vec2 texCoords;
out vec4 colour;

uniform vec3 cameraWorldRight;
uniform vec3 cameraWorldUp;
uniform mat4 viewProjection;

void main()
{
	float spriteScale = aPositionScale.w;
	vec3 spritePos = aPositionScale.xyz;

	vec3 vertexPos = spritePos
        + cameraWorldRight * aVertices.x * spriteScale
		+ cameraWorldUp * aVertices.y * spriteScale;

	// Output position of the vertex
	gl_Position = viewProjection * vec4(vertexPos, 1.0f);

    vec2 topLeftUV = aTexCoords.xy;
    vec2 clipSize = aTexCoords.zw;
    vec2 normalizedVerts = aVertices.xy + vec2(0.5, 0.5);
    texCoords.x = topLeftUV.x + normalizedVerts.x * clipSize.x;
    texCoords.y = topLeftUV.y + normalizedVerts.y * clipSize.y;

	colour = aColour;
}
