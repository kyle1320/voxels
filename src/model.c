#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "model.h"

typedef struct RLE_S {
    unsigned int count;
    Color color;
} RLE;

extern int useMeshing;

Model *createModel() {
    Model *model = calloc(1, sizeof(Model));

    return model;
}

void readModel(Model *model, char *file_path) {
    #define COUNT CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

    FILE *in = fopen(file_path, "rb");
    RLE buffer[COUNT];
    int size;

    if (!in) {
        fprintf(stderr, "Error reading %s: file not found\n", file_path);
        return;
    }

    size = fread(buffer, sizeof(RLE), COUNT, in);

    fclose(in);

    int i, j, c, n = 0;
    for (i = 0; i < size; i++) {
        c = buffer[i].count;

        for (j = 0; j < c; j++) {
            if (n + j > COUNT) {
                fprintf(stderr, "Error reading %s: model too large\n", file_path);
                return;
            }

            model->blocks[n + j] = buffer[i].color;
        }

        n += c;
    }

    if (n != COUNT) {
        fprintf(stderr, "Error reading %s: model too small\n", file_path);
        return;
    }

    renderModel(model);

    #undef COUNT
}

void writeModel(Chunk *chunk, char *file_path) {
    FILE *out = fopen(file_path, "wb");

    if (!out) {
        fprintf(stderr, "Error reading %s: file not found\n", file_path);
        return;
    }

    RLE buf = {0, {{0, 0, 0, 0}}};

    int x, y, z;
    for (x = 0; x < CHUNK_SIZE; x++) {
        for (y = 0; y < CHUNK_SIZE; y++) {
            for (z = 0; z < CHUNK_SIZE; z++) {
                if (buf.color.all == getBlock(chunk, x, y, z)->color.all) {
                    buf.count++;
                    if (buf.count == 0) {
                        buf.count--;
                        fwrite((void*)(&buf), sizeof(RLE), 1, out);
                        buf = (RLE){1, getBlock(chunk, x, y, z)->color};
                    }
                } else {
                    if (buf.count != 0)
                        fwrite((void*)(&buf), sizeof(RLE), 1, out);
                    buf = (RLE){1, getBlock(chunk, x, y, z)->color};
                }
            }
        }
    }

    if (buf.count != 0)
        fwrite((void*)(&buf), sizeof(RLE), 1, out);

    fclose(out);
}

void renderModel(Model *model) {
    int x, y, z, i;

    model->chunk = createChunk(0, 0, 0);
    model->points = malloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 12 * 3 * 3);
    model->normals = malloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 12 * 3 * 3);
    model->colors = malloc(CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE * 12 * 3 * 3);

    for (x = 0; x < CHUNK_SIZE; x++) {
        for (y = 0; y < CHUNK_SIZE; y++) {
            for (z = 0; z < CHUNK_SIZE; z++) {
                i = (((x << LOG_CHUNK_SIZE) + y) << LOG_CHUNK_SIZE) + z;
                if (model->blocks[i].all)
                    *getBlock(model->chunk, x, y, z) = (Block){1, model->blocks[i], NULL};
            }
        }
    }

    renderChunk(model->chunk);
    if (useMeshing)
        model->n_points = renderChunkWithMeshing(model->chunk, model->points, model->normals, model->colors, (vec3){0, 0, 0}, 1.0);
    else
        model->n_points = renderChunkToArrays(model->chunk, model->points, model->normals, model->colors, (vec3){0, 0, 0}, 1.0);
}

int addRenderedModel(Model *model, GLfloat *points, GLfloat *normals, GLfloat *colors, vec3 offset, float scale) {
    int i;

    for (i = 0; i < model->n_points; i++) {
        points[i] = model->points[i] * scale + offset[i % 3];
        normals[i] = model->normals[i];
        colors[i] = model->colors[i];
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
    freeChunk(model->chunk);

    free(model->points);
    free(model->normals);
    free(model->colors);

    free(model);
}

Model *rotateModel(Model *src, int times) {
    Model *dest = createModel();
    int x, y, z, i1, i2;

    if (!(times % 4)) {
        memcpy(dest->blocks, src->blocks, sizeof(src->blocks));
    } else {
        for (x = 0; x < CHUNK_SIZE; x++) {
            for (y = 0; y < CHUNK_SIZE; y++) {
                for (z = 0; z < CHUNK_SIZE; z++) {
                    i1 = (((x << LOG_CHUNK_SIZE) + y) << LOG_CHUNK_SIZE) + z;
                    switch (times % 4) {
                        case 1:
                            i2 = ((((CHUNK_SIZE-z-1) << LOG_CHUNK_SIZE) + y) << LOG_CHUNK_SIZE) + x;
                            break;
                        case 2:
                            i2 = ((((CHUNK_SIZE-x-1) << LOG_CHUNK_SIZE) + y) << LOG_CHUNK_SIZE) + (CHUNK_SIZE-z-1);
                            break;
                        case 3:
                            i2 = (((z << LOG_CHUNK_SIZE) + y) << LOG_CHUNK_SIZE) + (CHUNK_SIZE-x-1);
                            break;
                        default:
                        i2 = (((x << LOG_CHUNK_SIZE) + y) << LOG_CHUNK_SIZE) + z;
                        break;
                    }
                    dest->blocks[i1] = src->blocks[i2];
                }
            }
        }
    }
    renderModel(dest);
    return dest;
}
