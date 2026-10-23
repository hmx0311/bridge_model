#include "scene_constances.h"
#include "lighting_day_defines.h"
#include "lighting_night.glslh"
#include "fog.glslh"

layout(binding = 0) uniform sampler2D albedo;

in vec3 world_position;
in vec3 position_in_view;
in vec3 aNormal;
in vec2 aTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 2) out vec3 delta_position;

void main()
{
	vec3 color = texture(albedo, aTexCoord).rgb;
	
	vec3 lighting = sun.ambient;
	ivec2 grid_idx = ivec2(1.0 / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH * world_position.xy + 0.5 * vec2(TILE_LIGHT_AFFECTED_MAP_SIZE_X, TILE_LIGHT_AFFECTED_MAP_SIZE_Y));
	ivec2 idx_range = grid_idx.x < 0 || grid_idx.x >= TILE_LIGHT_AFFECTED_MAP_SIZE_X || grid_idx.y < 0 || grid_idx.y >= TILE_LIGHT_AFFECTED_MAP_SIZE_Y ?
		ivec2(0, 0) : tile_light_affected_map.idx_range[grid_idx.x * TILE_LIGHT_AFFECTED_MAP_SIZE_Y + grid_idx.y];
	for(int i = idx_range.x; i < idx_range.y; i++)
	{
		int light_idx = tile_light_affected_map.light_indices[i];
		vec3 pos_to_light = world_position - vec3(tile_lights[light_idx].position);
		float light_distance = length(pos_to_light);
		pos_to_light /= light_distance;
		float intensity = -dot(pos_to_light, aNormal) / (light_distance * light_distance);
		if(intensity > 0.0)
		{
			intensity *= lightCoverage(world_position, light_idx);
			lighting += intensity * CAR_LIGHT_INTENSITY;
		}
	}
	color *= lighting;
	
	float fog_factor = fogFactor(world_position);
	fragColor = fog_factor * color + (1.0 - fog_factor) * sun.sky_color.rgb;
	delta_position = vec3(0.0);
}
