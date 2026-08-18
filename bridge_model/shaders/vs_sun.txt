#include "camera_defines.h"
#include "lighting_day_defines.h"

uniform mat4x3 transform;

layout(location = 0) in vec2 vertex;

void main()
{
	gl_Position = camera.projection * vec4(mat3(camera.view) * (transform * vec4(vertex, 0.0, 1.0)), 1.0);
}
