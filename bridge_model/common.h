#pragma once

#include <random>
#include "glew.h"

#define PI 3.14159265f
constexpr float EARTH_RADIUS = 6.371e8f;

constexpr int MAX_CAR_CNT = 512;

constexpr GLfloat COLOR_WHITE[4] = { 1.0f,1.0f,1.0f,1.0f };
constexpr GLfloat COLOR_BLACK[4] = { 0.0f,0.0f,0.0f,1.0f };

extern GLuint spCarDay, spCarNight;
extern std::mt19937 rdEng;