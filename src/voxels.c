#include <GL/glew.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "voxels.h"
#include "main.h"
#include "model.h"
#include "logic.h"
#include <time.h>

#define BLOCK_MASK (CHUNK_SIZE - 1)

static int countChunkSize(Chunk *chunk);
static void getFaceData(const GLfloat *dest, const GLfloat *src, const GLuint *indices);

int useMeshing = 1;

static const GLuint cubeIndices[] = {
    7, 5, 4, 7, 4, 6, // x+
    7, 6, 2, 7, 2, 3, // y+
    7, 3, 1, 7, 1, 5, // z+
    0, 1, 3, 0, 3, 2, // x-
    0, 4, 5, 0, 5, 1, // y-
    0, 2, 6, 0, 6, 4  // z-
};

static const GLfloat cubeNormals[] = {
    0, 0, -1, 0, 0, 1, 0, 0
};

static const GLuint zeroIndices[] = {
    0, 0, 0, 0, 0, 0
};

static mat4 identityMatrix = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};

Chunk * createChunk(int x, int y, int z) {
    Chunk *chunk = calloc(1, sizeof(Chunk));

    chunk->x = x;
    chunk->y = y;
    chunk->z = z;
    chunk->flag = 0;

    chunk->mesh = calloc(1, sizeof(Mesh));

    return chunk;
}

void renderChunk(Chunk *chunk) {
    // free the previously used buffers. Memory leaks are bad, mmkay.
    if (chunk->mesh)
        freeMesh(chunk->mesh);

    // 6 faces per cube * 2 triangles per face * 3 vertices per triangle * 3 coordinates per vertex
    unsigned int max_points = countChunkSize(chunk);//6 * 6 * 3 * CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE;

    // don't render an empty chunk :p
    if (max_points == 0) {
        *chunk->mesh = EMPTY_MESH;
        return;
    }

    GLfloat *points = malloc(max_points * sizeof(GLfloat));
    GLfloat *normals = malloc(max_points * sizeof(GLfloat));
    GLfloat *colors = malloc(max_points * sizeof(GLfloat));

    int size;

    if (useMeshing)
        size = renderChunkWithMeshing(chunk, points, normals, colors, (vec3){0, 0, 0}, 1.0);
    else
        size = renderChunkToArrays(chunk, points, normals, colors, (vec3){0, 0, 0}, 1.0);

    buildMesh(chunk->mesh, points, normals, colors, NULL, NULL,
              size * sizeof(GLfloat), size * sizeof(GLfloat),
              size * sizeof(GLfloat), 0, 0,
              size / 3);

    translate_m4(chunk->mesh->modelMatrix,
                 chunk->x * CHUNK_WIDTH,
                 chunk->y * CHUNK_WIDTH,
                 chunk->z * CHUNK_WIDTH);

    free(points);
    free(normals);
    free(colors);
}

static int countChunkSize(Chunk *chunk) {
    int x, y, z;
    int count = 0;
    Block *block;

    for (x = 0; x < CHUNK_SIZE; x++) {
        for (y = 0; y < CHUNK_SIZE; y++) {
            for (z = 0; z < CHUNK_SIZE; z++) {
                block = getBlock(chunk, x, y, z);

                if (block->active) {
                    if (block->data)
                        count += block->data->n_points;
                    else
                        count += 12 * 3 * 3;
                }
            }
        }
    }

    return count;
}

// returns the number of elements added to the arrays
int renderChunkToArrays(Chunk *chunk, GLfloat *points, GLfloat *normals, GLfloat *colors, vec3 offset, float scale) {
    float blockWidth = BLOCK_WIDTH * scale;

    Block *block;
    vec3 color;
    int x, y, z;
    float min_x, min_y, min_z, max_x, max_y, max_z;

    GLfloat cube_vertices[8 * 3];
    GLuint zeroIndices[] = {0, 0, 0, 0, 0, 0};

    int points_index = 0;

    for (x = 0; x < CHUNK_SIZE; x++) {
        min_x = x * blockWidth + offset[0];
        max_x = min_x + blockWidth;

        for (y = 0; y < CHUNK_SIZE; y++) {
            min_y = y * blockWidth + offset[1];
            max_y = min_y + blockWidth;

            for (z = 0; z < CHUNK_SIZE; z++) {
                min_z = z * blockWidth + offset[2];
                max_z = min_z + blockWidth;

                block = getBlock(chunk, x, y, z);

                if (!block->active) {
                    continue;
                }

                if (block->data) {
                    if (block->logic)
                        points_index += addRenderedModel(
                            block->data,
                            &points[points_index],
                            &normals[points_index],
                            &colors[points_index],
                            *block->logic->rotationMatrix,
                            (vec3){min_x, min_y, min_z},
                            scale / CHUNK_SIZE
                        );
                    else
                        points_index += addRenderedModel(
                            block->data,
                            &points[points_index],
                            &normals[points_index],
                            &colors[points_index],
                            identityMatrix,
                            (vec3){min_x, min_y, min_z},
                            scale / CHUNK_SIZE
                        );
                    continue;
                }

                cube_vertices[ 0] = min_x; cube_vertices[ 1] = min_y; cube_vertices[ 2] = min_z;
                cube_vertices[ 3] = min_x; cube_vertices[ 4] = min_y; cube_vertices[ 5] = max_z;
                cube_vertices[ 6] = min_x; cube_vertices[ 7] = max_y; cube_vertices[ 8] = min_z;
                cube_vertices[ 9] = min_x; cube_vertices[10] = max_y; cube_vertices[11] = max_z;
                cube_vertices[12] = max_x; cube_vertices[13] = min_y; cube_vertices[14] = min_z;
                cube_vertices[15] = max_x; cube_vertices[16] = min_y; cube_vertices[17] = max_z;
                cube_vertices[18] = max_x; cube_vertices[19] = max_y; cube_vertices[20] = min_z;
                cube_vertices[21] = max_x; cube_vertices[22] = max_y; cube_vertices[23] = max_z;

                color[0] = (float)block->color.r / 255.0;
                color[1] = (float)block->color.g / 255.0;
                color[2] = (float)block->color.b / 255.0;

                // check if each face is visible
                if (x == CHUNK_SIZE - 1 || !getBlock(chunk, x+1, y, z)->active || getBlock(chunk, x+1, y, z)->data) {
                    getFaceData(&points[points_index],     cube_vertices,      &cubeIndices[ 0]);
                    getFaceData(&normals[points_index],    &cubeNormals[5],    zeroIndices);
                    getFaceData(&colors[points_index],     color,              zeroIndices);
                    points_index += 6 * 3;
                }

                if (y == CHUNK_SIZE - 1 || !getBlock(chunk, x, y+1, z)->active || getBlock(chunk, x, y+1, z)->data) {
                    getFaceData(&points[points_index],     cube_vertices,      &cubeIndices[ 6]);
                    getFaceData(&normals[points_index],    &cubeNormals[4],    zeroIndices);
                    getFaceData(&colors[points_index],     color,              zeroIndices);
                    points_index += 6 * 3;
                }

                if (z == CHUNK_SIZE - 1 || !getBlock(chunk, x, y, z+1)->active || getBlock(chunk, x, y, z+1)->data) {
                    getFaceData(&points[points_index],     cube_vertices,      &cubeIndices[12]);
                    getFaceData(&normals[points_index],    &cubeNormals[3],    zeroIndices);
                    getFaceData(&colors[points_index],     color,              zeroIndices);
                    points_index += 6 * 3;
                }

                if (x == 0 || !getBlock(chunk, x-1, y, z)->active || getBlock(chunk, x-1, y, z)->data) {
                    getFaceData(&points[points_index],     cube_vertices,      &cubeIndices[18]);
                    getFaceData(&normals[points_index],    &cubeNormals[2],    zeroIndices);
                    getFaceData(&colors[points_index],     color,              zeroIndices);
                    points_index += 6 * 3;
                }

                if (y == 0 || !getBlock(chunk, x, y-1, z)->active || getBlock(chunk, x, y-1, z)->data) {
                    getFaceData(&points[points_index],     cube_vertices,      &cubeIndices[24]);
                    getFaceData(&normals[points_index],    &cubeNormals[1],    zeroIndices);
                    getFaceData(&colors[points_index],     color,              zeroIndices);
                    points_index += 6 * 3;
                }

                if (z == 0 || !getBlock(chunk, x, y, z-1)->active || getBlock(chunk, x, y, z-1)->data) {
                    getFaceData(&points[points_index],     cube_vertices,      &cubeIndices[30]);
                    getFaceData(&normals[points_index],    &cubeNormals[0],    zeroIndices);
                    getFaceData(&colors[points_index],     color,              zeroIndices);
                    points_index += 6 * 3;
                }
            }
        }
    }

    return points_index;
}

int renderChunkWithMeshing(Chunk *chunk, GLfloat *points, GLfloat *normals, GLfloat *colors, vec3 offset, float scale) {
    float blockWidth = scale * BLOCK_WIDTH;

    Color *face[CHUNK_SIZE][CHUNK_SIZE];
    unsigned int axis1, axis2, axis3, w, h, i, j, k, points_index, pos[3], dir[3];
    int sign, empty, models;
    Block *voxel1, *voxel2;
    vec3 d_axis2, d_axis3, fpos;

    //Color covered; // placeholder value for covered faces

    vec3 color;
    GLuint indices[] = {0, 1, 2, 0, 2, 3, 0, 3, 2, 0, 2, 1};
    GLfloat verts[12];

    /*
        axis1 is our "working" axis. We look at both sides of each "slice" of the chunk
        along that axis, and determine whether each face is visible by checking
        the block in front of it. For each face that is visible, its color is
        added to the face array. Then we look at the face array and split it up
        into rectangles of the same color using a greedy algorithm. We draw each
        rectangle and then move on to the next axis.


        // ***** this is currently removed *****
        I also implemented (my own idea!) a system where if a block is covered,
        a placeholder value is inserted in the face array instead. Then, when
        calculating the rectangles, the placeholder value is treated as a solid
        block, except for the fact that a rectangle cannot start on a face with
        the placeholder value. This still prevents faces that are completely
        hidden from being drawn, but allows for rectangles to be combined under
        blocks that are hiding different colors. This should result in a strictly
        smaller number of rectangles being drawn, at near-zero cost :D
        The only drawback is that large rectangles that are mostly hidden may
        still be drawn. Everything will still look fine, but there is a little
        extra cost from drawing the overlapping triangles. My hope is that
        overall the scene will render even faster.
    */

    memset(face, 0, sizeof(face));

    points_index = 0;
    models = 0;

    for (axis1 = 0; axis1 < 3; axis1++) {
        axis2 = (axis1 + 1) % 3;
        axis3 = (axis1 + 2) % 3;

        dir[0] = dir[1] = dir[2] = 0;
        d_axis2[0] = d_axis2[1] = d_axis2[2] = 0;
        d_axis3[0] = d_axis3[1] = d_axis3[2] = 0;

        for (sign = -1; sign < 2; sign += 2) {
            dir[axis1] = sign;

            for (pos[axis1] = 0; pos[axis1] < CHUNK_SIZE; pos[axis1]++) {
                empty = 1;

                // generate the face array
                for (pos[axis2] = 0; pos[axis2] < CHUNK_SIZE; pos[axis2]++) {
                    for (pos[axis3] = 0; pos[axis3] < CHUNK_SIZE; pos[axis3]++) {
                        voxel1 = getBlock(chunk, pos[0], pos[1], pos[2]);
                        voxel2 = ((sign > 0) ? (pos[axis1] < CHUNK_SIZE - 1) : (pos[axis1] > 0)) ?
                                 getBlock(chunk, pos[0]+dir[0], pos[1]+dir[1], pos[2]+dir[2]) :
                                 NULL;

                        if (voxel1->active) {

                            // there's a block that is potentially drawable
                            empty = 0;

                            if (voxel1->data)
                                // there's a model in the chunk. These have to be
                                // handled separately, so we mark a boolean flag
                                // so that we know to go back and render them
                                models = 1;
                            else if (!voxel2 || !voxel2->active || voxel2->data)
                                face[pos[axis3]][pos[axis2]] = &voxel1->color;
                        }
                    }
                }

                // nothing to do here. Continue on.
                if (empty)
                    continue;

                /*puts("---------------");
                printf("____%c%c AXIS____\n", sign > 0 ? '+' : '-', (char[]){'X','Y','Z'}[axis1]);
                printf("___SLICE #%02d___\n", pos[axis1]);
                for (i = 0; i < CHUNK_SIZE; i++) {
                    for (j = 0; j < CHUNK_SIZE; j++) {
                        printf("%c ", face[j][i] ? '#' : ' ');
                    }
                    puts("");
                }
                puts("---------------\n");*/

                // cut the face up into rectangles and draw them
                for (j = 0; j < CHUNK_SIZE; j++) {
                    for (i = 0; i < CHUNK_SIZE;) {
                        w = 1;

                        if (face[j][i]) {

                            // get the width
                            while (
                                (i + w < CHUNK_SIZE) &&
                                (face[j][i + w]) &&
                                (face[j][i + w]->all == face[j][i]->all)
                            ) w++;

                            // get the height
                            for (h = 1; j + h < CHUNK_SIZE; h++) {

                                // we look at the next row, and make sure each
                                // block on the face is solid and the same color.
                                // if it's not, we break from the outer loop
                                for (k = 0; k < w; k++) {
                                    if ((face[j + h][i + k] == NULL) ||
                                        (face[j + h][i + k]->all != face[j][i]->all)
                                    ) goto done;
                                }
                            }
                            done:

                            // draw it

                            color[0] = (float)face[j][i]->r / 255.0;
                            color[1] = (float)face[j][i]->g / 255.0;
                            color[2] = (float)face[j][i]->b / 255.0;

                            fpos[axis1] = pos[axis1] * blockWidth + offset[axis1];
                            fpos[axis2] = i * blockWidth + offset[axis2];
                            fpos[axis3] = j * blockWidth + offset[axis3];

                            d_axis2[axis2] = w * blockWidth;
                            d_axis3[axis3] = h * blockWidth;

                            if (sign > 0) fpos[axis1] += blockWidth;

                            verts[ 0] = fpos[0];
                            verts[ 1] = fpos[1];
                            verts[ 2] = fpos[2];
                            verts[ 3] = fpos[0] + d_axis2[0];
                            verts[ 4] = fpos[1] + d_axis2[1];
                            verts[ 5] = fpos[2] + d_axis2[2];
                            verts[ 6] = fpos[0] + d_axis2[0] + d_axis3[0];
                            verts[ 7] = fpos[1] + d_axis2[1] + d_axis3[1];
                            verts[ 8] = fpos[2] + d_axis2[2] + d_axis3[2];
                            verts[ 9] = fpos[0] + d_axis3[0];
                            verts[10] = fpos[1] + d_axis3[1];
                            verts[11] = fpos[2] + d_axis3[2];

                            getFaceData(&points[points_index], verts, &indices[((sign < 0) ? 6 : 0)]);
                            getFaceData(&normals[points_index], &cubeNormals[(sign > 0) ? 5-axis1 : 2-axis1], zeroIndices);
                            getFaceData(&colors[points_index], color, zeroIndices);

                            points_index += 18;

                            // empty the face array wherever we rendered it
                            for(k = 0; k < h; k++) {
                                memset(&face[j + k][i], 0, w * sizeof(Color*));
                            }
                        }

                        i += w;
                    }
                }
            }
        }
    }

    if (models) {

        // there were models in the chunk. Render them
        for (pos[0] = 0; pos[0] < CHUNK_SIZE; pos[0]++) {
            for (pos[1] = 0; pos[1] < CHUNK_SIZE; pos[1]++) {
                for (pos[2] = 0; pos[2] < CHUNK_SIZE; pos[2]++) {
                    voxel1 = getBlock(chunk, pos[0], pos[1], pos[2]);

                    if (voxel1->active && voxel1->data) {
                        if (voxel1->logic)
                            points_index += addRenderedModel(
                                    voxel1->data, &points[points_index], &normals[points_index], &colors[points_index], *voxel1->logic->rotationMatrix,
                                    (vec3){pos[0]*blockWidth + offset[0], pos[1]*blockWidth + offset[1], pos[2]*blockWidth + offset[2]},
                                    scale / CHUNK_SIZE
                                );
                        else
                            points_index += addRenderedModel(
                                    voxel1->data, &points[points_index], &normals[points_index], &colors[points_index], identityMatrix,
                                    (vec3){pos[0]*blockWidth + offset[0], pos[1]*blockWidth + offset[1], pos[2]*blockWidth + offset[2]},
                                    scale / CHUNK_SIZE
                                );
                    }
                }
            }
        }
    }

    return points_index;
}

void freeChunk(Chunk *chunk) {
    int x, y, z;

    for (x = 0; x < CHUNK_SIZE; x++) {
        for (y = 0; y < CHUNK_SIZE; y++) {
            for (z = 0; z < CHUNK_SIZE; z++) {
                if (getBlock(chunk, x, y, z)->logic)
                    free(getBlock(chunk, x, y, z)->logic);
            }
        }
    }

    freeMesh(chunk->mesh);
    free(chunk->mesh);
    free(chunk);
}

// World *readWorld(char *filename) {
//     World *world = createWorld();
//
//
// }

World * createWorld() {
    World *world = malloc(sizeof(World));

    int x, y, z;

    for (x = 0; x < WORLD_SIZE; x++) {
        for (y = 0; y < WORLD_SIZE; y++) {
            for (z = 0; z < WORLD_SIZE; z++) {
                world->chunks[x][y][z] = createChunk(x, y, z);
            }
        }
    }

    #define WORLD_BLOCK_WIDTH WORLD_SIZE * CHUNK_SIZE

    // link neighbors in the world
    for (x = 0; x < WORLD_BLOCK_WIDTH; x++) {
        for (y = 0; y < WORLD_BLOCK_WIDTH; y++) {
            for (z = 0; z < WORLD_BLOCK_WIDTH; z++) {
                Block *block = worldBlock(world, x, y, z);

                block->nb_pos_x = worldBlock(world, x+1, y, z);
                block->nb_neg_x = worldBlock(world, x-1, y, z);
                block->nb_pos_y = worldBlock(world, x, y+1, z);
                block->nb_neg_y = worldBlock(world, x, y-1, z);
                block->nb_pos_z = worldBlock(world, x, y, z+1);
                block->nb_neg_z = worldBlock(world, x, y, z-1);
            }
        }
    }

    #undef WORLD_BLOCK_WIDTH

    return world;
}

void fillWorld(World *world) {
    int cx, cy, cz, bx, by, bz;

    for (cx = 0; cx < WORLD_SIZE; cx++) {
        for (cy = 0; cy < WORLD_SIZE; cy++) {
            for (cz = 0; cz < WORLD_SIZE; cz++) {
                for (bx = 0; bx < CHUNK_SIZE; bx++) {
                    for (by = 0; by < CHUNK_SIZE; by++) {
                        for (bz = 0; bz < CHUNK_SIZE; bz++) {
                            int m = (bx + by + bz) % 2 ? -1 : (-1 << 6);
                            if (by == 0 && cy == 0) {
                                setBlock(world->chunks[cx][cy][cz], bx, by, bz,
                                    (Block){1, {{255 & m, 255 & m, 255 & m, 255}}});
                            } else if (((bx == 0 && cx == 0) || (bz == 0 && cz == 0)) && by == 1 && cy == 0) {
                                setBlock(world->chunks[cx][cy][cz], bx, by, bz,
                                    (Block){1, {{255 & m, 255 & m, 255 & m, 255}}});
                            }
                        }
                    }
                }

                renderChunk(world->chunks[cx][cy][cz]);
            }
        }
    }
}

void drawWorld(World *world, mat4 viewMatrix, mat4 projectionMatrix) {
    int x, y, z;
    Chunk *chunk;

    for (x = 0; x < WORLD_SIZE; x++) {
        for (y = 0; y < WORLD_SIZE; y++) {
            for (z = 0; z < WORLD_SIZE; z++) {
                chunk = world->chunks[x][y][z];
                if (chunk->mesh->size != 0 && isVisible(chunk, viewMatrix, projectionMatrix)) {
                    drawMesh(chunk->mesh);
                }
            }
        }
    }
}

void freeWorld(World *world) {
    int x, y, z;

    for (x = 0; x < WORLD_SIZE; x++) {
        for (y = 0; y < WORLD_SIZE; y++) {
            for (z = 0; z < WORLD_SIZE; z++) {
                freeChunk(world->chunks[x][y][z]);
            }
        }
    }

    free(world);
}

void setBlock(Chunk *chunk, int x, int y, int z, Block block) {
    Block *current = getBlock(chunk, x, y, z);

    block.nb_pos_x = current->nb_pos_x;
    block.nb_neg_x = current->nb_neg_x;
    block.nb_pos_y = current->nb_pos_y;
    block.nb_neg_y = current->nb_neg_y;
    block.nb_pos_z = current->nb_pos_z;
    block.nb_neg_z = current->nb_neg_z;

    *getBlock(chunk, x, y, z) = block;
    renderChunk(chunk);
}

void buildBlockFrame(Mesh *mesh) {
    const GLfloat C0_0 = - 1*BLOCK_WIDTH/32;
    const GLfloat C0_1 =   1*BLOCK_WIDTH/32;
    const GLfloat C1_0 =  31*BLOCK_WIDTH/32;
    const GLfloat C1_1 =  33*BLOCK_WIDTH/32;

    GLfloat points[] = {
        C0_0, C0_0, C0_0, C0_0, C0_0, C0_1, C0_0, C0_1, C0_0, C0_0, C0_1, C0_1,
        C0_1, C0_0, C0_0, C0_1, C0_0, C0_1, C0_1, C0_1, C0_0, C0_1, C0_1, C0_1,
        C0_0, C0_0, C1_0, C0_0, C0_0, C1_1, C0_0, C0_1, C1_0, C0_0, C0_1, C1_1,
        C0_1, C0_0, C1_0, C0_1, C0_0, C1_1, C0_1, C0_1, C1_0, C0_1, C0_1, C1_1,
        C0_0, C1_0, C0_0, C0_0, C1_0, C0_1, C0_0, C1_1, C0_0, C0_0, C1_1, C0_1,
        C0_1, C1_0, C0_0, C0_1, C1_0, C0_1, C0_1, C1_1, C0_0, C0_1, C1_1, C0_1,
        C0_0, C1_0, C1_0, C0_0, C1_0, C1_1, C0_0, C1_1, C1_0, C0_0, C1_1, C1_1,
        C0_1, C1_0, C1_0, C0_1, C1_0, C1_1, C0_1, C1_1, C1_0, C0_1, C1_1, C1_1,
        C1_0, C0_0, C0_0, C1_0, C0_0, C0_1, C1_0, C0_1, C0_0, C1_0, C0_1, C0_1,
        C1_1, C0_0, C0_0, C1_1, C0_0, C0_1, C1_1, C0_1, C0_0, C1_1, C0_1, C0_1,
        C1_0, C0_0, C1_0, C1_0, C0_0, C1_1, C1_0, C0_1, C1_0, C1_0, C0_1, C1_1,
        C1_1, C0_0, C1_0, C1_1, C0_0, C1_1, C1_1, C0_1, C1_0, C1_1, C0_1, C1_1,
        C1_0, C1_0, C0_0, C1_0, C1_0, C0_1, C1_0, C1_1, C0_0, C1_0, C1_1, C0_1,
        C1_1, C1_0, C0_0, C1_1, C1_0, C0_1, C1_1, C1_1, C0_0, C1_1, C1_1, C0_1,
        C1_0, C1_0, C1_0, C1_0, C1_0, C1_1, C1_0, C1_1, C1_0, C1_0, C1_1, C1_1,
        C1_1, C1_0, C1_0, C1_1, C1_0, C1_1, C1_1, C1_1, C1_0, C1_1, C1_1, C1_1,
    };

    GLfloat normals[] = {
        REP_8(REP_8(REP_8(0)))
    };

    GLfloat colors[] = {
        REP_8(REP_8(REP_3(0)))
        /*0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0,
        0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
        1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0*/
    };

    #define IND(x,y) (x * 8 + y)

    #define OUTER_FACE(a, b, c, d) \
            IND(a, a), IND(b, b), IND(b, d), IND(a, a), IND(b, d), IND(a, c), \
            IND(b, d), IND(b, b), IND(c, c), IND(b, d), IND(c, c), IND(c, a), \
            IND(d, b), IND(c, a), IND(c, c), IND(d, b), IND(c, c), IND(d, d), \
            IND(a, a), IND(a, c), IND(d, b), IND(a, a), IND(d, b), IND(d, d)

    #define INNER_FACE(a, b, c, d, e, f, g, h) \
            IND(a, g), IND(b, h), IND(b, e), IND(a, g), IND(b, e), IND(a, f), \
            IND(c, e), IND(c, f), IND(b, g), IND(c, e), IND(b, g), IND(b, h), \
            IND(d, g), IND(c, h), IND(c, e), IND(d, g), IND(c, e), IND(d, f), \
            IND(d, e), IND(d, f), IND(a, g), IND(d, e), IND(a, g), IND(a, h)

    GLuint indices[] = {
        OUTER_FACE(7, 5, 4, 6),
        OUTER_FACE(7, 6, 2, 3),
        OUTER_FACE(7, 3, 1, 5),
        OUTER_FACE(0, 1, 3, 2),
        OUTER_FACE(0, 4, 5, 1),
        OUTER_FACE(0, 2, 6, 4),

        INNER_FACE(7, 5, 4, 6, 3, 1, 0, 2),
        INNER_FACE(7, 6, 2, 3, 5, 4, 0, 1),
        INNER_FACE(7, 3, 1, 5, 6, 2, 0, 4),
        INNER_FACE(0, 1, 3, 2, 4, 5, 7, 6),
        INNER_FACE(0, 4, 5, 1, 2, 6, 7, 3),
        INNER_FACE(0, 2, 6, 4, 1, 3, 7, 5),
    };

    #undef IND
    #undef OUTER_FACE
    #undef INNER_FACE

    buildMesh(mesh, points, normals, colors, NULL, indices,
              sizeof(points), sizeof(normals),
              sizeof(colors), 0, sizeof(indices),
              (sizeof(indices) / sizeof(GLuint)));
}

int isVisible(Chunk *chunk, mat4 view, mat4 perspective) {
    int i;
    int inside[4] = {0, 0, 0, 0};
    int drawable = 0;
    mat4 MVP;

    copy_m4(MVP, chunk->mesh->modelMatrix);
    multiply_m4(MVP, view);
    multiply_m4(MVP, perspective);

    vec4 corners[] = {
        {0.0f,            0.0f,            0.0f,            1.0f},
        {0.0f,            0.0f,            CHUNK_WIDTH,     1.0f},
        {0.0f,            CHUNK_WIDTH,     0.0f,            1.0f},
        {0.0f,            CHUNK_WIDTH,     CHUNK_WIDTH,     1.0f},
        {CHUNK_WIDTH,     0.0f,            0.0f,            1.0f},
        {CHUNK_WIDTH,     0.0f,            CHUNK_WIDTH,     1.0f},
        {CHUNK_WIDTH,     CHUNK_WIDTH,     0.0f,            1.0f},
        {CHUNK_WIDTH,     CHUNK_WIDTH,     CHUNK_WIDTH,     1.0f}
    };

    // loop through each vertex, translate it, and check if it's on-screen.
    for (i = 0; i < 8; i++) {
        multiply_v4_m4(corners[i], MVP);

        if (corners[i][2] > -corners[i][3] && corners[i][2] < corners[i][3]) {

            // some corner is on the visible z-axis
            drawable = 1;

            // there's a corner on-screen. Just draw the thing.
            if (corners[i][0] > -corners[i][3] && corners[i][0] < corners[i][3] &&
                corners[i][1] > -corners[i][3] && corners[i][1] < corners[i][3]) {
                return 1;
            }
        }
    }

    // if there aren't any points on the visible z-axis, there's no way it's visible
    if (!drawable)
        return 0;

    // loop through each vertex, and check where it is
    // in relation to each x and y plane of the frustum.
    // If there's at least one point in front of every plane,
    // then there's a chance the chunk is on-screen
    for (i = 0; i < 8; i++) {
        if (!inside[0] && corners[i][0] > -corners[i][3])
            inside[0]++;
        if (!inside[1] && corners[i][1] > -corners[i][3])
            inside[1]++;
        if (!inside[2] && corners[i][0] < corners[i][3])
            inside[2]++;
        if (!inside[3] && corners[i][1] < corners[i][3])
            inside[3]++;

        // if we have a vertex in front of each plane, we can return
        if (inside[0] && inside[1] && inside[2] && inside[3])
            return 1;
    }

    // since we didn't return from inside the loop,
    // we know the chunk can't be on-screen
    return 0;
}

int solidBlockInArea(World *world, int minx, int miny, int minz, int maxx, int maxy, int maxz) {
    int x, y, z;
    int bx, by, bz, cx, cy, cz;

    for (x = minx; x < maxx; x++) {
        for (y = miny; y < maxy; y++) {
            for (z = minz; z < maxz; z++) {
                bx = x & BLOCK_MASK;
                by = y & BLOCK_MASK;
                bz = z & BLOCK_MASK;
                cx = x >> LOG_CHUNK_SIZE;
                cy = y >> LOG_CHUNK_SIZE;
                cz = z >> LOG_CHUNK_SIZE;

                if (bx >= 0 && by >= 0 && bz >= 0 &&
                    bx < CHUNK_SIZE && by < CHUNK_SIZE && bz < CHUNK_SIZE &&
                    cx >= 0 && cy >= 0 && cz >= 0 &&
                    cx < WORLD_SIZE && cy < WORLD_SIZE && cz < WORLD_SIZE &&
                    getBlock(world->chunks[cx][cy][cz], bx, by, bz)->active)
                    return 1;
            }
        }
    }

    return 0;
}

static float intbound(float s, float ds) {
    if (ds < 0) {
        if (fmod(s, 1) == 0) // edge case
            return 0.0;

        return intbound(-s, -ds);
    } else {
        s -= floor(s);//fmod(fmod(s, 1) + 1, 1);
        return (1.0 - s) / ds;
    }
}

Selection selectBlock(World *world, vec3 position, vec3 direction, float radius) {
    Selection ret;
    ret.selected_active = ret.previous_active = 0;

    int x = floor(position[0]);
    int y = floor(position[1]);
    int z = floor(position[2]);

    float dx = direction[0];
    float dy = direction[1];
    float dz = direction[2];

    int stepX = dx > 0 ? 1 : dx < 0 ? -1 : 0;
    int stepY = dy > 0 ? 1 : dy < 0 ? -1 : 0;
    int stepZ = dz > 0 ? 1 : dz < 0 ? -1 : 0;

    float tMaxX = intbound(position[0], dx);
    float tMaxY = intbound(position[1], dy);
    float tMaxZ = intbound(position[2], dz);

    float tDeltaX = stepX / dx;
    float tDeltaY = stepY / dy;
    float tDeltaZ = stepZ / dz;

    int px = -1, py = -1, pz = -1;

    if (dx == 0 && dy == 0 && dz == 0) return ret;

    while (1) {
        if (!(x < 0 || y < 0 || z < 0 || x >= CHUNK_SIZE * WORLD_SIZE || y >= CHUNK_SIZE * WORLD_SIZE || z >= CHUNK_SIZE * WORLD_SIZE)) {
            ret.selected_chunk_x = x >> LOG_CHUNK_SIZE;
            ret.selected_chunk_y = y >> LOG_CHUNK_SIZE;
            ret.selected_chunk_z = z >> LOG_CHUNK_SIZE;
            ret.selected_block_x = x & BLOCK_MASK;
            ret.selected_block_y = y & BLOCK_MASK;
            ret.selected_block_z = z & BLOCK_MASK;

            // now check if the block is solid
            if (getBlock(world->chunks[ret.selected_chunk_x][ret.selected_chunk_y][ret.selected_chunk_z],
                         ret.selected_block_x, ret.selected_block_y, ret.selected_block_z)->active) {
                ret.selected_active = 1;

                if (px >= 0 && py >= 0 && pz >= 0 &&
                    px < CHUNK_SIZE * WORLD_SIZE &&
                    py < CHUNK_SIZE * WORLD_SIZE &&
                    pz < CHUNK_SIZE * WORLD_SIZE) {

                    ret.previous_chunk_x = px >> LOG_CHUNK_SIZE;
                    ret.previous_chunk_y = py >> LOG_CHUNK_SIZE;
                    ret.previous_chunk_z = pz >> LOG_CHUNK_SIZE;
                    ret.previous_block_x = px & BLOCK_MASK;
                    ret.previous_block_y = py & BLOCK_MASK;
                    ret.previous_block_z = pz & BLOCK_MASK;
                    ret.previous_active = 1;
                }

                break;
            }
        }

        px = x;
        py = y;
        pz = z;

        if (tMaxX < tMaxY) {
            if (tMaxX < tMaxZ) {
                if (tMaxX > radius) break;
                x += stepX;
                tMaxX += tDeltaX;
            } else {
                if (tMaxZ > radius) break;
                z += stepZ;
                tMaxZ += tDeltaZ;
            }
        } else {
            if (tMaxY < tMaxZ) {
                if (tMaxY > radius) break;
                y += stepY;
                tMaxY += tDeltaY;
            } else {
                if (tMaxZ > radius) break;
                z += stepZ;
                tMaxZ += tDeltaZ;
            }
        }
    }

    return ret;
}

Block* selectedBlock(World *world, Selection* selection) {
    return &world->chunks[selection->selected_chunk_x][selection->selected_chunk_y][selection->selected_chunk_z]
                 ->blocks[selection->selected_block_x][selection->selected_block_y][selection->selected_block_z];
}

Block* worldBlock(World *world, int x, int y, int z) {
    #define WORLD_BLOCK_WIDTH WORLD_SIZE * CHUNK_SIZE
    if (x < 0 || y < 0 || z < 0 || x >= WORLD_BLOCK_WIDTH || y >= WORLD_BLOCK_WIDTH || z >= WORLD_BLOCK_WIDTH)
        return NULL;

    return &world->chunks[x >> LOG_CHUNK_SIZE][y >> LOG_CHUNK_SIZE][z >> LOG_CHUNK_SIZE]
                 ->blocks[x & BLOCK_MASK][y & BLOCK_MASK][z & BLOCK_MASK];
    #undef WORLD_BLOCK_WIDTH
}

static void getFaceData(const GLfloat *dest, const GLfloat *src, const GLuint *indices) {
    memcpy((void*)&dest[0*3], (void*)&src[indices[0]*3], 3 * sizeof(GLfloat));
    memcpy((void*)&dest[1*3], (void*)&src[indices[1]*3], 3 * sizeof(GLfloat));
    memcpy((void*)&dest[2*3], (void*)&src[indices[2]*3], 3 * sizeof(GLfloat));
    memcpy((void*)&dest[3*3], (void*)&src[indices[3]*3], 3 * sizeof(GLfloat));
    memcpy((void*)&dest[4*3], (void*)&src[indices[4]*3], 3 * sizeof(GLfloat));
    memcpy((void*)&dest[5*3], (void*)&src[indices[5]*3], 3 * sizeof(GLfloat));
}

#undef BLOCK_MASK
