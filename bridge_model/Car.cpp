#include "Car.h"

#include "common.h"

using namespace glm;

Car::Car(Lane* lane, float sunHeight)
{
	std::uniform_real_distribution<float> distb(0.1f, 0.95f);
	color = vec3(pow(distb(rdEng), 2.2f), pow(distb(rdEng), 2.2f), pow(distb(rdEng), 2.2f));
	s = 0;
	speed = lane->speedLimit;
	aimSpeed = speed;
	while (lane != nullptr)
	{
		path.push_back(lane);
		lane = lane->getNextLane();
	}
	distb.param(std::uniform_real<float>::param_type(-0.2f, 0.0f));
	if (sunHeight < distb(rdEng))
	{
		lightOn = true;
		wasDay = false;
	}
	else
	{
		lightOn = false;
		wasDay = true;
	}
}

bool Car::update(uint32_t time, float sunHeight)
{
	if (time > 0)
	{
		if (speed > aimSpeed)
		{
			if (speed - breakIntensity * time >= aimSpeed)
			{
				s += speed * time - breakIntensity * time * time / 2;
				speed = speed - breakIntensity * time;
			}
			else
			{
				float breakTime = (speed - aimSpeed) / breakIntensity;
				s += (speed + aimSpeed) * breakTime / 2 + aimSpeed * (time - breakTime);
				speed = aimSpeed;
			}
		}
		else if (speed < aimSpeed)
		{
			if (speed + acceleration * time <= aimSpeed)
			{
				s += speed * time + acceleration * time * time / 2;
				speed = speed + acceleration * time;
			}
			else
			{
				float accelerateTime = (aimSpeed - speed) / acceleration;
				s += (speed + aimSpeed) * accelerateTime / 2 + aimSpeed * (time - accelerateTime);
				speed = aimSpeed;
			}
		}
		else
		{
			s += speed * time;
		}
		Lane* curLane = path.front();
		while (s > curLane->length)
		{
			s -= curLane->length;
			path.pop_front();
			if (path.empty())
			{
				return false;
			}
			curLane = path.front();
		}
		aimSpeed = curLane->speedLimit;
		if (path.size() > 1)
		{
			Lane* nextLane = *std::next(path.begin());
			if (nextLane->speedLimit < speed && (speed + nextLane->speedLimit) * (speed - nextLane->speedLimit) / (2 * breakIntensity) > curLane->length - s)
			{
				aimSpeed = nextLane->speedLimit;
			}
		}
		modelMat = curLane->transform(s);
		dir = (mat3)modelMat * vec3(0, 1, 0);

		if (sunHeight > 0)
		{
			wasDay = true;
			lightOn = false;
		}
		else if (sunHeight < -0.2f)
		{
			wasDay = false;
			lightOn = true;
		}
		else if (wasDay && !lightOn)
		{
			std::uniform_real_distribution<float> distb(0, 300);
			if (distb(rdEng) < time * (-sunHeight))
			{
				lightOn = true;
			}
		}
		else if (!wasDay && lightOn)
		{
			std::uniform_real_distribution<float> distb(0, 300);
			if (distb(rdEng) < time * (0.2f + sunHeight))
			{
				lightOn = false;
			}
		}
	}
	return true;
}

void Car::collisionTest(Car* testCar)
{
	if (this == testCar || abs(modelMat[3].z - testCar->modelMat[3].z) > 200)
	{
		return;
	}
	auto thisLane = path.begin();
	float thisDistanceToIntersect = -s;
	auto testLane = testCar->path.begin();
	float testDistanceToIntersect = -testCar->s;
	while (true)
	{
		if (*thisLane == *testLane)
		{
			float minDistance = REACT_TIME * speed + (speed + testCar->speed) * (speed - testCar->speed) / (2 * breakIntensity) + CAR_LENGTH;
			if (testDistanceToIntersect < thisDistanceToIntersect && thisDistanceToIntersect - testDistanceToIntersect < minDistance)
			{
				float b, c;
				if (testDistanceToIntersect < 0)
				{
					b = REACT_TIME * (2 * breakIntensity);
					c = -testCar->speed * testCar->speed + (CAR_LENGTH - thisDistanceToIntersect + testDistanceToIntersect) * (2 * breakIntensity);
				}
				else
				{
					b = (REACT_TIME+ testDistanceToIntersect/ testCar->speed) * (2 * breakIntensity);
					c = -testCar->speed * testCar->speed + (CAR_LENGTH - thisDistanceToIntersect) * (2 * breakIntensity);
				}
				float delta = b * b - 4 * c;
				if (delta < 0)
				{
					aimSpeed = 0;
				}
				else
				{
					aimSpeed = fmin(aimSpeed, (-b + sqrt(delta)) / 2);
				}
			}
			return;
		}
		if (thisDistanceToIntersect < testDistanceToIntersect)
		{
			thisDistanceToIntersect += (*thisLane)->length;
			thisLane++;
			if (thisDistanceToIntersect > BREAK_DISTANCE || thisLane == path.end())
			{
				return;
			}
		}
		else
		{
			testDistanceToIntersect += (*testLane)->length;
			testLane++;
			if (testLane == testCar->path.end())
			{
				return;
			}
		}
	}
}

bool Car::isLightOn()
{
	return lightOn;
}

const glm::vec3 Car::getColor()
{
	return color;
}

const glm::mat4& Car::getModelMat()
{
	return modelMat;
}

const glm::vec3& Car::getDir()
{
	return dir;
}