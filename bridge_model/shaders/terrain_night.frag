#include "scene_constances.h"
#include "lighting_day_defines.h"
#include "lighting_night.glslh"
#include "fog.glslh"

layout(binding = 0) uniform sampler2D albedo_tex;

in vec3 world_position;
in vec3 position_in_view;
in vec3 aNormal;
in vec2 aTexCoord;

layout(location = RENDER_TARGET_COLOR) out vec3 frag_color;
layout(location = RENDER_TARGET_DELTA_POSITION) out vec3 frag_delta_position;
layout(location = RENDER_TARGET_ALBEDO) out vec3 frag_albedo;

void main()
{
	vec3 color = mix(texture(albedo_tex, aTexCoord).rgb, vec3(0.5, 0.5, 0.3), smoothstep(-0.5, -3.0, world_position.z));
	frag_albedo = color;
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
	frag_color = fog_factor * color + (1.0 - fog_factor) * sun.sky_color.rgb;
	frag_delta_position = vec3(0.0);
}
