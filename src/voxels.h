#ifndef VOXELS_H_
#define VOXELS_H_

#include "matrix.h"
#include "mesh.h"
#include "color.h"

#define BLOCK_WIDTH 0.05
#define LOG_CHUNK_SIZE 4
#define CHUNK_SIZE (1 << LOG_CHUNK_SIZE)
#define CHUNK_WIDTH (CHUNK_SIZE * BLOCK_WIDTH)
#define WORLD_SIZE 6
#define WORLD_WIDTH (WORLD_SIZE * CHUNK_WIDTH)

#define getBlock(chunk, x, y, z) (&chunk->blocks[x][y][z])

#define BIN_3(_0, _1) _0, _0, _0, _0, _0, _1, _0, _1, _0, _0, _1, _1, _1, _0, _0, _1, _0, _1, _1, _1, _0, _1, _1, _1

/*
                 3_____________ 7
    y+  z+       /|           /|
    | /         / |          / |
    |/         /  |         /  |
    o---- x+  /____________/   |
            2|    |        |6  |
             |    |________|___|
             |  1/         |   / 5
             |  /          |  /
             | /           | /
             |/____________|/
            0               4
*/

struct Model_S;

typedef struct Block_S {
    char active;
    Color color;
    struct Model_S *data;
} Block;

typedef struct Chunk_S {
    Block blocks[CHUNK_SIZE][CHUNK_SIZE][CHUNK_SIZE];
    int x, y, z;
    int flag;
    Mesh *mesh;
} Chunk;

typedef struct World_S {
    Chunk *chunks[WORLD_SIZE][WORLD_SIZE][WORLD_SIZE];
} World;

typedef struct Selection_S {
    int selected_active;
    int selected_chunk_x, selected_chunk_y, selected_chunk_z;
    int selected_block_x, selected_block_y, selected_block_z;

    int previous_active;
    int previous_chunk_x, previous_chunk_y, previous_chunk_z;
    int previous_block_x, previous_block_y, previous_block_z;
} Selection;

// chunks

Chunk * createChunk();
void renderChunk(Chunk *chunk);
void freeChunk(Chunk *chunk);

// worlds

World * createWorld();
void fillWorld(World *world);
void drawWorld(World *world, mat4 viewMatrix, mat4 projectionMatrix);
void freeWorld(World *world);

// blocks

Selection selectBlock(World *world, vec3 position, vec3 direction, float radius);
Block* selectedBlock(World *world, Selection* selection);

int isVisible(Chunk *chunk, mat4 view, mat4 perspective);
void setBlock(Chunk *chunk, int x, int y, int z, Block block);

// utils

int renderChunkToArrays(Chunk *chunk, GLfloat *points, GLfloat *normals, GLfloat *colors, vec3 offset, float scale);
int renderChunkWithMeshing(Chunk *chunk, GLfloat *points, GLfloat *normals, GLfloat *colors, vec3 offset, float scale);

void buildBlockFrame(Mesh *mesh);
int solidBlockInArea(World *world, int minx, int miny, int minz, int maxx, int maxy, int maxz);

#endif
