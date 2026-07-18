#pragma once
#include "glm.hpp"

#include <list>

constexpr float REACT_TIME = 0.8f;
constexpr float CAR_LENGTH = 400;
constexpr float CAR_LIGHT_V_COS_ANGLE = 0.994f;
constexpr float CAR_LIGHT_ASPECT = 2.5f;
constexpr float CAR_LIGHT_RANGE = 3200;

constexpr glm::vec4 CAR_LEFT_LIGHT_POS(-45, 150, 72, 1);
constexpr glm::vec4 CAR_RIGHT_LIGHT_POS(45, 150, 72, 1);
constexpr glm::vec4 CAR_LEFT_LIGHT_DIR(-0.096f, 0.9856f, -0.1392f, 0);
constexpr glm::vec4 CAR_RIGHT_LIGHT_DIR(0.096f, 0.9856f, -0.1392f, 0);

class Lane;

class Car
{
private:
	float m_s;	//cm
	float m_speed;	// cm/s
	float m_aim_speed;	// cm/s
	bool m_is_light_on;
	bool m_was_day;
	std::list<Lane*> m_path;
	glm::mat4 m_transform = glm::mat4(0);
	glm::vec3 m_dir = glm::vec3(0);
	glm::vec3 m_color;

public:
	Car(Lane* lane, float sun_height);
	bool update(float dt, float sun_height);
	void collisionTest(Car* car);
	bool isLightOn();
	const glm::vec3& getColor();
	const glm::mat4& getModelMat();
	const glm::vec3& getDir();
};