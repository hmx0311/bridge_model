#pragma once
#include "glm.hpp"

struct WorldBound
{
	glm::vec3 m_min;
	glm::vec3 m_max;

	WorldBound(glm::vec3 min, glm::vec3 max) :m_min(min), m_max(max) {}

	glm::vec3 center() const
	{
		return (m_min + m_max) * 0.5f;
	}
};

