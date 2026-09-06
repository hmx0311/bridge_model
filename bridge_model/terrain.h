#pragma once
#include "glad/glad.h"
#include "glm.hpp"

constexpr float TERRAIN_FINEST_VERTEX_SPACING = 1.0f;
constexpr int TERRAIN_TILE_SIZE = 32;
constexpr int NUM_TERRAIN_LOD = 5;
constexpr int TERRAIN_TILE_ROOT_SIZE = (TERRAIN_TILE_SIZE << (NUM_TERRAIN_LOD - 1));
constexpr int NUM_TERRAIN_TILE_ROOTS_X = 6;
constexpr int NUM_TERRAIN_TILE_ROOTS_Y = 5;
constexpr int NUM_TERRAIN_GRID_X = NUM_TERRAIN_TILE_ROOTS_X * TERRAIN_TILE_ROOT_SIZE;
constexpr int NUM_TERRAIN_GRID_Y = NUM_TERRAIN_TILE_ROOTS_Y * TERRAIN_TILE_ROOT_SIZE;

void buildTerrainMesh();
void updateTerrainLOD(float lod_factor, const glm::vec3& camera_position);
void drawTerrainMesh();
