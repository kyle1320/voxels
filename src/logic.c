#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#include "voxels.h"
#include "model.h"
#include "logic.h"
#include "matrix.h"

// lots of magic numbers.. :)
// static unsigned long int outputs[NUM_GATES] = {
//     0x5151404051514040, // wire (0)  - single
//     0xFB73EA62D951C840, // wire (1)  - bridge
//     0xFFFFFFF7FFFBFDE0, // wire (2)  - connect-4
//     0xDDDDD5D5D9D9C0C0, // wire (3)  - connect-3
//     0xCCCC444488880000, // wire (4)  - single L
//     0xFDEC7564B9A83120, // wire (5)  - double L
//     0x1111000011110000, // gate (6)  - diode
//     0x0000111100001111, // gate (7)  - not
//     0x1100000000000000, // gate (8)  - and
//     0x1111111111111100, // gate (9)  - or
//     0x1100001100111100, // gate (10) - xor
//     0x1111111111111111, // gate (11) - constant 1
// };

/*
 * Okay so I know you're going to forget what the hell these numbers mean,
 * so here's how they work:
 *
 * Each array corresponds to one gate.
 *
 * Each element in an array corresponds to one possible state of inputs.
 * The last element corresponds to all inputs on,
 * while the first element corresponds to all inputs off.
 *
 * The bits in each element correspond to the resulting outputs.
 * The ordering of bits goes this way:
 * +x +y +z -x -y -z
 * (Note that this applies for both input and outputs)
 */

static unsigned char outputs[NUM_GATES][64] = {
    {0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x01,0x09,0x01,0x09,0x01,0x09,0x01,0x09,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x01,0x09,0x01,0x09,0x01,0x09,0x01,0x09,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x01,0x09,0x01,0x09,0x01,0x09,0x01,0x09,0x00,0x08,0x00,0x08,0x00,0x08,0x00,0x08,0x01,0x09,0x01,0x09,0x01,0x09,0x01,0x09,}, // wire (0)  - single
    {0x00,0x08,0x00,0x08,0x20,0x28,0x20,0x28,0x01,0x09,0x01,0x09,0x21,0x29,0x21,0x29,0x00,0x08,0x00,0x08,0x20,0x28,0x20,0x28,0x01,0x09,0x01,0x09,0x21,0x29,0x21,0x29,0x04,0x0C,0x04,0x0C,0x24,0x2C,0x24,0x2C,0x05,0x0D,0x05,0x0D,0x25,0x2D,0x25,0x2D,0x04,0x0C,0x04,0x0C,0x24,0x2C,0x24,0x2C,0x05,0x0D,0x05,0x0D,0x25,0x2D,0x25,0x2D,}, // wire (1)  - bridge
    {0x00,0x2C,0x00,0x2C,0x29,0x2D,0x29,0x2D,0x25,0x2D,0x25,0x2D,0x2D,0x2D,0x2D,0x2D,0x00,0x2C,0x00,0x2C,0x29,0x2D,0x29,0x2D,0x25,0x2D,0x25,0x2D,0x2D,0x2D,0x2D,0x2D,0x0D,0x2D,0x0D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x0D,0x2D,0x0D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,0x2D,}, // wire (2)  - connect-4
    {0x00,0x28,0x00,0x28,0x00,0x28,0x00,0x28,0x21,0x29,0x21,0x29,0x21,0x29,0x21,0x29,0x00,0x28,0x00,0x28,0x00,0x28,0x00,0x28,0x21,0x29,0x21,0x29,0x21,0x29,0x21,0x29,0x09,0x29,0x09,0x29,0x09,0x29,0x09,0x29,0x29,0x29,0x29,0x29,0x29,0x29,0x29,0x29,0x09,0x29,0x09,0x29,0x09,0x29,0x09,0x29,0x29,0x29,0x29,0x29,0x29,0x29,0x29,0x29,}, // wire (3)  - connect-3
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x08,0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,}, // wire (4)  - single L
    {0x00,0x04,0x00,0x04,0x01,0x05,0x01,0x05,0x20,0x24,0x20,0x24,0x21,0x25,0x21,0x25,0x00,0x04,0x00,0x04,0x01,0x05,0x01,0x05,0x20,0x24,0x20,0x24,0x21,0x25,0x21,0x25,0x08,0x0C,0x08,0x0C,0x09,0x0D,0x09,0x0D,0x28,0x2C,0x28,0x2C,0x29,0x2D,0x29,0x2D,0x08,0x0C,0x08,0x0C,0x09,0x0D,0x09,0x0D,0x28,0x2C,0x28,0x2C,0x29,0x2D,0x29,0x2D,}, // wire (5)  - double L
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,}, // gate (6)  - diode
    {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,}, // gate (7)  - not
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,}, // gate (8)  - and
    {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,}, // gate (9)  - or
    {0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x01,}, // gate (10) - xor
    {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,}, // gate (11) - constant 1
    {0x00,0x20,0x00,0x20,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x00,0x20,0x00,0x20,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,0x01,0x21,}, // wire (12) - bidirectional connect-3
    {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,}, // wire (13) - color output
};

static unsigned char input_faces[NUM_GATES] = {
    0x09, 0x2D, 0x2D, 0x29, 0x28, 0x2D, 0x08, 0x08, 0x2C, 0x2C, 0x2C, 0x00, 0x29, 0x2C
};

static unsigned char output_faces[NUM_GATES] = {
    0x09, 0x2D, 0x2D, 0x29, 0x28, 0x2D, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x21, 0x00
};

int showLogic = 1;

static Model *logic_models[NUM_GATES][64];
static mat4 rotation_matrices[4][4][4];

static pthread_t thread;
static int quitThread = 0;

void initLogicModels() {
    char *fname = malloc(40);
    Model *model;

    World *world = readWorld("worlds/gates");

    // int i, j, x, y, z, r, g, b, p;
    int i, j, r, p, y;

    for (i=0; i < NUM_GATES; i++) {
        for (j=0; j < 64; j++) {
            model = createModel();
            model->chunk = world->chunks[(i * 64) + j];
            // for (x=0; x < CHUNK_SIZE; x++) {
            //     for (y=0; y < CHUNK_SIZE; y++) {
            //         for (z=0; z < CHUNK_SIZE; z++) {
            //             Block *blk = getBlock(model->chunk, x, y, z);
            //             if (blk->color.r == 0x42 && blk->color.g == 0xFF && blk->color.b == 0x09)
            //                 blk->color.all = 0xFF00FF00;
            //         }
            //     }
            // }
            renderModel(model);

            logic_models[i][j] = model;
        }
    }

    // Block block;
    //
    // for (i=0; i < 64; i++) {
    //     model = createModel();
    //
    //     r = (i >> 2) & 1;
    //     g = (i >> 3) & 1;
    //     b = (i >> 5) & 1;
    //
    //     for (x = 0; x < 1; x++) {
    //         block = (Block){1, (Color){{255*(!r), 255*r, 0, 255}}, NULL, NULL};
    //         setBlock(model->chunk, x, 7, 7, block);
    //         setBlock(model->chunk, x, 7, 8, block);
    //         setBlock(model->chunk, x, 8, 7, block);
    //         setBlock(model->chunk, x, 8, 8, block);
    //
    //         block = (Block){1, (Color){{255*(!g), 255*g, 0, 255}}, NULL, NULL};
    //         setBlock(model->chunk, 7, 7, (CHUNK_SIZE-x-1), block);
    //         setBlock(model->chunk, 7, 8, (CHUNK_SIZE-x-1), block);
    //         setBlock(model->chunk, 8, 7, (CHUNK_SIZE-x-1), block);
    //         setBlock(model->chunk, 8, 8, (CHUNK_SIZE-x-1), block);
    //
    //         block = (Block){1, (Color){{255*(!b), 255*b, 0, 255}}, NULL, NULL};
    //         setBlock(model->chunk, (CHUNK_SIZE-x-1), 7, 7, block);
    //         setBlock(model->chunk, (CHUNK_SIZE-x-1), 7, 8, block);
    //         setBlock(model->chunk, (CHUNK_SIZE-x-1), 8, 7, block);
    //         setBlock(model->chunk, (CHUNK_SIZE-x-1), 8, 8, block);
    //     }
    //
    //     for (x=1; x < CHUNK_SIZE-1; x++) {
    //         for (y=1; y < CHUNK_SIZE-1; y++) {
    //             for (z=1; z < CHUNK_SIZE-1; z++) {
    //                 block = (Block){1, (Color){{255*r, 255*g, 255*b, 255}}, NULL, NULL};
    //                 setBlock(model->chunk, x, y, z, block);
    //             }
    //         }
    //     }
    //
    //     world->chunks[((NUM_GATES - 1) * 64) + i] = model->chunk;
    //
    //     // freeModel(model);
    // }
    //
    // writeWorld(world, "worlds/gates");

    free(world->chunks);
    free(world);

    // // World *world = createWorld(10);
    //
    // for (int i=0; i < NUM_GATES; i++) {
    //     for (int j=0; j < 64; j++) {
    //         sprintf(fname, "models/%d/%d%d%d%d%d%d", i, (j>>5)&1, (j>>4)&1, (j>>3)&1, (j>>2)&1, (j>>1)&1, j&1);
    //         model = createModel();
    //         readModel(model, fname);
    //
    //         // world->chunks[(i * 64) + j] = model->chunk;
    //
    //         // char *fname2 = malloc(40);
    //         // sprintf(fname2, "chunks/%d/%d%d%d%d%d%d", i, (j>>5)&1, (j>>4)&1, (j>>3)&1, (j>>2)&1, (j>>1)&1, j&1);
    //         // FILE *out = fopen(fname2, "wb");
    //         //
    //         // writeChunk(model->chunk, out);
    //         //
    //         // fclose(out);
    //         // free(fname2);
    //
    //         logic_models[i][j] = model;
    //     }
    // }
    //
    // // writeWorld(world, "worlds/logic_models");

    for (r = 0; r < 4; r++) {
        for (p = 0; p < 4; p++) {
            for (y = 0; y < 4; y++) {
                rotation_Z_m4(rotation_matrices[r][p][y], (r * 1.5707963268));
                rotate_X_m4(rotation_matrices[r][p][y], (p * 1.5707963268));
                rotate_Y_m4(rotation_matrices[r][p][y], (y * 1.5707963268));
            }
        }
    }

    free(fname);
}

Model *getLogicModel(int type, int inputs) {
    return logic_models[type][inputs];
}

void freeLogicModels() {
    for (int i=0; i < NUM_GATES; i++) {
        for (int j=0; j < 64; j++) {
            freeModel(logic_models[i][j]);
        }
    }
}

unsigned int rotate_inputs(unsigned int input, int roll, int pitch, int yaw) {
    int turn;

    input = input & 0x3F;
    for (turn = 0; turn < yaw; turn++)
        input = ((input>>2)&0x9)|((input>>1)&0x4)|((input<<5)&0x20)|(input&0x12);
    for (turn = 0; turn < pitch; turn++)
        input = ((input<<2)&0x8)|((input<<1)&0x12)|((input>>4)&0x1)|(input&0x24);
    for (turn = 0; turn < roll; turn++)
        input = ((input<<2)&0x10)|((input<<1)&0x24)|((input>>4)&0x2)|(input&0x9);
    return input;
}

unsigned int rotate_outputs(unsigned int output, int roll, int pitch, int yaw) {
    int turn;

    for (turn = 0; turn < roll; turn++)
        output = ((output>>2)&0x4)|((output>>1)&0x12)|((output<<4)&0x20)|(output&0x9);
    for (turn = 0; turn < pitch; turn++)
        output = ((output>>2)&0x2)|((output>>1)&0x9)|((output<<4)&0x10)|(output&0x24);
    for (turn = 0; turn < yaw; turn++)
        output = ((output<<2)&0x24)|((output<<1)&0x8)|((output>>5)&0x1)|(output&0x12);
    return output;
}

void autoOrient(Block *block) {
    if (block->logic) {
        unsigned int target_inputs =
            ((block->nb_neg_z && block->nb_neg_z->logic && (rotate_outputs(output_faces[block->nb_neg_z->logic->type], block->nb_neg_z->logic->roll, block->nb_neg_z->logic->pitch, block->nb_neg_z->logic->yaw) >> 3) & 1) << 0) |
            ((block->nb_neg_y && block->nb_neg_y->logic && (rotate_outputs(output_faces[block->nb_neg_y->logic->type], block->nb_neg_y->logic->roll, block->nb_neg_y->logic->pitch, block->nb_neg_y->logic->yaw) >> 4) & 1) << 1) |
            ((block->nb_neg_x && block->nb_neg_x->logic && (rotate_outputs(output_faces[block->nb_neg_x->logic->type], block->nb_neg_x->logic->roll, block->nb_neg_x->logic->pitch, block->nb_neg_x->logic->yaw) >> 5) & 1) << 2) |
            ((block->nb_pos_z && block->nb_pos_z->logic && (rotate_outputs(output_faces[block->nb_pos_z->logic->type], block->nb_pos_z->logic->roll, block->nb_pos_z->logic->pitch, block->nb_pos_z->logic->yaw) >> 0) & 1) << 3) |
            ((block->nb_pos_y && block->nb_pos_y->logic && (rotate_outputs(output_faces[block->nb_pos_y->logic->type], block->nb_pos_y->logic->roll, block->nb_pos_y->logic->pitch, block->nb_pos_y->logic->yaw) >> 1) & 1) << 4) |
            ((block->nb_pos_x && block->nb_pos_x->logic && (rotate_outputs(output_faces[block->nb_pos_x->logic->type], block->nb_pos_x->logic->roll, block->nb_pos_x->logic->pitch, block->nb_pos_x->logic->yaw) >> 2) & 1) << 5);
        unsigned int target_outputs =
            ((block->nb_neg_z && block->nb_neg_z->logic && (rotate_outputs(input_faces[block->nb_neg_z->logic->type], block->nb_neg_z->logic->roll, block->nb_neg_z->logic->pitch, block->nb_neg_z->logic->yaw) >> 3) & 1) << 0) |
            ((block->nb_neg_y && block->nb_neg_y->logic && (rotate_outputs(input_faces[block->nb_neg_y->logic->type], block->nb_neg_y->logic->roll, block->nb_neg_y->logic->pitch, block->nb_neg_y->logic->yaw) >> 4) & 1) << 1) |
            ((block->nb_neg_x && block->nb_neg_x->logic && (rotate_outputs(input_faces[block->nb_neg_x->logic->type], block->nb_neg_x->logic->roll, block->nb_neg_x->logic->pitch, block->nb_neg_x->logic->yaw) >> 5) & 1) << 2) |
            ((block->nb_pos_z && block->nb_pos_z->logic && (rotate_outputs(input_faces[block->nb_pos_z->logic->type], block->nb_pos_z->logic->roll, block->nb_pos_z->logic->pitch, block->nb_pos_z->logic->yaw) >> 0) & 1) << 3) |
            ((block->nb_pos_y && block->nb_pos_y->logic && (rotate_outputs(input_faces[block->nb_pos_y->logic->type], block->nb_pos_y->logic->roll, block->nb_pos_y->logic->pitch, block->nb_pos_y->logic->yaw) >> 1) & 1) << 4) |
            ((block->nb_pos_x && block->nb_pos_x->logic && (rotate_outputs(input_faces[block->nb_pos_x->logic->type], block->nb_pos_x->logic->roll, block->nb_pos_x->logic->pitch, block->nb_pos_x->logic->yaw) >> 2) & 1) << 5);
        // unsigned int neighbors =
        //     ((block->nb_neg_z && block->nb_neg_z->logic) << 0) |
        //     ((block->nb_neg_y && block->nb_neg_y->logic) << 1) |
        //     ((block->nb_neg_x && block->nb_neg_x->logic) << 2) |
        //     ((block->nb_pos_z && block->nb_pos_z->logic) << 3) |
        //     ((block->nb_pos_y && block->nb_pos_y->logic) << 4) |
        //     ((block->nb_pos_x && block->nb_pos_x->logic) << 5);
        int i, r, p, y, br = 0, bp = 0, by = 0, max = 0, count;
        unsigned int hit_inputs, hit_outputs;
        for (r = 0; r < 4; r++) {
            for (p = 0; p < 4; p++) {
                for (y = 0; y < 4; y++) {
                    count = 0;

                    hit_inputs = ~(rotate_outputs(input_faces[block->logic->type], r, p, y) ^ target_inputs);
                    for (i=0; i < 6; i++)
                        count += (hit_inputs >> i) & 1;

                    hit_outputs = ~(rotate_outputs(output_faces[block->logic->type], r, p, y) ^ target_outputs);
                    for (i=0; i < 6; i++)
                        count += (hit_outputs >> i) & 1;

                    if (count > max) {
                        br = r; bp = p; by = y;
                        max = count;
                    }
                }
            }
        }
        block->logic->roll = br;
        block->logic->pitch = bp;
        block->logic->yaw = by;
    }
}

void updateLogicModel(Block *block) {
    if (block->logic) {
        unsigned int input = rotate_inputs(block->logic->input.all, block->logic->roll, block->logic->pitch, block->logic->yaw);
        block->data = logic_models[block->logic->type][input];
        block->logic->rotationMatrix = &rotation_matrices[block->logic->roll][block->logic->pitch][block->logic->yaw];
    }
}

#define BLOCKS_PER_CHUNK CHUNK_SIZE * CHUNK_SIZE * CHUNK_SIZE

static void updateChunkLogic(Chunk *chunk, Block ***logicBlocks, int *count, int *max_count) {
    int i, j, k, type, input, output;
    Block *block;

    // don't bother looping if there aren't any blocks.
    if (chunk->mesh->size) {
        for (i = 0; i < BLOCKS_PER_CHUNK; i++) {
            block = &chunk->blocks_lin[i];

            // calculate logic
            if (block->logic) {

                // we only allocate as much space as we need.
                if (*count >= *max_count) {
                    *max_count = *count + BLOCKS_PER_CHUNK;
                    *logicBlocks = realloc(*logicBlocks, *max_count * sizeof(Block*));
                }

                (*logicBlocks)[(*count)++] = block;

                type = block->logic->type & 0xF;

                input = rotate_inputs(block->logic->input.all, block->logic->roll, block->logic->pitch, block->logic->yaw);
                output = rotate_outputs(outputs[type][input], block->logic->roll, block->logic->pitch, block->logic->yaw);

                block->logic->output.all = output;
                if (showLogic || type == 13) {
                    if (block->data != (block->data = logic_models[type][input]))
                        chunk->needsUpdate = 1;
                }
                if (block->logic->rotationMatrix != (block->logic->rotationMatrix = &rotation_matrices[block->logic->roll][block->logic->pitch][block->logic->yaw])) {
                    chunk->needsUpdate = 1;
                }
            }
            // else if (block->data) {
            //     // int x, y, z;
            //     // if (block->nb_pos_x && block->nb_pos_x->data)
            //     //     for (y = 0; y < CHUNK_SIZE; y++)
            //     //         for (z = 0; z < CHUNK_SIZE; z++)
            //     //             getBlock(block->data->chunk, CHUNK_SIZE-1, y, z)->nb_pos_x = getBlock(block->nb_pos_x->data->chunk, 0, y, z);
            //     // if (block->nb_neg_x && block->nb_neg_x->data)
            //     //     for (y = 0; y < CHUNK_SIZE; y++)
            //     //         for (z = 0; z < CHUNK_SIZE; z++)
            //     //             getBlock(block->data->chunk, 0, y, z)->nb_neg_x = getBlock(block->nb_neg_x->data->chunk, CHUNK_SIZE-1, y, z);
            //     // if (block->nb_pos_y && block->nb_pos_y->data)
            //     //     for (x = 0; x < CHUNK_SIZE; x++)
            //     //         for (z = 0; z < CHUNK_SIZE; z++)
            //     //             getBlock(block->data->chunk, x, CHUNK_SIZE-1, z)->nb_pos_y = getBlock(block->nb_pos_y->data->chunk, x, 0, z);
            //     // if (block->nb_neg_y && block->nb_neg_y->data)
            //     //     for (x = 0; x < CHUNK_SIZE; x++)
            //     //         for (z = 0; z < CHUNK_SIZE; z++)
            //     //             getBlock(block->data->chunk, x, 0, z)->nb_neg_y = getBlock(block->nb_neg_y->data->chunk, x, CHUNK_SIZE-1, z);
            //     // if (block->nb_pos_z && block->nb_pos_z->data)
            //     //     for (x = 0; x < CHUNK_SIZE; x++)
            //     //         for (y = 0; y < CHUNK_SIZE; y++)
            //     //             getBlock(block->data->chunk, x, y, CHUNK_SIZE-1)->nb_pos_z = getBlock(block->nb_pos_z->data->chunk, x, y, 0);
            //     // if (block->nb_neg_z && block->nb_neg_z->data)
            //     //     for (x = 0; x < CHUNK_SIZE; x++)
            //     //         for (y = 0; y < CHUNK_SIZE; y++)
            //     //             getBlock(block->data->chunk, x, y, 0)->nb_neg_z = getBlock(block->nb_neg_z->data->chunk, x, y, CHUNK_SIZE-1);
            //     updateChunkLogic(block->data->chunk, logicBlocks, count, max_count);
            //     if (block->data->chunk->needsUpdate)
            //         chunk->needsUpdate = 1;
            // }
        }
    }
}

void logicLoop(World *world) {
    unsigned int i;
    Block *block;

    unsigned int num_chunks = world->size * world->size * world->size;
    // unsigned int num_blocks = num_chunks * BLOCKS_PER_CHUNK;

    int count;
    int max_count = BLOCKS_PER_CHUNK;
    Block **logicBlocks = malloc(max_count * sizeof(Block*));

    while (!quitThread) {
        // usleep(1000);

        count = 0;

        for (i = 0; i < num_chunks; i++) {
            updateChunkLogic(world->chunks[i], &logicBlocks, &count, &max_count);
        }

        // advance logic (send updated outputs)
        for (i = 0; i < count; i++) {
            block = logicBlocks[i];

            if (block->logic) {
                block->logic->input.pos_x = (block->nb_pos_x && block->nb_pos_x->logic && block->nb_pos_x->logic->output.neg_x);
                block->logic->input.neg_x = (block->nb_neg_x && block->nb_neg_x->logic && block->nb_neg_x->logic->output.pos_x);
                block->logic->input.pos_y = (block->nb_pos_y && block->nb_pos_y->logic && block->nb_pos_y->logic->output.neg_y);
                block->logic->input.neg_y = (block->nb_neg_y && block->nb_neg_y->logic && block->nb_neg_y->logic->output.pos_y);
                block->logic->input.pos_z = (block->nb_pos_z && block->nb_pos_z->logic && block->nb_pos_z->logic->output.neg_z);
                block->logic->input.neg_z = (block->nb_neg_z && block->nb_neg_z->logic && block->nb_neg_z->logic->output.pos_z);
            }
        }
    }

    free(logicBlocks);

    #undef BLOCKS_PER_CHUNK
}

void runLogicThread(World *world) {
    quitThread = 0;

    pthread_create(&thread, NULL, logicLoop, world);
}

void stopLogicThread() {
    quitThread = 1;

    pthread_join(thread, NULL);
}
