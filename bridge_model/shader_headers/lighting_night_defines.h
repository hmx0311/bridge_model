#include "common_defines.h"
#include "scene_constances.h"
#ifndef LIGHTING_NIGHT_DEFINES_H
#define LIGHTING_NIGHT_DEFINES_H

#define NUM_TILE_LIGHT_SHADOW_LAYERS 4

struct CarLightMapData
{
	ivec2 idx_range[LIGHT_MAP_SIZE_X * LIGHT_MAP_SIZE_Y];
};

DECLARE_BUFFER(CarLightMapData, car_light_map, 3);

struct CarLightData
{
	vec4 positions[2 * MAX_CAR_CNT];
};

DECLARE_UNIFORM(CarLightData, car_light, 4);

struct CarLightShadowTransformData
{
	mat4 view_proj[2 * MAX_CAR_CNT];
};

DECLARE_UNIFORM(CarLightShadowTransformData, car_light_shadow, 5);

struct CarLightingData
{
	int light_indices[MAX_CAR_CNT * MAX_LIGHT_PER_CAR];
};

DECLARE_BUFFER(CarLightingData, car_lighting, 6);

#endif // !LIGHTING_NIGHT_DEFINES_H
