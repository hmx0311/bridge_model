#include "common_defines.h"
#include "scene_constances.h"
#include "binding_points.h"
#ifndef LIGHTING_NIGHT_DEFINES_H
#define LIGHTING_NIGHT_DEFINES_H

#define NUM_TILE_LIGHT_SHADOW_LAYERS 4
#define MAX_AVERANGE_TRIANGLES_PER_LIGHT 4096

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

STORAGE_BUFFER(TileLightMapData, TILE_LIGHT_MAP_BUFFER_BINDING)
{
	ivec2 idx_range[LIGHT_MAP_SIZE_X * LIGHT_MAP_SIZE_Y];
}BUFFER_NAME(tile_light_map)

struct TileLight
{
	vec4 position;
	mat4 view_proj;
};

STORAGE_BUFFER(TileLightData, TILE_LIGHT_BUFFER_BINDING)
{
	TileLight tile_lights[2 * MAX_CAR_CNT];
};

STORAGE_BUFFER(CarLightingData, CAR_LIGHTING_BUFFER_BINDING)
{
	int light_indices[MAX_CAR_CNT * LIGHTING_SIZE_PER_CAR];
}BUFFER_NAME(car_lighting)

struct DrawArraysIndirectCommand
{
	int count;
	int instance_count;
	int first;
	int base_instance;
};

struct TileLightShadowTriangle
{
	uint vert_idx[3];
	int light_idx;
};

STORAGE_BUFFER(TileLightShadowTriangleData, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING)
{
	DrawArraysIndirectCommand commands[NUM_TILE_LIGHT_SHADOW_LAYERS];
	TileLightShadowTriangle triangles[];
}BUFFER_NAME(tile_light_shadow_triangles)

#endif // !LIGHTING_NIGHT_DEFINES_H
