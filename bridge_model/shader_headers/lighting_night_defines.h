#include "common_defines.h"
#include "scene_constances.h"
#include "binding_points.h"
#ifndef LIGHTING_NIGHT_DEFINES_H
#define LIGHTING_NIGHT_DEFINES_H

#define NUM_TILE_LIGHT_SHADOW_LAYERS 3
#define CAR_LIGHT_RANGE 48.0f
#define CAR_LIGHT_UP_CLIP 0.62f
#define LIGHTING_SIZE_PER_CAR 32
#define MAX_AVERAGE_TRIANGLES_PER_LIGHT 4096
#define MAX_AVERAGE_AFFECTED_CARS_PER_LIGHT 8
#define MAX_AVERAGE_AFFECTED_GRIDS_PER_LIGHT (4 + int(\
	0.2f * (CAR_LIGHT_RANGE / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH) * (CAR_LIGHT_RANGE / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH) + \
	2.0f * (CAR_LIGHT_RANGE / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH)))

#define TILE_LIGHT_POSITION_MAP_GRID_LENGTH (0.5f * CAR_LIGHT_RANGE)
#define TILE_LIGHT_POSITION_MAP_SIZE_X (int(SCENE_GRID_SIZE_X * SCENE_GRID_UNIT / TILE_LIGHT_POSITION_MAP_GRID_LENGTH) + 3)
#define TILE_LIGHT_POSITION_MAP_SIZE_Y (int(SCENE_GRID_SIZE_Y * SCENE_GRID_UNIT / TILE_LIGHT_POSITION_MAP_GRID_LENGTH) + 3)

#define TILE_LIGHT_AFFECTED_MAP_SCENE_GRID_LEVEL_DIFF 3
#define TILE_LIGHT_AFFECTED_MAP_GRID_LEVEL (MAX_SCENE_GRID_LEVEL - TILE_LIGHT_AFFECTED_MAP_SCENE_GRID_LEVEL_DIFF)
#define TILE_LIGHT_AFFECTED_MAP_SIZE_X (SCENE_GRID_SIZE_X >> TILE_LIGHT_AFFECTED_MAP_SCENE_GRID_LEVEL_DIFF)
#define TILE_LIGHT_AFFECTED_MAP_SIZE_Y (SCENE_GRID_SIZE_Y >> TILE_LIGHT_AFFECTED_MAP_SCENE_GRID_LEVEL_DIFF)
#define TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH ((1 << TILE_LIGHT_AFFECTED_MAP_SCENE_GRID_LEVEL_DIFF) * SCENE_GRID_UNIT)

INLINE int findTileLightShadowLayer(int idx)
{
	return findMSB((idx >> 5) * 3 + 1) >> 1;
}

constexpr int tileLightShadowLayerOffset(int layer)
{
	return ((32 << (2 * layer)) - 32) / 3;
}

constexpr ivec2 tileLightShadowLayerSize(int layer)
{
	return ivec2(4 << layer, 8 << layer);
}

STORAGE_BUFFER(TileLightPositionMapData, TILE_LIGHT_POSITION_MAP_BUFFER_BINDING)
{
	ivec2 light_idx_range[TILE_LIGHT_POSITION_MAP_SIZE_X * TILE_LIGHT_POSITION_MAP_SIZE_Y];
}BUFFER_NAME(tile_light_position_map)

struct TileLight
{
	vec4 position;
	mat4 view_proj;
};

STORAGE_BUFFER(TileLightData, TILE_LIGHT_BUFFER_BINDING)
{
	TileLight tile_lights[2 * MAX_CAR_CNT];
};

STORAGE_BUFFER(TileLightAffectedMapData, TILE_LIGHT_AFFECTED_MAP_BUFFER_BINDING)
{
	ivec2 idx_range[TILE_LIGHT_AFFECTED_MAP_SIZE_X * TILE_LIGHT_AFFECTED_MAP_SIZE_Y];
	int light_indices[2 * MAX_CAR_CNT * MAX_AVERAGE_AFFECTED_GRIDS_PER_LIGHT];
}BUFFER_NAME(tile_light_affected_map);

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
	int vert_idx[3];
	int light_idx;
};

STORAGE_BUFFER(TileLightShadowTriangleData, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING)
{
	DrawArraysIndirectCommand commands[NUM_TILE_LIGHT_SHADOW_LAYERS];
	TileLightShadowTriangle triangles[];
}BUFFER_NAME(tile_light_shadow_triangles)

#endif // !LIGHTING_NIGHT_DEFINES_H
