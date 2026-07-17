#pragma once

#include "matrix.hpp"

class Lane 
{
public:
	const float length;
	const float speed_limit;

	Lane(float length, float speed_limit, const glm::mat4& start_mat, glm::mat4(*motion)(float));
	glm::mat4 transform(float s);
	void setNextLane(Lane* next_lane);
	Lane* getNextLane();

private:
	Lane* m_next_lanes[2];
	int m_num_next_lanes = 0;
	glm::mat4 m_start_mat;
	glm::mat4(*m_motion)(float);
};
