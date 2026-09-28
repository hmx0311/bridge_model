#pragma once
#include "glm.hpp"

#undef NEAR
#undef FAR

struct BoundBox;
struct BoundSphere;

struct Plane
{
	glm::vec3 normal;
	float d;
	// n * v + d = 0
};

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
		LEFT = 0,
		RIGHT = 1,
		BOTTOM = 2,
		TOP = 3,
		NEAR = 4,
		FAR = 5,
	};

private:
	Plane m_planes[6];

public:
	Frustum(const glm::mat4& vp);
	Frustum(const glm::mat4& view, float fov_y, float aspect, float z_near, float z_far);
	Frustum(const glm::mat4& view, float left, float right, float bottom, float top, float z_near, float z_far);

	VIEW_TEST_RESULT intersectTest(const BoundBox& bound) const;
	VIEW_TEST_RESULT intersectTestNoNearFar(const BoundBox& bound) const;

	VIEW_TEST_RESULT intersectTest(const BoundSphere& bound) const;

	const Plane& getPlane(PLANE_INDEX plane) const
	{
		return m_planes[plane];
	}
};

