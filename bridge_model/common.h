#pragma once

#include <random>
#include "glew.h"

#define PI 3.14159265f
constexpr float EARTH_RADIUS = 6.317e8f;

#define MAX_CAR_COUNT 512

constexpr GLfloat COLOR_WITHE[4] = { 1.0f,1.0f,1.0f,1.0f };
constexpr GLfloat COLOR_BLACK[4] = { 0.0f,0.0f,0.0f,1.0f };

extern GLuint spCarDay, spCarNight;
extern std::mt19937 rdEng;