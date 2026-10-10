#pragma once
#include "glad/glad.h"

#include "Bound.h"

constexpr int HIGHWAY_EBO_SIZE = 14511;
constexpr int BRIDGE_EBO_SIZE = 16914;
constexpr int CAR_EBO_SIZE = 2496;
constexpr int CAR_SHADOW_EBO_SIZE = 1566;
constexpr int SUN_VBO_SIZE = 80;

extern GLuint highway_tex;

extern GLuint highway_VAO, highway_VBO, highway_EBO;
extern GLuint bridge_VAO, bridge_VBO, bridge_EBO;
extern GLuint car_VAO, car_shadow_day_VAO, car_transform_VBO, car_color_VBO;
extern GLuint car_shadow_night_VAO, car_tile_light_shadow_transform_VBO, car_tile_light_shadow_idx_VBO;
extern GLuint sun_VAO;

extern const BoundAABB car_local_bound;

void buildMeshes();