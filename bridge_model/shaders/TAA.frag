#include "camera_defines.h"

#define Y_HIST_LEN 11
#define Y_HIST_EXTRA 3

#define EPSILON16F 0.01
#define EPSILON 1e-6
#define INFINITY 1e20
#define INVALID_Y -100.0

uniform vec2 tan_half_fov;

layout(binding = 0) uniform sampler2D color_tex;
layout(binding = 1) uniform sampler2D color_old_tex;
layout(binding = 2) uniform sampler2D bloom_tex;
layout(binding = 3) uniform sampler2D bloom_old_tex;
layout(binding = 4) uniform sampler2D albedo_tex;
layout(binding = 5) uniform sampler2D albedo_old_and_chroma_variance_tex;
layout(binding = 6) uniform sampler2D depth_tex;
layout(binding = 7) uniform sampler2D depth_last_tex;
layout(binding = 8) uniform sampler2D delta_position_tex;
layout(binding = 9) uniform sampler2D stats_tex0;
layout(binding = 10) uniform sampler2D stats_tex1;
layout(binding = 11) uniform sampler2D stats_tex2;

in vec2 texCoord;

layout(location = 0) out vec4 frag_color;
layout(location = 1) out vec4 frag_bloom;
layout(location = 2) out vec4 frag_albedo_and_chroma_variance;
layout(location = 3) out vec4 frag_stats0;
layout(location = 4) out vec4 frag_stats1;
layout(location = 5) out vec4 frag_stats2;

const float frame_count = 20.0;

vec4 CatmullRomFilter(vec2 uv)
{
	vec2 p = uv * textureSize(color_tex, 0).xy;
	vec2 r_size = 1.0 / textureSize(color_tex, 0).xy;
	vec2 uv_grid = ivec2(p + 0.5) - 0.5;
	vec2 offset = uv_grid - p;
	uv_grid *= r_size.xy;
	float offset_x[4] = { offset.x - 1.0, offset.x, offset.x + 1.0, offset.x + 2.0 };
	float offset_y[4] = { offset.y - 1.0, offset.y, offset.y + 1.0, offset.y + 2.0 };
	float weight_x[4] = { 0.5 * (offset_x[0] * (offset_x[0] * (offset_x[0] + 5.0) + 8.0) + 4.0),
						0.5 * (offset_x[1] * offset_x[1] * (-3.0 * offset_x[1] - 5.0) + 2.0),
						0.5 * (offset_x[2] * offset_x[2] * (3.0 * offset_x[2] - 5.0) + 2.0),
						0.5 * (offset_x[3] * (offset_x[3] * (-offset_x[3] + 5.0) - 8.0) + 4.0) };
	float weight_y[4] = { 0.5 * (offset_y[0] * (offset_y[0] * (offset_y[0] + 5.0) + 8.0) + 4.0),
						0.5 * (offset_y[1] * offset_y[1] * (-3.0 * offset_y[1] - 5.0) + 2.0),
						0.5 * (offset_y[2] * offset_y[2] * (3.0 * offset_y[2] - 5.0) + 2.0),
						0.5 * (offset_y[3] * (offset_y[3] * (-offset_y[3] + 5.0) - 8.0) + 4.0) };
	float weight_correction = 1.0 / (1.0 - (weight_x[0] + weight_x[3]) * (weight_y[0] + weight_y[3]));
	float weight_x12 = weight_x[1] + weight_x[2];
	float weight_y12 = weight_y[1] + weight_y[2];
	float weight_center = weight_x12 * weight_y12;
	float weight_nx = weight_x[0] * weight_y12;
	float weight_px = weight_x[3] * weight_y12;
	float weight_ny = weight_y[0] * weight_x12;
	float weight_py = weight_y[3] * weight_x12;
	vec2 uv_center = uv_grid + vec2(r_size.x * weight_x[2] / weight_x12, r_size.y * weight_y[2] / weight_y12);
	vec2 uv_nx = vec2(uv_grid.x - r_size.x, uv_center.y);
	vec2 uv_px = vec2(uv_grid.x + 2 * r_size.x, uv_center.y);
	vec2 uv_ny = vec2(uv_center.x, uv_grid.y - r_size.y);
	vec2 uv_py = vec2(uv_center.x, uv_grid.y + 2 * r_size.y);
	vec4 ret = weight_center * texture(color_old_tex, uv_center);
	ret += weight_nx * texture(color_old_tex, uv_nx);
	ret += weight_px * texture(color_old_tex, uv_px);
	ret += weight_ny * texture(color_old_tex, uv_ny);
	ret += weight_py * texture(color_old_tex, uv_py);
	ret *= weight_correction;
	return ret;
}

vec3 rgb2YCoCg(vec3 c) 
{
	float Y  =  0.25 * c.r + 0.5 *c.g + 0.25 * c.b;
	float Co =  0.5  * c.r - 0.5 * c.b;
	float Cg = -0.25 * c.r + 0.5 * c.g - 0.25 * c.b;
	return vec3(Y, Co, Cg);
}

vec3 yCoCg2RGB(vec3 ycc) 
{
	float Y  = ycc.x;
	float Co = ycc.y;
	float Cg = ycc.z;
	
	float r = Y + Co - Cg;
	float g = Y + Cg;
	float b = Y - Co - Cg;
	
	return vec3(r, g, b);
}

vec3 depthToPosition(float depth, vec2 uv, float z_near)
{
	float z = 2.0 * z_near * VIEW_Z_FAR / (z_near + VIEW_Z_FAR - (2.0 * depth - 1.0) * (VIEW_Z_FAR - z_near));
	return vec3((2.0f * uv - 1.0f) * tan_half_fov * z, -z);
}

bool reprojection(vec3 position, vec3 delta_position, out vec2 uv_motion)
{
	vec4 position_in_screen = camera.projection * vec4(position, 1.0);
	vec4 position_last_in_screen = camera.projection * camera.inv_delta_view * vec4(position - delta_position, 1.0);
	if(position_last_in_screen.w <= 0 ||
		position_last_in_screen.x <= -position_last_in_screen.w || position_last_in_screen.x >= position_last_in_screen.w ||
		position_last_in_screen.y <= -position_last_in_screen.w || position_last_in_screen.y >= position_last_in_screen.w)
	{
		return false;
	}
	uv_motion = 0.5 * (position_last_in_screen.xy / position_last_in_screen.w - position_in_screen.xy / position_in_screen.w);
	return true;
}

bool validateSplit(int begin, int end, int split, float mean_step2, float history[Y_HIST_LEN])
{
	float sum_l = 0.0, sum_r = 0.0;
	float min_l = INFINITY, max_l = 0;
	float min_r = INFINITY, max_r = 0;
	for(int i = begin; i < split; i++)
	{
		sum_l += history[i];
		min_l = min(min_l, history[i]);
		max_l = max(max_l, history[i]);
	}
	for(int i = split; i < end; i++)
	{
		sum_r += history[i];
		min_r = min(min_r, history[i]);
		max_r = max(max_r, history[i]);
	}
	float mean_l = sum_l / (split - begin);
	float mean_r = sum_r / (end - split);
	float span_l = max_l - min_l;
	float span_r = max_r - min_r;
	float mean_diff = abs(mean_l - mean_r);
	return mean_diff > 1.5 * max(span_l / (end - split), span_r / (split - begin)) && mean_diff * mean_diff > 3.5 * mean_step2;
}

void main()
{
	vec2 uv = texCoord;
	vec2 r_size = 1.0 / textureSize(color_tex, 0).xy;
	
	vec4 color = texelFetch(color_tex, ivec2(gl_FragCoord), 0);
	vec4 bloom = texelFetch(bloom_tex, ivec2(gl_FragCoord), 0);
	vec3 albedo = texelFetch(albedo_tex, ivec2(gl_FragCoord), 0).rgb;

	vec3 position_nearest = depthToPosition(texelFetch(depth_tex, ivec2(gl_FragCoord), 0).x, uv, camera.z_near);
	vec3 position_farthest = position_nearest;
	ivec2 in_position_nearest = ivec2(gl_FragCoord);
	ivec2 in_position_farthest = in_position_nearest;
	const ivec2 offsets[4] = { ivec2(-1, -1), ivec2(1, 1), ivec2(1, -1), ivec2(-1, 1) };
	
	for(int i = 0; i < 4 ;i++ )
	{
		ivec2 offset_in_position = clamp(ivec2(gl_FragCoord) +  offsets[i], ivec2(0, 0), ivec2(textureSize(color_tex, 0).xy - 0.25));
		vec3 offset_position = depthToPosition(texelFetch(depth_tex, offset_in_position, 0).x, (offset_in_position + 0.5) * r_size, camera.z_near);
		if(offset_position.z > position_nearest.z)
		{
			position_nearest = offset_position;
			in_position_nearest = offset_in_position;
		}
		if(offset_position.z < position_farthest.z)
		{
			position_farthest = offset_position;
			in_position_farthest = offset_in_position;
		}
	}
	
	vec3 delta_position_nearest = texelFetch(delta_position_tex, in_position_nearest, 0).xyz;
	vec2 uv_motion_nearest;
	if(!reprojection(position_nearest, delta_position_nearest, uv_motion_nearest))
	{
		frag_color = color;
		frag_bloom = bloom;
		frag_albedo_and_chroma_variance = vec4(albedo, 0.0);
		frag_stats0 = vec4(INVALID_Y, INVALID_Y, INVALID_Y, INVALID_Y);
		frag_stats1 = vec4(INVALID_Y, INVALID_Y, INVALID_Y, INVALID_Y);
		frag_stats2 = vec4(INVALID_Y, rgb2YCoCg(albedo).x, 1.0, 0.0);
		return;
	}
	
	vec3 delta_position_farthest = texelFetch(delta_position_tex, in_position_farthest, 0).xyz;
	vec2 uv_motion_farthest;
	float motion_diff;
	if(!reprojection(position_farthest, delta_position_farthest, uv_motion_farthest))
	{
		 motion_diff = 0.0;
	}
	else
	{
		motion_diff = length((uv_motion_farthest - uv_motion_nearest) * textureSize(color_tex, 0).xy);
	}

	uv += uv_motion_nearest;
	
	vec3 position_last = depthToPosition(texture(depth_last_tex, uv).x, uv, camera.old_z_near);
	float z_last_in_cur_nearest = dot(vec4(position_last, 1), camera.delta_view_z);
	float z_last_in_cur_farthest = z_last_in_cur_nearest;
	for(int i = 0; i < 4; i++)
	{
		vec2 uv_offset = uv + offsets[i] * r_size.xy;
		position_last = depthToPosition(texture(depth_last_tex, uv_offset).x, uv_offset, camera.old_z_near);
		float z_last_in_cur = dot(vec4(position_last, 1), camera.delta_view_z);
		z_last_in_cur_nearest = max(z_last_in_cur_nearest, z_last_in_cur);
		z_last_in_cur_farthest = min(z_last_in_cur_farthest, z_last_in_cur);
	}
	if(z_last_in_cur_farthest > 0.99 * (position_nearest.z - delta_position_nearest.z)
		|| position_farthest.z - delta_position_farthest.z > 0.99 * z_last_in_cur_nearest)
	{
		frag_color = color;
		frag_bloom = bloom;
		frag_albedo_and_chroma_variance = vec4(albedo, 0.0);
		frag_stats0 = vec4(INVALID_Y, INVALID_Y, INVALID_Y, INVALID_Y);
		frag_stats1 = vec4(INVALID_Y, INVALID_Y, INVALID_Y, INVALID_Y);
		frag_stats2 = vec4(INVALID_Y, rgb2YCoCg(albedo).x, 1.0, 0.0);
		return;
	}
	
	vec3 lighting = color.rgb / (albedo + EPSILON16F);
	vec3 box_min_lighting = lighting;
	vec3 box_max_lighting = lighting;
	vec3 ycc = rgb2YCoCg(albedo);
	vec3 box_min_ycc = ycc;
	vec3 box_max_ycc = ycc;

	ivec2 box_offsets[4] = { {0, -1}, {-1, 0}, {1, 0}, {0, 1} };
	for(int i = 0; i < 4; i++)
	{
		vec3 neighbor_color = texelFetch(color_tex, ivec2(gl_FragCoord) + box_offsets[i], 0).rgb;
		vec3 neighbor_albedo = texelFetch(albedo_tex, ivec2(gl_FragCoord) + box_offsets[i], 0).rgb;
		vec3 neighbor_lighting = neighbor_color / (neighbor_albedo + EPSILON16F);
		box_min_lighting = min(box_min_lighting, neighbor_lighting);
		box_max_lighting = max(box_max_lighting, neighbor_lighting);
		vec3 neighbor_ycc = rgb2YCoCg(neighbor_albedo);
		box_min_ycc = min(box_min_ycc, neighbor_ycc);
		box_max_ycc = max(box_max_ycc, neighbor_ycc);
	}
	
	vec4 color_old = CatmullRomFilter(uv);
	vec4 bloom_old = textureLod(bloom_old_tex, uv, 0.0);
	vec3 albedo_old;
	float Y_history[Y_HIST_LEN];
	float blend_factor_old, chroma_variance, motion_diff_old;
	{ 
		vec4 ac = texture(albedo_old_and_chroma_variance_tex, uv);
		albedo_old = ac.rgb;
		chroma_variance = ac.w;
		vec4 s0 = texture(stats_tex0, uv);
		vec4 s1 = texture(stats_tex1, uv);
		vec4 s2 = texture(stats_tex2, uv);
		Y_history[0] = s0.x; Y_history[1] = s0.y; Y_history[2] = s0.z; Y_history[3] = s0.w;
		Y_history[4] = s1.x; Y_history[5] = s1.y; Y_history[6] = s1.z; Y_history[7] = s1.w;
		Y_history[8] = s2.x; Y_history[9] = s2.y; Y_history[10] = ycc.x;
		blend_factor_old = s2.z;
		motion_diff_old = s2.w;
	}
	vec3 lighting_old = color_old.rgb / (albedo_old + EPSILON16F);
	vec3 ycc_old = rgb2YCoCg(albedo_old.rgb);
	float motion_confidence = clamp(1 + (motion_diff - motion_diff_old) / 3.0, 0.0, 1.0);
	float chroma_tolerance = sqrt(chroma_variance) * motion_confidence + EPSILON16F;
	
	vec3 ycc_diff = ycc - ycc_old;
	float chroma_diff_length = length(ycc_diff.yz) + EPSILON16F;
	float Y_diff_length = abs(ycc_diff.x) + EPSILON16F;
	vec3 clamp_ycc = ycc_old - clamp(ycc_old, box_min_ycc, box_max_ycc);
	float chroma_clamp_length = length(clamp_ycc.yz);
	float Y_clamp_length = abs(clamp_ycc.x);

	vec3 lighting_diff = lighting - lighting_old;
	float lighting_diff_length = length(lighting_diff) + EPSILON16F;
	vec3 lighting_clamp = lighting_old - clamp(lighting_old, box_min_lighting, box_max_lighting);
	float lighting_clamp_length = length(lighting_clamp);

	float lighting_extra_clip = max(0.0, lighting_clamp_length / lighting_diff_length - max(chroma_clamp_length / chroma_diff_length, Y_clamp_length / Y_diff_length));

	chroma_clamp_length = max(0, min(chroma_clamp_length, chroma_diff_length - 3.0 * chroma_tolerance));
	float chroma_clip_strenth = chroma_clamp_length / chroma_diff_length;

	int Y_valid_begin;
	for(Y_valid_begin = Y_HIST_LEN - 2; Y_valid_begin > 0 && Y_history[Y_valid_begin - 1] >= 0; Y_valid_begin--);
	Y_valid_begin = max(Y_valid_begin, int(round((1 - motion_confidence) * (Y_HIST_LEN - 1))));
	for(int i = 0; i < Y_valid_begin; i++)
	{
		Y_history[i] = INVALID_Y;
	}
	int Y_valid_cnt = Y_HIST_LEN - Y_valid_begin;
	
	
	if(Y_valid_cnt > 2)
	{
		int split = Y_valid_begin;
		float max_split_k = -1.0;
		float Y_sum_l = 0.0, Y_sum_r = 0.0;
		for(int i = Y_valid_begin; i < Y_HIST_LEN; i++)
		{
			Y_sum_r += Y_history[i];
		}
		float Y_sum2_step = 0.0;
		for(int i = Y_valid_begin; i < Y_HIST_LEN - 1; i++)
		{
			float diff = Y_history[i] - Y_history[i + 1];
			Y_sum2_step += diff * diff;
		}
		int extra_split_gap = (position_farthest.z - position_nearest.z) / position_farthest.z < 0.1 ? 0: 1;
		float Y_sum_split_l = Y_sum_l , Y_sum_split_r = Y_sum_r;
		for(int i = Y_valid_begin + 1; i < Y_HIST_LEN - extra_split_gap; i++)
		{
			Y_sum_l += Y_history[i - 1];
			Y_sum_r -= Y_history[i - 1];
			float Y_mean_diff = Y_sum_l / (i - Y_valid_begin) - Y_sum_r / (Y_HIST_LEN - i);
			float split_k = Y_mean_diff * Y_mean_diff * (i - Y_valid_begin) * (Y_HIST_LEN - i);
			if(split_k > max_split_k)
			{
				max_split_k = split_k;
				split = i;
				Y_sum_split_l = Y_sum_l;
				Y_sum_split_r = Y_sum_r;
			}
		}
		Y_sum2_step -= (Y_history[split - 1] - Y_history[split]) * (Y_history[split - 1] - Y_history[split]);
		
		if(!validateSplit(Y_valid_begin, Y_HIST_LEN, split, Y_sum2_step / (Y_valid_cnt - 2), Y_history))
		{
			split = Y_valid_begin;
		}
		if(Y_valid_begin == 0)
		{
			float period_err = 0;
			for(int i = 0; i < Y_HIST_EXTRA; i++)
			{
				period_err += abs(Y_history[i] - Y_history[i + TAA_PERIOD]) / (Y_history[i] + Y_history[i + TAA_PERIOD] + 1);
			}
			if(period_err < 0.1)
			{
				split = 0;
			}
		}
		
		int split_n = Y_HIST_LEN - split;
		if(split_n > 2)
		{
			float Y_sum = 0.0;
			for (int i = 0; i < split_n; i++) 
			{
				Y_sum  += Y_history[i + split];
			}
			float Y_mean  = Y_sum / split_n;			
			
			float SSE_discrete = 0.0;
			for (int i = 0; i < split_n; i++)
			{
				float err = Y_history[i + split] - Y_mean;
				SSE_discrete += err * err;
			}
			float MSE = SSE_discrete / (split_n - 1);
			
			float Y_pred = Y_mean;
			float spatial_weight = 0;
			if(Y_valid_cnt <= 3)
			{
				spatial_weight = (4 - Y_valid_cnt) / (4.0);
			}
			Y_clamp_length = clamp(abs(ycc_old.x - Y_pred) - sqrt(MSE), spatial_weight * Y_clamp_length, Y_clamp_length);
		}
	}
	
	float Y_clip_strenth = Y_clamp_length / Y_diff_length;
	
	float blend_factor = 1.0 / frame_count;
	blend_factor += (1 - blend_factor) * (max(chroma_clip_strenth, Y_clip_strenth) + lighting_extra_clip);
	blend_factor = max(blend_factor, 1 - motion_confidence);
	blend_factor = max(blend_factor, blend_factor_old / (1 + blend_factor_old));
	
	chroma_variance = mix(chroma_variance, chroma_diff_length * chroma_diff_length, 1.0 / TAA_PERIOD);
	
	frag_color = mix(color_old, color, blend_factor);
	frag_bloom = mix(bloom_old, bloom, blend_factor);
	frag_albedo_and_chroma_variance = vec4(mix(albedo_old, albedo, blend_factor), chroma_variance);
	frag_stats0 = vec4(Y_history[1], Y_history[2], Y_history[3], Y_history[4]);
	frag_stats1 = vec4(Y_history[5], Y_history[6], Y_history[7], Y_history[8]);
	frag_stats2 = vec4(Y_history[9], ycc.x, blend_factor, motion_diff);
}
