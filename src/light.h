#ifndef LIGHT_H_
#define LIGHT_H_

#include "mesh.h"
#include "matrix.h"

#define SHADOW_RES 1024

typedef struct Light_S {

    // the order of these is specifically arranged so that
    // a Light can be passed into the shader. So, don't change.
    vec3 position;
    GLfloat radius;
    vec3 color;

    Mesh *mesh;

    // for shadows
    GLuint shadowMapFBO;
    GLuint shadowMapTex;
} Light;

Light *createLight(vec3 pos, vec3 color, float size, float radius);
void moveLight(Light *light, vec3 pos);
void freeLight(Light *light);

#endif
