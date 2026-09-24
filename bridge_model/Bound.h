#pragma once
#include "glm.hpp"

struct BoundBox
{
	glm::vec3 m_min;
	glm::vec3 m_max;

	BoundBox(glm::vec3 min, glm::vec3 max) :m_min(min), m_max(max) {}

	glm::vec3 center() const
	{
		return 0.5f * (m_min + m_max);
	}

	glm::vec3 size() const
	{
		return m_max - m_min;
	}
};

