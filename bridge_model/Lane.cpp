#include "Lane.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "common.h"

Lane::Lane(float length, float speedLimit, glm::mat4& startMat, glm::mat4(*motion)(float)) :
	length(length), speedLimit(speedLimit), startMat(startMat), motion(motion) {}

glm::mat4 Lane::transform(float s)
{
	return motion(s) * startMat;
}

void Lane::setNextLane(Lane* nextLane)
{
	this->nextLanes[numNextLanes++] = nextLane;
}

Lane* Lane::getNextLane()
{
	switch (numNextLanes)
	{
	case 0:
		return nullptr;
	case 1:
		return nextLanes[0];
	default:
		std::uniform_int_distribution<int> distb(0, numNextLanes - 1);
		return nextLanes[distb(rdEng)];
	}
}
