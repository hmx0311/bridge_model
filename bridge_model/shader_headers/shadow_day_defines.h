#include "common_defines.h"
#define CSM_LEVELS 4

DECLARE_UNIFORM(ShadowTransform, 2)
{
	mat4 shadow_mat[CSM_LEVELS];
	mat4 shadow_tex_mat[CSM_LEVELS];
};
