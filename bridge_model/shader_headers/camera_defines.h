#include "common_defines.h"
#ifndef CAMERA_DEFINES_H
#define CAMERA_DEFINES_H

struct CameraData
{
	mat4 projection;
	mat4 view;
	mat4 inv_view;
};

DECLARE_UNIFORM(CameraData, camera, 0);

#endif // !CAMERA_DEFINES_H
