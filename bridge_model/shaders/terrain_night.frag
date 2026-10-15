#include "scene_constances.h"
#include "lighting_day_defines.h"
#include "lighting_night.glslh"
#include "fog.glslh"

layout(binding = 0) uniform sampler2D albedo;
layout(binding = 1) uniform sampler2DArrayShadow shadowTex;

in vec3 modelPos;
in vec3 modelPosView;
in vec3 aNormal;
in vec2 aTexCoord;

out vec3 fragColor;

void main()
{
	vec3 color = mix(texture(albedo, aTexCoord).rgb, vec3(0.5, 0.5, 0.3), smoothstep(-0.5, -3.0, modelPos.z));
	
	vec3 lighting = sun.ambient;
	vec3 offset_x = dFdx(modelPos);
	vec3 offset_y = dFdy(modelPos);
	ivec2 grid_idx = ivec2(1.0 / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH * modelPos.xy + 0.5 * vec2(TILE_LIGHT_AFFECTED_MAP_SIZE_X, TILE_LIGHT_AFFECTED_MAP_SIZE_Y));
	ivec2 idx_range = grid_idx.x < 0 || grid_idx.x >= TILE_LIGHT_AFFECTED_MAP_SIZE_X || grid_idx.y < 0 || grid_idx.y >= TILE_LIGHT_AFFECTED_MAP_SIZE_Y ?
		ivec2(0, 0) : tile_light_affected_map.idx_range[grid_idx.x * TILE_LIGHT_AFFECTED_MAP_SIZE_Y + grid_idx.y];
	for(int i = idx_range.x; i < idx_range.y; i++)
	{
		int light_idx = tile_light_affected_map.light_indices[i];
		int layer = findTileLightShadowLayer(light_idx);
		ivec2 shadow_map_cnt = tileLightShadowLayerSize(layer);
		int shadow_map_idx = light_idx - tileLightShadowLayerOffset(layer);
		vec2 shadow_map_pos = vec2(shadow_map_idx % shadow_map_cnt.x, shadow_map_idx / shadow_map_cnt.x) / shadow_map_cnt;
		vec4 pos_in_light = tile_lights[light_idx].view_proj * vec4(modelPos, 1.0);
		vec4 offset_x_light = mat3x4(tile_lights[light_idx].view_proj) * offset_x;
		vec4 offset_y_light = mat3x4(tile_lights[light_idx].view_proj) * offset_y;
		const vec2 offsets[4] = { { -0.18, 0.36 }, { 0.36, 0.18 }, { -0.36, -0.18 }, { 0.18, -0.36 } };
		float ave_intensity = 0;
		vec3 pos_to_light = modelPos - vec3(tile_lights[light_idx].position);
		for(int j = 0; j < 4; j++)
		{
			vec3 offseted_pos_to_light = pos_to_light + offsets[j].x * offset_x + offsets[j].y * offset_y;
			float light_distance = length(offseted_pos_to_light);
			offseted_pos_to_light /= light_distance;
			float intensity = -dot(offseted_pos_to_light, aNormal) / (light_distance * light_distance);
			vec4 coord = pos_in_light + offsets[j].x * offset_x_light + offsets[j].y * offset_y_light;
			coord.xy /= coord.w;
			float r = length(coord.xy);
			if(intensity > 0 && -coord.w < coord.z && coord.z < coord.w && r < 1.0)
			{
				intensity *= lightSmooth(r);
				if(layer < NUM_TILE_LIGHT_SHADOW_LAYERS)
				{
					coord.xy = (coord.xy + 1.0) / (2 * shadow_map_cnt) + shadow_map_pos;
					coord.w = 0.5 * (coord.z / coord.w + 1.0);
					coord.z = layer;
					float shadow = texture(shadowTex, coord).x;
					intensity *= shadow;
				}
				ave_intensity += intensity;
			}
		}
		ave_intensity /= 4;
		lighting += ave_intensity * CAR_LIGHT_INTENSITY;
	}
	color *= lighting;
	
	float fog_factor = fogFactor(modelPos);
	fragColor = fog_factor * color + (1.0 - fog_factor) * sun.sky_color.rgb;
}
