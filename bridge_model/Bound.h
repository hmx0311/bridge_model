#pragma once
#include "glm.hpp"

struct BoundBox
{
	glm::vec3 min;
	glm::vec3 max;

	BoundBox(const glm::vec3& min, const glm::vec3& max) :min(min), max(max) {}
	BoundBox(const BoundBox& local, const glm::mat4& transform);

	glm::vec3 center() const
	{
		return 0.5f * (min + max);
	}

	glm::vec3 size() const
	{
		return max - min;
	}
};

struct BoundSphere
{
	glm::vec3 center;
	float r;

	BoundSphere(const glm::vec3& center, float r) :center(center), r(r) {}
};
