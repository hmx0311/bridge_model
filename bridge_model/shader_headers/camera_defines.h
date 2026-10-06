#include "common_defines.h"
#include "binding_points.h"
#ifndef CAMERA_DEFINES_H
#define CAMERA_DEFINES_H

UNIFORM_BUFFER_BEGIN(CameraData, CAMERA_BUFFER_BINDING)
{
	mat4 projection;
	mat4 view;
	mat4 view_proj;
	mat4 inv_view;
}BUFFER_END(camera)

#endif // !CAMERA_DEFINES_H
