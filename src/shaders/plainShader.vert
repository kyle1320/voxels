#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexColor;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 fragmentColor;

void main(void)
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1);

    fragmentColor = vertexColor;
}
