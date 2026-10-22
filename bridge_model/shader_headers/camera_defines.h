#include "common_defines.h"
#include "binding_points.h"
#ifndef CAMERA_DEFINES_H
#define CAMERA_DEFINES_H

#define VIEW_Z_FAR 18000.0f
#define TAA_PERIOD 8

UNIFORM_BUFFER(CameraData, CAMERA_BUFFER_BINDING)
{
	mat4 projection;
	mat4 view;
	mat4 inv_view;
	mat4 view_proj;
	mat4 inv_delta_view;
	vec4 delta_view_z;
	vec2 taa_offset;
	float z_near;
	float old_z_near;
}BUFFER_NAME(camera)

#endif // !CAMERA_DEFINES_H
