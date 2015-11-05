#include <stdio.h>
#include <math.h>

#include "physics.h"

#define HORIZONTAL_MOVE_SPEED 0.3
#define VERTICAL_MOVE_SPEED 0.1
#define MIN_SPEED 0.0001

extern double deltaTime;

vec3 gravity = {0, -3 * CHUNK_WIDTH, 0};
vec3 movementDecay = {0.7, 1, 0.7};

void movePlayer(Player *player, World *world, int fwd_b, int bwd_b, int left_b, int right_b, int up_b, int down_b, int inertia) {
	vec3 fwd = {
		cos(player->horizontalAngle + PI/2) * HORIZONTAL_MOVE_SPEED,
		0,
		sin(player->horizontalAngle + PI/2) * HORIZONTAL_MOVE_SPEED
	};

	vec3 right = {
		cos(player->horizontalAngle) * HORIZONTAL_MOVE_SPEED,
		0,
		sin(player->horizontalAngle) * HORIZONTAL_MOVE_SPEED
	};

	vec3 up = {0, VERTICAL_MOVE_SPEED, 0};

	vec3 movement = {0, 0, 0};
	vec3 force = {0, 0, 0};

	translate_v3v(force, gravity);
	scale_v3(force, deltaTime);

	if (fwd_b)
		translate_v3v(movement, fwd);
	if (bwd_b)
		subtract_v3v(movement, fwd);
	if (left_b)
		subtract_v3v(movement, right);
	if (right_b)
		translate_v3v(movement, right);
	if (up_b)
		translate_v3v(movement, up);
	if (down_b)
		subtract_v3v(movement, up);

	if (inertia) {
		multiply_v3(player->velocity, movementDecay);

		translate_v3v(player->velocity, force);
	} else {
		zero_v3(player->velocity);

		// force / (1 - decay) = mov[i] / (1 - decay[i])
		// the middle value doesn't work for this, because there's no speed limit in the y direction.
		// so I just did 3.333 * (HORIZONTAL_MOVE_SPEED / VERTICAL_MOVE_SPEED) to make it even
		scale_v3f(movement, 3.333, 3.333 * (HORIZONTAL_MOVE_SPEED / VERTICAL_MOVE_SPEED), 3.333);
	}

	translate_v3v(player->velocity, movement);

	if (fabsf(player->velocity[0]) < MIN_SPEED) player->velocity[0] = 0;
	if (fabsf(player->velocity[1]) < MIN_SPEED) player->velocity[1] = 0;
	if (fabsf(player->velocity[2]) < MIN_SPEED) player->velocity[2] = 0;

	collidePlayer(player, world);

	translate_v3f(player->position, VALUES(deltaTime * player->velocity));
}

void collidePlayer(Player *player, World *world) {
	vec3 minp, maxp;

	#define EPSILON 0.000001

	int minv, maxv;
	for (int i=2; i >= 0; i--) {
		if (player->velocity[i] != 0) {
			copy_v3(minp, player->position);
			translate_v3v(minp, player->bounding_box.min);
			copy_v3(maxp, minp);
			translate_v3v(maxp, player->bounding_box.size);

			ivec3 min = {floor(minp[0]/BLOCK_WIDTH), floor(minp[1]/BLOCK_WIDTH), floor(minp[2]/BLOCK_WIDTH)};
			ivec3 max = {floor(maxp[0]/BLOCK_WIDTH), floor(maxp[1]/BLOCK_WIDTH), floor(maxp[2]/BLOCK_WIDTH)};

			// the only difference between these two loops is that
			// one increments and the other decrements. as far as I can tell, this is necessary.
			if (player->velocity[i] > 0) {
				minv = max[i];
				maxv = floor((maxp[i] + deltaTime * player->velocity[i]) / BLOCK_WIDTH);

				for(min[i] = minv, max[i] = minv; min[i] <= maxv; min[i]++, max[i]++) {
					if (solidBlockInArea(world, min[0], min[1], min[2], max[0]+1, max[1]+1, max[2]+1)) {
						if (player->velocity[i] > 0) {
							player->position[i] = (min[i]*BLOCK_WIDTH) - (player->bounding_box.min[i] + player->bounding_box.size[i]) - EPSILON;
						} else {
							player->position[i] = ((max[i]+1)*BLOCK_WIDTH) - player->bounding_box.min[i] + EPSILON;
						}
						player->velocity[i] = 0;
						break;
					}
				}
			} else {
				minv = min[i];
				maxv = floor((minp[i] + deltaTime * player->velocity[i]) / BLOCK_WIDTH);

				for(min[i] = minv, max[i] = minv; min[i] >= maxv; min[i]--, max[i]--) {
					if (solidBlockInArea(world, min[0], min[1], min[2], max[0]+1, max[1]+1, max[2]+1)) {
						if (player->velocity[i] > 0) {
							player->position[i] = (min[i]*BLOCK_WIDTH) - (player->bounding_box.min[i] + player->bounding_box.size[i]) - EPSILON;
						} else {
							player->position[i] = ((max[i]+1)*BLOCK_WIDTH) - player->bounding_box.min[i] + EPSILON;
						}
						player->velocity[i] = 0;
						break;
					}
				}
			}
		}
	}

	#undef EPSILON
}
