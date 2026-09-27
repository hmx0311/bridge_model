#include "Frustum.h"

#include "Bound.h"

using namespace glm;

static Plane MakePlane(const vec4& v)
{
	Plane p;
	vec3 n(v.x, v.y, v.z);
	float len = length(n);
	p.normal = n / len;
	p.d = v.w / len;
	return p;
}

Frustum::Frustum(const mat4& vp)
{
	vec4 row0(vp[0][0], vp[1][0], vp[2][0], vp[3][0]);
	vec4 row1(vp[0][1], vp[1][1], vp[2][1], vp[3][1]);
	vec4 row2(vp[0][2], vp[1][2], vp[2][2], vp[3][2]);
	vec4 row3(vp[0][3], vp[1][3], vp[2][3], vp[3][3]);

	m_planes[LEFT] = MakePlane(row3 + row0);
	m_planes[RIGHT] = MakePlane(row3 - row0);
	m_planes[BOTTOM] = MakePlane(row3 + row1);
	m_planes[TOP] = MakePlane(row3 - row1);
	m_planes[NEAR] = MakePlane(row3 + row2);
	m_planes[FAR] = MakePlane(row3 - row2);
}

Frustum::Frustum(const mat4& view, float fov_y, float aspect, float z_near, float z_far)
{
	float tan_y = std::tan(0.5f * fov_y);
	float tan_x = tan_y * aspect;
	float r_len_x = 1.0f / sqrt(1 + tan_x * tan_x);
	float r_len_y = 1.0f / sqrt(1 + tan_y * tan_y);

	vec4 planes[6] = {
		{ r_len_x, 0.0f, -r_len_x * tan_x, 0.0f },	// left
		{ -r_len_x, 0.0f, -r_len_x * tan_x, 0.0f },	// right
		{ 0.0f, r_len_y, -r_len_y * tan_y, 0.0f },	// bottom
		{ 0.0f, -r_len_y, -r_len_y * tan_y, 0.0f },	// top
		{ 0.0f, 0.0f, -1.0f, -z_near},				// near
		{ 0.0f, 0.0f, 1.0f, z_far},					// far
	};

	mat4 transpose_view = transpose(view);
	for (int i = 0; i < 6; i++)
	{
		vec4 plane = transpose_view * planes[i];
		m_planes[i] = { vec3(plane), plane.w };
	}
}

Frustum::Frustum(const mat4& view, float left, float right, float bottom, float top, float z_near, float z_far)
{
	vec4 planes[6] = {
		{ 1.0f, 0.0f, 0.0f, -left },	// left
		{ -1.0f, 0.0f, 0.0f, right },	// right
		{ 0.0f, 1.0f, 0.0f, -bottom },	// bottom
		{ 0.0f, -1.0f, 0.0f, top },		// top
		{ 0.0f, 0.0f, -1.0f, -z_near},	// near
		{ 0.0f, 0.0f, 1.0f, z_far},		// far
	};

	mat4 transpose_view = transpose(view);
	for (int i = 0; i < 6; i++)
	{
		vec4 plane = transpose_view * planes[i];
		m_planes[i] = { vec3(plane), plane.w };
	}
}

Frustum::VIEW_TEST_RESULT Frustum::intersectTest(const BoundBox& bound) const
{
	bool inside = true;
	for (int i = 0; i < 6; i++)
	{
		auto& p = m_planes[i];
		float m = dot(p.normal, bound.center()) + p.d;
		vec3 half_size = 0.5f * bound.size();
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

Frustum::VIEW_TEST_RESULT Frustum::intersectTestNoNearFar(const BoundBox& bound) const
{
	bool inside = true;
	for (int i = 0; i < 4; i++)
	{
		auto& p = m_planes[i];
		float m = dot(p.normal, bound.center()) + p.d;
		vec3 half_size = 0.5f * bound.size();
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

