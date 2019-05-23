#version 330 core

layout(location = 0) in vec2 aVertices;
layout(location = 1) in vec4 aPosSize;
layout(location = 2) in vec4 aColour;

out vec4 colour;

uniform vec3 cameraWorldRight;
uniform vec3 cameraWorldUp;
uniform mat4 viewProjection;

void main()
{
    vec3 position = vec3(aPosSize.xy, 0.0f);
    vec2 size = aPosSize.zw;

    vec3 vertexPos = position
        + cameraWorldRight * aVertices.x * size.x
        + cameraWorldUp * aVertices.y * size.y;

    // Output position of the vertex
    gl_Position = viewProjection * vec4(vertexPos, 1.0f);

	colour = aColour;
}
