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
//     0x0101404001014040, // (wire 0)  - single
//     0x012340628901C840, // (wire 1)  - bridge
//     0x0123456789ABCDE0, // (wire 2)  - connect-4
//     0x010145458989C0C0, // (wire 3)  - connect-3
//     0x0000444488880000, // (wire 4)  - single L
//     0x0120456489A80120, // (wire 5)  - double L
//     0x0101000001010000, // (wire 6)  - diode
//     0x0000010100000101, // (wire 7)  - not
//     0x0100000000000000, // (wire 8)  - and
//     0x0101010101010100, // (wire 9)  - or
//     0x1111111111111111, // (wire 10) - constant 1
// };

/*
 * Okay so I know you're going to forget what the hell these numbers mean,
 * so here's how they work:
 *
 * Each hex digit corresponds to one possible state of inputs.
 * The highest order digit corresponds to all inputs on,
 * while the lowest order digit corresponds to all inputs off.
 *
 * The bits in each digit correspond to the resulting outputs.
 * The ordering of bits goes this way:
 * +x +z -x -z
 * (Note that this applies for both input and outputs)
 */

// lots of magic numbers.. :)
static unsigned long int outputs[NUM_GATES] = {
    0x5151404051514040, // wire (0)  - single
    0xFB73EA62D951C840, // wire (1)  - bridge
    0xFFFFFFF7FFFBFDE0, // wire (2)  - connect-4
    0xDDDDD5D5D9D9C0C0, // wire (3)  - connect-3
    0xCCCC444488880000, // wire (4)  - single L
    0xFDEC7564B9A83120, // wire (5)  - double L
    0x1111000011110000, // gate (6)  - diode
    0x0000111100001111, // gate (7)  - not
    0x1100000000000000, // gate (8)  - and
    0x1111111111111100, // gate (9)  - or
    0x1100001100111100, // gate (10) - xor
    0x1111111111111111, // gate (11) - constant 1
};

static Model *logic_models[NUM_GATES][16][4];

static pthread_t thread;
static int quitThread = 0;

void initLogicModels() {
    char *fname = malloc(40);
    Model *model;

    for (int i=0; i < NUM_GATES; i++) {
        for (int j=0; j < 16; j++) {
            sprintf(fname, "models/%d/%d%d%d%d", i, (j>>3)&1, (j>>2)&1, (j>>1)&1, j&1);
            model = createModel();
            readModel(model, fname);
            logic_models[i][j][0] = rotateModel(model, 0);
            logic_models[i][j][1] = rotateModel(model, 1);
            logic_models[i][j][2] = rotateModel(model, 2);
            logic_models[i][j][3] = rotateModel(model, 3);
        }
    }

    free(fname);
}

void freeLogicModels() {
    for (int i=0; i < NUM_GATES; i++) {
        for (int j=0; j < 16; j++) {
            freeModel(logic_models[i][j][0]);
            freeModel(logic_models[i][j][1]);
            freeModel(logic_models[i][j][2]);
            freeModel(logic_models[i][j][3]);
        }
    }
}

void calculateLogic(World* world, int x, int y, int z) {
    Chunk *chunk = world->chunks[x >> LOG_CHUNK_SIZE][y >> LOG_CHUNK_SIZE][z >> LOG_CHUNK_SIZE];
    Block *block = getBlock(chunk, x&(CHUNK_SIZE-1), y&(CHUNK_SIZE-1), z&(CHUNK_SIZE-1));

    if (block->logic) {
        int type = block->logic->type & 0xF;
        int turn = block->logic->rotation & 0x3;

        int input = block->logic->input.all & 0xF;
        input = (input >> turn) | ((input << (4 - turn)) & 0xF);

        int output = (outputs[type] >> (input << 2)) & 0xF;
        output = (output >> (4 - turn)) | ((output << turn) & 0xF);

        block->logic->output.all = output;
        if (block->data != (block->data = logic_models[type][input][turn]))
            chunk->needsUpdate = 1;
    }
}

void advanceLogic(World* world, int x, int y, int z) {
    Block *block = worldBlock(world, x, y, z);

    if (block->logic) {
        Block *pos_x = worldBlock(world, x+1, y, z);
        Block *neg_x = worldBlock(world, x-1, y, z);
        Block *pos_z = worldBlock(world, x, y, z+1);
        Block *neg_z = worldBlock(world, x, y, z-1);

        // if (pos_x && pos_x->logic) pos_x->logic->input.neg_x = block->logic->output.pos_x
        // if (neg_x && neg_x->logic) neg_x->logic->input.pos_x = block->logic->output.neg_x
        // if (pos_z && pos_z->logic) pos_z->logic->input.neg_z = block->logic->output.pos_z
        // if (neg_z && neg_z->logic) neg_z->logic->input.pos_z = block->logic->output.neg_z

        block->logic->input.pos_x = (pos_x && pos_x->logic && pos_x->logic->output.neg_x);
        block->logic->input.neg_x = (neg_x && neg_x->logic && neg_x->logic->output.pos_x);
        block->logic->input.pos_z = (pos_z && pos_z->logic && pos_z->logic->output.neg_z);
        block->logic->input.neg_z = (neg_z && neg_z->logic && neg_z->logic->output.pos_z);
    }
}

void logicLoop(World *world) {
    int x, y, z;
    int world_size = WORLD_SIZE * CHUNK_SIZE;

    while (!quitThread) {
        // usleep(1000);

        for (x = 0; x < world_size; x++) {
            for (y = 0; y < world_size; y++) {
                for (z = 0; z < world_size; z++) {
                    calculateLogic(world, x, y, z);
                }
            }
        }

        for (x = 0; x < world_size; x++) {
            for (y = 0; y < world_size; y++) {
                for (z = 0; z < world_size; z++) {
                    advanceLogic(world, x, y, z);
                }
            }
        }
    }
}

void runLogicThread(World *world) {
    quitThread = 0;

    pthread_create(&thread, NULL, logicLoop, world);
}

void stopLogicThread() {
    quitThread = 1;

    pthread_join(thread, NULL);
}
