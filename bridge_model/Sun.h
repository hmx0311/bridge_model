#pragma once
#include "glm.hpp"

constexpr double YEAR_PERIOD = 9000.0;
constexpr double DAY_PERIOD = 600.0;

class Sun
{
public:
	Sun(float latitude);

	const glm::vec3& getDir();
	void updatePosition(double time);
private:
	glm::vec3 m_dir = { 0.0f, 0.0f, 0.0f };
	float m_latitude;
};