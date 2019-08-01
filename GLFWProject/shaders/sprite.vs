#version 330 core

layout(location = 0) in vec3 aVertices;
layout(location = 1) in vec4 aColour;
layout(location = 2) in vec4 aTexCoords;
layout(location = 3) in mat4 aModelView;

out vec2 texCoords;
out vec4 colour;

uniform mat4 projection;

void main()
{
    vec2 topLeftUV = aTexCoords.xy;
    vec2 clipSize = aTexCoords.zw;
    vec2 normalizedVerts = aVertices.xy + vec2(0.5, 0.5);
    texCoords.x = topLeftUV.x + normalizedVerts.x * clipSize.x;
    texCoords.y = topLeftUV.y + normalizedVerts.y * clipSize.y;

    gl_Position = projection * aModelView * vec4(aVertices, 1.0f);

	colour = aColour;
}
