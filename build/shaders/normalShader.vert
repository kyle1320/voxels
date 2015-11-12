#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec3 vertexColor;
layout(location = 3) in vec2 vertexUV;

struct LightInfo {
	vec4 positionRadius;
	vec4 color;
};

layout(std140) uniform Light {
	LightInfo light[10];
};
uniform int lightCount;

uniform mat4 projectionMatrix;
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec3 fragmentColor;
out vec3 normal_cameraspace;
out vec3 lightDirection_cameraspace[10];
out vec3 lightDirection_worldspace[10];
out vec3 eyeDirection_cameraspace;

void main(void)
{
	vec4 position_worldspace = modelMatrix * vec4(vertexPosition, 1);
	vec4 position_cameraspace = viewMatrix * position_worldspace;

	gl_Position = projectionMatrix * position_cameraspace;

	eyeDirection_cameraspace = -position_cameraspace.xyz;

	for (int i = 0; i < lightCount; i++) {
		lightDirection_cameraspace[i] = (viewMatrix * vec4(light[i].positionRadius.xyz, 1) - position_cameraspace).xyz;
		lightDirection_worldspace[i] = light[i].positionRadius.xyz - position_worldspace.xyz;
	}

	normal_cameraspace = (viewMatrix * modelMatrix * vec4(vertexNormal, 0)).xyz;

	fragmentColor = vertexColor;
}