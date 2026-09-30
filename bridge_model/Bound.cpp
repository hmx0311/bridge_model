#include "Bound.h"

#include "ext/matrix_common.hpp"

using namespace glm;

BoundBox::BoundBox(const BoundBox& local, const mat4& transform)
{
	vec3 center = transform * vec4(local.center(), 1.0f);
	vec3 local_half = 0.5f * local.size();
	vec3 half = abs(mat3(transform)) * local_half;
	min = center - half;
	max = center + half;
}

BoundFrustum::BoundFrustum(const glm::mat4& transform, float fov_y, float aspect, float z_near, float z_far)
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
