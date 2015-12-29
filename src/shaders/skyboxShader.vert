#version 330 core

layout(location = 0) in vec3 vertexPosition;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 fragmentPos;

void main(void)
{
    vec4 pos = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1);
    gl_Position = pos.xyww;
    fragmentPos = vertexPosition;
}
