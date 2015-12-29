#include "matrix.h"
#include "voxels.h"

typedef struct AABB_S {
    vec3 min;
    vec3 size;
} AABB;

typedef struct Player_S {
    vec3 position;
    vec3 direction;
    vec3 velocity;

    float horizontalAngle;
    float verticalAngle;

    AABB bounding_box;
} Player;

Player *createPlayer(World *world);
void freePlayer(Player *player);

void movePlayer(Player *player, World *world, int fwd_b, int bwd_b, int left_b, int right_b, int up_b, int down_b, int inertia);

void collidePlayer(Player *player, World *world);
