#pragma once
#include <list>
#include "glm.hpp"

#include "Bound.h"
#include "shader_headers/scene_constances.h"

constexpr float REACT_TIME = 0.8f;
constexpr float CAR_LENGTH = 3.8f;
constexpr float CAR_LIGHT_V_RAD = 0.51f;
constexpr float CAR_LIGHT_ASPECT = 2.0f;
constexpr float CAR_LIGHT_NEAR = 0.7f;

constexpr glm::vec4 CAR_LEFT_LIGHT_POS(-0.56f, 1.5f, 0.81f, 1);
constexpr glm::vec4 CAR_RIGHT_LIGHT_POS(0.56f, 1.5f, 0.81f, 1);
constexpr glm::vec4 CAR_LEFT_LIGHT_DIR(-0.096f, 0.9584f, -0.2688f, 0);
constexpr glm::vec4 CAR_RIGHT_LIGHT_DIR(0.096f, 0.9584f, -0.2688f, 0);

class Lane;

class Car
{
private:
	float m_s = 0.0f;
	float m_speed;
	float m_aim_speed;
	float m_lane_offset = 0.0f;
	float m_aim_lane_offset = 0.0f;
	bool m_is_light_on;
	bool m_was_day;
	std::list<Lane*> m_path;
	glm::mat4 m_transform = glm::mat4(0);
	glm::vec3 m_color;

public:
	Car(Lane* lane, float sun_height);

	bool update(float dt, float sun_height);
	void avoidCollisionWith(Car* car);
	bool isLightOn();
	const glm::vec3& getColor();
	const glm::mat4& getTransform();
};