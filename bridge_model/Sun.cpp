#include "Sun.h"

#include "ext/scalar_constants.hpp"

using namespace glm;

constexpr double YEAR_PERIOD = 9000.0;
constexpr double DAY_PERIOD = 600.0;

Sun::Sun(float latitude)
{
	m_latitude = latitude * pi<float>() / 180;
	updatePosition(0);
}

void Sun::updatePosition(double time)
{
	float season = asin(0.3987490689f * sin(2 * pi<float>() / YEAR_PERIOD * fmod(time, YEAR_PERIOD)));
	float day_angle = 2 * pi<float>() / DAY_PERIOD * fmod(time, DAY_PERIOD);
	m_dir.x = cos(season) * cos(day_angle);
	m_dir.y = cos(m_latitude) * sin(season) - sin(m_latitude) * cos(season) * sin(day_angle);
	m_dir.z = sin(m_latitude) * sin(season) + cos(m_latitude) * cos(season) * sin(day_angle);
}

const vec3& Sun::getDir()
{
	return m_dir;
}