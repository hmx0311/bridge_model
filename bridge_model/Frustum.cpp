#include "Frustum.h"

#include "Bound.h"

using namespace glm;

static Frustum::Plane MakePlane(const vec4& v)
{
	Frustum::Plane p;
	vec3 n(v.x, v.y, v.z);
	float len = length(n);
	p.normal = n / len;
	p.d = v.w / len;
	return p;
}

Frustum::Frustum(const glm::mat4& view, const glm::mat4& proj)
{
    glm::mat4 vp = proj * view;

   vec4 row0(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
   vec4 row1(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
   vec4 row2(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
   vec4 row3(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);
   
    m_planes[0] = MakePlane(row3 + row0); // left
    m_planes[1] = MakePlane(row3 - row0); // right
    m_planes[2] = MakePlane(row3 + row1); // bottom
    m_planes[3] = MakePlane(row3 - row1); // top
    m_planes[4] = MakePlane(row3 + row2); // near (OpenGL)
    m_planes[5] = MakePlane(row3 - row2); // far
}

Frustum::VIEW_TEST_RESULT Frustum::viewTest(const WorldBound& bound) const
{
	bool inside = true;
	for (int i = 0; i < 6; i++)
	{
		auto& p = m_planes[i];
		float m = dot(p.normal, bound.center()) + p.d;
		vec3 half_size = (bound.m_max - bound.m_min) * 0.5f;
		float r = dot(half_size, abs(p.normal));
		if (m + r < 0)
		{
			return VIEW_TEST_OUTSIDE;
		}
		if (m - r < 0)
		{
			inside = false;
		}
	}
	return inside ? VIEW_TEST_INSIDE : VIEW_TEST_INTERSECT;
}
