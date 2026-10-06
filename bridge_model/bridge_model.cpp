#pragma comment(lib,"imm32.lib")
#include <Windows.h>

#include <thread>

#include "glad/glad.h"
#include "glfw3.h"
#include "gtx/transform.hpp"
#include "ext/matrix_common.hpp"

#include "resource.h"
#include "scene.h"
#include "mesh.h"
#include "Car.h"
#include "Sun.h"
#include "terrain.h"
#include "logical_frame.h"
#include "Frustum.h"
#include "Bound.h"
#include "shader.h"
#include "CircularQueue.h"

#include "shader_headers/scene_constances.h"
#include "shader_headers/camera_defines.h"
#include "shader_headers/lighting_day_defines.h"
#include "shader_headers/lighting_night_defines.h"
#include "shader_headers/text_altas_constances.h"

using namespace glm;

constexpr float TERRAIN_LOD_FACTOR = 2000.0f;

constexpr float MAX_CSM_RATIO = 3.6f;
GLuint shadow_day_FBO;
GLuint shadow_day_tex;
GLuint shadow_PCF_sampler;
GLuint shadow_depth_sampler;
GLuint shadow_mip_gen_FBO;
GLuint shadow_mip_gen_sampler;

constexpr int SHADOW_NIGHT_TEX_SIZE = 4096;
GLuint shadow_night_FBO;
GLuint shadow_night_tex;

constexpr float FOV_Y = pi<float>() / 4;
constexpr float MIN_VIEW_Z_NEAR = 0.6f;
constexpr float VIEW_Z_FAR = 18000.0f;
constexpr float MIN_VIEW_DISTANCE = 2.0f;
constexpr float MAX_VIEW_DISTANCE = 1000.0f;
constexpr float MIN_SHADOW_FAR = 3.695f;

GLint window_width, window_height;

float aim_azimuth = 0.3f, aim_relative_depression = 0.2f, aim_view_distance = 1000.0f;
uint32_t focus_move_dir = 0;
bool show_fps = false;
bool need_update_view = true;
uint64_t last_time_us;

constexpr float HEIGHT_RANGE[2] = { -10.0f, 646.0f };

GLuint multisample_render_FBO;
GLuint multisample_render_RBOs[2];
GLuint depth_RBO;

GLuint render_FBO;
GLuint render_tex;

constexpr int BLOOM_BUFFER_HEIGHT = 540;
int bloom_buffer_width;
GLuint bloom_FBOs[2];
GLuint bloom_texs[2];

GLuint tex_blit_VAO, tex_mapping_VBO;

constexpr GLsizei TEXT_WIDTH = 16;
constexpr GLsizei TEXT_HEIGHT = 24;
constexpr int TEXT_ALTAS_ASCII_OFFSET = 32;
GLuint text_atlas_tex;

/*
* binding = 0
*	CameraData camera
*
* binding = 1
*	SunData sun;
*/
GLuint scene_UBO;
GLintptr scene_UBO_offset1;

/*
* binding = 2
*	ShadowTransformData sun_shadow;
*	mat4 view_proj[CSM_LEVELS];
*	mat4 tex[CSM_LEVELS];
*/
GLuint shadow_UBO;

// binding = 3
GLuint tile_light_map_SSBO;
// binding = 4
GLuint tile_light_pos_UBO;
// binding = 5
GLuint tile_light_transform_SSBO;
// binding = 6
GLuint car_lighting_SSBO;
// binding = 7
GLuint tile_light_shadow_triangles_SSBO;

CameraData camera;
ShadowTransformData sun_shadow;
const mat4 CAR_LIGHT_SHADOW_PROJ = perspective(CAR_LIGHT_V_RAD, CAR_LIGHT_ASPECT, CAR_LIGHT_NEAR, CAR_LIGHT_NEAR + CAR_LIGHT_RANGE);

TileLightMapData tile_light_map;
TileLightData tile_light_pos;
TileLightTransformData tile_light_transforms;
CarLightingData car_lightings;

GLuint SP_highway_day;
GLuint SP_highway_night;
GLuint SP_terrain_day;
GLuint SP_terrain_night;
GLuint SP_car_day;
GLuint SP_car_night;
GLuint SP_sun;
GLuint CSP_shadow_highway_night;
GLuint SP_shadow_highway_day;
GLuint SP_shadow_highway_night;
GLuint SP_shadow_car_day;
GLuint SP_shadow_car_night;
GLuint SP_gen_PCSS_mips;
GLuint SP_tex_blit;
GLuint SP_gaussian_blur;
GLuint SP_buffer_to_screen;
GLuint SP_text;

static void initShader()
{
	GLuint VS_highway = loadShader(SHADER_NAME(IDR_VS_HIGHWAY), GL_VERTEX_SHADER);
	GLuint FS_highway_day = loadShader(SHADER_NAME(IDR_FS_HIGHWAY_DAY), GL_FRAGMENT_SHADER);
	GLuint FS_highway_night = loadShader(SHADER_NAME(IDR_FS_HIGHWAY_NIGHT), GL_FRAGMENT_SHADER);
	GLuint FS_terrain_day = loadShader(SHADER_NAME(IDR_FS_TERRAIN_DAY), GL_FRAGMENT_SHADER);
	GLuint FS_terrain_night = loadShader(SHADER_NAME(IDR_FS_TERRAIN_NIGHT), GL_FRAGMENT_SHADER);
	SP_highway_day = linkShaderProgram(VS_highway, FS_highway_day);
	SP_highway_night = linkShaderProgram(VS_highway, FS_highway_night);
	SP_terrain_day = linkShaderProgram(VS_highway, FS_terrain_day);
	SP_terrain_night = linkShaderProgram(VS_highway, FS_terrain_night);
	glDeleteShader(VS_highway);
	glDeleteShader(FS_highway_day);
	glDeleteShader(FS_highway_night);
	glDeleteShader(FS_terrain_day);
	glDeleteShader(FS_terrain_night);

	GLuint VS_car = loadShader(SHADER_NAME(IDR_VS_CAR), GL_VERTEX_SHADER);
	GLuint FS_car_day = loadShader(SHADER_NAME(IDR_FS_CAR_DAY), GL_FRAGMENT_SHADER);
	GLuint FS_car_night = loadShader(SHADER_NAME(IDR_FS_CAR_NIGHT), GL_FRAGMENT_SHADER);
	SP_car_day = linkShaderProgram(VS_car, FS_car_day);
	SP_car_night = linkShaderProgram(VS_car, FS_car_night);
	glDeleteShader(VS_car);
	glDeleteShader(FS_car_day);
	glDeleteShader(FS_car_night);

	GLuint VS_sun = loadShader(SHADER_NAME(IDR_VS_SUN), GL_VERTEX_SHADER);
	GLuint FS_sun = loadShader(SHADER_NAME(IDR_FS_SUN), GL_FRAGMENT_SHADER);
	SP_sun = linkShaderProgram(VS_sun, FS_sun);
	glDeleteShader(VS_sun);
	glDeleteShader(FS_sun);

	CSP_shadow_highway_night = loadComputeProgram(SHADER_NAME(IDR_CS_SHADOW_HIGHWAY_NIGHT));
	GLuint VS_shadow_highway_day = loadShader(SHADER_NAME(IDR_VS_SHADOW_HIGHWAY_DAY), GL_VERTEX_SHADER);
	GLuint VS_shadow_highway_night = loadShader(SHADER_NAME(IDR_VS_SHADOW_HIGHWAY_NIGHT), GL_VERTEX_SHADER);
	GLuint VS_shadow_car_day = loadShader(SHADER_NAME(IDR_VS_SHADOW_CAR_DAY), GL_VERTEX_SHADER);
	GLuint VS_shadow_car_night = loadShader(SHADER_NAME(IDR_VS_SHADOW_CAR_NIGHT), GL_VERTEX_SHADER);
	GLuint FS_shadow = loadShader(SHADER_NAME(IDR_FS_SHADOW), GL_FRAGMENT_SHADER);
	GLuint GS_shadow_highway_night = loadShader(SHADER_NAME(IDR_GS_SHADOW_HIGHWAY_NIGHT), GL_GEOMETRY_SHADER);
	SP_shadow_highway_day = linkShaderProgram(VS_shadow_highway_day, FS_shadow);
	SP_shadow_highway_night = linkShaderProgram(VS_shadow_highway_night, FS_shadow, GS_shadow_highway_night);
	SP_shadow_car_day = linkShaderProgram(VS_shadow_car_day, FS_shadow);
	SP_shadow_car_night = linkShaderProgram(VS_shadow_car_night, FS_shadow);
	glDeleteShader(VS_shadow_highway_day);
	glDeleteShader(VS_shadow_highway_night);
	glDeleteShader(GS_shadow_highway_night);
	glDeleteShader(VS_shadow_car_day);
	glDeleteShader(VS_shadow_car_night);
	glDeleteShader(FS_shadow);

	GLuint VS_tex_blit = loadShader(SHADER_NAME(IDR_VS_TEX_BLIT), GL_VERTEX_SHADER);

	GLuint FS_gen_PCSS_mips = loadShader(SHADER_NAME(IDR_FS_GEN_PCSS_MIPS), GL_FRAGMENT_SHADER);
	SP_gen_PCSS_mips = linkShaderProgram(VS_tex_blit, FS_gen_PCSS_mips);
	glDeleteShader(FS_gen_PCSS_mips);

	GLuint FS_tex_blit = loadShader(SHADER_NAME(IDR_FS_TEX_BLIT), GL_FRAGMENT_SHADER);
	SP_tex_blit = linkShaderProgram(VS_tex_blit, FS_tex_blit);
	glDeleteShader(FS_tex_blit);

	GLuint FS_gaussian_blur = loadShader(SHADER_NAME(IDR_FS_GAUSSIAN_BLUR), GL_FRAGMENT_SHADER);
	SP_gaussian_blur = linkShaderProgram(VS_tex_blit, FS_gaussian_blur);
	glDeleteShader(FS_gaussian_blur);

	GLuint FS_buffer_to_screen = loadShader(SHADER_NAME(IDR_FS_BUFFER_TO_SCREEN), GL_FRAGMENT_SHADER);
	SP_buffer_to_screen = linkShaderProgram(VS_tex_blit, FS_buffer_to_screen);
	glDeleteShader(FS_buffer_to_screen);

	GLuint FS_text = loadShader(SHADER_NAME(IDR_FS_TEXT), GL_FRAGMENT_SHADER);
	SP_text = linkShaderProgram(VS_tex_blit, FS_text);
	glDeleteShader(FS_text);

	glDeleteShader(VS_tex_blit);
}

static void init()
{
	initShader();

	glPolygonOffset(1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int UBO_offset_alignment;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UBO_offset_alignment);

	scene_UBO_offset1 = ((sizeof(camera) - 1) / UBO_offset_alignment + 1) * UBO_offset_alignment;
	glGenBuffers(1, &scene_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, scene_UBO);
	glBufferData(GL_UNIFORM_BUFFER, scene_UBO_offset1 + sizeof(SunData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, CAMERA_BUFFER_BINDING, scene_UBO, 0, sizeof(CameraData));
	glBindBufferRange(GL_UNIFORM_BUFFER, SUN_BUFFER_BINDING, scene_UBO, scene_UBO_offset1, sizeof(SunData));

	glGenBuffers(1, &shadow_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, shadow_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(sun_shadow), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, SHADOW_TRANSFORM_BUFFER_BINDING, shadow_UBO, 0, sizeof(sun_shadow));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &tile_light_map_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tile_light_map_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(tile_light_map), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_MAP_BUFFER_BINDING, tile_light_map_SSBO, 0, sizeof(tile_light_map));
	glGenBuffers(1, &tile_light_pos_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, tile_light_pos_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(tile_light_pos), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, TILE_LIGHT_BUFFER_BINDING, tile_light_pos_UBO, 0, sizeof(tile_light_pos));
	glGenBuffers(1, &tile_light_transform_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tile_light_transform_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, 2 * MAX_CAR_CNT * sizeof(mat4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_TRANSFORM_BUFFER_BINDING, tile_light_transform_SSBO, 0, sizeof(TileLightTransformData));
	glGenBuffers(1, &car_lighting_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, car_lighting_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(car_lightings), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, CAR_LIGHTING_BUFFER_BINDING, car_lighting_SSBO, 0, sizeof(car_lightings));
	glGenBuffers(1, &tile_light_shadow_triangles_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, tile_light_shadow_triangles_SSBO);
	constexpr GLsizeiptr tile_light_shadow_triangles_SSBO_size = sizeof(TileLightShadowTriangleData) + tileLightShadowLayerOffset(NUM_TILE_LIGHT_SHADOW_LAYERS) * MAX_AVERANGE_TRIANGLES_PER_LIGHT * sizeof(TileLightShadowTriangle);
	glBufferData(GL_SHADER_STORAGE_BUFFER, tile_light_shadow_triangles_SSBO_size, nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING, tile_light_shadow_triangles_SSBO, 0, tile_light_shadow_triangles_SSBO_size);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glGenFramebuffers(1, &multisample_render_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, multisample_render_FBO);
	glGenRenderbuffers(2, multisample_render_RBOs);
	for (int i = 0; i < 2; i++)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, multisample_render_RBOs[i]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, multisample_render_RBOs[i]);
	}
	glGenRenderbuffers(1, &depth_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_RBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glGenFramebuffers(1, &render_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, render_FBO);
	glGenTextures(1, &render_tex);
	glBindTexture(GL_TEXTURE_2D, render_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, render_tex, 0);

	glGenFramebuffers(2, bloom_FBOs);
	glGenTextures(2, bloom_texs);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloom_FBOs[i]);
		glBindTexture(GL_TEXTURE_2D, bloom_texs[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloom_texs[i], 0);
	}

	vec2 screen_coords[4] = { { 1, 1 },{ -1, 1 }, { -1, -1 }, { 1, -1 } };
	glGenVertexArrays(1, &tex_blit_VAO);
	glBindVertexArray(tex_blit_VAO);
	glGenBuffers(1, &tex_mapping_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, tex_mapping_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_coords), screen_coords, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenFramebuffers(1, &shadow_day_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_day_FBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glGenTextures(1, &shadow_day_tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_day_tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1 + PCSS_MIP_LEVELS, GL_DEPTH_COMPONENT24, SHADOW_DAY_TEX_SIZE, SHADOW_DAY_TEX_SIZE, CSM_LEVELS);

	glGenFramebuffers(1, &shadow_mip_gen_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_mip_gen_FBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenSamplers(1, &shadow_PCF_sampler);
	glSamplerParameteri(shadow_PCF_sampler, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glSamplerParameteri(shadow_PCF_sampler, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glSamplerParameteri(shadow_PCF_sampler, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glSamplerParameteri(shadow_PCF_sampler, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glSamplerParameteri(shadow_PCF_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(shadow_PCF_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glGenSamplers(1, &shadow_depth_sampler);
	glSamplerParameteri(shadow_depth_sampler, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glSamplerParameteri(shadow_depth_sampler, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glSamplerParameteri(shadow_depth_sampler, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glSamplerParameteri(shadow_depth_sampler, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenFramebuffers(1, &shadow_night_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_night_FBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glGenTextures(1, &shadow_night_tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_night_tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH_COMPONENT24, SHADOW_NIGHT_TEX_SIZE, SHADOW_NIGHT_TEX_SIZE, NUM_TILE_LIGHT_SHADOW_LAYERS);

	glGenTextures(1, &text_atlas_tex);
	HRSRC rc_info = FindResource(nullptr, MAKEINTRESOURCE(IDR_TEXT_ATLAS), L"TEXTURE");
	if (rc_info != nullptr)
	{
		HGLOBAL rc_data = LoadResource(nullptr, rc_info);
		if (rc_data != nullptr)
		{
			glBindTexture(GL_TEXTURE_2D, text_atlas_tex);
			const char* data = (const char*)(LockResource(rc_data));
			glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, TEXT_WIDTH * TEXT_ALTAS_CNT_X, TEXT_HEIGHT * TEXT_ALTAS_CNT_Y, 0, GL_RED, GL_UNSIGNED_BYTE, data);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		}
		else
		{
			printf("ERROR: Can't Load Resource Text Altas\n");
		}
	}
	else
	{
		printf("ERROR: Can't Find Resource Text Altas\n");
	}

	buildMeshes();
	initScene();
	last_time_us = getTimestampMicroseconds();
}

static void drawGraphics()
{
	static float azimuth = aim_azimuth, relative_depression = aim_relative_depression, view_distance = aim_view_distance;
	static float view_z_near;
	static float scene_z_far;
	static vec3 focus(0);
	static float horizon_y;
	static std::vector<mat4> car_tile_light_shadow_transform[NUM_TILE_LIGHT_SHADOW_LAYERS];
	static std::vector<int> car_tile_light_shadow_idx[NUM_TILE_LIGHT_SHADOW_LAYERS];

	uint64_t time_us = getTimestampMicroseconds();
	uint64_t dt_us = time_us - last_time_us;
	last_time_us = time_us;
	LogicalData& logical_data = getLatestLogicalData();
	if (focus_move_dir != 0)
	{
		vec2 dir = vec2(0.0f);
		if ((focus_move_dir & 0b0011) == 0b0001)
		{
			dir += vec2(-cos(azimuth), -sin(azimuth));
		}
		if ((focus_move_dir & 0b0011) == 0b0010)
		{
			dir += vec2(cos(azimuth), sin(azimuth));
		}
		if ((focus_move_dir & 0b1100) == 0b0100)
		{
			dir += vec2(-sin(azimuth), cos(azimuth));
		}
		if ((focus_move_dir & 0b1100) == 0b1000)
		{
			dir += vec2(sin(azimuth), -cos(azimuth));
		}
		if (dir != vec2(0.0f))
		{
			dir = normalize(dir);
			float move_speed = 1.0f * view_distance + 10.0f;
			float move_distance = move_speed * 1e-6f * dt_us;
			vec3 new_focus = focus + vec3(dir.x * move_distance, dir.y * move_distance, 0);
			new_focus.x = clamp(new_focus.x, -700.0f, 700.0f);
			new_focus.y = clamp(new_focus.y, -200.0f, 200.0f);
			if (focus != new_focus)
			{
				need_update_view = true;
				focus = new_focus;
			}
		}
	}
	bool is_view_updated = need_update_view;
	if (need_update_view)
	{
		need_update_view = false;
		if (azimuth != aim_azimuth)
		{
			constexpr float ROTATE_SPEED = 10.0f;
			float max_rotate_angle = ROTATE_SPEED * 1e-6f * dt_us;
			if (abs(azimuth - aim_azimuth) > max_rotate_angle)
			{
				if (azimuth > aim_azimuth)
				{
					azimuth -= max_rotate_angle;
				}
				else
				{
					azimuth += max_rotate_angle;
				}
				need_update_view = true;
			}
			else
			{
				aim_azimuth = fmod(aim_azimuth, (2 * pi<float>()));
				azimuth = aim_azimuth;
			}
		}
		if (relative_depression != aim_relative_depression)
		{
			constexpr float ROTATE_SPEED = 5.0f;
			float max_rotate_angle = ROTATE_SPEED * 1e-6f * dt_us;
			if (abs(relative_depression - aim_relative_depression) > max_rotate_angle)
			{
				if (relative_depression > aim_relative_depression)
				{
					relative_depression -= max_rotate_angle;
				}
				else
				{
					relative_depression += max_rotate_angle;
				}
				need_update_view = true;
			}
			else
			{
				relative_depression = aim_relative_depression;
			}
		}
		if (view_distance != aim_view_distance)
		{
			constexpr float ZOOM_SPEED = 5.0f;
			float max_zoom_distance = ZOOM_SPEED * 1e-6f * dt_us * view_distance;
			if (abs(view_distance - aim_view_distance) > max_zoom_distance)
			{
				if (view_distance > aim_view_distance)
				{
					view_distance -= max_zoom_distance;
				}
				else
				{
					view_distance += max_zoom_distance;
				}
				need_update_view = true;
			}
			else
			{
				view_distance = aim_view_distance;
			}
		}

		float depression = pi<float>() / 2 * (1 - FLT_EPSILON) * (1 - ((1 - 0.3f * view_distance / MAX_VIEW_DISTANCE) * (1 - relative_depression)));
		vec3 view_dir(-cos(depression) * sin(azimuth), cos(depression) * cos(azimuth), -sin(depression));
		focus.z = 0.0f;
		vec3 eye = focus - view_distance * view_dir;
		vec2 height_map_coord((eye.x - SCENE_GRID_AREA.x) / (SCENE_GRID_AREA.z - SCENE_GRID_AREA.x) * SCENE_GRID_SIZE_X - 0.5f,
			(eye.y - SCENE_GRID_AREA.y) / (SCENE_GRID_AREA.w - SCENE_GRID_AREA.y) * SCENE_GRID_SIZE_Y - 0.5f);
		if (0 < height_map_coord.x && height_map_coord.x < SCENE_GRID_SIZE_X - 1 && 0 < height_map_coord.y && height_map_coord.y < SCENE_GRID_SIZE_Y - 1)
		{
			auto height_map = g_scene_quad_tree[NUM_SCENE_GRID_LEVELS - 1];
			float scene_height = height_map[int(height_map_coord.x)][int(height_map_coord.y)].max_height * (int(height_map_coord.x) + 1 - height_map_coord.x) * (int(height_map_coord.y) + 1 - height_map_coord.y) +
				height_map[int(height_map_coord.x)][int(height_map_coord.y) + 1].max_height * (int(height_map_coord.x) + 1 - height_map_coord.x) * (height_map_coord.y - int(height_map_coord.y)) +
				height_map[int(height_map_coord.x) + 1][int(height_map_coord.y)].max_height * (height_map_coord.x - int(height_map_coord.x)) * (int(height_map_coord.y) + 1 - height_map_coord.y) +
				height_map[int(height_map_coord.x) + 1][int(height_map_coord.y) + 1].max_height * (height_map_coord.x - int(height_map_coord.x)) * (height_map_coord.y - int(height_map_coord.y));
			if (scene_height > 0.0f && eye.z < 2.0f * scene_height)
			{
				float height_offset = scene_height - 0.5f * eye.z;
				eye.z += height_offset;
				focus.z += height_offset;
			}
		}
		camera.view = lookAt(eye, focus, vec3(-sin(azimuth), cos(azimuth), 0));
		camera.inv_view = inverse(camera.view);
		horizon_y = (tan(depression - acos(EARTH_RADIUS / (focus.z + EARTH_RADIUS))) / tan(FOV_Y / 2) + 1) * window_height / 2;

		updateTerrainLOD(TERRAIN_LOD_FACTOR, eye);

		Frustum camera_frustum(camera.view, FOV_Y, float(window_width) / window_height, MIN_VIEW_Z_NEAR, VIEW_Z_FAR);
		static CircularQueue<QuadTreeIdx> grids_to_calc(5);
		view_z_near = VIEW_Z_FAR;
		scene_z_far = MIN_VIEW_Z_NEAR;
		for (int i = 0; i < NUM_SCENE_GRID_ROOTS_X; i++)
		{
			for (int j = 0; j < NUM_SCENE_GRID_ROOTS_Y; j++)
			{
				grids_to_calc.emplace_back(0, i, j);
			}
		}
		while (!grids_to_calc.empty())
		{
			uint32_t level = grids_to_calc.front().level;
			size_t x = grids_to_calc.front().x;
			size_t y = grids_to_calc.front().y;
			grids_to_calc.pop_front();
			BoundAABB bound = sceneGridBound(level, x, y);
			vec3 half_size = 0.5f * bound.size();
			auto& p_near = camera_frustum.getPlane(Frustum::NEAR);
			auto& p_far = camera_frustum.getPlane(Frustum::FAR);
			float r = dot(half_size, abs(p_near.normal));
			float m_near = dot(p_near.normal, bound.center()) + p_near.d;
			float m_far = dot(p_far.normal, bound.center()) + p_far.d;
			float z_near = MIN_VIEW_Z_NEAR + m_near - r;
			float z_far = VIEW_Z_FAR - m_far + r;
			if (z_near >= view_z_near && z_far <= scene_z_far)
			{
				continue;
			}
			INTERSECTION_TEST_RESULT result = camera_frustum.intersectionTestNoNearFar(bound);
			if (result == INTERSECTION_TEST_OUTSIDE)
			{
				continue;
			}
			if (level == NUM_SCENE_GRID_LEVELS - 1)
			{
				view_z_near = std::min(view_z_near, z_near);
				scene_z_far = std::max(scene_z_far, z_far);
				continue;
			}
			if (result == INTERSECTION_TEST_INSIDE)
			{
				view_z_near = std::min(view_z_near, z_near + 4.0f / 3.0f * r);
				scene_z_far = std::max(scene_z_far, z_far - 4.0f / 3.0f * r);
			}
			grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y);
			grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y);
			grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y + 1);
			grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y + 1);
		}
		view_z_near = std::max(view_z_near, MIN_VIEW_Z_NEAR);
		scene_z_far = std::min(scene_z_far, VIEW_Z_FAR);
		camera.projection = perspective(FOV_Y, float(window_width) / window_height, view_z_near, VIEW_Z_FAR);
		camera.view_proj = camera.projection * camera.view;
	}

	SunData sun;
	sun.light_dir_and_radius = vec4(logical_data.sun_dir, 1.0f);
	sun.diffuse_specular = vec3(0.4);
	if (-SUN_RADIUS_DIST_RATIO < logical_data.sun_dir.z && logical_data.sun_dir.z < SUN_RADIUS_DIST_RATIO)
	{
		float x = -logical_data.sun_dir.z / SUN_RADIUS_DIST_RATIO;
		float sqrt_one_minus_x2 = sqrt(1 - x * x);
		float area_unit = 0.5 * pi<float>() - (sqrt_one_minus_x2 * x + asin(x));
		if (area_unit < sqrt_one_minus_x2 * (1 - x))
		{
			area_unit = sqrt_one_minus_x2 * (1 - x);
		}
		else if (area_unit > pi<float>() - sqrt_one_minus_x2 * (x + 1))
		{
			area_unit = pi<float>() - sqrt_one_minus_x2 * (x + 1);
		}
		float area_percent = area_unit / pi<float>();
		float center = 2.0f / 3.0f * sqrt_one_minus_x2 * sqrt_one_minus_x2 * sqrt_one_minus_x2 / area_unit;
		if (center < x + (1 - x) / 3)
		{
			center = x + (1 - x) / 3;
		}
		sun.diffuse_specular *= area_percent;
		sun.light_dir_and_radius.z += center * SUN_RADIUS_DIST_RATIO;
		sun.light_dir_and_radius = vec4(normalize(vec3(sun.light_dir_and_radius)), sqrt(area_percent));
	}
	constexpr float ATMOSPHERE = EARTH_RADIUS + 2e4f;
	float absorb_factor = -1e-5f * (-EARTH_RADIUS * sun.light_dir_and_radius.z + sqrt(ATMOSPHERE * ATMOSPHERE - EARTH_RADIUS * EARTH_RADIUS * (1 - sun.light_dir_and_radius.z * sun.light_dir_and_radius.z)));
	sun.diffuse_specular *= vec3(exp(0.2f * absorb_factor), exp(0.3f * absorb_factor), exp(1.1f * absorb_factor));
	sun.ambient = vec3(0.01f);
	sun.sky_color = vec4(0.01f, 0.015f, 0.055f, 1.0f);
	if (sun.light_dir_and_radius.z > -0.2f)
	{
		sun.ambient += vec3((sun.light_dir_and_radius.z + 0.2f) * 0.2f);
		sun.sky_color += (sun.light_dir_and_radius.z + 0.2f) * vec4(0.2f, 0.3f, 1.1f, 0.0f);
	}

	Frustum camera_frustum{ camera.view_proj };

	int num_visible_cars;
	int num_visible_light_on_cars;
	int num_visible_car_lights;

	if (sun.light_dir_and_radius.z > 0)
	{
		num_visible_cars = logical_data.num_cars;

		mat4 sun_shadow_view = lookAt(vec3(0.0f), -vec3(sun.light_dir_and_radius), vec3(-sun.light_dir_and_radius.x, -sun.light_dir_and_radius.y, sun.light_dir_and_radius.z));
		mat3 abs_sun_shadow_rot = abs(mat3(sun_shadow_view));
		vec3 view_top(0.0f, tan(FOV_Y / 2), 0.0f);
		vec3 view_left = vec3(view_top.y / window_height * window_width, 0.0f, 0.0f);
		view_top = mat3(camera.inv_view) * view_top;
		view_left = mat3(camera.inv_view) * view_left;
		float z_min = FLT_MAX;
		float slope = sqrt(1.0f / (sun.light_dir_and_radius.z * sun.light_dir_and_radius.z) - 1.0f);
		float x_maxs[CSM_LEVELS], x_mins[CSM_LEVELS], y_maxs[CSM_LEVELS], y_mins[CSM_LEVELS], z_mins[CSM_LEVELS];
		float CSM_ratio = pow(scene_z_far / view_z_near, 1.0f / CSM_LEVELS);
		float camera_z_far = scene_z_far;
		for (int i = CSM_LEVELS - 1; i >= 0; i--)
		{
			x_mins[i] = FLT_MAX, x_maxs[i] = FLT_MIN, y_mins[i] = FLT_MAX, y_maxs[i] = FLT_MIN;
			static CircularQueue<QuadTreeIdx> grids_to_calc(5);
			float camera_z_near = camera_z_far / CSM_ratio;
			Frustum camera_level_frustum{ camera.view, FOV_Y, float(window_width) / window_height, camera_z_near, camera_z_far };
			BoundFrustum camera_level_bound{ camera.inv_view, FOV_Y, float(window_width) / window_height, camera_z_near, camera_z_far };
			float frustum_x_min = FLT_MAX, frustum_x_max = FLT_MIN, frustum_y_min = FLT_MAX, frustum_y_max = FLT_MIN, frustum_z_far = FLT_MAX;
			for (int i = 0; i < 8; i++)
			{
				vec3 corner_in_shadow = sun_shadow_view * vec4{ camera_level_bound.points[i], 1.0f };
				frustum_z_far = std::min(frustum_z_far, corner_in_shadow.z);
				frustum_x_min = std::min(frustum_x_min, corner_in_shadow.x);
				frustum_x_max = std::max(frustum_x_max, corner_in_shadow.x);
				frustum_y_min = std::min(frustum_y_min, corner_in_shadow.y);
				frustum_y_max = std::max(frustum_y_max, corner_in_shadow.y);
			}

			camera_z_far = camera_z_near;
			for (int j = 0; j < NUM_SCENE_GRID_ROOTS_X; j++)
			{
				for (int k = 0; k < NUM_SCENE_GRID_ROOTS_Y; k++)
				{
					grids_to_calc.emplace_back(0, j, k);
				}
			}
			while (!grids_to_calc.empty())
			{
				uint32_t level = grids_to_calc.front().level;
				size_t x = grids_to_calc.front().x;
				size_t y = grids_to_calc.front().y;
				grids_to_calc.pop_front();
				BoundAABB bound = sceneGridBound(level, x, y);
				INTERSECTION_TEST_RESULT result = camera_level_frustum.intersectionTest(bound);
				if (result == INTERSECTION_TEST_OUTSIDE)
				{
					continue;
				}
				vec3 half_size = 0.5f * bound.size();
				vec3 center = sun_shadow_view * vec4(bound.center(), 1.0f);
				vec3 r = abs_sun_shadow_rot * half_size;
				if ((center.x - r.x >= x_mins[i] || x_mins[i] <= frustum_x_min)
					&& (center.x + r.x <= x_maxs[i] || x_maxs[i] >= frustum_x_max)
					&& (center.y - r.y >= y_mins[i] || y_mins[i] <= frustum_y_min)
					&& (center.y + r.y <= y_maxs[i] || y_maxs[i] >= frustum_y_max))
				{
					z_min = std::min(z_min, center.z - r.z);
					continue;
				}
				if (level == NUM_SCENE_GRID_LEVELS - 1)
				{
					z_min = std::min(z_min, center.z - r.z);
					x_mins[i] = std::min(x_mins[i], center.x - r.x);
					x_maxs[i] = std::max(x_maxs[i], center.x + r.x);
					y_mins[i] = std::min(y_mins[i], center.y - r.y);
					y_maxs[i] = std::max(y_maxs[i], center.y + r.y);
					continue;
				}
				if (result == INTERSECTION_TEST_INSIDE)
				{
					x_mins[i] = std::min(x_mins[i], center.x + 1.0f / 3.0f * r.x);
					x_maxs[i] = std::max(x_maxs[i], center.x - 1.0f / 3.0f * r.x);
					y_mins[i] = std::min(y_mins[i], center.y + 1.0f / 3.0f * r.y);
					y_maxs[i] = std::max(y_maxs[i], center.y - 1.0f / 3.0f * r.y);
				}
				grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y);
				grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y);
				grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y + 1);
				grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y + 1);
			}
			z_min = std::max(z_min, frustum_z_far);
			x_mins[i] = std::max(x_mins[i], frustum_x_min);
			x_maxs[i] = std::min(x_maxs[i], frustum_x_max);
			y_mins[i] = std::max(y_mins[i], frustum_y_min);
			y_maxs[i] = std::min(y_maxs[i], frustum_y_max);
			z_mins[i] = z_min;
		}

		for (int i = 0; i < CSM_LEVELS - 3; i += 4)
		{
			float group_x_min = FLT_MAX, group_x_max = FLT_MIN, group_y_min = FLT_MAX, group_y_max = FLT_MIN, group_z_min = FLT_MAX;
			for (int j = 0; j < 4; j++)
			{
				group_z_min = std::min(group_z_min, z_mins[i + j]);
				group_x_min = std::min(group_x_min, x_mins[i + j]);
				group_x_max = std::max(group_x_max, x_maxs[i + j]);
				group_y_min = std::min(group_y_min, y_mins[i + j]);
				group_y_max = std::max(group_y_max, y_maxs[i + j]);
			}
			if (2 * (x_maxs[i] - x_mins[i]) >= group_x_max - group_x_min && 2 * (y_maxs[i] - y_mins[i]) >= group_y_max - group_y_min)
			{
				x_mins[i] = group_x_min;
				x_maxs[i] = (group_x_min + group_x_max) / 2;
				y_mins[i] = group_y_min;
				y_maxs[i] = (group_y_min + group_y_max) / 2;
				x_mins[i + 1] = (group_x_min + group_x_max) / 2;
				x_maxs[i + 1] = group_x_max;
				y_mins[i + 1] = group_y_min;
				y_maxs[i + 1] = (group_y_min + group_y_max) / 2;
				x_mins[i + 2] = group_x_min;
				x_maxs[i + 2] = (group_x_min + group_x_max) / 2;
				y_mins[i + 2] = (group_y_min + group_y_max) / 2;
				y_maxs[i + 2] = group_y_max;
				x_mins[i + 3] = (group_x_min + group_x_max) / 2;
				x_maxs[i + 3] = group_x_max;
				y_mins[i + 3] = (group_y_min + group_y_max) / 2;
				y_maxs[i + 3] = group_y_max;
				z_mins[i] = z_mins[i + 1] = z_mins[i + 2] = z_mins[i + 3] = group_z_min;
			}
		}
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			Frustum shadow_frustum(sun_shadow_view, x_mins[i], x_maxs[i], y_mins[i], y_maxs[i], 0.0f, 1.0f);
			float z_max = FLT_MIN;
			static CircularQueue<QuadTreeIdx> grids_to_calc(5);
			for (int j = 0; j < NUM_SCENE_GRID_ROOTS_X; j++)
			{
				for (int k = 0; k < NUM_SCENE_GRID_ROOTS_Y; k++)
				{
					grids_to_calc.emplace_back(0, j, k);
				}
			}
			int test_cnt = 0;
			while (!grids_to_calc.empty())
			{
				test_cnt++;
				uint32_t level = grids_to_calc.front().level;
				size_t x = grids_to_calc.front().x;
				size_t y = grids_to_calc.front().y;
				grids_to_calc.pop_front();
				BoundAABB bound = sceneGridBound(level, x, y);
				auto& p = shadow_frustum.getPlane(Frustum::NEAR);
				float bound_z_max = -dot(p.normal, bound.center()) + dot(abs(p.normal), 0.5f * bound.size());
				if (bound_z_max <= z_max)
				{
					continue;
				}
				INTERSECTION_TEST_RESULT result = shadow_frustum.intersectionTestNoNearFar(bound);
				if (result == INTERSECTION_TEST_OUTSIDE)
				{
					continue;
				}
				if (result == INTERSECTION_TEST_INSIDE || level == NUM_SCENE_GRID_LEVELS - 1)
				{
					z_max = std::max(z_max, bound_z_max);
					continue;
				}
				grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y);
				grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y);
				grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y + 1);
				grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y + 1);
			}
			constexpr float MIN_PADDING = (1 / (1 - 2 * MIN_SHADOW_MAP_PADDING) - 1) / 2;
			float x_padding = std::max(MAX_PENUMBRA_RADIUS, MIN_PADDING * (x_maxs[i] - x_mins[i]));
			float y_padding = std::max(MAX_PENUMBRA_RADIUS, MIN_PADDING * (y_maxs[i] - y_mins[i]));
			float z_padding = std::max(x_padding / (x_maxs[i] - x_mins[i]), y_padding / (y_maxs[i] - y_mins[i])) * (z_max - z_mins[i]);
			mat4 shadow_mat = ortho(x_mins[i] - x_padding, x_maxs[i] + x_padding, y_mins[i] - y_padding, y_maxs[i] + y_padding, -z_max, -z_mins[i] + z_padding) * sun_shadow_view;
			sun_shadow.view_proj[i] = shadow_mat;
			sun_shadow.tex[i] = mat4(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f) * shadow_mat;
		}
	}
	else
	{
		num_visible_cars = 0;
		num_visible_light_on_cars = 0;
		num_visible_car_lights = 0;
		memset(&tile_light_map, 0, sizeof(tile_light_map));
		struct CarLightInfo
		{
			vec4 pos;
			vec4 dir;
			BoundEllipticFrustum bound;
			float distance_to_camera;
			ivec2* light_map_grid;
		};
		static std::vector<CarLightInfo>car_light_infos;
		static BoundEllipticFrustum tile_light_bounds[2 * MAX_CAR_CNT];
		car_light_infos.clear();
		int num_car_lights = 2 * logical_data.num_light_on_cars;
		int cnt = 0;
		BoundFrustum camera_bound{ camera.inv_view, FOV_Y, float(window_width) / window_height, view_z_near, VIEW_Z_FAR };
		for (int i = 0; i < num_car_lights; i++)
		{
			const vec4& light_pos = logical_data.car_light_pos[i];
			const vec4& light_dir = logical_data.car_light_dir[i];
			vec4 light_x = normalize(vec4{ light_dir.y, -light_dir.x, 0.0f, 0.0f });
			vec4 light_y = vec4{ cross(vec3{ light_x }, vec3{ light_dir }), 0.0f };

			BoundEllipticFrustum bound{ mat4{ light_x, light_y, -light_dir, light_pos }, CAR_LIGHT_V_RAD, CAR_LIGHT_ASPECT , CAR_LIGHT_NEAR, CAR_LIGHT_NEAR + CAR_LIGHT_RANGE };
			if (camera_frustum.cullingTest(bound))
			{
				constexpr float POS_OFFSET = CAR_LIGHT_NEAR + LIGHT_MAP_GRID_LENGTH;
				vec3 center_pos = light_pos + POS_OFFSET * light_dir;
				ivec2 light_map_idx = ivec2{ 1.0f / LIGHT_MAP_GRID_LENGTH * vec2(center_pos) + 0.5f * vec2(LIGHT_MAP_SIZE_X, LIGHT_MAP_SIZE_Y) };
				ivec2* light_map_grid = &tile_light_map.idx_range[light_map_idx.x * LIGHT_MAP_SIZE_Y + light_map_idx.y];
				car_light_infos.push_back({ light_pos, light_dir, bound, length(center_pos - vec3(camera.inv_view[3])), light_map_grid });
				light_map_grid->x++;
			}
		}
		std::sort(car_light_infos.begin(), car_light_infos.end(), [](const CarLightInfo& a, const CarLightInfo& b) { return a.distance_to_camera < b.distance_to_camera; });
		for (CarLightInfo& car_light_info : car_light_infos)
		{
			if (car_light_info.light_map_grid->y == 0)
			{
				car_light_info.light_map_grid->y = num_visible_car_lights;
				num_visible_car_lights += car_light_info.light_map_grid->x;
				car_light_info.light_map_grid->x = car_light_info.light_map_grid->y;
			}
			int idx = car_light_info.light_map_grid->y++;
			tile_light_pos.positions[idx] = car_light_info.pos;
			tile_light_transforms.view_proj[idx] = CAR_LIGHT_SHADOW_PROJ * lookAt(vec3(car_light_info.pos), vec3(car_light_info.pos) + vec3(car_light_info.dir), vec3(0.0f, 0.0f, 1.0f));
			tile_light_bounds[idx] = car_light_info.bound;
		}
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			car_tile_light_shadow_idx[i].clear();
			car_tile_light_shadow_transform[i].clear();
		}
		auto cullCars = [&camera_frustum](mat4* transforms, vec3* colors, int* light_indices, int size)->int
			{
				int i = 0, j = size;
				while (i < j)
				{
					mat4& transform = transforms[i];
					ivec2 pos_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(transform[3]) + 0.5f * vec2(LIGHT_MAP_SIZE_X, LIGHT_MAP_SIZE_Y));
					BoundOBB car_bound{ car_local_bound, transform };
					int num_lighting = 0;
					for (int m = -1; m < 2; m++)
					{
						for (int n = -1; n < 2; n++)
						{
							ivec2 idx = pos_idx + ivec2(m, n);
							for (int k = tile_light_map.idx_range[idx.x * LIGHT_MAP_SIZE_Y + idx.y].x; k < tile_light_map.idx_range[idx.x * LIGHT_MAP_SIZE_Y + idx.y].y; k++)
							{
								if (intersectionTest(tile_light_bounds[k], car_bound))
								{
									num_lighting++;
									int layer = findTileLightShadowLayer(k);
									if (layer < NUM_TILE_LIGHT_SHADOW_LAYERS)
									{
										car_tile_light_shadow_idx[layer].push_back(k);
										car_tile_light_shadow_transform[layer].push_back(tile_light_transforms.view_proj[k] * transform);
									}
									light_indices[i * LIGHTING_SIZE_PER_CAR + num_lighting] = k;
								}
							}
						}
					}
					light_indices[i * LIGHTING_SIZE_PER_CAR] = num_lighting;
					if (num_lighting > 0 || camera_frustum.cullingTest(BoundAABB{ car_local_bound, transform }))
					{
						i++;
					}
					else
					{
						j--;
						std::swap(transform, transforms[j]);
						std::swap(colors[i], colors[j]);
					}
				}
				return i;
			};
		num_visible_light_on_cars += cullCars(logical_data.car_transform, logical_data.car_color, car_lightings.light_indices, logical_data.num_light_on_cars);
		num_visible_cars += num_visible_light_on_cars;
		num_visible_cars += cullCars(&logical_data.car_transform[logical_data.num_light_on_cars],
			&logical_data.car_color[logical_data.num_light_on_cars],
			&car_lightings.light_indices[num_visible_light_on_cars * LIGHTING_SIZE_PER_CAR],
			logical_data.num_cars - logical_data.num_light_on_cars);
	}

	if (is_view_updated)
	{
		glNamedBufferSubData(scene_UBO, 0, sizeof(camera), &camera);
		glProgramUniform1f(SP_sun, glGetUniformLocation(SP_sun, "horizonY"), horizon_y);
	}

	glNamedBufferSubData(scene_UBO, scene_UBO_offset1, sizeof(SunData), &sun);
	if (sun.light_dir_and_radius.z > 0)
	{
		glNamedBufferSubData(shadow_UBO, 0, sizeof(sun_shadow), &sun_shadow);
		glNamedBufferSubData(car_transform_VBO, 0, num_visible_cars * sizeof(mat4), logical_data.car_transform);
		glNamedBufferSubData(car_color_VBO, 0, num_visible_cars * sizeof(vec3), logical_data.car_color);
	}
	else
	{
		glNamedBufferSubData(tile_light_map_SSBO, 0, sizeof(tile_light_map), &tile_light_map);
		glNamedBufferSubData(tile_light_pos_UBO, 0, num_visible_car_lights * sizeof(vec4), &tile_light_pos);
		glNamedBufferSubData(tile_light_transform_SSBO, 0, num_visible_car_lights * sizeof(mat4), &tile_light_transforms);
		glNamedBufferSubData(car_lighting_SSBO, 0, num_visible_cars * LIGHTING_SIZE_PER_CAR * sizeof(int), &car_lightings);
		glNamedBufferSubData(car_transform_VBO, 0, num_visible_light_on_cars * sizeof(mat4), logical_data.car_transform);
		glNamedBufferSubData(car_color_VBO, 0, num_visible_light_on_cars * sizeof(vec3), logical_data.car_color);
		glNamedBufferSubData(car_transform_VBO, num_visible_light_on_cars * sizeof(mat4), (num_visible_cars - num_visible_light_on_cars) * sizeof(mat4), &logical_data.car_transform[logical_data.num_light_on_cars]);
		glNamedBufferSubData(car_color_VBO, num_visible_light_on_cars * sizeof(vec3), (num_visible_cars - num_visible_light_on_cars) * sizeof(vec3), &logical_data.car_color[logical_data.num_light_on_cars]);
		glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "numLightOnCars"), num_visible_light_on_cars);
		int car_tile_light_shadow_offset = 0;
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			int count = car_tile_light_shadow_transform[i].size();
			glNamedBufferSubData(car_tile_light_shadow_transform_VBO, car_tile_light_shadow_offset * sizeof(mat4), count * sizeof(mat4), car_tile_light_shadow_transform[i].data());
			glNamedBufferSubData(car_tile_light_shadow_idx_VBO, car_tile_light_shadow_offset * sizeof(int), count * sizeof(int), car_tile_light_shadow_idx[i].data());
			car_tile_light_shadow_offset += count;
		}
	}
	if (sun.light_dir_and_radius.z > -0.2f)
	{
		mat4 transform = rotate(acos(sun.light_dir_and_radius.z), vec3(-sun.light_dir_and_radius.y, sun.light_dir_and_radius.x, 0));
		transform[3] = vec4(20.0f / SUN_RADIUS_DIST_RATIO * logical_data.sun_dir, 1.0f);
		glProgramUniformMatrix4fv(SP_sun, glGetUniformLocation(SP_sun, "transform"), 1, GL_FALSE, (GLfloat*)&transform);
	}

	glBindTextureUnit(0, highway_tex);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	if (sun.light_dir_and_radius.z > 0)
	{
		glBindTextureUnit(1, shadow_day_tex);
		glBindTextureUnit(2, shadow_day_tex);
		glBindSampler(1, shadow_PCF_sampler);
		glBindSampler(2, shadow_depth_sampler);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_day_FBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOW_DAY_TEX_SIZE, SHADOW_DAY_TEX_SIZE);
		glUseProgram(SP_shadow_highway_day);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glDisable(GL_CULL_FACE);
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0, i);
			glProgramUniform1i(SP_shadow_highway_day, glGetUniformLocation(SP_shadow_highway_day, "csm_level"), i);
			Frustum shadow_frustum(sun_shadow.view_proj[i]);
			drawTerrainMesh(shadow_frustum);
		}
		glEnable(GL_CULL_FACE);
		glBindVertexArray(highway_VAO);
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0, i);
			glProgramUniform1i(SP_shadow_highway_day, glGetUniformLocation(SP_shadow_highway_day, "csm_level"), i);
			glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
		}
		glBindVertexArray(bridge_VAO);
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0, i);
			glProgramUniform1i(SP_shadow_highway_day, glGetUniformLocation(SP_shadow_highway_day, "csm_level"), i);
			glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		}
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0);
		glUseProgram(SP_shadow_car_day);
		glBindVertexArray(car_shadow_day_VAO);
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0, i);
			glProgramUniform1i(SP_shadow_car_day, glGetUniformLocation(SP_shadow_car_day, "csm_level"), i);
			glDrawElementsInstanced(GL_TRIANGLES, CAR_SHADOW_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
		}
		glDisable(GL_POLYGON_OFFSET_FILL);

		glUseProgram(SP_gen_PCSS_mips);
		glBindVertexArray(tex_blit_VAO);
		glDepthFunc(GL_ALWAYS);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_mip_gen_FBO);
		for (int i = 0; i < PCSS_MIP_LEVELS; i++)
		{
			glProgramUniform1i(SP_gen_PCSS_mips, glGetUniformLocation(SP_gen_PCSS_mips, "mip_level"), i);
			for (int j = 0; j < CSM_LEVELS; j++)
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, i + 1, j);
				glViewport(0, 0, SHADOW_DAY_TEX_SIZE >> (i + 1), SHADOW_DAY_TEX_SIZE >> (i + 1));
				glProgramUniform1i(SP_gen_PCSS_mips, glGetUniformLocation(SP_gen_PCSS_mips, "layer"), j);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
		}
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, multisample_render_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glClearBufferfv(GL_COLOR, 0, &sun.sky_color.r);
		glClearBufferfv(GL_COLOR, 1, COLOR_BLACK);
		glViewport(0, 0, window_width, window_height);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glUseProgram(SP_highway_day);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glBindVertexArray(highway_VAO);
		glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_terrain_day);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawTerrainMesh(camera_frustum);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glUseProgram(SP_car_day);
		glBindVertexArray(car_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, CAR_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
		glBindSampler(1, 0);
		glBindSampler(2, 0);
	}
	else
	{
		glBindTextureUnit(1, shadow_night_tex);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_night_FBO);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_night_tex, 0);
		glClear(GL_DEPTH_BUFFER_BIT);
		for (int i = 0; i < 4; i++)
		{
			glEnable(GL_CLIP_DISTANCE0 + i);
		}
		glEnable(GL_POLYGON_OFFSET_FILL);
		glViewport(0, 0, SHADOW_NIGHT_TEX_SIZE, SHADOW_NIGHT_TEX_SIZE);
		/*
		glUseProgram(CSP_shadow_highway_night);
		glClearNamedBufferSubData(tile_light_shadow_triangles_SSBO, GL_R32UI, 0, sizeof(TileLightShadowTriangleData::counts), GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, VERTEX_BUFFER_BINDING, highway_VBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INDEX_BUFFER_BINDING, highway_EBO);
		glProgramUniform1i(CSP_shadow_highway_night, glGetUniformLocation(CSP_shadow_highway_night, "num_triangles"), HIGHWAY_EBO_SIZE / 3);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		glDispatchCompute((HIGHWAY_EBO_SIZE / 3 + 63) / 64, 1, 1);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		glClearNamedBufferSubData(tile_light_shadow_triangles_SSBO, GL_R32UI, 0, sizeof(TileLightShadowTriangleData::counts), GL_RED_INTEGER, GL_UNSIGNED_INT, nullptr);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, VERTEX_BUFFER_BINDING, bridge_VBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INDEX_BUFFER_BINDING, bridge_EBO);
		glProgramUniform1i(CSP_shadow_highway_night, glGetUniformLocation(CSP_shadow_highway_night, "num_triangles"), BRIDGE_EBO_SIZE / 3);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		glDispatchCompute((BRIDGE_EBO_SIZE / 3 + 63) / 64, 1, 1);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
		*/
		glUseProgram(SP_shadow_highway_night);
		glBindVertexArray(highway_VAO);
		glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_shadow_car_night);
		glBindVertexArray(car_shadow_night_VAO);
		int car_tile_light_shadow_offset = 0;
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			int count = car_tile_light_shadow_transform[i].size();
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_night_tex, 0, i);
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, CAR_SHADOW_EBO_SIZE, GL_UNSIGNED_INT, 0, count, car_tile_light_shadow_offset);
			car_tile_light_shadow_offset += count;
		}
		for (int i = 0; i < 4; i++)
		{
			glDisable(GL_CLIP_DISTANCE0 + i);
		}
		glDisable(GL_POLYGON_OFFSET_FILL);

		glBindFramebuffer(GL_FRAMEBUFFER, multisample_render_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glClearBufferfv(GL_COLOR, 0, &sun.sky_color.r);
		glClearBufferfv(GL_COLOR, 1, COLOR_BLACK);
		glViewport(0, 0, window_width, window_height);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glUseProgram(SP_highway_night);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glBindVertexArray(highway_VAO);
		glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_terrain_night);
		drawTerrainMesh(camera_frustum);
		glUseProgram(SP_car_night);
		glDrawBuffers(2, attachments);
		glBindVertexArray(car_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, CAR_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
	}
	if (sun.light_dir_and_radius.z > -0.2f)
	{
		glUseProgram(SP_sun);
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glBindVertexArray(sun_VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, SUN_VBO_SIZE);
	}

	glDisable(GL_DEPTH_TEST);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, window_width, window_height, 0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloom_FBOs[0]);
	glBindVertexArray(tex_blit_VAO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, bloom_buffer_width, BLOOM_BUFFER_HEIGHT);
	glBindTextureUnit(0, render_tex);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUseProgram(SP_tex_blit);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloom_FBOs[1]);
	glUseProgram(SP_gaussian_blur);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, BLOOM_BUFFER_HEIGHT, bloom_buffer_width);
	glBindTextureUnit(0, bloom_texs[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloom_FBOs[0]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, bloom_buffer_width, BLOOM_BUFFER_HEIGHT);
	glBindTextureUnit(0, bloom_texs[1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multisample_render_FBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, render_FBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, window_width, window_height, 0, 0, window_width, window_height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);
	glBindTextureUnit(0, render_tex);
	glBindTextureUnit(1, bloom_texs[0]);
	glUseProgram(SP_buffer_to_screen);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	static float fps = 60;
	fps = (fps + 1) / (1.0f + dt_us * 1e-6f);

	if (show_fps)
	{
		char str[40];
		static int displayed_fps = fps, displayed_tick_rate = tick_rate;
		if (fps > displayed_fps + 1 || fps < displayed_fps - 1)
		{
			displayed_fps = round(fps);
		}
		if (tick_rate > displayed_tick_rate + 1 || tick_rate < displayed_tick_rate - 1)
		{
			displayed_tick_rate = round(tick_rate);
		}
		sprintf_s(str, 40, "fps: %d|%d", displayed_fps, displayed_tick_rate);
		glBindTextureUnit(0, text_atlas_tex);
		glEnable(GL_BLEND);
		glUseProgram(SP_text);
		for (int i = 0; str[i] != '\0'; i++)
		{
			constexpr int x = 10;
			constexpr int y = 10;
			float offset_x = static_cast<float>((str[i] - TEXT_ALTAS_ASCII_OFFSET) % TEXT_ALTAS_CNT_X) / TEXT_ALTAS_CNT_X;
			float offset_y = static_cast<float>((str[i] - TEXT_ALTAS_ASCII_OFFSET) / TEXT_ALTAS_CNT_X) / TEXT_ALTAS_CNT_Y;
			glProgramUniform2f(SP_text, glGetUniformLocation(SP_text, "tex_offset"), offset_x, offset_y);
			glViewport(x + i * (TEXT_WIDTH - 5), y, TEXT_WIDTH, TEXT_HEIGHT);
			glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		}
		glDisable(GL_BLEND);
	}

	glBindVertexArray(0);
	glUseProgram(0);
}

static void onResize(GLFWwindow*, int width, int height)
{
	if (width <= 0 || height <= 0 || window_width == width && window_height == height)
	{
		return;
	}
	window_width = width;
	window_height = height;
	need_update_view = true;

	GLint max_tex_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
	bloom_buffer_width = std::min(BLOOM_BUFFER_HEIGHT * (width + 1) / (height + 1), max_tex_size);

	int MSAA_level = 8;
	for (int i = 0; i < 2; i++)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, multisample_render_RBOs[i]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_level, GL_RGB16F, width, height);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, depth_RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_level, GL_DEPTH_COMPONENT32, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, render_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, bloom_texs[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, bloom_buffer_width, BLOOM_BUFFER_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, bloom_texs[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, BLOOM_BUFFER_HEIGHT, bloom_buffer_width, 0, GL_RGB, GL_FLOAT, nullptr);
}

static void onKey(GLFWwindow*, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case ' ':
			is_paused = !is_paused;
			return;
		case '=':
			if (simulate_speed < 5)
			{
				simulate_speed++;
			}
			return;
		case '-':
			if (simulate_speed > 1)
			{
				simulate_speed--;
			}
			return;
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
			simulate_speed = 1 << (key - '1');
			return;
		case 'F':
			show_fps = !show_fps;
			return;
		case 'A':
			focus_move_dir |= 0b0001;
			return;
		case 'D':
			focus_move_dir |= 0b0010;
			return;
		case 'W':
			focus_move_dir |= 0b0100;
			return;
		case 'S':
			focus_move_dir |= 0b1000;
			return;
		}
	}
	else if (action == GLFW_RELEASE)
	{
		switch (key)
		{
		case 'A':
			focus_move_dir &= ~(0b0001);
			return;
		case 'D':
			focus_move_dir &= ~(0b0010);
			return;
		case 'W':
			focus_move_dir &= ~(0b0100);
			return;
		case 'S':
			focus_move_dir &= ~(0b1000);
			return;
		}
	}
}

static void onMouseWheel(GLFWwindow* window, double xoffset, double yoffset)
{
	float view_distance;
	if (yoffset > 0)
	{
		view_distance = aim_view_distance / 1.1f;
		if (view_distance < MIN_VIEW_DISTANCE)
		{
			view_distance = MIN_VIEW_DISTANCE;
		}
	}
	else
	{
		view_distance = aim_view_distance * 1.1f;
		if (view_distance > MAX_VIEW_DISTANCE)
		{
			view_distance = MAX_VIEW_DISTANCE;
		}
	}
	if (aim_view_distance != view_distance)
	{
		aim_view_distance = view_distance;
		need_update_view = true;
	}
}

static void onMouseMiddleMove(GLFWwindow* window, double x_mouse, double y_mouse)
{
	if (x_mouse != window_width / 2 || y_mouse != window_height / 2)
	{
		float azimuth = aim_azimuth + 0.001f * (window_width / 2 - x_mouse);
		float relative_depression = aim_relative_depression - 0.001f * (window_height / 2 - y_mouse);
		if (relative_depression > 1)
		{
			relative_depression = 1;
		}
		if (relative_depression < 0)
		{
			relative_depression = 0;
		}
		if (aim_azimuth != azimuth || aim_relative_depression != relative_depression)
		{
			aim_azimuth = azimuth;
			aim_relative_depression = relative_depression;
			need_update_view = true;
		}
		int x, y;
		glfwGetWindowPos(window, &x, &y);
		SetCursorPos(x + window_width / 2, y + window_height / 2);
	}
}

static void onMouseButton(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		if (action == GLFW_PRESS)
		{
			int x, y;
			glfwGetWindowPos(window, &x, &y);
			SetCursorPos(x + window_width / 2, y + window_height / 2);
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
			glfwSetCursorPosCallback(window, onMouseMiddleMove);
		}
		else if (action == GLFW_RELEASE)
		{
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
			glfwSetCursorPosCallback(window, nullptr);
		}
	}
}

int main(int argc, char** argv)
{
	ImmDisableIME(GetCurrentThreadId());

	int width = 2400;
	int height = 1350;

	if (!glfwInit())
	{
		return -1;
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	glfwWindowHint(GLFW_RED_BITS, 10);
	glfwWindowHint(GLFW_GREEN_BITS, 10);
	glfwWindowHint(GLFW_BLUE_BITS, 10);
	glfwWindowHint(GLFW_ALPHA_BITS, 2);

	GLFWwindow* window = glfwCreateWindow(width, height, "", nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		glfwTerminate();
		return -1;
	}

	printf("%s\n", glGetString(GL_VERSION));
	printf("%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint redBits, greenBits, blueBits;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &redBits);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &greenBits);
	glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &blueBits);
	if (redBits == greenBits && redBits == blueBits)
	{
		printf("pixel format: RGB%d\n", redBits);
	}
	else
	{
		printf("pixel format: R%dG%dB%d\n", redBits, greenBits, blueBits);
	}

	glfwSetWindowPos(window, 70, 40);
	glfwSwapInterval(0);

	glfwSetWindowSizeCallback(window, onResize);
	glfwSetKeyCallback(window, onKey);
	glfwSetMouseButtonCallback(window, onMouseButton);
	glfwSetScrollCallback(window, onMouseWheel);

	init();
	onResize(window, width, height);

	initLogic();
	std::thread logical_thread(logicalFrame);
	simulate_speed = 1000000;
	while (logical_time < 0.6 * DAY_PERIOD)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	simulate_speed = 1;

	while (!glfwWindowShouldClose(window))
	{
		drawGraphics();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	stopLogic();

	glfwTerminate();

	logical_thread.join();
	return 0;
}