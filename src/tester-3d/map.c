#include "map.h"
#include "light.h"
#include <math.h>
#include <stdlib.h> // for abs, etc if needed

uint8_t map[MAP_W][MAP_H][MAP_D];

void init_map(void)
{
	reset_lights();
	for (int x = 0; x < MAP_W; x++) {
		for (int y = 0; y < MAP_H; y++) {
			for (int z = 0; z < MAP_D; z++) {
				if (z == 0) {
					map[x][y][z] = 1; // Floor
				} else if (z == MAP_D - 1) {
					map[x][y][z] = 2; // Ceiling
				} else if (x == 0 || x == MAP_W - 1 || y == 0 || y == MAP_H - 1) {
					map[x][y][z] = 2; // Wall
				} else {
					map[x][y][z] = 0; // Air
				}
			}
		}
	}

	// Add a single light source in the ceiling center
	int lx = MAP_W / 2;
	int ly = MAP_H / 2;
	int lz = MAP_D - 1; // In the ceiling
	map[lx][ly][lz] = 3; // White Block (Light)
	add_light(lx, ly, lz);
}

int cast_ray(float start_x, float start_y, float start_z, float dir_x, float dir_y, float dir_z,
	     float max_dist, int *out_x, int *out_y, int *out_z, int *prev_x, int *prev_y,
	     int *prev_z, int *out_side)
{
	int mapX = (int)start_x;
	int mapY = (int)start_y;
	int mapZ = (int)start_z;

	float deltaDistX = (dir_x == 0) ? 1e30f : fabsf(1.0f / dir_x);
	float deltaDistY = (dir_y == 0) ? 1e30f : fabsf(1.0f / dir_y);
	float deltaDistZ = (dir_z == 0) ? 1e30f : fabsf(1.0f / dir_z);

	int stepX, stepY, stepZ;
	float sideDistX, sideDistY, sideDistZ;

	if (dir_x < 0) {
		stepX = -1;
		sideDistX = (start_x - mapX) * deltaDistX;
	} else {
		stepX = 1;
		sideDistX = (mapX + 1.0f - start_x) * deltaDistX;
	}
	if (dir_y < 0) {
		stepY = -1;
		sideDistY = (start_y - mapY) * deltaDistY;
	} else {
		stepY = 1;
		sideDistY = (mapY + 1.0f - start_y) * deltaDistY;
	}
	if (dir_z < 0) {
		stepZ = -1;
		sideDistZ = (start_z - mapZ) * deltaDistZ;
	} else {
		stepZ = 1;
		sideDistZ = (mapZ + 1.0f - start_z) * deltaDistZ;
	}

	int hit = 0;
	int side = -1;
	float dist = 0.0f;

	// Track previous step
	int lastX = mapX, lastY = mapY, lastZ = mapZ;

	while (hit == 0 && dist < max_dist) {
		lastX = mapX;
		lastY = mapY;
		lastZ = mapZ;

		if (sideDistX < sideDistY) {
			if (sideDistX < sideDistZ) {
				sideDistX += deltaDistX;
				mapX += stepX;
				side = 0;
			} else {
				sideDistZ += deltaDistZ;
				mapZ += stepZ;
				side = 2;
			}
		} else {
			if (sideDistY < sideDistZ) {
				sideDistY += deltaDistY;
				mapY += stepY;
				side = 1;
			} else {
				sideDistZ += deltaDistZ;
				mapZ += stepZ;
				side = 2;
			}
		}

		// Check Bounds
		if (mapX < 0 || mapX >= MAP_W || mapY < 0 || mapY >= MAP_H || mapZ < 0 ||
		    mapZ >= MAP_D) {
			return 0; // Out of bounds
		}

		if (map[mapX][mapY][mapZ] > 0) {
			hit = 1;
		}

		// Approximate distance check (Manhattan-ish for loop safety)
		// Accurate distance isn't strictly needed for the loop condition
		// if we trust max_dist steps, but let's just use a step counter or large number
	}

	if (hit) {
		if (out_x)
			*out_x = mapX;
		if (out_y)
			*out_y = mapY;
		if (out_z)
			*out_z = mapZ;
		if (prev_x)
			*prev_x = lastX;
		if (prev_y)
			*prev_y = lastY;
		if (prev_z)
			*prev_z = lastZ;
		if (out_side)
			*out_side = side;
		return 1;
	}
	return 0;
}
