#pragma once
#include "Bound.h"

#undef NEAR
#undef FAR

struct Plane
{
	glm::vec3 normal;
	float d;
	// n * v + d = 0
};

class Frustum
{
public:
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

	bool cullingTest(const BoundAABB& bound) const;
	INTERSECTION_TEST_RESULT intersectionTest(const BoundAABB& bound) const;
	bool cullingTestNoNearFar(const BoundAABB& bound) const;
	INTERSECTION_TEST_RESULT intersectionTestNoNearFar(const BoundAABB& bound) const;

	bool cullingTest(const BoundOBB& bound) const;
	INTERSECTION_TEST_RESULT intersectionTest(const BoundOBB& bound) const;

	bool cullingTest(const BoundSphere& bound) const;
	INTERSECTION_TEST_RESULT intersectionTest(const BoundSphere& bound) const;

	bool cullingTest(const BoundEllipticFrustum& bound) const;
	INTERSECTION_TEST_RESULT intersectionTest(const BoundEllipticFrustum& bound) const;

	const Plane& getPlane(PLANE_INDEX plane) const
	{
		return m_planes[plane];
	}
};

