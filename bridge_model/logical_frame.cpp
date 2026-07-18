#include "logical_frame.h"

#include "Sun.h"
#include "Car.h"
#include "Lane.h"
#include "scene.h"

#include "gtx/transform.hpp"

#include <vector>
#include <list>
#include <thread>

using namespace glm;

static constexpr ivec2 CAR_POS_MAP_SIZE = ivec2(54, 50);
static constexpr float CAR_POS_MAP_GRID_LENGTH = 4000;

static constexpr uint32_t MAX_FRAME_DT_US = 50'000;
static constexpr double INITIAL_TIME = 2;


std::atomic<float> tick_rate = 60;
std::atomic<bool> is_paused = false;
std::atomic<int> simulate_speed = 1;
std::mt19937 rd_eng;

static LogicalData logical_data[3];
static std::atomic<int> latest_data = 0;
static std::atomic<int> reading_data = 0;

static Lane* lanes[6];
static Sun sun(39.9f);

static void initLanes()
{
	Lane* iter;
	Lane* next_lane;
	Lane* critical_lanes[2];
	lanes[0] = new Lane(210000, 2500.0f, (translate(vec3(-105000, -200, 0)) * rotate(-pi<float>() / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});

	lanes[1] = new Lane(98920, 2500.0f, (translate(vec3(-105000, -520, 0)) * rotate(-pi<float>() / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter = lanes[1];
	next_lane = new Lane(27160, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(83920, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	critical_lanes[0] = next_lane;
	iter = lanes[1];
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-6080, -29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(6080, 29480, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, 28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(0, -28120, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(3000, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(-5.0f / 13) * 2800, 1000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, 0));
			transform *= rotate(-s / 2800, vec3(0, 0, 1));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(asin(61.0f / 1861) * 37220 * 560 / 600, 1000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, 37220 - 37220 * cos(s * 600 / (560 * 37220))));
			transform *= rotate(-7444 * sin(s * 600 / (560 * 37220)) / 600, vec3(0, 0, 1));
			transform *= rotate(-s * 600 / (560 * 37220), vec3(12, -5, 0));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(6556.4215f, 1000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, s / 6556.4215f * 305 * (1 - 456.0f / 1860)));
			transform *= rotate(-s * (acos(-12.0f / 13) - 244.0f / 600) / 6556.4215f, vec3(0, 0, 1));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(2280.0f * 1861 / 1860, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s * 1860 / 1861, s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(2 * asin(61.0f / 1861) * 37220, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -140, -36895));
			transform *= rotate(-s / 37220, vec3(1, 0, 0));
			transform *= translate(vec3(0, 140, 36895));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(9305, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s * 1860 / 1861, -s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(asin(61.0f / 1861) * 37220, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, 11600, 37220));
			transform *= rotate(-s / 37220, vec3(-1, 0, 0));
			transform *= translate(vec3(0, -11600, -37220));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(83400, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s, 0));
			return transform;
		});
	iter->setNextLane(next_lane);

	lanes[2] = new Lane(210000, 2500.0f, (translate(vec3(105000, 200, 0)) * rotate(pi<float>() / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});

	lanes[3] = new Lane(83440, 2500.0f, (translate(vec3(105000, 520, 0)) * rotate(pi<float>() / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter = lanes[3];
	next_lane = new Lane(43120, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(83440, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	critical_lanes[1] = next_lane;
	iter = lanes[3];
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(21560, 29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-21560, -29480, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(15480, -28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-15480, 28120, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(3000, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(11960 * pi<float>() / 2, 1500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(12480, 12800, 0));
			transform *= rotate(-s / 11960, vec3(0, 0, 1));
			transform *= translate(vec3(-12480, -12800, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(82200, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s, 0));
			return transform;
		});
	iter->setNextLane(next_lane);

	lanes[4] = new Lane(83400, 2500.0f, (translate(vec3(-200, 95000, 0)) * rotate(pi<float>(), vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s, 0));
			return transform;
		});
	iter = lanes[4];
	next_lane = new Lane(asin(61.0f / 1861) * 37220, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, 11600, 37220));
			transform *= rotate(s / 37220, vec3(-1, 0, 0));
			transform *= translate(vec3(0, -11600, -37220));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(9305, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s * 1860 / 1861, s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(2 * asin(61.0f / 1861) * 37220, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -140, -36895));
			transform *= rotate(s / 37220, vec3(1, 0, 0));
			transform *= translate(vec3(0, 140, 36895));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(2280.0f * 1861 / 1860, 2000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s * 1860 / 1861, -s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(7491.97f, 1000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, -s / 7491.97f * 305 * (1 - 456.0f / 1860)));
			transform *= rotate(s * (acos(-12.0f / 13) - 244.0f / 600) / 7491.97f, vec3(0, 0, 1));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(asin(61.0f / 1861) * 37220 * 640 / 600, 1000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, 37220 - 37220 * cos(asin(61.0f / 1861) - s * 600 / (640 * 37220))));
			transform *= rotate((1220.0f - 37220 * sin(asin(61.0f / 1861) - s * 600 / (640 * 37220))) / 3000, vec3(0, 0, 1));
			transform *= rotate(s * 600 / (640 * 37220), vec3(cos(asin(5.0f / 13) + 244.0f / 600), -sin(asin(5.0f / 13) + 244.0f / 600), 0));
			transform *= translate(vec3(-3000, 3640, -20));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(asin(12.0f / 13) * 6550, 1000.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(12000, -7390, 0));
			transform *= rotate(-s / 6550, vec3(0, 0, 1));
			transform *= translate(vec3(-12000, 7390, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(3000, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(15000, 28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-15000, -28120, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(21080, -29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-21080, 29480, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	next_lane->setNextLane(critical_lanes[0]);

	lanes[5] = new Lane(82200, 2500.0f, (translate(vec3(-520, 95000, 0)) * rotate(pi<float>(), vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s, 0));
			return transform;
		});
	iter = lanes[5];
	next_lane = new Lane(11960 * pi<float>() / 2, 1500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-12480, 12800, 0));
			transform *= rotate(-s / 11960, vec3(0, 0, 1));
			transform *= translate(vec3(12480, -12800, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(3000, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-15480, -28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(15480, 28120, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	iter = next_lane;
	next_lane = new Lane(acos(180.0f / 181) * 28960, 2500.0f, iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-21560, 29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(21560, -29480, 0));
			return transform;
		});
	iter->setNextLane(next_lane);
	next_lane->setNextLane(critical_lanes[1]);
}

void initLogic()
{
	std::random_device rd;
	rd_eng.seed(rd());
	initLanes();
	sun.updatePosition(INITIAL_TIME);
	logical_data[latest_data].sun_dir = sun.getDir();
}

LogicalData& getLatestLogicalData()
{
	reading_data = static_cast<int>(latest_data);
	return logical_data[reading_data];
}

void logicalFrame()
{
	std::uniform_real_distribution<float> spawn_distb(0, 0.32f);
	double elapsed_time = INITIAL_TIME;
	uint64_t last_frame_time_us = getTimestampMicroseconds();
	double next_car_time[6];
	for (int i = 0; i < 6; i++)
	{
		next_car_time[i] = elapsed_time + spawn_distb(rd_eng);
	}
	std::list<Car> vehicles;
	while (true)
	{
		int writing_data = (latest_data + ((latest_data + 1) % 3 == reading_data ? 2 :1)) % 3;

		uint64_t time_us = getTimestampMicroseconds();
		uint64_t frame_dt_us = time_us - last_frame_time_us;
		last_frame_time_us = time_us;
		if (is_paused)
		{
			std::this_thread::yield();
			continue;
		}
		tick_rate = (0.1f * tick_rate + 1) / (frame_dt_us * 1e-6f + 0.1f);
		if (frame_dt_us > MAX_FRAME_DT_US)
		{
			frame_dt_us = MAX_FRAME_DT_US;
		}
		float logical_dt_s = 1e-6 * simulate_speed * frame_dt_us;
		elapsed_time += logical_dt_s;

		sun.updatePosition(elapsed_time);
		logical_data[writing_data].sun_dir = sun.getDir();

		for (int i = 0; i < 6; i++)
		{
			if (elapsed_time > next_car_time[i])
			{
				vehicles.emplace_back(lanes[i], sun.getDir().z);
				next_car_time[i] += (i < 4 ? 1 : 8) * spawn_distb(rd_eng) + REACT_TIME + CAR_LENGTH / lanes[i]->speed_limit;
			}
		}
		ivec2 car_pos_map[CAR_POS_MAP_SIZE.x][CAR_POS_MAP_SIZE.y]{};
		struct CarPosInfo
		{
			Car* car;
			ivec2* pos_map_grids;
		};
		std::vector<CarPosInfo> car_pos_infos;
		for (auto iter = vehicles.begin(); iter != vehicles.end(); )
		{
			if (iter->update(logical_dt_s, sun.getDir().z))
			{
				const mat4& transform = iter->getModelMat();
				vec2 car_xy_pos(transform[3]);
				ivec2 pos_map_idx = ivec2(1.0f / CAR_POS_MAP_GRID_LENGTH * car_xy_pos + 0.5f * vec2(CAR_POS_MAP_SIZE));
				car_pos_infos.push_back(CarPosInfo{ &*iter, &car_pos_map[pos_map_idx.x][pos_map_idx.y] });
				car_pos_map[pos_map_idx.x][pos_map_idx.y].x++;
				iter++;
			}
			else
			{
				iter = vehicles.erase(iter);
			}
		}
		Car* car_pos[MAX_CAR_CNT];
		int num_car_pos = 0;
		for (CarPosInfo& car_pos_info : car_pos_infos)
		{
			if (car_pos_info.pos_map_grids->y == 0)
			{
				car_pos_info.pos_map_grids->y = num_car_pos;
				num_car_pos += car_pos_info.pos_map_grids->x;
				car_pos_info.pos_map_grids->x = car_pos_info.pos_map_grids->y;
			}
			car_pos[car_pos_info.pos_map_grids->y++] = car_pos_info.car;
		}
		for (Car& car : vehicles)
		{
			constexpr float POS_OFFSET = -30.0f / CAR_POS_MAP_GRID_LENGTH + 0.5f;
			ivec2 pos_idx = ivec2(1.0f / CAR_POS_MAP_GRID_LENGTH * vec2(car.getModelMat()[3]) + POS_OFFSET * vec2(car.getDir()) + 0.5f * vec2(CAR_POS_MAP_SIZE) - 0.5f);
			constexpr ivec2 OFFSET[4] = { { 0, 0 }, { 0, 1 }, { 1, 0 }, { 1, 1 } };
			for (int i = 0; i < 4; i++)
			{
				ivec2 idx = pos_idx + OFFSET[i];
				for (int k = car_pos_map[idx.x][idx.y].x; k < car_pos_map[idx.x][idx.y].y; k++)
				{
					car.collisionTest(car_pos[k]);
				}
			}
		}

		logical_data[writing_data].num_cars = vehicles.size();
		if (sun.getDir().z > 0)
		{
			int num_cars = 0;
			for (Car& car : vehicles)
			{
				logical_data[writing_data].car_transform[num_cars] = car.getModelMat();
				logical_data[writing_data].car_color[num_cars] = car.getColor();
				num_cars++;
			}
			logical_data[writing_data].num_light_on_cars = 0;
		}
		else
		{
			int num_light_on_cars = 0;
			int num_light_off_cars = 0;
			for (Car& car : vehicles)
			{
				if (car.isLightOn())
				{
					const mat4& transform = car.getModelMat();
					logical_data[writing_data].car_light_pos[2 * num_light_on_cars] = transform * CAR_LEFT_LIGHT_POS;
					logical_data[writing_data].car_light_dir[2 * num_light_on_cars] = transform * CAR_LEFT_LIGHT_DIR;
					logical_data[writing_data].car_light_pos[2 * num_light_on_cars + 1] = transform * CAR_RIGHT_LIGHT_POS;
					logical_data[writing_data].car_light_dir[2 * num_light_on_cars + 1] = transform * CAR_RIGHT_LIGHT_DIR;
					logical_data[writing_data].car_transform[num_light_on_cars] = car.getModelMat();
					logical_data[writing_data].car_color[num_light_on_cars] = car.getColor();
					num_light_on_cars++;
				}
				else
				{
					logical_data[writing_data].car_transform[vehicles.size() - num_light_off_cars - 1] = car.getModelMat();
					logical_data[writing_data].car_color[vehicles.size() - num_light_off_cars - 1] = car.getColor();
					num_light_off_cars++;
				}
			}
			logical_data[writing_data].num_light_on_cars = num_light_on_cars;
		}
		latest_data = writing_data;
	}
}
