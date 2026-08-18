#include "common_defines.h"
#ifndef LIGHTING_DAY_DEFINES_H
#define LIGHTING_DAY_DEFINES_H

#define CSM_LEVELS 4
#define MAX_PENUMBRA_RADIUS 1.0f
#define MIN_SHADOW_MAP_PADDING 0.02f

struct SunData
{
	alignas(16) vec3 light_dir;
	alignas(16) vec3 ambient;
	alignas(16) vec3 diffuse_specular;
	alignas(16) vec3 sky_color;
};

DECLARE_UNIFORM(SunData, sun, 1);

struct ShadowTransformData
{
	mat4 view_proj[CSM_LEVELS];
	mat4 tex[CSM_LEVELS];
};

DECLARE_UNIFORM(ShadowTransformData, sun_shadow, 2);

#endif // !LIGHTING_DAY_DEFINES_H
