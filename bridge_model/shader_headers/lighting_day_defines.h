#include "common_defines.h"
#ifndef LIGHTING_DAY_DEFINES_H
#define LIGHTING_DAY_DEFINES_H

#define CSM_LEVELS 8
#define MAX_PENUMBRA_RADIUS 1.0f
#define MIN_SHADOW_MAP_PADDING 0.02f
#define SHADOW_DAY_TEX_SIZE_EXP  12
#define SHADOW_DAY_TEX_SIZE  (1 << SHADOW_DAY_TEX_SIZE_EXP)
#define PCSS_MIP_LEVELS (SHADOW_DAY_TEX_SIZE_EXP - 1)

struct SunData
{
	vec4 light_dir_and_radius;
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
