#ifndef LOGIC_H_
#define LOGIC_H_

#include "voxels.h"

#define NUM_GATES 12

typedef struct Logic_S {
    unsigned int type:4;
    unsigned int rotation:2;
    union {
        struct {
            unsigned int neg_z:1;
            unsigned int neg_x:1;
            unsigned int pos_z:1;
            unsigned int pos_x:1;
        };
        unsigned int all:4;
    } input, output;
} Logic;

void initLogicModels();
void freeLogicModels();
void runLogicThread(World *world);
void stopLogicThread();

#endif
