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
	const int idxOffsets[9] = { -LIGHT_MAP_SIZE_Y - 1, -LIGHT_MAP_SIZE_Y, -LIGHT_MAP_SIZE_Y + 1, -1, 0, 1, LIGHT_MAP_SIZE_Y - 1, LIGHT_MAP_SIZE_Y, LIGHT_MAP_SIZE_Y + 1 };
	vec3 offset_x = dFdx(modelPos);
	vec3 offset_y = dFdy(modelPos);
	for(int i = 0, j = 0, end = 0; true; j++)
	{
		while(j == end)
		{
			if(i == 9)
			{
				i++;
				break;
			}
			ivec2 lightRange =  tile_light_map.idx_range[int(modelPos.x / LIGHT_MAP_GRID_LENGTH + 0.5 * LIGHT_MAP_SIZE_X) * LIGHT_MAP_SIZE_Y + int(modelPos.y / LIGHT_MAP_GRID_LENGTH + 0.5 * LIGHT_MAP_SIZE_Y) + idxOffsets[i]];
			j = lightRange.x;
			end = lightRange.y;
			i++;
		}
		if(i == 10)
		{
			break;
		}
		int layer = findTileLightShadowLayer(j);
		ivec2 shadow_map_cnt = tileLightShadowLayerSize(layer);
		int shadow_map_idx = j - tileLightShadowLayerOffset(layer);
		vec2 shadow_map_pos = vec2(shadow_map_idx % shadow_map_cnt.x, shadow_map_idx / shadow_map_cnt.x) / shadow_map_cnt;
		vec4 pos_in_light = tile_lights[j].view_proj * vec4(modelPos, 1.0);
		vec4 offset_x_light = mat3x4(tile_lights[j].view_proj) * offset_x;
		vec4 offset_y_light = mat3x4(tile_lights[j].view_proj) * offset_y;
		const vec2 offsets[4] = { { -0.18, 0.36 }, { 0.36, 0.18 }, { -0.36, -0.18 }, { 0.18, -0.36 } };
		float ave_intensity = 0;
		vec3 pos_to_light = modelPos - vec3(tile_lights[j].position);
		for(int k = 0; k < 4; k++)
		{
			vec3 offseted_pos_to_light = pos_to_light + offsets[k].x * offset_x + offsets[k].y * offset_y;
			float light_distance = length(offseted_pos_to_light);
			offseted_pos_to_light /= light_distance;
			float intensity = -dot(offseted_pos_to_light, aNormal) / (light_distance * light_distance);
			vec4 coord = pos_in_light + offsets[k].x * offset_x_light + offsets[k].y * offset_y_light;
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
	return;
}
