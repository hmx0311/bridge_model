#pragma once

#include "glm\matrix.hpp"

class Lane {
public:
	const float length;
	const float speedLimit;

	Lane(float length, float speedLimit, glm::mat4& startMat, glm::mat4(*motion)(float));
	glm::mat4 transform(float s);
	void setNextLane(Lane* nextLane);
	Lane* getNextLane();

private:
	Lane* nextLanes[2];
	int numNextLanes = 0;
	glm::mat4 startMat;
	glm::mat4(*motion)(float);
};