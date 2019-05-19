#version 330 core

layout(location = 0) in vec2 aVertices;

out vec2 texCoord;

uniform vec3 cameraWorldRight;
uniform vec3 cameraWorldUp;
uniform mat4 viewProjection;

void main()
{
	vec3 vertexPos = cameraWorldRight * aVertices.x * 16 * 26
		+ cameraWorldUp * aVertices.y * 16 * 16;

	// Output position of the vertex
	gl_Position = viewProjection * vec4(vertexPos, 1.0f);
    texCoord = aVertices;
}
