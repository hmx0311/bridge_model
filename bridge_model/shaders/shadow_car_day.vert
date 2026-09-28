#include "lighting_day_defines.h"

layout(location = 0) in vec3 vertex;
layout(location = 3) in mat4 transform;

uniform int csm_level;

void main()
{
	gl_Position = sun_shadow.view_proj[csm_level] * transform * vec4(vertex, 1.0);
}
