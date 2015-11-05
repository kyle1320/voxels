#version 330

in vec3 worldPos;

uniform vec3 lightSource;
uniform float lightRadius;

void main()
{
	gl_FragDepth = length(lightSource - worldPos) / lightRadius;
}