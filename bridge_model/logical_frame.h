#pragma once
#include "common.h"
#include "glm.hpp"

#include <atomic>

extern std::atomic<float> tickRate;
extern std::atomic<bool> isPaused;
extern std::atomic<int> simulateSpeed;

struct LogicalData
{
	glm::vec3 sunDir{ 0.0f, 0.0f, 0.0f };
	int numCars = 0;
	int numLightOnCars =0;
	glm::mat4 carModelMat[MAX_CAR_COUNT];
	glm::vec3 carColor[MAX_CAR_COUNT];
	glm::vec4 carLightPos[2 * MAX_CAR_COUNT];
	glm::vec4 carLightDir[2 * MAX_CAR_COUNT];
};

void initLogic();

LogicalData& getLatestLogicalData();

void logicalFrame();
