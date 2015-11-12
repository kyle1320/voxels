#include <stdio.h>
#include <stdlib.h>

#include "light.h"
#include "voxels.h" // BIN_3

static Mesh *makeLightMesh(float size, vec3 color);
static void setupShadowMap(Light *light);

Light *createLight(vec3 pos, vec3 color, float size, float radius) {
    Light *light = malloc(sizeof(Light));

    copy_v3(light->position, pos);
    copy_v3(light->color, color);

    light->radius = radius;

    light->mesh = makeLightMesh(size, color);

    translate_m4(light->mesh->modelMatrix, VALUES(pos));

    setupShadowMap(light);

    return light;
}

void moveLight(Light *light, vec3 pos) {
    translate_m4(light->mesh->modelMatrix,
                 pos[0]-light->position[0],
                 pos[1]-light->position[1],
                 pos[2]-light->position[2]);
    copy_v3(light->position, pos);
}

void freeLight(Light *light) {
    free(light->mesh);

    glDeleteFramebuffers(1, &light->shadowMapFBO);
    glDeleteTextures(1, &light->shadowMapTex);

    free(light);
}

static Mesh *makeLightMesh(float size, vec3 color) {
    Mesh *mesh = malloc(sizeof(Mesh));
    float min = -size / 2;

    makeBox(mesh, min, min, min, size, size, size, color);

    return mesh;
}

static void setupShadowMap(Light *light) {
    glGenFramebuffers(1, &light->shadowMapFBO);
    glGenTextures(1, &light->shadowMapTex);

    // bind the texture so it will be modified
    glBindTexture(GL_TEXTURE_CUBE_MAP, light->shadowMapTex);

    // give a "texture" (all zeros) to openGL for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT32, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT32, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT32, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT32, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT32, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT32, SHADOW_RES, SHADOW_RES, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

    // filtering / wrapping parameters for the cube map
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    // bind the framebuffer to the texture
    glBindFramebuffer(GL_FRAMEBUFFER, light->shadowMapFBO);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_CUBE_MAP_POSITIVE_X, light->shadowMapTex, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        glDeleteFramebuffers(1, &light->shadowMapFBO);
        glDeleteTextures(1, &light->shadowMapTex);

        light->shadowMapFBO = 0;
        light->shadowMapTex = 0;

        puts("Error generating framebuffer for shadow map");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}
