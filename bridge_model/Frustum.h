#pragma once
#include "glm.hpp"

struct BoundBox;

class Frustum
{
public:
	enum VIEW_TEST_RESULT
	{
		VIEW_TEST_OUTSIDE = 0,
		VIEW_TEST_INSIDE = 1,
		VIEW_TEST_INTERSECT = 2,
	};

	enum PLANE_INDEX
	{
		PLANE_LEFT = 0,
		PLANE_RIGHT = 1,
		PLANE_BOTTOM = 2,
		PLANE_TOP = 3,
		PLANE_NEAR = 4,
		PLANE_FAR = 5,
	};

	struct Plane
	{
		glm::vec3 normal;
		float d;
	};

private:
	Plane m_planes[6];

public:
	Frustum(const glm::mat4& view, const glm::mat4& proj);

	VIEW_TEST_RESULT intersectTest(const BoundBox& bound) const;

	VIEW_TEST_RESULT intersectTestNoNearFar(const BoundBox& bound) const;

	Plane getPlane(PLANE_INDEX plane) const
	{
		return m_planes[plane];
	}
};

