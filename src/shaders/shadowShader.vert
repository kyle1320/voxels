#version 330

layout(location = 0) in vec3 vertexPosition;

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 worldPos;

void main()
{
    gl_Position = projectionMatrix * viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
    worldPos = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
}
