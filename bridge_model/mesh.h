#pragma once
#include "glad/glad.h"
#include "glm.hpp"

constexpr int HIGHWAY_EBO_SIZE = 3981;
constexpr int BRIDGE_EBO_SIZE = 16914;
constexpr int CAR_EBO_SIZE = 2496;
constexpr int CAR_SHADOW_EBO_SIZE = 1566;
constexpr int SUN_VBO_SIZE = 80;

extern GLuint highway_VAO, bridge_VAO;
extern GLuint car_VAO, car_shadow_VAO, car_transform_VBO, car_color_VBO;
extern GLuint sun_VAO;

extern const glm::vec3 car_boundray[8];

void buildMeshes();