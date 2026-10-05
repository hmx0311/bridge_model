#include "Bound.h"

#include <utility>

#include "ext/matrix_common.hpp"

using namespace glm;

BoundAABB::BoundAABB(const BoundAABB& local, const mat4& transform)
{
	vec3 center = transform * vec4(local.center(), 1.0f);
	vec3 local_half = 0.5f * local.size();
	vec3 half = abs(mat3(transform)) * local_half;
	min = center - half;
	max = center + half;
}

vec3 BoundAABB::support(const vec3& dir) const
{
	vec3 half = 0.5f * size();
	if (dir.x < 0) half.x = -half.x;
	if (dir.y < 0) half.y = -half.y;
	if (dir.z < 0) half.z = -half.z;
	return center() + half;
}

void BoundAABB::biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const
{
	vec3 c = center();
	vec3 half = 0.5f * size();
	if (dir.x < 0) half.x = -half.x;
	if (dir.y < 0) half.y = -half.y;
	if (dir.z < 0) half.z = -half.z;
	rsupport_point = c - half;
	support_point = c + half;
}

vec3 BoundOBB::support(const vec3& dir) const
{
	vec3 dir_in_bound = dir * rotation;
	vec3 half = half_size;
	if (dir_in_bound.x < 0) half.x = -half.x;
	if (dir_in_bound.y < 0) half.y = -half.y;
	if (dir_in_bound.z < 0) half.z = -half.z;
	return center + rotation * half;
}

void BoundOBB::biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const
{
	vec3 dir_in_bound = dir * rotation;
	vec3 half = half_size;
	if (dir_in_bound.x < 0) half.x = -half.x;
	if (dir_in_bound.y < 0) half.y = -half.y;
	if (dir_in_bound.z < 0) half.z = -half.z;
	half = rotation * half;
	rsupport_point = center - half;
	support_point = center + half;
}

BoundFrustum::BoundFrustum(const mat4& transform, float fov_y, float aspect, float z_near, float z_far)
{
	vec3 top(0.0f, tanf(fov_y / 2), 0.0f);
	vec3 left = vec3(aspect * top.y, 0.0f, 0.0f);
	top = mat3(transform) * top;
	left = mat3(transform) * left;
	points[0] = transform[3] + z_near * (-transform[2] + vec4((top + left), 0.0f));
	points[1] = transform[3] + z_near * (-transform[2] + vec4((-top + left), 0.0f));
	points[2] = transform[3] + z_near * (-transform[2] + vec4((top - left), 0.0f));
	points[3] = transform[3] + z_near * (-transform[2] + vec4((-top - left), 0.0f));
	points[4] = transform[3] + z_far * (-transform[2] + vec4((top + left), 0.0f));
	points[5] = transform[3] + z_far * (-transform[2] + vec4((-top + left), 0.0f));
	points[6] = transform[3] + z_far * (-transform[2] + vec4((top - left), 0.0f));
	points[7] = transform[3] + z_far * (-transform[2] + vec4((-top - left), 0.0f));
}

vec3 BoundFrustum::support(const vec3& dir) const
{
	float max_support = dot(points[0], dir);
	int max_support_point = 0;
	for (int i = 1; i < 8; i++)
	{
		float p = dot(points[i], dir);
		if (max_support < p)
		{
			max_support = p;
			max_support_point = i;
		}
	}
	return points[max_support_point];
}

void BoundFrustum::biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const
{
	float min_support = dot(points[0], dir);
	float max_support = min_support;
	int min_support_point = 0;
	int max_support_point = 0;
	for (int i = 1; i < 8; i++)
	{
		float p = dot(points[i], dir);
		if (min_support > p)
		{
			min_support = p;
			min_support_point = i;
		}
		if (max_support < p)
		{
			max_support = p;
			max_support_point = i;
		}
	}
	rsupport_point = points[min_support_point];
	support_point = points[max_support_point];
}

BoundEllipticFrustum::BoundEllipticFrustum(const mat4& transform, float fov_y, float aspect, float z_near, float z_far)
{
	near_far_ratio = z_near / z_far;
	vec3 y = vec3{ 0.0f, tanf(fov_y / 2), 0.0f };
	vec3 x = vec3{ aspect * y.y, 0.0f, 0.0f };
	mat3 rot{ transform };
	y = rot * y;
	x = rot * x;
	x_far = z_far * x;
	y_far = z_far * y;
	center_near = transform[3] - z_near * transform[2];
	center_far = transform[3] - z_far * transform[2];
}

vec3 BoundEllipticFrustum::support(const vec3& dir) const
{
	float x = dot(x_far, dir);
	float y = dot(y_far, dir);
	float len = sqrt(x * x + y * y);
	vec3 r_far = len == 0.0f ? vec3{ 0.0f } : (x * x_far + y * y_far) / len;
	vec3 support_far = center_far + r_far;
	vec3 support_near = center_near + near_far_ratio * r_far;
	return dot(support_near, dir) > dot(support_far, dir) ? support_near : support_far;
}

void BoundEllipticFrustum::biSupport(const glm::vec3& dir, glm::vec3& support_point, glm::vec3& rsupport_point) const
{
	float x = dot(x_far, dir);
	float y = dot(y_far, dir);
	float len = sqrt(x * x + y * y);
	vec3 r_far = len == 0.0f ? vec3{ 0.0f } : (x * x_far + y * y_far) / len;
	vec3 support_far = center_far + r_far;
	vec3 support_near = center_near + near_far_ratio * r_far;
	support_point = dot(support_near, dir) > dot(support_far, dir) ? support_near : support_far;
	vec3 rsupport_far = center_far - r_far;
	vec3 rsupport_near = center_near - near_far_ratio * r_far;
	rsupport_point = dot(rsupport_near, dir) < dot(rsupport_far, dir) ? rsupport_near : rsupport_far;
}

bool intersectionTest(const BoundAABB& bound1, const BoundEllipticFrustum& bound2)
{
	vec3 support_min, support_max;
	bound2.biSupport(vec3{ 1.0f, 0.0f, 0.0f }, support_max, support_min);
	if (support_max.x <= bound1.min.x || support_min.x >= bound1.max.x)
	{
		return false;
	}
	bound2.biSupport(vec3{ 0.0f, 1.0f, 0.0f }, support_max, support_min);
	if (support_max.y <= bound1.min.y || support_min.y >= bound1.max.y)
	{
		return false;
	}
	bound2.biSupport(vec3{ 0.0f, 0.0f, 1.0f }, support_max, support_min);
	if (support_max.z <= bound1.min.z || support_min.z >= bound1.max.z)
	{
		return false;
	}
	return true;
}

bool intersectionTest(const BoundEllipticFrustum& bound1, const BoundAABB& bound2)
{
	vec3 box_center = bound2.center();
	vec3 box_half_size = 0.5f * bound2.size();
	vec3 frustum_dir = bound1.center_near - bound1.center_far;
	float center_dist = length(frustum_dir);
	frustum_dir *= 1.0f / center_dist;
	float m = dot(frustum_dir, box_center);
	float r = dot(abs(frustum_dir), box_half_size);
	if (m + r <= dot(frustum_dir, bound1.center_far) || m - r >= dot(frustum_dir, bound1.center_near))
	{
		return false;
	}
	float x_far_len = length(bound1.x_far);
	float y_far_len = length(bound1.y_far);
	vec3 frustum_x = 1.0f / x_far_len * bound1.x_far;
	vec3 frustum_y = 1.0f / y_far_len * bound1.y_far;
	vec3 v = box_center - bound1.center_far;
	float x = dot(v, frustum_x);
	float y = dot(v, frustum_y);
	float d = sqrt(x * x / (x_far_len * x_far_len) + y * y / (y_far_len * y_far_len));
	if (d > 0.0f)
	{
		x /= d;
		y /= d;
		vec3 n{ -x / (x_far_len * x_far_len), -y / (y_far_len * y_far_len), -(1 - bound1.near_far_ratio) / center_dist };
		n = normalize(n);
		n = n.x * frustum_x + n.y * frustum_y + n.z * frustum_dir;
		float plane_d = dot(bound1.center_far + x * frustum_x + y * frustum_y, n);
		if (dot(n, box_center) + dot(abs(n), box_half_size) <= plane_d)
		{
			return false;
		}
	}
	return true;
}

bool intersectionTest(const BoundOBB& bound1, const BoundEllipticFrustum& bound2)
{
	vec3 support_min, support_max;
	vec3 center_d = bound1.center * bound1.rotation;
	bound2.biSupport(bound1.rotation[0], support_max, support_min);
	if (dot(support_max, bound1.rotation[0]) <= center_d.x - bound1.half_size.x || dot(support_min, bound1.rotation[0]) >= center_d.x + bound1.half_size.x)
	{
		return false;
	}
	bound2.biSupport(bound1.rotation[1], support_max, support_min);
	if (dot(support_max, bound1.rotation[1]) <= center_d.y - bound1.half_size.y || dot(support_min, bound1.rotation[1]) >= center_d.y + bound1.half_size.y)
	{
		return false;
	}
	bound2.biSupport(bound1.rotation[2], support_max, support_min);
	if (dot(support_max, bound1.rotation[2]) <= center_d.z - bound1.half_size.z || dot(support_min, bound1.rotation[2]) >= center_d.z + bound1.half_size.z)
	{
		return false;
	}
	return true;
}

bool intersectionTest(const BoundEllipticFrustum& bound1, const BoundOBB& bound2)
{
	vec3 frustum_dir = bound1.center_near - bound1.center_far;
	float center_dist = length(frustum_dir);
	frustum_dir *= 1.0f / center_dist;
	vec3 support_min, support_max;
	bound2.biSupport(frustum_dir, support_max, support_min);
	if (dot(frustum_dir, support_max) <= dot(frustum_dir, bound1.center_far) || dot(frustum_dir, support_min) >= dot(frustum_dir, bound1.center_near))
	{
		return false;
	}
	float x_far_len = length(bound1.x_far);
	float y_far_len = length(bound1.y_far);
	vec3 frustum_x = 1.0f / x_far_len * bound1.x_far;
	vec3 frustum_y = 1.0f / y_far_len * bound1.y_far;
	vec3 v = bound2.center - bound1.center_far;
	float x = dot(v, frustum_x);
	float y = dot(v, frustum_y);
	float d = sqrt(x * x / (x_far_len * x_far_len) + y * y / (y_far_len * y_far_len));
	if (d > 0.0f)
	{
		x /= d;
		y /= d;
		vec3 n{ -x / (x_far_len * x_far_len), -y / (y_far_len * y_far_len), -(1 - bound1.near_far_ratio) / center_dist };
		n = normalize(n);
		n = n.x * frustum_x + n.y * frustum_y + n.z * frustum_dir;
		float plane_d = dot(bound1.center_far + x * frustum_x + y * frustum_y, n);
		if (dot(n, bound2.support(n)) <= plane_d)
		{
			return false;
		}
	}
	return true;
}
