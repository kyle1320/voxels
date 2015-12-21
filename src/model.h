#ifndef MODEL_H_
#define MODEL_H_

#include <GL/glew.h>

#include "voxels.h"
#include "color.h"

typedef struct Model_S {
    union {
        Color blocks[CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE];
        Color blocks3d[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    };

    GLfloat *points, *normals, *colors;
    int n_points;

    Chunk *chunk;
} Model;

Model *createModel();
void readModel(Model *model, char *file_path);
void writeModel(Chunk *chunk, char *file_path);
void renderModel(Model *model);
int addRenderedModel(Model *model, GLfloat *points, GLfloat *normals, GLfloat *colors, mat4 rotate, vec3 offset, float scale);
void insertModel(Model *model, Block *block);
void freeModel(Model *model);
void copyModel(Model *dest, Model *src);

#endif
