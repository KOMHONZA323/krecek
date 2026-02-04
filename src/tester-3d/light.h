#ifndef LIGHT_H
#define LIGHT_H

#include "common.h"

#define MAX_LIGHTS 64

extern LightSource lights[MAX_LIGHTS];
extern int num_lights;

// Calculate lighting intensity at a specific point
float calculate_light_intensity(float hit_x, float hit_y, float hit_z,
                                float dir_x, float dir_y, float dir_z);

void add_light(int x, int y, int z);
void remove_light(int x, int y, int z);
void reset_lights(void);

#endif // LIGHT_H
