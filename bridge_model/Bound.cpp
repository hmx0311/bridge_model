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

vec3 BoundFrustum::center() const
{
	return 0.125f * (points[0] + points[1] + points[2] + points[3] + points[4] + points[5] + points[6] + points[7]);
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

bool collisionTest(const BoundAABB& primary, const BoundEllipticFrustum& secondary)
{
	vec3 support_min, support_max;
	secondary.biSupport(vec3{ 1.0f, 0.0f, 0.0f }, support_max, support_min);
	if (support_max.x <= primary.min.x || support_min.x >= primary.max.x)
	{
		return false;
	}
	secondary.biSupport(vec3{ 0.0f, 1.0f, 0.0f }, support_max, support_min);
	if (support_max.y <= primary.min.y || support_min.y >= primary.max.y)
	{
		return false;
	}
	secondary.biSupport(vec3{ 0.0f, 0.0f, 1.0f }, support_max, support_min);
	if (support_max.z <= primary.min.z || support_min.z >= primary.max.z)
	{
		return false;
	}
	return true;
}

bool collisionTest(const BoundEllipticFrustum& primary, const BoundAABB& secondary)
{
	vec3 box_center = secondary.center();
	vec3 box_half_size = 0.5f * secondary.size();
	vec3 frustum_dir = primary.center_near - primary.center_far;
	float center_dist = length(frustum_dir);
	frustum_dir *= 1.0f / center_dist;
	float m = dot(frustum_dir, box_center);
	float r = dot(abs(frustum_dir), box_half_size);
	if (m + r <= dot(frustum_dir, primary.center_far) || m - r >= dot(frustum_dir, primary.center_near))
	{
		return false;
	}
	float x_far_len = length(primary.x_far);
	float y_far_len = length(primary.y_far);
	vec3 frustum_x = 1.0f / x_far_len * primary.x_far;
	vec3 frustum_y = 1.0f / y_far_len * primary.y_far;
	vec3 v = box_center - primary.center_far;
	float x = dot(v, frustum_x);
	float y = dot(v, frustum_y);
	float d = sqrt(x * x / (x_far_len * x_far_len) + y * y / (y_far_len * y_far_len));
	if (d > 0.0f)
	{
		x /= d;
		y /= d;
		vec3 n{ -x / (x_far_len * x_far_len), -y / (y_far_len * y_far_len), -(1 - primary.near_far_ratio) / center_dist };
		n = normalize(n);
		n = n.x * frustum_x + n.y * frustum_y + n.z * frustum_dir;
		float plane_d = dot(primary.center_far + x * frustum_x + y * frustum_y, n);
		if (dot(n, box_center) + dot(abs(n), box_half_size) <= plane_d)
		{
			return false;
		}
	}
	return true;
}

void collisionTests(const BoundEllipticFrustum& primary, const BoundAABB* bounds, int count, bool* results)
{
	vec3 frustum_dir = primary.center_near - primary.center_far;
	float center_dist = length(frustum_dir);
	frustum_dir *= 1.0f / center_dist;
	vec3 abs_frustum_dir = abs(frustum_dir);
	float ra = 1.0f / length(primary.x_far);
	float rb = 1.0f / length(primary.y_far);
	vec3 frustum_x = ra * primary.x_far;
	vec3 frustum_y = rb * primary.y_far;
	float d_far = dot(frustum_dir, primary.center_far);
	float d_near = dot(frustum_dir, primary.center_near);
	for (int i = 0; i < count; i++)
	{
		vec3 box_center = bounds[i].center();
		vec3 box_half_size = 0.5f * bounds[i].size();
		float m = dot(frustum_dir, box_center);
		float r = dot(abs_frustum_dir, box_half_size);
		if (m + r <= d_far || m - r >= d_near)
		{
			results[i] = false;
			continue;
		}
		vec3 v = box_center - primary.center_far;
		float x = dot(v, frustum_x);
		float y = dot(v, frustum_y);
		float d = sqrt(x * x * (ra * ra) + y * y * (rb * rb));
		if (d > 0.0f)
		{
			x /= d;
			y /= d;
			vec3 n{ -x * (ra * ra), -y * (rb * rb), -(1 - primary.near_far_ratio) / center_dist };
			n = normalize(n);
			n = n.x * frustum_x + n.y * frustum_y + n.z * frustum_dir;
			float plane_d = dot(primary.center_far + x * frustum_x + y * frustum_y, n);
			if (dot(n, box_center) + dot(abs(n), box_half_size) <= plane_d)
			{
				results[i] = false;
				continue;
			}
		}
		results[i] = true;
	}
}

bool collisionTest(const BoundOBB& primary, const BoundEllipticFrustum& secondary)
{
	vec3 support_min, support_max;
	vec3 center_d = primary.center * primary.rotation;
	secondary.biSupport(primary.rotation[0], support_max, support_min);
	if (dot(support_max, primary.rotation[0]) <= center_d.x - primary.half_size.x || dot(support_min, primary.rotation[0]) >= center_d.x + primary.half_size.x)
	{
		return false;
	}
	secondary.biSupport(primary.rotation[1], support_max, support_min);
	if (dot(support_max, primary.rotation[1]) <= center_d.y - primary.half_size.y || dot(support_min, primary.rotation[1]) >= center_d.y + primary.half_size.y)
	{
		return false;
	}
	secondary.biSupport(primary.rotation[2], support_max, support_min);
	if (dot(support_max, primary.rotation[2]) <= center_d.z - primary.half_size.z || dot(support_min, primary.rotation[2]) >= center_d.z + primary.half_size.z)
	{
		return false;
	}
	return true;
}

bool collisionTest(const BoundEllipticFrustum& primary, const BoundOBB& secondary)
{
	vec3 frustum_dir = primary.center_near - primary.center_far;
	float center_dist = length(frustum_dir);
	frustum_dir *= 1.0f / center_dist;
	vec3 support_min, support_max;
	secondary.biSupport(frustum_dir, support_max, support_min);
	if (dot(frustum_dir, support_max) <= dot(frustum_dir, primary.center_far) || dot(frustum_dir, support_min) >= dot(frustum_dir, primary.center_near))
	{
		return false;
	}
	float x_far_len = length(primary.x_far);
	float y_far_len = length(primary.y_far);
	vec3 frustum_x = 1.0f / x_far_len * primary.x_far;
	vec3 frustum_y = 1.0f / y_far_len * primary.y_far;
	vec3 v = secondary.center - primary.center_far;
	float x = dot(v, frustum_x);
	float y = dot(v, frustum_y);
	float d = sqrt(x * x / (x_far_len * x_far_len) + y * y / (y_far_len * y_far_len));
	if (d > 0.0f)
	{
		x /= d;
		y /= d;
		vec3 n{ -x / (x_far_len * x_far_len), -y / (y_far_len * y_far_len), -(1 - primary.near_far_ratio) / center_dist };
		n = normalize(n);
		n = n.x * frustum_x + n.y * frustum_y + n.z * frustum_dir;
		float plane_d = dot(primary.center_far + x * frustum_x + y * frustum_y, n);
		if (dot(n, secondary.support(n)) <= plane_d)
		{
			return false;
		}
	}
	return true;
}

bool collisionTest(const BoundEllipticFrustum& primary, const BoundFrustum& secondary)
{
	vec3 frustum_dir = primary.center_near - primary.center_far;
	float center_dist = length(frustum_dir);
	frustum_dir *= 1.0f / center_dist;
	vec3 support_min, support_max;
	secondary.biSupport(frustum_dir, support_max, support_min);
	if (dot(frustum_dir, support_max) <= dot(frustum_dir, primary.center_far) || dot(frustum_dir, support_min) >= dot(frustum_dir, primary.center_near))
	{
		return false;
	}
	float x_far_len = length(primary.x_far);
	float y_far_len = length(primary.y_far);
	vec3 frustum_x = 1.0f / x_far_len * primary.x_far;
	vec3 frustum_y = 1.0f / y_far_len * primary.y_far;
	vec3 v = secondary.center() - primary.center_far;
	float x = dot(v, frustum_x);
	float y = dot(v, frustum_y);
	float d = sqrt(x * x / (x_far_len * x_far_len) + y * y / (y_far_len * y_far_len));
	if (d > 0.0f)
	{
		x /= d;
		y /= d;
		vec3 n{ -x / (x_far_len * x_far_len), -y / (y_far_len * y_far_len), -(1 - primary.near_far_ratio) / center_dist };
		n = normalize(n);
		n = n.x * frustum_x + n.y * frustum_y + n.z * frustum_dir;
		float plane_d = dot(primary.center_far + x * frustum_x + y * frustum_y, n);
		if (dot(n, secondary.support(n)) <= plane_d)
		{
			return false;
		}
	}
	return true;
}
