#include "scene_constances.h"
#include "lighting_night_defines.h"

layout(triangles) in;

layout(triangle_strip, max_vertices = 111) out;
out float gl_ClipDistance[4];

in vec3 aNormal[];

void main()
{
	vec2 minCoord = vec2(gl_in[0].gl_Position);
	vec2 maxCoord = minCoord;
	minCoord = min(minCoord, vec2(gl_in[1].gl_Position));
	minCoord = min(minCoord, vec2(gl_in[2].gl_Position));
	maxCoord = max(maxCoord, vec2(gl_in[1].gl_Position));
	maxCoord = max(maxCoord, vec2(gl_in[2].gl_Position));
	ivec2 startIdx = ivec2(minCoord / LIGHT_MAP_GRID_LENGTH + vec2(0.5 * LIGHT_MAP_SIZE_X, 0.5 * LIGHT_MAP_SIZE_Y) -1);
	ivec2 endIdx = ivec2(maxCoord / LIGHT_MAP_GRID_LENGTH + vec2(0.5 * LIGHT_MAP_SIZE_X, 0.5 * LIGHT_MAP_SIZE_Y) + 1);

	int i = startIdx.x;
	int j = startIdx.y;
	int gird_idx = i * LIGHT_MAP_SIZE_Y + j;
	int k = tile_light_map.idx_range[gird_idx].x - 1;
	int end = tile_light_map.idx_range[gird_idx].y;
	while(true)
	{
		k++;
		while(k == end)
		{
			j++;
			if(j > endIdx.y)
			{
				i++;
				if(i > endIdx.x)
				{
					return;
				}
				j = startIdx.y;
			}
			gird_idx = i * LIGHT_MAP_SIZE_Y + j;
			k = tile_light_map.idx_range[gird_idx].x;
			end = tile_light_map.idx_range[gird_idx].y;	
		}
		int layer = findTileLightShadowLayer(k);
		if(layer >= NUM_TILE_LIGHT_SHADOW_LAYERS)
		{
			continue;
		}
		vec4 pos[3];
		vec2 test[3];
		for(int p = 0; p < 3; p++)
		{
			pos[p] = tile_lights[k].view_proj * gl_in[p].gl_Position;
			test[p]= pos[p].xy / pos[p].w;
		}
		vec3 normal = cross(gl_in[1].gl_Position.xyz - gl_in[0].gl_Position.xyz, gl_in[2].gl_Position.xyz - gl_in[0].gl_Position.xyz);
		if(pos[0].z >= pos[0].w && pos[1].z >= pos[1].w && pos[2].z >= pos[2].w || dot(normal, tile_lights[k].position.xyz - gl_in[0].gl_Position.xyz) <= 0)
		{
			continue;
		}
		bool surroundTest[3] = { (test[0].x * test[1].y - test[0].y * test[1].x) * pos[2].w > 0,
								(test[1].x * test[2].y - test[1].y * test[2].x) * pos[0].w > 0,
								(test[2].x * test[0].y - test[2].y * test[0].x) * pos[1].w > 0 };
		bool isIn = surroundTest[0] == surroundTest[1] && surroundTest[0] == surroundTest[2];
		for(int p = 0; p < 3 && !isIn; p++)
		{
			vec2 a = test[p];
			int p2 = (p + 1) % 3;
			vec2 v = test[p2] - a;
			float lt2 = dot(a, a);
			if(pos[p2].w < 0)
			{
				if(pos[p].w < 0)
				{
					continue;
				}
				lt2 = min(lt2, 1.0);
				v = -v;
			}
			else if(pos[p].w < 0)
			{
				lt2 = min(lt2, 1.0);
				a = test[p2];
			}
			float t = v.x * a.y - v.y * a.x;
			isIn = lt2 < 1.0 || dot(v, a) < 0 && dot(v, v) > max(t * t, lt2 - 1);
		}
		if(isIn)
		{
			ivec2 cnt = tileLightShadowLayerSize(layer);
			int idx = k - tileLightShadowLayerOffset(layer);
			vec2 shadowMapPos = vec2(idx % cnt.x, idx / cnt.x) / (0.5 * cnt) - 1.0;
			for(int p = 0; p < 3; p++)
			{
				gl_Position = pos[p];
				gl_ClipDistance[0] = gl_Position.w + gl_Position.x;
				gl_ClipDistance[1] = gl_Position.w - gl_Position.x;
				gl_ClipDistance[2] = gl_Position.w + gl_Position.y;
				gl_ClipDistance[3] = gl_Position.w - gl_Position.y;
				gl_Position.xy = (gl_Position.xy + gl_Position.w) / cnt + shadowMapPos * gl_Position.w;
				gl_Layer = layer;
				EmitVertex();
			}
			EndPrimitive();	
		}
	}
}