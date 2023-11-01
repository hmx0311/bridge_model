#include "Sun.h"

#include "common.h"

Sun::Sun(float latitude)
{
	this->latitude = latitude * PI / 180;
	updatePosition(0);
}

void Sun::updatePosition(uint64_t date)
{
	float season = asin(0.3987490689f * sin(date % 7200000 * 2 * PI / 7200000));
	float time = date % 360000 * 2 * PI / 360000;
	dir.x = -cos(season) * sin(time);
	dir.y = cos(latitude) * sin(season) - sin(latitude) * cos(season) * cos(time);
	dir.z = sin(latitude) * sin(season) + cos(latitude) * cos(season) * cos(time);
}