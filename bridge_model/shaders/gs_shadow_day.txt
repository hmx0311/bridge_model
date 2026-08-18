#include "lighting_day_defines.h"

layout(triangles) in;

layout(triangle_strip, max_vertices = 12) out;

void main()
{
	vec3 normal = normalize(cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz));
	float NdotD = dot(normal, sun.light_dir);
	float offset_scale = NdotD > 0 ? 0.1 * sqrt(1 / (NdotD * NdotD) - 1) + 0.01 : 0;
	for(int i = 0; i < CSM_LEVELS; i++)
	{
		float offset = -dot(sun.light_dir, vec3(sun_shadow.view_proj[i][0].z, sun_shadow.view_proj[i][1].z, sun_shadow.view_proj[i][2].z)) * offset_scale;
		for(int j = 0; j < 3; j++)
		{
			gl_Position = sun_shadow.view_proj[i] * gl_in[j].gl_Position;
			gl_Position.z += offset;
			gl_Layer = i;
			EmitVertex();
		}
		EndPrimitive();
	}
}
