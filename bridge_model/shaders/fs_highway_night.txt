#include "scene_constances.h"
#include "lighting_day_defines.h"
#include "lighting_night.glslh"
#include "fog.glslh"

layout(binding = 0) uniform sampler2D texture0;
layout(binding = 1) uniform sampler2DArrayShadow shadowTex;

in vec3 modelPos;
in vec3 modelPosView;
in vec3 aNormal;
in vec2 aTexCoord;

out vec3 fragColor;

void main()
{
	vec3 color = texture(texture0, aTexCoord).rgb;
	
	vec3 lighting = sun.ambient;
	const int idxOffsets[9] = { -LIGHT_MAP_SIZE_Y - 1, -LIGHT_MAP_SIZE_Y, -LIGHT_MAP_SIZE_Y + 1, -1, 0, 1, LIGHT_MAP_SIZE_Y - 1, LIGHT_MAP_SIZE_Y, LIGHT_MAP_SIZE_Y + 1 };
	vec3 offsetX = dFdx(modelPos);
	vec3 offsetY = dFdy(modelPos);
	for(int i = 0, j = 0, end = 0; true; j++)
	{
		while(j == end)
		{
			if(i == 9)
			{
				i++;
				break;
			}
			ivec2 lightRange =  car_light_map.idx_range[int(modelPos.x / LIGHT_MAP_GRID_LENGTH + 0.5 * LIGHT_MAP_SIZE_X) * LIGHT_MAP_SIZE_Y + int(modelPos.y / LIGHT_MAP_GRID_LENGTH + 0.5 * LIGHT_MAP_SIZE_Y) + idxOffsets[i]];
			j = lightRange.x;
			end = lightRange.y;
			i++;
		}
		if(i == 10)
		{
			break;
		}
		int layer = findMSB((j >> 3) * 3 + 1) >> 1;
		ivec2 shadowMapSize = ivec2(2 << layer, 4 << layer);
		int shadowMapIdx = j - ((8 << (2 * layer)) - 8) / 3 ;
		vec2 shadowMapPos = vec2(shadowMapIdx % shadowMapSize.x, shadowMapIdx / shadowMapSize.x) / shadowMapSize;
		vec4 modelPosLight = car_light_shadow.view_proj[j] * vec4(modelPos, 1.0);
		vec4 OffsetXLight = mat3x4(car_light_shadow.view_proj[j]) * offsetX;
		vec4 OffsetYLight = mat3x4(car_light_shadow.view_proj[j]) * offsetY;
		const vec2 offsets[4] = { { -0.18, 0.36 }, { 0.36, 0.18 }, { -0.36, -0.18 }, { 0.18, -0.36 } };
		float aveIntensity = 0;
		vec3 modelToLight = modelPos - vec3(car_light.positions[j]);
		for(int k = 0; k < 4; k++)
		{
			vec3 offsetToLight = modelToLight + offsets[k].x * offsetX + offsets[k].y * offsetY;
			float lightDistance = length(offsetToLight);
			offsetToLight /= lightDistance;
			float intensity = -dot(offsetToLight, aNormal) / (lightDistance * lightDistance);
			vec4 coord = modelPosLight + offsets[k].x * OffsetXLight + offsets[k].y * OffsetYLight;
			coord.xy /= coord.w;
			float r2 = dot(coord.xy, coord.xy);
			if(intensity > 0 && -coord.w < coord.z && coord.z < coord.w && r2 < 0.99)
			{
				coord.xy = (coord.xy + 1.0) / (2 * shadowMapSize) + shadowMapPos;
				coord.w = 0.5 * (coord.z / coord.w + 1.0);
				coord.z = layer;
				float shadow = texture(shadowTex, coord).x;
				intensity *= shadow * lightSmooth(r2);
				aveIntensity += intensity;
			}
		}
		aveIntensity /= 4;
		const vec3 carLightColor = vec3(1.0, 1.0, 0.9) * 50;
		lighting += aveIntensity * carLightColor;
	}
	color *= lighting;
	
	float fog_factor = fogFactor(length(modelPosView));
	fragColor = fog_factor * color + (1.0 - fog_factor) * sun.sky_color;
	return;
}
