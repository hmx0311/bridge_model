#pragma once

#include "glew.h"
#include "glm.hpp"

constexpr int GROUND_EBO_SIZE = 3906;
constexpr int BRIDGE_EBO_SIZE = 18966;
constexpr int CAR_EBO_SIZE = 2496;
constexpr int CAR_SHADOW_EBO_SIZE = 1566;
constexpr int SUN_VBO_SIZE = 80;

extern GLuint ground_VAO, bridge_VAO;
extern GLuint car_VAO, car_shadow_VAO, car_transform_VBO, car_color_VBO;
extern GLuint sun_VAO;

extern const glm::vec3 car_boundray[8];

void buildMeshes();