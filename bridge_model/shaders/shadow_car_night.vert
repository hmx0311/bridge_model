#include "lighting_night_defines.h"

layout(location = 0) in vec3 vertex;
layout(location = 2) in int light_idx;
layout(location = 3) in mat4 mvp_mat;

out float gl_ClipDistance[4];

void main()
{
	int layer = findTileLightShadowLayer(light_idx);
	ivec2 layer_size = tileLightShadowLayerSize(layer);
	int idx = light_idx - tileLightShadowLayerOffset(layer);
	vec2 shadow_map_pos = vec2(idx % layer_size.x, idx / layer_size.x) / (0.5 * layer_size) - 1.0;
	gl_Position = mvp_mat * vec4(vertex, 1.0);
	gl_ClipDistance[0] = gl_Position.w + gl_Position.x;
	gl_ClipDistance[1] = gl_Position.w - gl_Position.x;
	gl_ClipDistance[2] = gl_Position.w + gl_Position.y;
	gl_ClipDistance[3] = gl_Position.w - gl_Position.y;
	gl_Position.xy = (gl_Position.xy + gl_Position.w) / layer_size + shadow_map_pos * gl_Position.w;
}
