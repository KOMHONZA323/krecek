#ifndef MAP_H
#define MAP_H

#include "common.h"

void init_map(void);

// Raycast function for Rendering and Interaction
int cast_ray(float start_x, float start_y, float start_z, float dir_x, float dir_y, float dir_z,
	     float max_dist, int *out_x, int *out_y, int *out_z, int *prev_x, int *prev_y,
	     int *prev_z, int *out_side);

#endif // MAP_H
