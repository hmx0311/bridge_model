#pragma once
#include "glad/glad.h"
#include "glm.hpp"

constexpr int NUM_TERRAIN_GRID_X = 2112;
constexpr int NUM_TERRAIN_GRID_Y = 1920;
constexpr int TERRAIN_EBO_SIZE = 6 * NUM_TERRAIN_GRID_X * NUM_TERRAIN_GRID_Y;
constexpr int HIGHWAY_EBO_SIZE = 3981;
constexpr int BRIDGE_EBO_SIZE = 16914;
constexpr int CAR_EBO_SIZE = 2496;
constexpr int CAR_SHADOW_EBO_SIZE = 1566;
constexpr int SUN_VBO_SIZE = 80;

extern GLuint terrain_VAO, highway_VAO, bridge_VAO;
extern GLuint car_VAO, car_shadow_VAO, car_transform_VBO, car_color_VBO;
extern GLuint sun_VAO;

extern const glm::vec3 car_boundray[8];

void buildMeshes();