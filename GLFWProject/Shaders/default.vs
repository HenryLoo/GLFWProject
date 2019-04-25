#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 normal;
out vec2 texCoord;

uniform mat4 transform;

void main()
{
    normal = aNormal;
    texCoord = aTexCoord;

    gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);
}
