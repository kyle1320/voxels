#version 330 core

in vec3 fragmentColor;
in vec3 normal_cameraspace;
// in vec3 lightDirection_cameraspace[10];
// in vec3 lightDirection_worldspace[10];
in vec3 eyeDirection_cameraspace;

// struct LightInfo {
//     vec4 positionRadius;
//     vec4 color;
// };
//
// layout(std140) uniform Light {
//     LightInfo light[10];
// };
// uniform int lightCount;

// uniform samplerCube shadowMap;

layout(std140) uniform Materials {
    vec3 materialDiffuseColor;
    vec3 materialAmbientColor;
    vec3 materialSpecularColor;
};

out vec3 color;

void main()
{
    vec3 n = normalize(normal_cameraspace);
    // vec3 l = normalize(lightDirection_cameraspace[0]);
    vec3 l = normalize(eyeDirection_cameraspace);

    float cosTheta = clamp(dot(n, l), 0, 1);

    vec3 E = normalize(eyeDirection_cameraspace);
    vec3 R = reflect(-l, n);

    float cosAlpha = clamp(dot(E, R), 0, 1);

    // float distance = length(lightDirection_worldspace[0]) / light[0].positionRadius.w;
    // float shadowDist = texture(shadowMap, -lightDirection_worldspace[0]).r;
    // float shadow = 1 - distance;
    //
    // float bias = clamp(0.001*tan(acos(cosTheta)), 0, 1);
    //
    // if (distance > shadowDist + bias) {
    //     shadow = 0;
    // }

    //float shadow = 1;//texture(shadowMap, vec4(-lightDirection_worldspace, 0));

    color = (materialDiffuseColor * cosTheta +
             materialAmbientColor * fragmentColor) * fragmentColor +
             materialSpecularColor * cosAlpha;

}

/*
#version 330 core

in vec3 fragmentColor;
in vec3 normal_cameraspace;
in vec3 lightDirection_cameraspace[10];
in vec3 lightDirection_worldspace[10];
in vec3 eyeDirection_cameraspace;

struct LightInfo {
    vec4 positionRadius;
    vec4 color;
};

layout(std140) uniform Light {
    LightInfo light[10];
};
uniform int lightCount;

uniform samplerCube shadowMap;

layout(std140) uniform Materials {
    vec3 materialDiffuseColor;
    vec3 materialAmbientColor;
    vec3 materialSpecularColor;
};

out vec3 color;

void main()
{

    float diffuseBrightness = 0;
    float specularBrightness = 0;
    vec3 diffuseColor = vec3(0, 0, 0);
    vec3 specularColor = vec3(0, 0, 0);
    float diffusePower;
    float specularPower;

    vec3 n = normalize(normal_cameraspace);
    vec3 E = normalize(eyeDirection_cameraspace);
    vec3 l;
    vec3 R;
    float cosTheta;
    float cosAlpha;

    float distance;

    for (int i=0; i < lightCount; i++) {
        l = normalize(lightDirection_cameraspace[i]);
        R = reflect(-l, n);

        cosTheta = clamp(dot(n, l), 0, 1);
        cosAlpha = clamp(dot(E, R), 0, 1);

        //distance = clamp(length(lightDirection_worldspace[i]), 0, 1);

        diffusePower = (1 - diffuseBrightness) * cosTheta;
        specularPower = (1 - specularBrightness) * pow(cosAlpha, 5);

        diffuseColor += diffusePower * materialDiffuseColor * light[i].color.rgb;
        specularColor += specularPower * materialSpecularColor * light[i].color.rgb;
        diffuseBrightness += diffusePower;
        specularBrightness += specularPower;
    }

    //float shadowDist = texture(shadowMap, -lightDirection_worldspace[0]).r;
    //float shadow = 1 - distance;

    //float bias = clamp(0.001*tan(acos(cosTheta)), 0, 1);

    //if (distance > shadowDist + bias) {
    //    shadow = 0;
    //}

    float shadow = 1;

    //float shadow = 1;//texture(shadowMap, vec4(-lightDirection_worldspace, 0));

    color = (diffuseColor * fragmentColor + specularColor) * shadow +
                materialAmbientColor * fragmentColor;
}
*/
