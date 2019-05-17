#version 330 core

layout(location = 0) in vec3 aVertices;

out vec2 pos;

uniform vec3 cameraWorldRight;
uniform vec3 cameraWorldUp;
uniform mat4 viewProjection;

void main()
{
	vec3 vertexPos = cameraWorldRight * aVertices.x
		+ cameraWorldUp * aVertices.y;

	// Output position of the vertex
	gl_Position = viewProjection * vec4(vertexPos, 1.0f);
    pos = vertexPos.xy;
}
