#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model.h"
#include "mesh.h"

typedef struct RLE_S {
    unsigned int count;
    Color color;
} RLE;

extern int useMeshing;

Model *createModel() {
    Model *model = calloc(1, sizeof(Model));

    model->chunk = createChunk(0, 0, 0);

    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                Block *block = getBlock(model->chunk, x, y, z);

                if (x < CHUNK_SIZE - 1) block->nb_pos_x = getBlock(model->chunk, x+1, y, z);
                if (x > 0) block->nb_neg_x = getBlock(model->chunk, x-1, y, z);
                if (y < CHUNK_SIZE - 1) block->nb_pos_y = getBlock(model->chunk, x, y+1, z);
                if (y > 0) block->nb_neg_y = getBlock(model->chunk, x, y-1, z);
                if (z < CHUNK_SIZE - 1) block->nb_pos_z = getBlock(model->chunk, x, y, z+1);
                if (z > 0) block->nb_neg_z = getBlock(model->chunk, x, y, z-1);
            }
        }
    }

    return model;
}

void readModel(Model *model, char *file_path) {
    #define COUNT CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

    FILE *in = fopen(file_path, "rb");

    if (!in) {
        fprintf(stderr, "Error reading %s: file not found\n", file_path);
        return;
    }

    readChunk(model->chunk, in);

    renderModel(model);

    fclose(in);

    #undef COUNT
}

void writeModel(Chunk *chunk, char *file_path) {
    FILE *out = fopen(file_path, "wb");

    if (!out) {
        fprintf(stderr, "Error writing %s: file not found\n", file_path);
        return;
    }

    writeChunk(chunk, out);

    fclose(out);
}

void renderModel(Model *model) {
    if (model->chunk->mesh)
        freeMesh(model->chunk->mesh);

    unsigned int max_points = countChunkSize(model->chunk);

    model->points = malloc(max_points * sizeof(GLfloat));
    model->normals = malloc(max_points * sizeof(GLfloat));
    model->colors = malloc(max_points * sizeof(GLfloat));

    if (max_points == 0) {
        *model->chunk->mesh = EMPTY_MESH;
        model->n_points = 0;
        return;
    }

    if (useMeshing)
        model->n_points = renderChunkWithMeshing(model->chunk, model->points, model->normals, model->colors, (vec3){0, 0, 0}, 1.0);
    else
        model->n_points = renderChunkToArrays(model->chunk, model->points, model->normals, model->colors, (vec3){0, 0, 0}, 1.0);

    buildMesh(model->chunk->mesh, model->points, model->normals, model->colors, NULL, NULL,
              model->n_points * sizeof(GLfloat), model->n_points * sizeof(GLfloat),
              model->n_points * sizeof(GLfloat), 0, 0,
              model->n_points / 3);
}

int addRenderedModel(Model *model, GLfloat *points, GLfloat *normals, GLfloat *colors, mat4 rotate, vec3 offset, float scale) {
    int i;

    for (i=0; i < model->n_points; i += 3) {
        copy_v3(&points[i], &model->points[i]);
        translate_v3f(&points[i], -CHUNK_WIDTH/2.0, -CHUNK_WIDTH/2.0, -CHUNK_WIDTH/2.0);
        multiply_v3_m4(&points[i], rotate, 1.0);
        translate_v3f(&points[i], CHUNK_WIDTH/2.0, CHUNK_WIDTH/2.0, CHUNK_WIDTH/2.0);
        scale_v3(&points[i], scale);
        translate_v3v(&points[i], offset);
        copy_v3(&normals[i], &model->normals[i]);
        multiply_v3_m4(&normals[i], rotate, 1.0);
        copy_v3(&colors[i], &model->colors[i]);
    }

    return model->n_points;
}

void insertModel(Model *model, Block *block) {
    block->active = 1;
    block->color.all = 0;
    block->data = model;
    block->logic = NULL;
}

void freeModel(Model *model) {
    if (model->chunk)
        freeChunk(model->chunk);

    free(model->points);
    free(model->normals);
    free(model->colors);

    free(model);
}
