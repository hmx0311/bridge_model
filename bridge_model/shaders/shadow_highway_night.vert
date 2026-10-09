#include "lighting_night_defines.h"

layout(std430, binding = VERTEX_BUFFER_BINDING) buffer vertex_block
{ 
    float vertices[];
};

out float gl_ClipDistance[4];

void main()
{
	uint triangle_id = gl_VertexID / 3;
	uint vert_idx = tile_light_shadow_triangles.triangles[triangle_id].vert_idx[gl_VertexID % 3];
	uint offset = 3 * vert_idx;
	vec3 vertex = vec3(vertices[offset], vertices[offset + 1], vertices[offset + 2]);		
	int light_idx = tile_light_shadow_triangles.triangles[triangle_id].light_idx;
	int layer = findTileLightShadowLayer(light_idx);
	ivec2 layer_size = tileLightShadowLayerSize(layer);
	int shadow_idx = light_idx - tileLightShadowLayerOffset(layer);
	vec2 shadow_map_pos = vec2(shadow_idx % layer_size.x, shadow_idx / layer_size.x) / (0.5 * layer_size) - 1.0;
	gl_Position = tile_lights[light_idx].view_proj * vec4(vertex, 1.0);
	gl_ClipDistance[0] = gl_Position.w + gl_Position.x;
	gl_ClipDistance[1] = gl_Position.w - gl_Position.x;
	gl_ClipDistance[2] = gl_Position.w + gl_Position.y;
	gl_ClipDistance[3] = gl_Position.w - gl_Position.y;
	gl_Position.xy = (gl_Position.xy + gl_Position.w) / layer_size + shadow_map_pos * gl_Position.w;
}
