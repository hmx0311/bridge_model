#include "logical_frame.h"

#include "Sun.h"
#include "Car.h"
#include "scene.h"

#include <chrono>
#include <vector>
#include <list>

#define MAX_FRAME_TIME 50
#define CAR_POS_MAP_SIZE 52
#define CAR_POS_MAP_GRID_LENGTH 4000

#define INITIAL_TIME 2000

using namespace glm;

static LogicalData logicalData[3];
static std::atomic<int> latest_data = 0;
static std::atomic<int> reading_data = 0;
std::atomic<float> tickRate = 60;
std::atomic<bool> isPaused = false;
std::atomic<int> simulateSpeed = 1;

static Sun sun(39.9f);

void initLogic()
{
	sun.updatePosition(INITIAL_TIME);
	logicalData[latest_data].sunDir = sun.dir;
}

LogicalData& getLatestLogicalData()
{
	reading_data = static_cast<int>(latest_data);
	return logicalData[reading_data];
}

void logicalFrame()
{
	std::uniform_int_distribution<uint64_t> distb(0, 480);
	uint64_t elapsedTime = INITIAL_TIME;
	clock_t lastTime = clock();
	uint64_t nextCarTime[6];
	for (int i = 0; i < 6; i++)
	{
		nextCarTime[i] = elapsedTime + distb(rdEng);
	}
	std::list<Car> vehicles;
	while (true)
	{
		int writing_data;
		if ((latest_data + 1) % 3 == reading_data)
		{
			writing_data = (latest_data + 2) % 3;
		}
		else
		{
			writing_data = (latest_data + 1) % 3;
		}

		clock_t time = clock();
		uint32_t frameTime = time - lastTime;
		lastTime = time;
		tickRate = (0.1f * tickRate + 1) / (frameTime * 0.001f + 0.1f);
		uint32_t logicalTime = frameTime * simulateSpeed;
		if (isPaused)
		{
			logicalTime = 0;
		}
		else if (logicalTime > MAX_FRAME_TIME)
		{
			logicalTime = MAX_FRAME_TIME;
		}
		elapsedTime += logicalTime;

		sun.updatePosition(elapsedTime);
		logicalData[writing_data].sunDir = sun.dir;

		for (int i = 0; i < 6; i++)
		{
			if (elapsedTime > nextCarTime[i])
			{
				vehicles.emplace_back(lanes[i], sun.dir.z);
				nextCarTime[i] += (i < 4 ? 1 : 6) * distb(rdEng) + REACT_TIME + CAR_LENGTH / lanes[i]->speedLimit;
			}
		}
		ivec2 carPosMap[CAR_POS_MAP_SIZE][CAR_POS_MAP_SIZE]{};
		struct CarPosInfo
		{
			Car* car;
			ivec2* posMapGrids;
		};
		std::vector<CarPosInfo> carPosInfos;
		for (auto iter = vehicles.begin(); iter != vehicles.end(); )
		{
			if (iter->update(logicalTime, sun.dir.z))
			{
				const mat4& modelMat = iter->getModelMat();
				vec2 carXYPos(modelMat[3]);
				ivec2 posMapIdx = ivec2(1.0f / CAR_POS_MAP_GRID_LENGTH * carXYPos + 0.5f * CAR_POS_MAP_SIZE);
				carPosInfos.push_back(CarPosInfo{ &*iter, &carPosMap[posMapIdx.x][posMapIdx.y] });
				carPosMap[posMapIdx.x][posMapIdx.y].x++;
				iter++;
			}
			else
			{
				iter = vehicles.erase(iter);
			}
		}
		Car* carPos[MAX_CAR_COUNT];
		int numCarPos = 0;
		for (CarPosInfo& carPosInfo : carPosInfos)
		{
			if (carPosInfo.posMapGrids->y == 0)
			{
				carPosInfo.posMapGrids->y = numCarPos;
				numCarPos += carPosInfo.posMapGrids->x;
				carPosInfo.posMapGrids->x = carPosInfo.posMapGrids->y;
			}
			carPos[carPosInfo.posMapGrids->y++] = carPosInfo.car;
		}
		for (Car& car : vehicles)
		{
			constexpr float posOffset = -30.0f / CAR_POS_MAP_GRID_LENGTH + 0.5f;
			ivec2 posIdx = ivec2(1.0f / CAR_POS_MAP_GRID_LENGTH * vec2(car.getModelMat()[3]) + posOffset * vec2(car.getDir()) + (CAR_POS_MAP_SIZE / 2 - 0.5f));
			constexpr ivec2 offset[4] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };
			for (int i = 0; i < 4; i++)
			{
				ivec2 idx = posIdx + offset[i];
				for (int k = carPosMap[idx.x][idx.y].x; k < carPosMap[idx.x][idx.y].y; k++)
				{
					car.collisionTest(carPos[k]);
				}
			}
		}

		logicalData[writing_data].numCars = vehicles.size();
		if (sun.dir.z > 0)
		{
			int numCars = 0;
			for (Car& car : vehicles)
			{
				logicalData[writing_data].carModelMat[numCars] = car.getModelMat();
				logicalData[writing_data].carColor[numCars] = car.getColor();
				numCars++;
			}
			logicalData[writing_data].numLightOnCars = 0;
		}
		else
		{
			int numLightOnCars = 0;
			int numLightOffCars = 0;
			for (Car& car : vehicles)
			{
				if (car.isLightOn())
				{
					const mat4& modelMat = car.getModelMat();
					logicalData[writing_data].carLightPos[2 * numLightOnCars] = modelMat * CAR_LEFT_LIGHT_POS;
					logicalData[writing_data].carLightDir[2 * numLightOnCars] = modelMat * CAR_LEFT_LIGHT_DIR;
					logicalData[writing_data].carLightPos[2 * numLightOnCars + 1] = modelMat * CAR_RIGHT_LIGHT_POS;
					logicalData[writing_data].carLightDir[2 * numLightOnCars + 1] = modelMat * CAR_RIGHT_LIGHT_DIR;
					logicalData[writing_data].carModelMat[numLightOnCars] = car.getModelMat();
					logicalData[writing_data].carColor[numLightOnCars] = car.getColor();
					numLightOnCars++;
				}
				else
				{
					logicalData[writing_data].carModelMat[vehicles.size() - numLightOffCars - 1] = car.getModelMat();
					logicalData[writing_data].carColor[vehicles.size() - numLightOffCars - 1] = car.getColor();
					numLightOffCars++;
				}
			}
			logicalData[writing_data].numLightOnCars = numLightOnCars;
		}
		latest_data = writing_data;
	}
}
