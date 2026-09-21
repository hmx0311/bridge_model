#pragma once
#include <chrono>
#include "glad/glad.h"

#define STR(x) #x
#define SHADER_NAME(x) STR(x)

constexpr float EARTH_RADIUS = 6.371e6f;

constexpr GLfloat COLOR_WHITE[4] = { 1.0f,1.0f,1.0f,1.0f };
constexpr GLfloat COLOR_BLACK[4] = { 0.0f,0.0f,0.0f,1.0f };

extern GLuint scene_UBO;
extern GLuint SP_car_day, SP_car_night;

inline uint64_t getTimestampMicroseconds()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}