#pragma once

#include <list>

#include "glm\glm.hpp"

#include "Lane.h"

#define REACT_TIME 800
#define CAR_LENGTH 400
#define BREAK_DISTANCE 12000
#define CAR_LIGHT_V_COS_ANGLE 0.994f
#define CAR_LIGHT_ASPECT 2.5f

constexpr glm::vec4 CAR_LEFT_LIGHT_POS(-45, 150, 72, 1);
constexpr glm::vec4 CAR_RIGHT_LIGHT_POS(45, 150, 72, 1);
constexpr glm::vec4 CAR_LEFT_LIGHT_DIR(-0.096f, 0.9856f, -0.1392f, 0);
constexpr glm::vec4 CAR_RIGHT_LIGHT_DIR(0.096f, 0.9856f, -0.1392f, 0);

class Car
{
private:
	float s;	//cm
	float speed;	// cm/ms
	float aimSpeed;	// cm/ms
	const float acceleration = 0.0002f;	// cm/ms^2
	const float breakIntensity = 0.0004f;	// cm/ms^2
	bool lightOn;
	bool wasDay;
	std::list<Lane*> path;
	glm::vec3 color;
	glm::mat4 modelMat = glm::mat4(0);
	glm::vec3 dir = glm::vec3(0);

public:
	Car(Lane* lane, float sunHeight);
	bool update(uint32_t time, float sunHeight);
	void collisionTest(Car* car);
	bool isLightOn();
	const glm::vec3 getColor();
	const glm::mat4& getModelMat();
	const glm::vec3& getDir();
};