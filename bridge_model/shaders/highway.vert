#include "camera_defines.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 world_position;
out vec3 position_in_view;
out vec3 aNormal;
out vec2 aTexCoord;

void main()
{
	world_position = vertex;
	position_in_view = ((camera.view) * vec4(vertex, 1.0)).xyz;
	aNormal = normal;
	aTexCoord = texCoord;
	gl_Position = camera.projection * vec4(position_in_view, 1.0);
	gl_Position.xy += camera.taa_offset * gl_Position.w;
}
