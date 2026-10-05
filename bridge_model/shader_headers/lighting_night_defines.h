#include "common_defines.h"
#include "scene_constances.h"
#ifndef LIGHTING_NIGHT_DEFINES_H
#define LIGHTING_NIGHT_DEFINES_H

#define NUM_TILE_LIGHT_SHADOW_LAYERS 4

INLINE int findTileLightShadowLayer(int idx)
{
	return findMSB((idx >> 3) * 3 + 1) >> 1;
}

constexpr int tileLightShadowLayerOffset(int layer)
{
	return ((8 << (2 * layer)) - 8) / 3;
}

INLINE ivec2 tileLightShadowLayerSize(int layer)
{
	return ivec2(2 << layer, 4 << layer);
}

struct TileLightMapData
{
	ivec2 idx_range[LIGHT_MAP_SIZE_X * LIGHT_MAP_SIZE_Y];
};

DECLARE_BUFFER(TileLightMapData, tile_light_map, 3);

struct TileLightData
{
	vec4 positions[2 * MAX_CAR_CNT];
};

DECLARE_UNIFORM(TileLightData, tile_light, 4);

struct TileLightTransformData
{
	mat4 view_proj[2 * MAX_CAR_CNT];
};

DECLARE_BUFFER(TileLightTransformData, tile_light_transform, 5);

struct CarLightingData
{
	int light_indices[MAX_CAR_CNT * LIGHTING_SIZE_PER_CAR];
};

DECLARE_BUFFER(CarLightingData, car_lighting, 6);

#endif // !LIGHTING_NIGHT_DEFINES_H
