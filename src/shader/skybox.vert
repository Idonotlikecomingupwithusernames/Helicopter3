#version 330 core

layout(location = 0) in vec3 aPosition;

uniform mat4 uView;
uniform mat4 uProj;

out vec3 texCoords;

void main(void)
{
    gl_Position = uProj * uView * vec4(aPosition, 1.0);
    texCoords = aPosition;
}
