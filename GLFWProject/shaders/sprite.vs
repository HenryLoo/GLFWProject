#version 330 core

layout(location = 0) in vec3 aVertices;
layout(location = 1) in vec3 aPosition;
layout(location = 2) in vec4 aColour;
layout(location = 3) in vec4 aTexCoords;
layout(location = 4) in vec3 aTransform;

out vec2 texCoords;
out vec4 colour;

uniform vec3 cameraWorldRight;
uniform vec3 cameraWorldUp;
uniform mat4 viewProjection;

void main()
{
    vec2 topLeftUV = aTexCoords.xy;
    vec2 clipSize = aTexCoords.zw;
    vec2 normalizedVerts = aVertices.xy + vec2(0.5, 0.5);
    texCoords.x = topLeftUV.x + normalizedVerts.x * clipSize.x;
    texCoords.y = topLeftUV.y + normalizedVerts.y * clipSize.y;

    vec2 spriteScale = aTransform.xy;
    float spriteRotation = aTransform.z;

    vec3 vertexPos = aPosition
        + cameraWorldRight * aVertices.x * spriteScale.x
        + cameraWorldUp * aVertices.y * spriteScale.y;

    // Output position of the vertex
    gl_Position = viewProjection * vec4(vertexPos, 1.0f);

	colour = aColour;
}
