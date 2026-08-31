#include "lighting_day_defines.h"

#define TIMES3_RESULT_1 3
#define TIMES3_RESULT_2 6
#define TIMES3_RESULT_3 9
#define TIMES3_RESULT_4 12
#define TIMES3_RESULT_5 15
#define TIMES3_RESULT_6 18
#define TIMES3_RESULT_7 21
#define TIMES3_RESULT_8 24
#define TIMES3_RESULT_9 27
#define TIMES3_RESULT_10 30
#define TIMES3_RESULT_11 33
#define TIMES3_RESULT_12 36

#define TIMES3_EXPAND(n) TIMES3_RESULT_##n
#define TIMES3(n) TIMES3_EXPAND(n)

layout(triangles) in;

layout(triangle_strip, max_vertices = TIMES3(CSM_LEVELS)) out;

void main()
{
	for(int i = 0; i < CSM_LEVELS; i++)
	{
		for(int j = 0; j < 3; j++)
		{
			gl_Position = sun_shadow.view_proj[i] * gl_in[j].gl_Position;
			gl_Layer = i;
			EmitVertex();
		}
		EndPrimitive();
	}
}
