#pragma once
#include "glew.h"
#include <chrono>

constexpr float EARTH_RADIUS = 6.371e8f;

constexpr int MAX_CAR_CNT = 512;

constexpr GLfloat COLOR_WHITE[4] = { 1.0f,1.0f,1.0f,1.0f };
constexpr GLfloat COLOR_BLACK[4] = { 0.0f,0.0f,0.0f,1.0f };

extern GLuint SP_car_day, SP_car_night;

inline uint64_t getTimestampMicroseconds()
{
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}