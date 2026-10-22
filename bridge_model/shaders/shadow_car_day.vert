#include "car_attrib_locations.h"
#include "lighting_day_defines.h"

layout(location = CAR_ATTRIB_VERTEX_LOCATION)		in vec3 vertex;
layout(location = CAR_ATTRIB_TRANSFORM_LOCATION)	in mat4 transform;

uniform int csm_level;

void main()
{
	gl_Position = sun_shadow.view_proj[csm_level] * transform * vec4(vertex, 1.0);
}
