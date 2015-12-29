#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexNormal;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec3 vertexUV;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 fragmentColor;
out vec3 fragmentUV;

void main(void)
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1);
    fragmentColor = vertexColor;
    fragmentUV = vertexUV;
}
