#version 330 core
out vec4 FragColor;

in vec3 texCoords;

uniform samplerCube uCubeMap;
uniform int uIsDay;

void main()
{    
    FragColor = texture(uCubeMap, texCoords) * (0.2 + uIsDay * 0.8);
}