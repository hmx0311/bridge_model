#include "common_defines.h"
#include "binding_points.h"
#ifndef LIGHTING_DAY_DEFINES_H
#define LIGHTING_DAY_DEFINES_H

#define CSM_LEVELS 4
#define MAX_PENUMBRA_RADIUS 1.0f
#define SHADOW_DAY_TEX_SIZE_EXP  12
#define SHADOW_DAY_TEX_SIZE  (1 << SHADOW_DAY_TEX_SIZE_EXP)
#define MIN_SHADOW_MAP_PADDING (1.0f / SHADOW_DAY_TEX_SIZE)
#define PCSS_MIP_LEVELS (SHADOW_DAY_TEX_SIZE_EXP - 1)

UNIFORM_BUFFER(SunData, SUN_BUFFER_BINDING)
{
	vec4 light_dir_and_radius;
	alignas(16) vec3 ambient;
	alignas(16) vec3 diffuse_specular;
	alignas(16) vec4 sky_color;
}BUFFER_NAME(sun)

UNIFORM_BUFFER(ShadowTransformData, SHADOW_TRANSFORM_BUFFER_BINDING)
{
	mat4 view_proj[CSM_LEVELS];
	mat4 tex[CSM_LEVELS];
}BUFFER_NAME(sun_shadow)

#endif // !LIGHTING_DAY_DEFINES_H
