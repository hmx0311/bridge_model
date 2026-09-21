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

	struct Plane
	{
		glm::vec3 normal;
		float d;
	};
private:
	Plane m_planes[6];

public:
	Frustum(const glm::mat4& view, const glm::mat4& proj);

	VIEW_TEST_RESULT viewTest(const BoundBox& bound) const;
};

