#include "camera_defines.h"
#include "lighting_day_defines.h"

uniform mat4 transform;
uniform mat4 old_transform;

layout(location = 0) in vec2 vertex;

out vec3 a_delta_position;

void main()
{
	vec4 position = transform * vec4(vertex, 0.0, 1.0);
	vec4 old_position = old_transform * vec4(vertex, 0.0, 1.0);
	a_delta_position = (camera.view * (position - old_position)).xyz;
	gl_Position = camera.view_proj * position;
	//gl_Position.xy += camera.taa_offset * gl_Position.w;
}
