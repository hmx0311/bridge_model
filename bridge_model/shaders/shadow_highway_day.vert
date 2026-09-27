#include "lighting_day_defines.h"

layout(location = 0) in vec3 vertex;

uniform int csm_level;

void main()
{
	gl_Position = sun_shadow.view_proj[csm_level] * vec4(vertex, 1.0);
}
