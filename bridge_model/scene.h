#pragma once
#include "glm.hpp"

#include "StaticQuadTree.h"
#include "Bound.h"

struct SenceHeight
{
	float min_height;
	float max_height;
};

constexpr int NUM_SCENE_GRID_LEVELS = 11;
constexpr int NUM_SCENE_GRID_ROOTS_X = 5;
constexpr int NUM_SCENE_GRID_ROOTS_Y = 4;
constexpr int SCENE_GRID_SIZE_X = NUM_SCENE_GRID_ROOTS_X * (1 << (NUM_SCENE_GRID_LEVELS - 1));
constexpr int SCENE_GRID_SIZE_Y = NUM_SCENE_GRID_ROOTS_Y * (1 << (NUM_SCENE_GRID_LEVELS - 1));
constexpr float SCENE_GRID_UNIT = 1.0f;
constexpr glm::vec4 SCENE_GRID_AREA = 0.5f * SCENE_GRID_UNIT * glm::vec4(-SCENE_GRID_SIZE_X, -SCENE_GRID_SIZE_Y, SCENE_GRID_SIZE_X, SCENE_GRID_SIZE_Y);

extern StaticQuadTree<SenceHeight, NUM_SCENE_GRID_LEVELS, NUM_SCENE_GRID_ROOTS_X, NUM_SCENE_GRID_ROOTS_Y> g_scene_quad_tree;

void initScene();

inline BoundBox sceneGridFrustum(uint32_t level, size_t x, size_t y)
{
	float stride = (1 << (NUM_SCENE_GRID_LEVELS - 1 - level)) * SCENE_GRID_UNIT;
	return BoundBox(glm::vec3(SCENE_GRID_AREA.x + x * stride, SCENE_GRID_AREA.y + y * stride, g_scene_quad_tree[level][x][y].min_height),
		glm::vec3(SCENE_GRID_AREA.x + (x + 1) * stride, SCENE_GRID_AREA.y + (y + 1) * stride, g_scene_quad_tree[level][x][y].max_height));
}