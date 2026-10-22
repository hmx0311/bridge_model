#include "Car.h"

#include "Lane.h"
#include "logical_frame.h"

#include "shader_headers/scene_constances.h"

using namespace glm;

constexpr float ACCELERATION = 2.0f;
constexpr float BREAK_INTENSITY = 4.0f;
constexpr float BREAK_DISTANCE = 120.0f;
constexpr float LANE_OFFSET_SIGMA = 0.1f;
constexpr float MAX_LANE_OFFSET = 6.0f * LANE_OFFSET_SIGMA;
constexpr float LANE_OFFSET_SPEED_RATIO = 0.002f;

Car::Car(uint32_t id, Lane* lane, float sun_height) :
	id(id)
{
	std::uniform_real_distribution<float> color_distb(0.1f, 0.95f);
	m_color = vec3(pow(color_distb(rd_eng), 2.2f), pow(color_distb(rd_eng), 2.2f), pow(color_distb(rd_eng), 2.2f));
	m_speed = lane->speed_limit;
	m_aim_speed = m_speed;
	while (lane != nullptr)
	{
		m_path.push_back(lane);
		lane = lane->getNextLane();
	}

	std::uniform_real_distribution<float> light_distb(-0.2f, 0.0f);
	if (sun_height < light_distb(rd_eng))
	{
		m_is_light_on = true;
		m_was_day = false;
	}
	else
	{
		m_is_light_on = false;
		m_was_day = true;
	}
}

bool Car::update(float dt, float sun_height)
{
	if (m_speed > m_aim_speed)
	{
		if (m_speed - BREAK_INTENSITY * dt >= m_aim_speed)
		{
			m_s += m_speed * dt - BREAK_INTENSITY * dt * dt / 2;
			m_speed = m_speed - BREAK_INTENSITY * dt;
		}
		else
		{
			float break_time = (m_speed - m_aim_speed) / BREAK_INTENSITY;
			m_s += (m_speed + m_aim_speed) * break_time / 2 + m_aim_speed * (dt - break_time);
			m_speed = m_aim_speed;
		}
	}
	else if (m_speed < m_aim_speed)
	{
		if (m_speed + ACCELERATION * dt <= m_aim_speed)
		{
			m_s += m_speed * dt + ACCELERATION * dt * dt / 2;
			m_speed = m_speed + ACCELERATION * dt;
		}
		else
		{
			float accelerate_time = (m_aim_speed - m_speed) / ACCELERATION;
			m_s += (m_speed + m_aim_speed) * accelerate_time / 2 + m_aim_speed * (dt - accelerate_time);
			m_speed = m_aim_speed;
		}
	}
	else
	{
		m_s += m_speed * dt;
	}
	Lane* cur_lane = m_path.front();
	while (m_s > cur_lane->length)
	{
		m_s -= cur_lane->length;
		m_path.pop_front();
		if (m_path.empty())
		{
			return false;
		}
		cur_lane = m_path.front();
	}
	m_aim_speed = cur_lane->speed_limit;
	if (m_path.size() > 1)
	{
		Lane* next_lane = *std::next(m_path.begin());
		if (next_lane->speed_limit < m_speed && (m_speed + next_lane->speed_limit) * (m_speed - next_lane->speed_limit) / (2 * BREAK_INTENSITY) > cur_lane->length - m_s)
		{
			m_aim_speed = next_lane->speed_limit;
		}
	}
	m_transform = cur_lane->transform(m_s);

	if (abs(m_lane_offset - m_aim_lane_offset) < m_speed * dt * LANE_OFFSET_SPEED_RATIO)
	{
		std::normal_distribution<float> distb(0.0f, LANE_OFFSET_SIGMA);
		m_aim_lane_offset = clamp(distb(rd_eng), -MAX_LANE_OFFSET, MAX_LANE_OFFSET);
	}
	if (m_lane_offset < m_aim_lane_offset)
	{
		m_lane_offset = std::min(m_lane_offset + m_speed * dt * LANE_OFFSET_SPEED_RATIO, m_aim_lane_offset);
	}
	else
	{
		m_lane_offset = std::max(m_lane_offset - m_speed * dt * LANE_OFFSET_SPEED_RATIO, m_aim_lane_offset);
	}
	m_transform[3] += m_lane_offset * m_transform[0];

	if (sun_height > -SUN_RADIUS_DIST_RATIO)
	{
		m_was_day = true;
		m_is_light_on = false;
	}
	else if (sun_height < -0.2f)
	{
		m_was_day = false;
		m_is_light_on = true;
	}
	else if (m_was_day && !m_is_light_on)
	{
		std::uniform_real_distribution<float> distb(0, 0.3f);
		if (distb(rd_eng) < dt * (-sun_height - SUN_RADIUS_DIST_RATIO))
		{
			m_is_light_on = true;
		}
	}
	else if (!m_was_day && m_is_light_on)
	{
		std::uniform_real_distribution<float> distb(0, 0.3f);
		if (distb(rd_eng) < dt * (0.2f + sun_height))
		{
			m_is_light_on = false;
		}
	}
	return true;
}

void Car::avoidCollisionWith(Car* test_car)
{
	if (this == test_car || abs(m_transform[3].z - test_car->m_transform[3].z) > 2.0f)
	{
		return;
	}
	auto this_lane = m_path.begin();
	float this_distance_to_intersect = -m_s;
	auto test_lane = test_car->m_path.begin();
	float test_distance_to_intersect = -test_car->m_s;
	while (true)
	{
		if (*this_lane == *test_lane)
		{
			float min_distance = REACT_TIME * m_speed + (m_speed + test_car->m_speed) * (m_speed - test_car->m_speed) / (2 * BREAK_INTENSITY) + CAR_LENGTH;
			if (test_distance_to_intersect < this_distance_to_intersect && this_distance_to_intersect - test_distance_to_intersect < min_distance)
			{
				float b, c;
				if (test_distance_to_intersect < 0)
				{
					b = REACT_TIME * (2 * BREAK_INTENSITY);
					c = -test_car->m_speed * test_car->m_speed + (CAR_LENGTH - this_distance_to_intersect + test_distance_to_intersect) * (2 * BREAK_INTENSITY);
				}
				else
				{
					b = (REACT_TIME + test_distance_to_intersect / test_car->m_speed) * (2 * BREAK_INTENSITY);
					c = -test_car->m_speed * test_car->m_speed + (CAR_LENGTH - this_distance_to_intersect) * (2 * BREAK_INTENSITY);
				}
				float delta = b * b - 4 * c;
				if (delta < 0)
				{
					m_aim_speed = 0;
				}
				else
				{
					m_aim_speed = std::min(m_aim_speed, (-b + sqrt(delta)) / 2);
				}
			}
			return;
		}
		if (this_distance_to_intersect < test_distance_to_intersect)
		{
			this_distance_to_intersect += (*this_lane)->length;
			this_lane++;
			if (this_distance_to_intersect > BREAK_DISTANCE || this_lane == m_path.end())
			{
				return;
			}
		}
		else
		{
			test_distance_to_intersect += (*test_lane)->length;
			test_lane++;
			if (test_lane == test_car->m_path.end())
			{
				return;
			}
		}
	}
}

bool Car::isLightOn()
{
	return m_is_light_on;
}

const vec3& Car::getColor()
{
	return m_color;
}

const mat4& Car::getTransform()
{
	return m_transform;
}