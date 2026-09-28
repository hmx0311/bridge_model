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
