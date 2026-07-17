#include "Lane.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "common.h"

Lane::Lane(float length, float speed_limit, const glm::mat4& start_mat, glm::mat4(*motion)(float)) :
	length(length), speed_limit(speed_limit), m_start_mat(start_mat), m_motion(motion) {}

glm::mat4 Lane::transform(float s)
{
	return m_motion(s) * m_start_mat;
}

void Lane::setNextLane(Lane* next_lane)
{
	m_next_lanes[m_num_next_lanes++] = next_lane;
}

Lane* Lane::getNextLane()
{
	switch (m_num_next_lanes)
	{
	case 0:
		return nullptr;
	case 1:
		return m_next_lanes[0];
	default:
		std::uniform_int_distribution<int> distb(0, m_num_next_lanes - 1);
		return m_next_lanes[distb(rd_eng)];
	}
}
