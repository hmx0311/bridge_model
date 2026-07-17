#include "Sun.h"

#include "common.h"

using namespace glm;

Sun::Sun(float latitude)
{
	m_latitude = latitude * PI / 180;
	updatePosition(0);
}

void Sun::updatePosition(uint64_t time_ms)
{
	float season = asin(0.3987490689f * sin(time_ms % 7200000 * 2 * PI / 7200000));
	float day_angle = time_ms % 360000 * 2 * PI / 360000;
	m_dir.x = -cos(season) * sin(day_angle);
	m_dir.y = cos(m_latitude) * sin(season) - sin(m_latitude) * cos(season) * cos(day_angle);
	m_dir.z = sin(m_latitude) * sin(season) + cos(m_latitude) * cos(season) * cos(day_angle);
}

const vec3& Sun::getDir()
{
	return m_dir;
}