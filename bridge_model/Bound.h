#pragma once
#include "glm.hpp"

struct BoundBox
{
	glm::vec3 min;
	glm::vec3 max;

	BoundBox(glm::vec3 min, glm::vec3 max) :min(min), max(max) {}

	glm::vec3 center() const
	{
		return 0.5f * (min + max);
	}

	glm::vec3 size() const
	{
		return max - min;
	}
};

