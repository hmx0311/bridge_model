#pragma once

#include "glm\glm.hpp"

class Sun
{
public:
	glm::vec3 dir = { 0.0f, 0.0f, 0.0f };
	Sun(float latitude);

	void updatePosition(uint64_t date);
private:
	float latitude;
};