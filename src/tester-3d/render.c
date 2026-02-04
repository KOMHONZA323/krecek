#include "render.h"
#include "map.h"
#include "light.h"
#include <math.h>

void game_paint(unsigned dt_usec)
{
	(void)dt_usec;
	tft_fill(COL_SKY);

	// Camera Basis
	float cy = cosf(player.yaw);
	float sy = sinf(player.yaw);
	float cp = cosf(player.pitch);
	float sp = sinf(player.pitch);

	float fx = cy * cp;
	float fy = sy * cp;
	float fz = sp;

	float rx = sy;
	float ry = -cy;
	float rz = 0.0f;

	float ux = ry * fz - rz * fy;
	float uy = rz * fx - rx * fz;
	float uz = rx * fy - ry * fx;

	float aspect = (float)SCREEN_W / SCREEN_H;
	float tan_fov = TAN_HALF_FOV;

	// Render Loop
	for (int y = 0; y < SCREEN_H; y++) {
		float ndc_y = 1.0f - (2.0f * y / SCREEN_H);
		float cam_y = ndc_y * tan_fov;

		float ray_part_x = ux * cam_y;
		float ray_part_y = uy * cam_y;
		float ray_part_z = uz * cam_y;

		for (int x = 0; x < SCREEN_W; x++) {
			float ndc_x = (2.0f * x / SCREEN_W) - 1.0f;
			float cam_x = ndc_x * tan_fov * aspect;

			float dir_x = fx + rx * cam_x + ray_part_x;
			float dir_y = fy + ry * cam_x + ray_part_y;
			float dir_z = fz + rz * cam_x + ray_part_z;

			// DDA directly in render loop for performance
			// (Re-implementing DDA inline to avoid function call overhead per pixel)
			int mapX = (int)player.x;
			int mapY = (int)player.y;
			int mapZ = (int)player.z;

			float deltaDistX = (dir_x == 0) ? 1e30f : fabsf(1.0f / dir_x);
			float deltaDistY = (dir_y == 0) ? 1e30f : fabsf(1.0f / dir_y);
			float deltaDistZ = (dir_z == 0) ? 1e30f : fabsf(1.0f / dir_z);

			int stepX, stepY, stepZ;
			float sideDistX, sideDistY, sideDistZ;

			if (dir_x < 0) {
				stepX = -1;
				sideDistX = (player.x - mapX) * deltaDistX;
			} else {
				stepX = 1;
				sideDistX = (mapX + 1.0f - player.x) * deltaDistX;
			}
			if (dir_y < 0) {
				stepY = -1;
				sideDistY = (player.y - mapY) * deltaDistY;
			} else {
				stepY = 1;
				sideDistY = (mapY + 1.0f - player.y) * deltaDistY;
			}
			if (dir_z < 0) {
				stepZ = -1;
				sideDistZ = (player.z - mapZ) * deltaDistZ;
			} else {
				stepZ = 1;
				sideDistZ = (mapZ + 1.0f - player.z) * deltaDistZ;
			}

			int hit = 0;
			int side = -1;
			int steps = 0;

			while (hit == 0 && steps < 40) { // Limit draw distance
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

				if (mapX < 0 || mapX >= MAP_W || mapY < 0 || mapY >= MAP_H ||
				    mapZ < 0 || mapZ >= MAP_D) {
					break;
				}

				if (map[mapX][mapY][mapZ] > 0) {
					hit = 1;
				}
				steps++;
			}

			if (hit) {
				uint8_t block = map[mapX][mapY][mapZ];
				uint16_t color;

				// Calculate Hit Position
				float perpWallDist;
				if (side == 0)
					perpWallDist = sideDistX - deltaDistX;
				else if (side == 1)
					perpWallDist = sideDistY - deltaDistY;
				else
					perpWallDist = sideDistZ - deltaDistZ;

				float hit_x = player.x + dir_x * perpWallDist;
				float hit_y = player.y + dir_y * perpWallDist;
				float hit_z = player.z + dir_z * perpWallDist;

				// --- Lighting Calculation ---
                float light_intensity = calculate_light_intensity(hit_x, hit_y, hit_z, dir_x, dir_y, dir_z);


				// Outline Logic
				float u, v;
				if (side == 0) {
					u = hit_y - floorf(hit_y);
					v = hit_z - floorf(hit_z);
				} else if (side == 1) {
					u = hit_x - floorf(hit_x);
					v = hit_z - floorf(hit_z);
				} else {
					u = hit_x - floorf(hit_x);
					v = hit_y - floorf(hit_y);
				}

				float margin = 1.0f / 16.0f;
				int is_edge = 0;
				if (u < margin || u > 1.0f - margin || v < margin ||
				    v > 1.0f - margin) {
					is_edge = 1;
				}

				switch (block) {
				case 1:
					color = COL_FLOOR;
					break;
				case 2:
					color = COL_WALL;
					break;
				case 3:
					color = COL_BLOCK_1;
					// White blocks are self-illuminated
					light_intensity = 1.0f;
					break;
				case 4:
					color = COL_BLOCK_2;
					break;
				case 5:
					color = COL_BLOCK_3;
					break;
				default:
					color = COL_WALL;
					break;
				}

				if (is_edge) {
					color = COL_BLACK;
				}

				// Apply Lighting (White)
				// Base lighting (ambient) - Dark Room
				float ambient = 0.0f; // Pitch black ambient

				// Modulative Lighting: SurfaceColor * (Ambient + Intensity)
				// This preserves black outlines (0 * anything = 0)
				float intensity_factor = ambient + light_intensity;
				if (intensity_factor > 1.0f) intensity_factor = 1.0f;

				// Mix original color with White light
				uint8_t r = rgb565_red(color);
				uint8_t g = rgb565_green(color);
				uint8_t b = rgb565_blue(color);

				float lit_r = r * intensity_factor;
				float lit_g = g * intensity_factor;
				float lit_b = b * intensity_factor;

				// Use original shading as a modulation factor if needed
				// For now, let's keep the simple side shading
				float shade = 1.0f;
				if (side == 2 && stepZ > 0) shade = 0.5f;
				else if (side == 0) shade = 0.8f;
				else if (side == 1) shade = 0.7f;

				// Apply directional shading to the result?
				lit_r *= shade;
				lit_g *= shade;
				lit_b *= shade;

				// Dithering (Ordered Bayer 2x2)
				// Coordinates: x, y
				// Matrix: [ 0  2 ]
				//         [ 3  1 ]
				// Scale factor: 4
				int dither_val = 0;
				if ((x % 2 == 0) && (y % 2 == 0)) dither_val = 0;
				else if ((x % 2 != 0) && (y % 2 == 0)) dither_val = 2;
				else if ((x % 2 == 0) && (y % 2 != 0)) dither_val = 3;
				else dither_val = 1;

				// Map 0..3 to a small range, e.g., -4..4 or similar, to affect the LSBs
				float dither_offset = (dither_val - 1.5f) * 4.0f;

				lit_r += dither_offset;
				lit_g += dither_offset;
				lit_b += dither_offset;

				// Clamp
				if (lit_r > 255) lit_r = 255;
				if (lit_g > 255) lit_g = 255;
				if (lit_b > 255) lit_b = 255;
				if (lit_r < 0) lit_r = 0;
				if (lit_g < 0) lit_g = 0;
				if (lit_b < 0) lit_b = 0;

				color = rgb_to_rgb565((int)lit_r, (int)lit_g, (int)lit_b);

				tft_draw_pixel(x, y, color);
			} else {
				// Dark Sky for dark room
				tft_draw_pixel(x, y, COL_SKY);
			}

			// Draw Crosshair (in center pixels)
			if (x == SCREEN_W / 2 && y == SCREEN_H / 2) {
				tft_draw_pixel(x, y, rgb_to_rgb565(255, 255, 255));
			}
		}
	}

	// UI Overlay
	char buf[32];
	tft_draw_string(5, 5, rgb_to_rgb565(255, 255, 255), "Mode: %s",
			(current_mode == MODE_MOVE_LOOK) ? "MOVE/LOOK" : "EDIT");
	if (current_mode == MODE_EDIT) {
		tft_draw_string(5, 15, rgb_to_rgb565(255, 255, 255), "Block: %d",
				selected_block_type);
		tft_draw_string(5, 105, rgb_to_rgb565(200, 200, 200), "A: Place, B: Break");
	} else {
		tft_draw_string(5, 105, rgb_to_rgb565(200, 200, 200), "A/B: Fly, Y/X: Look");
	}
}
