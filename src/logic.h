#ifndef LOGIC_H_
#define LOGIC_H_

#include "voxels.h"

#define NUM_GATES 13

typedef struct Logic_S {
    unsigned int type:4;
    unsigned int yaw:2;
    unsigned int pitch:2;
    unsigned int roll:2;
    union {
        struct {
            unsigned int neg_z:1;
            unsigned int neg_y:1;
            unsigned int neg_x:1;
            unsigned int pos_z:1;
            unsigned int pos_y:1;
            unsigned int pos_x:1;
        };
        unsigned int all:6;
    } input, output;
    unsigned int auto_orient:1;
    mat4 *rotationMatrix;
} Logic;

void initLogicModels();
void freeLogicModels();
void autoOrient(Block *block);
void runLogicThread(World *world);
void stopLogicThread();

#endif
