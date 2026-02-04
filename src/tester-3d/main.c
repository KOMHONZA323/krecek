#include <pico/stdlib.h>
#include <sdk.h>
#include <tft.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "map.h"
#include "light.h"
#include "render.h"

// Player Global Definition
Player player;

// Game State Definition (Shared)
GameMode current_mode = MODE_MOVE_LOOK;
int selected_block_type = 1;

// Game Info
sdk_game_info("tester-3d", NULL);

void game_start(void)
{
	init_map();
	player.x = MAP_W / 2.0f;
	player.y = MAP_H / 2.0f;
	player.z = 2.5f;
	player.yaw = 0.0f;
	player.pitch = 0.0f;
	current_mode = MODE_MOVE_LOOK;

	sdk_set_backlight(SDK_BACKLIGHT_STD);
}

void game_reset(void)
{
	game_start();
}

void game_input(unsigned dt_usec)
{
	float dt = dt_usec / 1000000.0f;
	float move_speed = 5.0f;
	float rot_speed = 2.0f;

	// --- Mode Switching ---
	if (sdk_inputs_delta.select == 1) {
		current_mode = (current_mode == MODE_MOVE_LOOK) ? MODE_EDIT : MODE_MOVE_LOOK;
	}

	// --- Movement (Always Active) ---
	// Joystick Y: Forward/Backward
	if (abs(sdk_inputs.joy_y) > 200) {
		float move = -(sdk_inputs.joy_y / 2048.0f) * move_speed * dt;
		float new_x = player.x + cosf(player.yaw) * move;
		float new_y = player.y + sinf(player.yaw) * move;

		// Simple bounds check
		if (new_x >= 0.1f && new_x < MAP_W - 0.1f)
			player.x = new_x;
		if (new_y >= 0.1f && new_y < MAP_H - 0.1f)
			player.y = new_y;
	}

	// Joystick X: Yaw
	if (abs(sdk_inputs.joy_x) > 200) {
		float rot = (sdk_inputs.joy_x / 2048.0f) * rot_speed * dt;
		player.yaw -= rot; // Inverted rotation
	}

	// --- Actions based on Mode ---
	if (current_mode == MODE_MOVE_LOOK) {
		// Fly Up (Vol Up)
		if (sdk_inputs.vol_up) {
			player.z += move_speed * dt;
			if (player.z >= MAP_D - 0.5f)
				player.z = MAP_D - 0.5f;
		}
		// Fly Down (Vol Down)
		if (sdk_inputs.vol_down) {
			player.z -= move_speed * dt;
			if (player.z < 0.5f)
				player.z = 0.5f;
		}
		// Pitch Up (Brack R)
		if (sdk_inputs.brack_r > 200) {
			player.pitch += rot_speed * dt;
		}
		// Pitch Down (Brack L)
		if (sdk_inputs.brack_l > 200) {
			player.pitch -= rot_speed * dt;
		}
	} else { // MODE_EDIT
		// Interaction Ray
		float cy = cosf(player.yaw);
		float sy = sinf(player.yaw);
		float cp = cosf(player.pitch);
		float sp = sinf(player.pitch);
		float dir_x = cy * cp;
		float dir_y = sy * cp;
		float dir_z = sp;

		// Place Block (A)
		if (sdk_inputs_delta.a == 1) {
			int hx, hy, hz, px, py, pz;
			if (cast_ray(player.x, player.y, player.z, dir_x, dir_y, dir_z, 10.0f, &hx,
				     &hy, &hz, &px, &py, &pz, NULL)) {
				// Place at previous (air) spot
				if (px >= 0 && px < MAP_W && py >= 0 && py < MAP_H && pz >= 0 &&
				    pz < MAP_D) {
					// Don't place inside player
					if (!((int)player.x == px && (int)player.y == py &&
					      (int)player.z == pz)) {
						// Check if we are overwriting a light (unlikely since we target air, but safe)
						if (map[px][py][pz] == 3) {
							remove_light(px, py, pz);
						}

						map[px][py][pz] = selected_block_type;
						// Update lights if block is white
						if (selected_block_type == 3) {
							add_light(px, py, pz);
						}
					}
				}
			}
		}

		// Break Block (B)
		if (sdk_inputs_delta.b == 1) {
			int hx, hy, hz;
			if (cast_ray(player.x, player.y, player.z, dir_x, dir_y, dir_z, 10.0f, &hx,
				     &hy, &hz, NULL, NULL, NULL, NULL)) {
				// Remove light if it was white
				if (map[hx][hy][hz] == 3) {
					remove_light(hx, hy, hz);
				}
				map[hx][hy][hz] = 0;
			}
		}

		// Change Block Type (X/Y)
		if (sdk_inputs_delta.y == 1) {
			selected_block_type++;
			if (selected_block_type > 5)
				selected_block_type = 1;
		}
	}

	// Clamp Pitch
	float max_pitch = 89.0f * M_PI / 180.0f;
	if (player.pitch > max_pitch)
		player.pitch = max_pitch;
	if (player.pitch < -max_pitch)
		player.pitch = -max_pitch;
}

void game_audio(int nsamples)
{
	(void)nsamples;
}

void game_inbox(sdk_message_t msg)
{
	(void)msg;
}

// game_paint is now in render.c

int main()
{
	struct sdk_config config = {
		.wait_for_usb = true,
		.show_fps = true,
		.off_on_select = true,
		.fps_color = 0,
	};
	sdk_main(&config);
}
