#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <math.h>
#include <tft.h>

// Ensure M_PI is defined
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// Game Constants
#define MAP_W 30
#define MAP_H 30
#define MAP_D 5

#define SCREEN_W 160
#define SCREEN_H 120

#define FOV_DEG 90.0f
#define TAN_HALF_FOV (tanf(FOV_DEG * M_PI / 360.0f))

// Colors
#define COL_SKY rgb_to_rgb565(10, 10, 20) // Dark Sky
#define COL_GRAY rgb_to_rgb565(160, 160, 160)
#define COL_WHITE rgb_to_rgb565(255, 255, 255)
#define COL_BLACK rgb_to_rgb565(0, 0, 0)

#define COL_FLOOR rgb_to_rgb565(50, 50, 50)
#define COL_WALL COL_GRAY
#define COL_BLOCK_1 COL_WHITE
#define COL_BLOCK_2 rgb_to_rgb565(50, 200, 50)
#define COL_BLOCK_3 rgb_to_rgb565(50, 50, 200)

// Game Mode Enum
typedef enum { MODE_MOVE_LOOK = 0, MODE_EDIT = 1 } GameMode;

// Structs
typedef struct {
	float x, y, z;
	float yaw, pitch;
} Player;

typedef struct {
	int x, y, z;
} LightSource;

// Shared Globals
extern uint8_t map[MAP_W][MAP_H][MAP_D];
extern Player player;

// UI State shared for rendering
extern GameMode current_mode;
extern int selected_block_type;

#endif // COMMON_H
