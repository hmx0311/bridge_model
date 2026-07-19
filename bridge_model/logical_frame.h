#pragma once
#include <random>
#include <atomic>

#include "glm.hpp"

#include "common.h"

extern std::mt19937 rd_eng;

extern std::atomic<float> tick_rate;
extern std::atomic<bool> is_paused;
extern std::atomic<int> simulate_speed;
extern std::atomic<int> simulate_speed;

struct LogicalData
{
	glm::vec3 sun_dir{ 0.0f, 0.0f, 0.0f };
	int num_cars = 0;
	int num_light_on_cars = 0;
	glm::mat4 car_transform[MAX_CAR_CNT];
	glm::vec3 car_color[MAX_CAR_CNT];
	glm::vec4 car_light_pos[2 * MAX_CAR_CNT];
	glm::vec4 car_light_dir[2 * MAX_CAR_CNT];
};

void initLogic();
void stopLogic();

LogicalData& getLatestLogicalData();

void logicalFrame();
