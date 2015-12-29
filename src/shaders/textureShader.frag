#version 330 core

in vec3 fragmentColor;
in vec3 fragmentUV;

uniform samplerCube currTexture;

out vec3 color;

void main()
{
    color = fragmentColor * texture(currTexture, fragmentUV).xyz;
}
