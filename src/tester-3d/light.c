#include "light.h"
#include "map.h"
#include <stddef.h>
#include <math.h>

LightSource lights[MAX_LIGHTS];
int num_lights = 0;

void reset_lights(void)
{
	num_lights = 0;
}

void add_light(int x, int y, int z)
{
	if (num_lights < MAX_LIGHTS) {
		lights[num_lights++] = (LightSource){ x, y, z };
	}
}

void remove_light(int x, int y, int z)
{
	for (int i = 0; i < num_lights; i++) {
		if (lights[i].x == x && lights[i].y == y && lights[i].z == z) {
			lights[i] = lights[--num_lights];
			break;
		}
	}
}

float calculate_light_intensity(float hit_x, float hit_y, float hit_z,
                                float dir_x, float dir_y, float dir_z)
{
	float light_intensity = 0.0f;
	float max_light_dist = 20.0f; // Increased for room coverage

	for (int i = 0; i < num_lights; i++) {
		float lx = lights[i].x + 0.5f;
		float ly = lights[i].y + 0.5f;
		float lz = lights[i].z + 0.5f;

		float dx = lx - hit_x;
		float dy = ly - hit_y;
		float dz = lz - hit_z;
		float dist_sq = dx * dx + dy * dy + dz * dz;
		float dist = sqrtf(dist_sq);

		if (dist < max_light_dist) {
			// Shadow Ray
			float ldir_x = dx / dist;
			float ldir_y = dy / dist;
			float ldir_z = dz / dist;

			// Offset start position slightly to avoid self-intersection
			int shadow_hit = 0;
			if (dist > 1.0f) {
				// Simple raycast to light
				// We can reuse cast_ray logic or a simplified version
				// For performance, let's use cast_ray but only for geometry check
				// Note: cast_ray is relatively expensive to call in a loop.
				// Since user said "don't worry about performance", we do it.
				int hx, hy, hz;
				if (cast_ray(hit_x - dir_x * 0.01f, hit_y - dir_y * 0.01f, hit_z - dir_z * 0.01f,
							 ldir_x, ldir_y, ldir_z, dist + 1.0f, // Go past light center to ensure we hit it if valid
							 &hx, &hy, &hz, NULL, NULL, NULL, NULL)) {
					// Check if we hit the light source itself
					if (hx == lights[i].x && hy == lights[i].y && hz == lights[i].z) {
						shadow_hit = 0; // Visible!
					} else {
						shadow_hit = 1; // Blocked by something else
					}
				}
			}

			if (!shadow_hit) {
				// Smoother falloff (quadratic-ish)
				float norm_dist = dist / max_light_dist;
				float attenuation = 1.0f - (norm_dist * norm_dist);
				if (attenuation > 0)
					light_intensity += attenuation * 1.5f; // Boost slightly
			}
		}
	}
	if (light_intensity > 1.0f) light_intensity = 1.0f;
    return light_intensity;
}
