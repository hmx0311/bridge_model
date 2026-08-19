#include "camera_defines.h"

layout(location = 0) in vec3 vertex;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

out vec3 modelPos;
out vec3 modelPosView;
out vec3 aNormal;
out vec2 aTexCoord;

void main()
{
	modelPos = vertex;
	modelPosView = mat4x3(camera.view) * vec4(vertex, 1.0);
	aNormal = normal;
	aTexCoord = vec2(texCoord);
	vec3 viewRay = camera.inv_view[3].xyz - modelPos;
	gl_Position = camera.projection * vec4(modelPosView, 1.0);
}
