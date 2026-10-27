#pragma comment(lib,"imm32.lib")
#include <Windows.h>

#include <numeric>
#include <thread>
#include <unordered_map>
#include <omp.h>

#include "glad/glad.h"
#include "glfw3.h"
#include "gtx/transform.hpp"
#include "ext/matrix_common.hpp"

#include "GPUTripleBuffer.h"
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

#ifdef _WIN32
extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
#endif

using namespace glm;

constexpr float TERRAIN_LOD_FACTOR = 2000.0f;

constexpr float MAX_CSM_RATIO = 3.6f;
GLuint shadow_day_FBO;
GLuint shadow_day_tex;
GLuint shadow_PCF_sampler;
GLuint shadow_depth_sampler;
GLuint shadow_mip_gen_FBO;
GLuint shadow_mip_gen_sampler;

constexpr int SHADOW_NIGHT_TEX_SIZE = 8192;
GLuint shadow_night_FBO;
GLuint shadow_night_tex;

constexpr float FOV_Y = pi<float>() / 4;
constexpr float MIN_VIEW_Z_NEAR = 0.6f;
constexpr float MIN_VIEW_DISTANCE = 2.0f;
constexpr float MAX_VIEW_DISTANCE = 800.0f;

GLint window_width, window_height;

float aim_azimuth = 0.3f, aim_relative_depression = 0.2f, aim_view_distance = 150.0f;
uint32_t focus_move_dir = 0;
bool show_fps = false;
bool need_update_view = true;

GLuint render_FBO;
GLuint render_texs[4];

GLuint taa_FBOs[2];
struct TAATextures
{
	GLuint color;
	GLuint bloom;
	GLuint albedo;
	GLuint depth;
	GLuint stats[4];
}taa_texs[2];

constexpr int BLOOM_BUFFER_HEIGHT = 540;
int bloom_buffer_width;
GLuint bloom_FBOs[2];
GLuint bloom_texs[2];

GLuint attri_less_VAO;
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

GPUTripleBuffer tile_light_position_map_SSBO;
GPUTripleBuffer tile_lights_SSBO;
GPUTripleBuffer tile_light_affected_map_SSBO;
GPUTripleBuffer car_lighting_SSBO;
GPUTripleBuffer highway_tile_light_shadow_triangles_SSBO;
GPUTripleBuffer bridge_tile_light_shadow_triangles_SSBO;

const mat4 CAR_LIGHT_SHADOW_PROJ = perspective(CAR_LIGHT_V_RAD, CAR_LIGHT_ASPECT, CAR_LIGHT_NEAR, CAR_LIGHT_NEAR + CAR_LIGHT_RANGE);

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
GLuint SP_TAA;
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
	SP_shadow_highway_day = linkShaderProgram(VS_shadow_highway_day, FS_shadow);
	SP_shadow_highway_night = linkShaderProgram(VS_shadow_highway_night, FS_shadow);
	SP_shadow_car_day = linkShaderProgram(VS_shadow_car_day, FS_shadow);
	SP_shadow_car_night = linkShaderProgram(VS_shadow_car_night, FS_shadow);
	glDeleteShader(VS_shadow_highway_day);
	glDeleteShader(VS_shadow_highway_night);
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

	GLuint FS_TAA = loadShader(SHADER_NAME(IDR_FS_TAA), GL_FRAGMENT_SHADER);
	SP_TAA = linkShaderProgram(VS_tex_blit, FS_TAA);
	glDeleteShader(FS_TAA);

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
	printf("%s\n", glGetString(GL_VERSION));
	printf("%s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	GLint redBits, greenBits, blueBits;
	glGetNamedFramebufferAttachmentParameteriv(0, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_RED_SIZE, &redBits);
	glGetNamedFramebufferAttachmentParameteriv(0, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_GREEN_SIZE, &greenBits);
	glGetNamedFramebufferAttachmentParameteriv(0, GL_BACK_LEFT, GL_FRAMEBUFFER_ATTACHMENT_BLUE_SIZE, &blueBits);

	if (redBits == greenBits && redBits == blueBits)
	{
		printf("pixel format: RGB%d\n", redBits);
	}
	else
	{
		printf("pixel format: R%dG%dB%d\n", redBits, greenBits, blueBits);
	}

	initShader();

	glPolygonOffset(1.0f, 1.0f);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	int UBO_offset_alignment;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UBO_offset_alignment);

	scene_UBO_offset1 = ((sizeof(CameraData) - 1) / UBO_offset_alignment + 1) * UBO_offset_alignment;
	glCreateBuffers(1, &scene_UBO);
	glNamedBufferData(scene_UBO, scene_UBO_offset1 + sizeof(SunData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, CAMERA_BUFFER_BINDING, scene_UBO, 0, sizeof(CameraData));
	glBindBufferRange(GL_UNIFORM_BUFFER, SUN_BUFFER_BINDING, scene_UBO, scene_UBO_offset1, sizeof(SunData));

	glCreateBuffers(1, &shadow_UBO);
	glNamedBufferData(shadow_UBO, sizeof(ShadowTransformData), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferBase(GL_UNIFORM_BUFFER, SHADOW_TRANSFORM_BUFFER_BINDING, shadow_UBO);

	tile_light_position_map_SSBO.create(sizeof(TileLightPositionMapData));
	tile_light_position_map_SSBO.bindBase(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_POSITION_MAP_BUFFER_BINDING);
	tile_lights_SSBO.create(sizeof(TileLightData));
	tile_lights_SSBO.bindBase(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_BUFFER_BINDING);
	tile_light_affected_map_SSBO.create(sizeof(TileLightAffectedMapData));
	tile_light_affected_map_SSBO.bindBase(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_AFFECTED_MAP_BUFFER_BINDING);
	car_lighting_SSBO.create(sizeof(CarLightingData));
	car_lighting_SSBO.bindBase(GL_SHADER_STORAGE_BUFFER, CAR_LIGHTING_BUFFER_BINDING);
	constexpr GLsizeiptr tile_light_shadow_triangles_SSBO_size = sizeof(TileLightShadowTriangleData) + tileLightShadowLayerOffset(NUM_TILE_LIGHT_SHADOW_LAYERS) * MAX_AVERAGE_TRIANGLES_PER_LIGHT * sizeof(TileLightShadowTriangle);
	highway_tile_light_shadow_triangles_SSBO.create(tile_light_shadow_triangles_SSBO_size);
	bridge_tile_light_shadow_triangles_SSBO.create(tile_light_shadow_triangles_SSBO_size);

	glCreateFramebuffers(1, &render_FBO);
	glCreateFramebuffers(2, taa_FBOs);
	GLuint attachments[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6 };
	glNamedFramebufferDrawBuffers(taa_FBOs[0], sizeof(attachments) / sizeof(GLuint), attachments);
	glNamedFramebufferDrawBuffers(taa_FBOs[1], sizeof(attachments) / sizeof(GLuint), attachments);

	glCreateFramebuffers(2, bloom_FBOs);

	vec2 screen_coords[4] = { { 1.0f, 1.0f },{ -1.0f, 1.0f }, { -1.0f, -1.0f }, { 1.0f, -1.0f } };
	glCreateBuffers(1, &tex_mapping_VBO);
	glNamedBufferData(tex_mapping_VBO, sizeof(screen_coords), screen_coords, GL_STATIC_DRAW);
	glGenVertexArrays(1, &tex_blit_VAO);
	glBindVertexArray(tex_blit_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, tex_mapping_VBO);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glBindVertexArray(0);

	glCreateFramebuffers(1, &shadow_day_FBO);
	glNamedFramebufferDrawBuffer(shadow_day_FBO, GL_NONE);
	glNamedFramebufferReadBuffer(shadow_day_FBO, GL_NONE);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &shadow_day_tex);
	glTextureParameteri(shadow_day_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(shadow_day_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(shadow_day_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(shadow_day_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTextureParameteri(shadow_day_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(shadow_day_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage3D(shadow_day_tex, 1 + PCSS_MIP_LEVELS, GL_DEPTH_COMPONENT24, SHADOW_DAY_TEX_SIZE, SHADOW_DAY_TEX_SIZE, CSM_LEVELS);

	glCreateFramebuffers(1, &shadow_mip_gen_FBO);
	glNamedFramebufferDrawBuffer(shadow_mip_gen_FBO, GL_NONE);
	glNamedFramebufferReadBuffer(shadow_mip_gen_FBO, GL_NONE);

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

	glCreateFramebuffers(1, &shadow_night_FBO);
	glNamedFramebufferDrawBuffer(shadow_night_FBO, GL_NONE);
	glNamedFramebufferReadBuffer(shadow_night_FBO, GL_NONE);
	glCreateTextures(GL_TEXTURE_2D_ARRAY, 1, &shadow_night_tex);
	glTextureParameteri(shadow_night_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTextureParameteri(shadow_night_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(shadow_night_tex, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTextureParameteri(shadow_night_tex, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTextureParameteri(shadow_night_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTextureParameteri(shadow_night_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTextureStorage3D(shadow_night_tex, 1, GL_DEPTH_COMPONENT24, SHADOW_NIGHT_TEX_SIZE, SHADOW_NIGHT_TEX_SIZE, NUM_TILE_LIGHT_SHADOW_LAYERS);

	glCreateTextures(GL_TEXTURE_2D, 1, &text_atlas_tex);
	HRSRC rc_info = FindResource(nullptr, MAKEINTRESOURCE(IDR_TEXT_ATLAS), L"TEXTURE");
	if (rc_info != nullptr)
	{
		HGLOBAL rc_data = LoadResource(nullptr, rc_info);
		if (rc_data != nullptr)
		{
			const char* data = (const char*)(LockResource(rc_data));
			glTextureStorage2D(text_atlas_tex, 1, GL_R8, TEXT_WIDTH * TEXT_ALTAS_CNT_X, TEXT_HEIGHT * TEXT_ALTAS_CNT_Y);
			glTextureSubImage2D(text_atlas_tex, 0, 0, 0, TEXT_WIDTH * TEXT_ALTAS_CNT_X, TEXT_HEIGHT * TEXT_ALTAS_CNT_Y, GL_RED, GL_UNSIGNED_BYTE, data);
			glTextureParameteri(text_atlas_tex, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(text_atlas_tex, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(text_atlas_tex, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTextureParameteri(text_atlas_tex, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

	glGenVertexArrays(1, &attri_less_VAO);

	buildMeshes();
	initScene();
}

const vec2 TAA_OFFSETS[] =
{
	vec2(1.0f / 2.0f, 1.0f / 3.0f),
	vec2(1.0f / 4.0f, 2.0f / 3.0f),
	vec2(3.0f / 4.0f, 1.0f / 9.0f),
	vec2(1.0f / 8.0f, 4.0f / 9.0f),
	vec2(5.0f / 8.0f, 7.0f / 9.0f),
	vec2(3.0f / 8.0f, 2.0f / 9.0f),
	vec2(7.0f / 8.0f, 5.0f / 9.0f),
	vec2(1.0f / 16.0f, 8.0f / 9.0f),
};

static void drawGraphics()
{
	static float azimuth = aim_azimuth, relative_depression = aim_relative_depression, view_distance = aim_view_distance;
	static float scene_z_far;
	static vec3 focus(0);
	static float horizon_y;

	static CameraData camera;
	static ShadowTransformData sun_shadow;
	static TileLightData tile_light_data;
	static TileLightPositionMapData tile_light_position_map;
	static TileLightAffectedMapData tile_light_affected_map;
	static CarLightingData car_lightings;
	static vec3 visible_car_colors[MAX_CAR_CNT];
	static mat4 visible_car_transforms[MAX_CAR_CNT];
	static mat4 visible_car_old_transforms[MAX_CAR_CNT];
	static std::unordered_map<uint32_t, std::pair<bool, mat4>> car_old_transforms;
	static std::vector<mat4> car_sun_shadow_transform[CSM_LEVELS];
	static std::vector<mat4> car_tile_light_shadow_transform[NUM_TILE_LIGHT_SHADOW_LAYERS];
	static std::vector<int> car_tile_light_shadow_idx[NUM_TILE_LIGHT_SHADOW_LAYERS];

	static uint64_t frame_idx = 0;
	static uint64_t last_time_us = 0;
	static float fps = -1.0f;
	uint64_t time_us = getTimestampMicroseconds();
	uint64_t dt_us = last_time_us > 0 ? time_us - last_time_us : 0;
	if (dt_us > 0)
	{
		fps = fps > 0.0f ? (fps + 1.0f) / (1.0f + dt_us * 1e-6f) : 1e6f / dt_us;
	}
	last_time_us = time_us;

	if (focus_move_dir != 0)
	{
		vec2 dir = vec2{ 0.0f };
		if ((focus_move_dir & 0b0011) == 0b0001)
		{
			dir += vec2{ -cos(azimuth), -sin(azimuth) };
		}
		if ((focus_move_dir & 0b0011) == 0b0010)
		{
			dir += vec2{ cos(azimuth), sin(azimuth) };
		}
		if ((focus_move_dir & 0b1100) == 0b0100)
		{
			dir += vec2{ -sin(azimuth), cos(azimuth) };
		}
		if ((focus_move_dir & 0b1100) == 0b1000)
		{
			dir += vec2{ sin(azimuth), -cos(azimuth) };
		}
		if (dir != vec2{ 0.0f })
		{
			dir = normalize(dir);
			float move_speed = 1.0f * view_distance + 10.0f;
			float move_distance = move_speed * 1e-6f * dt_us;
			vec3 new_focus = focus + vec3{ dir.x * move_distance, dir.y * move_distance, 0.0f };
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
	camera.old_z_near = camera.z_near;
	if (!need_update_view)
	{
		camera.delta_view_z = { 0.0f, 0.0f, 1.0f, 0.0f };
		camera.inv_delta_view = identity<mat4>();
	}
	else
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
		vec2 height_map_coord{ (eye.x - SCENE_GRID_AREA.x) / (SCENE_GRID_AREA.z - SCENE_GRID_AREA.x) * SCENE_GRID_SIZE_X - 0.5f,
			(eye.y - SCENE_GRID_AREA.y) / (SCENE_GRID_AREA.w - SCENE_GRID_AREA.y) * SCENE_GRID_SIZE_Y - 0.5f };
		if (0 < height_map_coord.x && height_map_coord.x < SCENE_GRID_SIZE_X - 1 && 0 < height_map_coord.y && height_map_coord.y < SCENE_GRID_SIZE_Y - 1)
		{
			auto height_map = g_scene_quad_tree[MAX_SCENE_GRID_LEVEL];
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
		mat4 view = lookAt(eye, focus, vec3(-sin(azimuth), cos(azimuth), 0));
		camera.inv_view = inverse(view);
		camera.inv_delta_view = camera.view * camera.inv_view;
		camera.delta_view_z = vec4{ vec3{ camera.inv_delta_view[2] }, -dot(vec3{camera.inv_delta_view[2]}, vec3{camera.inv_delta_view[3]}) };
		camera.view = view;
		horizon_y = (tan(depression - acos(EARTH_RADIUS / (focus.z + EARTH_RADIUS))) / tan(FOV_Y / 2) + 1) * window_height / 2;

		updateTerrainLOD(TERRAIN_LOD_FACTOR, eye);

		Frustum camera_frustum(camera.view, FOV_Y, float(window_width) / window_height, MIN_VIEW_Z_NEAR, VIEW_Z_FAR);
		static CircularQueue<QuadTreeIdx> grids_to_calc(5);
		camera.z_near = VIEW_Z_FAR;
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
			vec3 abs_noraml = abs(p_near.normal);
			float r = dot(half_size, abs_noraml);
			float m_near = dot(p_near.normal, bound.center()) + p_near.d;
			float m_far = dot(p_far.normal, bound.center()) + p_far.d;
			float z_near = MIN_VIEW_Z_NEAR + m_near - r;
			float z_far = VIEW_Z_FAR - m_far + r;
			if (z_near >= camera.z_near && z_far <= scene_z_far)
			{
				continue;
			}
			INTERSECTION_TEST_RESULT result = camera_frustum.intersectionTestNoNearFar(bound);
			if (result == INTERSECTION_TEST_OUTSIDE)
			{
				continue;
			}
			if (level == MAX_SCENE_GRID_LEVEL)
			{
				camera.z_near = std::min(camera.z_near, z_near);
				scene_z_far = std::max(scene_z_far, z_far);
				continue;
			}
			if (result == INTERSECTION_TEST_INSIDE)
			{
				vec3 v = abs_noraml * half_size;
				float r_separate = std::max(v.x, std::max(v.y, v.z)) / (v.x + v.y + v.z);
				r_separate = 2.0f * r * (1.0f - r_separate);
				camera.z_near = std::min(camera.z_near, z_near + r_separate);
				scene_z_far = std::max(scene_z_far, z_far - r_separate);
			}
			grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y);
			grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y);
			grids_to_calc.emplace_back(level + 1, 2 * x, 2 * y + 1);
			grids_to_calc.emplace_back(level + 1, 2 * x + 1, 2 * y + 1);
		}
		camera.z_near = std::max(camera.z_near, MIN_VIEW_Z_NEAR);
		scene_z_far = std::min(scene_z_far, VIEW_Z_FAR);
	}

	const LogicalData& logical_data = getLatestLogicalData();
	SunData sun;
	sun.light_dir_and_radius = vec4{ logical_data.sun_dir, 1.0f };
	sun.diffuse_specular = vec3{ 0.4f };
	if (-SUN_RADIUS_DIST_RATIO < logical_data.sun_dir.z && logical_data.sun_dir.z < SUN_RADIUS_DIST_RATIO)
	{
		float x = -logical_data.sun_dir.z / SUN_RADIUS_DIST_RATIO;
		float sqrt_one_minus_x2 = sqrt(1 - x * x);
		float area_unit = 0.5f * pi<float>() - (sqrt_one_minus_x2 * x + asin(x));
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
		sun.light_dir_and_radius = vec4{ normalize(vec3{ sun.light_dir_and_radius }), sqrt(area_percent) };
	}
	constexpr float ATMOSPHERE = EARTH_RADIUS + 2e4f;
	float absorb_factor = -1e-5f * (-EARTH_RADIUS * sun.light_dir_and_radius.z + sqrt(ATMOSPHERE * ATMOSPHERE - EARTH_RADIUS * EARTH_RADIUS * (1 - sun.light_dir_and_radius.z * sun.light_dir_and_radius.z)));
	sun.diffuse_specular *= vec3{ exp(0.2f * absorb_factor), exp(0.3f * absorb_factor), exp(1.1f * absorb_factor) };
	sun.ambient = vec3(0.01f);
	sun.sky_color = vec4(0.01f, 0.015f, 0.055f, 1.0f);
	if (sun.light_dir_and_radius.z > -0.2f)
	{
		sun.ambient += (sun.light_dir_and_radius.z + 0.2f) * 0.2f;
		sun.sky_color += (sun.light_dir_and_radius.z + 0.2f) * vec4 { 0.2f, 0.3f, 1.1f, 0.0f };
	}

	camera.taa_phase = frame_idx % TAA_PERIOD;
	camera.taa_offset = (2.0f * TAA_OFFSETS[camera.taa_phase] - 1.0f) / vec2{ window_width, window_height };
	camera.projection = perspective(FOV_Y, float(window_width) / window_height, camera.z_near, VIEW_Z_FAR);
	camera.view_proj = camera.projection * camera.view;
	Frustum camera_frustum{ camera.view_proj };

	int num_visible_cars = 0;
	int num_visible_light_on_cars;
	int num_visible_car_lights;
	int total_tile_light_affected_map_indices_cnt;

	auto add_visible_car = [&num_visible_cars, &logical_data](int i)
		{
			auto [old_transform, is_new] = car_old_transforms.try_emplace(logical_data.car_ids[i], true, logical_data.car_transform[i]);
			visible_car_old_transforms[num_visible_cars] = old_transform->second.second;
			if (!is_new)
			{
				old_transform->second.first = true;
				old_transform->second.second = logical_data.car_transform[i];
			}
			visible_car_colors[num_visible_cars] = logical_data.car_color[i];
			visible_car_transforms[num_visible_cars] = logical_data.car_transform[i];
			num_visible_cars++;
		};

	if (sun.light_dir_and_radius.z > 0)
	{
		mat4 sun_shadow_view = lookAt(vec3{ 0.0f }, -vec3{ sun.light_dir_and_radius }, vec3{ -sun.light_dir_and_radius.x, -sun.light_dir_and_radius.y, sun.light_dir_and_radius.z });
		mat3 abs_sun_shadow_rot = abs(mat3{ sun_shadow_view });
		vec3 view_top(0.0f, tan(FOV_Y / 2), 0.0f);
		vec3 view_left = vec3{ view_top.y / window_height * window_width, 0.0f, 0.0f };
		view_top = mat3{ camera.inv_view } *view_top;
		view_left = mat3{ camera.inv_view } *view_left;
		float z_min = FLT_MAX;
		float x_maxs[CSM_LEVELS], x_mins[CSM_LEVELS], y_maxs[CSM_LEVELS], y_mins[CSM_LEVELS], z_mins[CSM_LEVELS];
		float CSM_ratio = pow(scene_z_far / camera.z_near, 1.0f / CSM_LEVELS);
		float camera_z_far = scene_z_far;
		for (int i = CSM_LEVELS - 1; i >= 0; i--)
		{
			x_mins[i] = FLT_MAX, x_maxs[i] = -FLT_MAX, y_mins[i] = FLT_MAX, y_maxs[i] = -FLT_MAX;
			static CircularQueue<QuadTreeIdx> grids_to_calc(5);
			float camera_z_near = camera_z_far / CSM_ratio;
			Frustum camera_level_frustum{ camera.view, FOV_Y, float(window_width) / window_height, camera_z_near, camera_z_far };
			BoundFrustum camera_level_bound{ camera.inv_view, FOV_Y, float(window_width) / window_height, camera_z_near, camera_z_far };
			float frustum_x_min = FLT_MAX, frustum_x_max = -FLT_MAX, frustum_y_min = FLT_MAX, frustum_y_max = -FLT_MAX, frustum_z_far = FLT_MAX;
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
				if (level == MAX_SCENE_GRID_LEVEL)
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
					vec3 v_x{ abs_sun_shadow_rot[0][0] * half_size.x, abs_sun_shadow_rot[1][0] * half_size.x, abs_sun_shadow_rot[2][0] * half_size.z };
					vec3 v_y{ abs_sun_shadow_rot[0][1] * half_size.y, abs_sun_shadow_rot[1][1] * half_size.y, abs_sun_shadow_rot[2][1] * half_size.z };
					float r_separate_x = std::max(v_x.x, std::max(v_x.y, v_x.z)) / (v_x.x + v_x.y + v_x.z);
					float r_separate_y = std::max(v_y.x, std::max(v_y.y, v_y.z)) / (v_y.x + v_y.y + v_y.z);
					r_separate_x = (1.0f - 2.0f * r_separate_x) * r.x;
					r_separate_y = (1.0f - 2.0f * r_separate_y) * r.y;
					x_mins[i] = std::min(x_mins[i], center.x + r_separate_x);
					x_maxs[i] = std::max(x_maxs[i], center.x - r_separate_x);
					y_mins[i] = std::min(y_mins[i], center.y + r_separate_y);
					y_maxs[i] = std::max(y_maxs[i], center.y - r_separate_y);
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
			float group_x_min = FLT_MAX, group_x_max = -FLT_MAX, group_y_min = FLT_MAX, group_y_max = -FLT_MAX, group_z_min = FLT_MAX;
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
			float z_max = -FLT_MAX;
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
				if (result == INTERSECTION_TEST_INSIDE || level == MAX_SCENE_GRID_LEVEL)
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
			sun_shadow.tex[i] = mat4{
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f
			} *shadow_mat;
		}
		Frustum shadow_frustums[CSM_LEVELS];
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			shadow_frustums[i] = Frustum{ sun_shadow.view_proj[i] };
			car_sun_shadow_transform[i].clear();
		}
		int num_invisible‌_cars = 0;
		for (int i = 0; i < logical_data.num_cars; i++)
		{
			BoundAABB car_bound{ car_local_bound, logical_data.car_transform[i] };
			for (int j = 0; j < CSM_LEVELS; j++)
			{
				if (shadow_frustums[j].cullingTestNoNearFar(car_bound))
				{
					car_sun_shadow_transform[j].push_back(logical_data.car_transform[i]);
				}
			}
			if (camera_frustum.cullingTest(car_bound))
			{
				add_visible_car(i);
			}
		}
	}
	else
	{
		num_visible_light_on_cars = 0;
		num_visible_car_lights = 0;
		memset(&tile_light_position_map, 0, sizeof(tile_light_position_map));
		struct CarLightInfo
		{
			vec4 pos;
			vec4 dir;
			BoundCarLight bound;
			float distance_to_camera;
			ivec2* light_map_grid;
		};
		static std::vector<CarLightInfo>car_light_infos;
		static BoundCarLight tile_light_bounds[2 * MAX_CAR_CNT];
		car_light_infos.clear();
		int num_car_lights = 2 * logical_data.num_light_on_cars;
		int cnt = 0;
		BoundFrustum camera_bound{ camera.inv_view, FOV_Y, float(window_width) / window_height, camera.z_near, VIEW_Z_FAR };
		for (int i = 0; i < num_car_lights; i++)
		{
			const vec4& light_pos = logical_data.car_light_pos[i];
			const vec4& light_dir = logical_data.car_light_dir[i];
			vec4 light_x = normalize(vec4{ light_dir.y, -light_dir.x, 0.0f, 0.0f });
			vec4 light_y = vec4{ cross(vec3{ light_x }, vec3{ light_dir }), 0.0f };
			BoundCarLight bound{ mat4{ light_x, light_y, -light_dir, light_pos } };
			if (camera_frustum.cullingTest(bound))
			{
				constexpr float POS_OFFSET = CAR_LIGHT_NEAR + TILE_LIGHT_POSITION_MAP_GRID_LENGTH;
				vec3 center_pos = light_pos + POS_OFFSET * light_dir;
				ivec2 light_pos_map_idx = 1.0f / TILE_LIGHT_POSITION_MAP_GRID_LENGTH * vec2{ center_pos } + 0.5f * vec2{ TILE_LIGHT_POSITION_MAP_SIZE_X, TILE_LIGHT_POSITION_MAP_SIZE_Y };
				ivec2* light_pos_map_grid = &tile_light_position_map.light_idx_range[light_pos_map_idx.x * TILE_LIGHT_POSITION_MAP_SIZE_Y + light_pos_map_idx.y];
				car_light_infos.push_back({ light_pos, light_dir, bound, length(vec3{ light_pos + CAR_LIGHT_NEAR * light_dir - camera.inv_view[3] }), light_pos_map_grid });
				light_pos_map_grid->x++;
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
			tile_light_data.tile_lights[idx].position = car_light_info.pos;
			tile_light_data.tile_lights[idx].view_proj = CAR_LIGHT_SHADOW_PROJ * lookAt(vec3{ car_light_info.pos }, vec3{ car_light_info.pos + car_light_info.dir }, vec3{ 0.0f, 0.0f, 1.0f });
			tile_light_bounds[idx] = car_light_info.bound;
		}
		constexpr int SEARCH_RANGE = int(0.9999f + 0.5f * (CAR_LIGHT_RANGE / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH));
		static std::pair<int, int[(2 * SEARCH_RANGE + 1) * (2 * SEARCH_RANGE + 1)]> affected_grids[2 * MAX_CAR_CNT];
#pragma omp parallel num_threads(omp_get_num_procs() - 2)
		{
#pragma omp for nowait
			for (int i = 0; i < num_visible_car_lights; i++)
			{
				affected_grids[i].first = 0;
				const vec2 center_pos = 0.5f * (tile_light_bounds[i].center_near + tile_light_bounds[i].center_far);
				const ivec2 affected_map_idx = 1.0f / TILE_LIGHT_AFFECTED_MAP_GRID_LENGTH * center_pos + 0.5f * vec2{ TILE_LIGHT_AFFECTED_MAP_SIZE_X, TILE_LIGHT_AFFECTED_MAP_SIZE_Y };
				const ivec2 begin_idx = max(ivec2(0), affected_map_idx - SEARCH_RANGE);
				const ivec2 end_idx = min(ivec2{ TILE_LIGHT_AFFECTED_MAP_SIZE_X, TILE_LIGHT_AFFECTED_MAP_SIZE_Y }, affected_map_idx + SEARCH_RANGE + 1);
				BoundAABB bounds[(2 * SEARCH_RANGE + 1) * (2 * SEARCH_RANGE + 1)];
				int count = 0;
				for (int j = begin_idx.x; j < end_idx.x; j++)
				{
					for (int k = begin_idx.y; k < end_idx.y; k++)
					{
						bounds[count++] = sceneGridBound(TILE_LIGHT_AFFECTED_MAP_GRID_LEVEL, j, k);
					}
				}
				bool intersect_results[(2 * SEARCH_RANGE + 1) * (2 * SEARCH_RANGE + 1)];
				collisionTests(tile_light_bounds[i], bounds, count, intersect_results);
				count = 0;
				for (int j = begin_idx.x; j < end_idx.x; j++)
				{
					for (int k = begin_idx.y; k < end_idx.y; k++)
					{
						if (intersect_results[count++])
						{
							affected_grids[i].second[affected_grids[i].first++] = j * TILE_LIGHT_AFFECTED_MAP_SIZE_Y + k;
						}
					}
				}
			}
#pragma omp for schedule(guided, 4)
			for (int i = 0; i < logical_data.num_cars; i++)
			{
				const mat4& transform = logical_data.car_transform[i];
				ivec2 pos_idx = 1.0f / TILE_LIGHT_POSITION_MAP_GRID_LENGTH * vec2(transform[3]) + 0.5f * vec2{ TILE_LIGHT_POSITION_MAP_SIZE_X, TILE_LIGHT_POSITION_MAP_SIZE_Y };
				BoundOBB car_bound{ car_local_bound, transform };
				int num_lighting = 0;
				for (int m = -1; m < 2; m++)
				{
					for (int n = -1; n < 2; n++)
					{
						ivec2 idx = pos_idx + ivec2{ m, n };
						ivec2 light_idx_range = tile_light_position_map.light_idx_range[idx.x * TILE_LIGHT_POSITION_MAP_SIZE_Y + idx.y];
						for (int k = light_idx_range.x; k < light_idx_range.y; k++)
						{
							if (collisionTest(tile_light_bounds[k], car_bound))
							{
								num_lighting++;
								car_lightings.light_indices[i * LIGHTING_SIZE_PER_CAR + num_lighting] = k;
							}
						}
					}
				}
				car_lightings.light_indices[i * LIGHTING_SIZE_PER_CAR] = num_lighting > 0 || camera_frustum.cullingTest(car_bound) ? num_lighting : -1;
			}
		}
		memset(tile_light_affected_map.idx_range, 0, sizeof(tile_light_affected_map.idx_range));
		for (int i = 0; i < num_visible_car_lights; i++)
		{
			for (int j = 0; j < affected_grids[i].first; j++)
			{
				tile_light_affected_map.idx_range[affected_grids[i].second[j]].x++;
			}
		}
		total_tile_light_affected_map_indices_cnt = 0;
		for (int i = 0; i < num_visible_car_lights; i++)
		{
			for (int j = 0; j < affected_grids[i].first; j++)
			{
				int idx = affected_grids[i].second[j];
				if (tile_light_affected_map.idx_range[idx].y == 0)
				{
					tile_light_affected_map.idx_range[idx].y = total_tile_light_affected_map_indices_cnt;
					total_tile_light_affected_map_indices_cnt += tile_light_affected_map.idx_range[idx].x;
					tile_light_affected_map.idx_range[idx].x = tile_light_affected_map.idx_range[idx].y;
				}
				tile_light_affected_map.light_indices[tile_light_affected_map.idx_range[idx].y++] = i;
			}
		}
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			car_tile_light_shadow_idx[i].clear();
			car_tile_light_shadow_transform[i].clear();
		}
		for (int i = 0; i < logical_data.num_cars; i++)
		{
			if (car_lightings.light_indices[i * LIGHTING_SIZE_PER_CAR] < 0)
			{
				continue;
			}
			car_lightings.light_indices[num_visible_cars * LIGHTING_SIZE_PER_CAR] = car_lightings.light_indices[i * LIGHTING_SIZE_PER_CAR];
			for (int j = 1; j <= car_lightings.light_indices[i * LIGHTING_SIZE_PER_CAR]; j++)
			{
				int k = car_lightings.light_indices[i * LIGHTING_SIZE_PER_CAR + j];
				car_lightings.light_indices[num_visible_cars * LIGHTING_SIZE_PER_CAR + j] = k;
				int layer = findTileLightShadowLayer(k);
				if (layer < NUM_TILE_LIGHT_SHADOW_LAYERS)
				{
					car_tile_light_shadow_idx[layer].push_back(k);
					car_tile_light_shadow_transform[layer].push_back(tile_light_data.tile_lights[k].view_proj * logical_data.car_transform[i]);
				}
			}
			add_visible_car(i);
			if (i < logical_data.num_light_on_cars)
			{
				num_visible_light_on_cars++;
			}
		}
	}
	for (auto iter = car_old_transforms.begin(); iter != car_old_transforms.end();)
	{
		if (!iter->second.first)
		{
			iter = car_old_transforms.erase(iter);
		}
		else
		{
			iter->second.first = false;
			++iter;
		}
	}

	if (is_view_updated)
	{
		glProgramUniform1f(SP_sun, glGetUniformLocation(SP_sun, "horizonY"), horizon_y);
	}
	glNamedBufferSubData(scene_UBO, 0, sizeof(camera), &camera);
	glNamedBufferSubData(scene_UBO, scene_UBO_offset1, sizeof(SunData), &sun);
	car_color_VBO.updateData(0, num_visible_cars * sizeof(vec3), visible_car_colors);
	car_transform_VBO.updateData(0, num_visible_cars * sizeof(mat4), visible_car_transforms);
	car_old_transform_VBO.updateData(0, num_visible_cars * sizeof(mat4), visible_car_old_transforms);
	if (sun.light_dir_and_radius.z > 0)
	{
		glNamedBufferSubData(shadow_UBO, 0, sizeof(sun_shadow), &sun_shadow);
		int car_sun_shadow_transform_offset = 0;
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			int count = car_sun_shadow_transform[i].size();
			car_shadow_transform_VBO.updateData(car_sun_shadow_transform_offset * sizeof(mat4), count * sizeof(mat4), car_sun_shadow_transform[i].data());
			car_sun_shadow_transform_offset += count;
		}
	}
	else
	{
		tile_light_position_map_SSBO.updateData(0, sizeof(tile_light_position_map), &tile_light_position_map);
		tile_lights_SSBO.updateData(0, num_visible_car_lights * sizeof(TileLight), &tile_light_data);
		tile_light_affected_map_SSBO.updateData(0, sizeof(TileLightAffectedMapData::idx_range) + total_tile_light_affected_map_indices_cnt * sizeof(int), &tile_light_affected_map);
		car_lighting_SSBO.updateData(0, num_visible_cars * LIGHTING_SIZE_PER_CAR * sizeof(int), &car_lightings);
		glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "numLightOnCars"), num_visible_light_on_cars);
		int car_tile_light_shadow_offset = 0;
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			int count = car_tile_light_shadow_transform[i].size();
			car_shadow_transform_VBO.updateData(car_tile_light_shadow_offset * sizeof(mat4), count * sizeof(mat4), car_tile_light_shadow_transform[i].data());
			car_tile_light_shadow_idx_VBO.updateData(car_tile_light_shadow_offset * sizeof(int), count * sizeof(int), car_tile_light_shadow_idx[i].data());
			car_tile_light_shadow_offset += count;
		}
	}
	if (sun.light_dir_and_radius.z > -0.2f)
	{
		static mat4 transform = identity<mat4>();
		glProgramUniformMatrix4fv(SP_sun, glGetUniformLocation(SP_sun, "old_transform"), 1, GL_FALSE, (GLfloat*)&transform);
		transform = rotate(acos(sun.light_dir_and_radius.z), vec3(-sun.light_dir_and_radius.y, sun.light_dir_and_radius.x, 0));
		transform[3] = vec4(0.99f * VIEW_Z_FAR * logical_data.sun_dir, 0.0f) + camera.inv_view[3];
		glProgramUniformMatrix4fv(SP_sun, glGetUniformLocation(SP_sun, "transform"), 1, GL_FALSE, (GLfloat*)&transform);
	}

	glBindTextureUnit(0, highway_tex);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glNamedFramebufferTexture(render_FBO, GL_DEPTH_ATTACHMENT, taa_texs[frame_idx & 1].depth, 0);
	float depth_value = 1.0f;
	glClearNamedFramebufferfv(render_FBO, GL_DEPTH, 0, &depth_value);
	glClearNamedFramebufferfv(render_FBO, GL_COLOR, RENDER_TARGET_COLOR, &sun.sky_color.r);
	glClearNamedFramebufferfv(render_FBO, GL_COLOR, RENDER_TARGET_BLOOM, COLOR_BLACK);
	glClearNamedFramebufferfv(render_FBO, GL_COLOR, RENDER_TARGET_DELTA_POSITION, ZERO_VEC);
	glClearNamedFramebufferfv(render_FBO, GL_COLOR, RENDER_TARGET_ALBEDO, COLOR_BLACK);
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
		glEnable(GL_POLYGON_OFFSET_FILL);
		int car_sun_shadow_offset = 0;
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, 0, i);
			glUseProgram(SP_shadow_highway_day);
			glProgramUniform1i(SP_shadow_highway_day, glGetUniformLocation(SP_shadow_highway_day, "csm_level"), i);
			glDisable(GL_CULL_FACE);
			Frustum shadow_frustum(sun_shadow.view_proj[i]);
			drawTerrainMesh(shadow_frustum);
			glEnable(GL_CULL_FACE);
			glBindVertexArray(highway_VAO);
			glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
			glBindVertexArray(bridge_VAO);
			glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
			glUseProgram(SP_shadow_car_day);
			car_shadow_day_VAO.bindCurrent();
			glProgramUniform1i(SP_shadow_car_day, glGetUniformLocation(SP_shadow_car_day, "csm_level"), i);
			int count = car_sun_shadow_transform[i].size();
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, CAR_SHADOW_EBO_SIZE, GL_UNSIGNED_INT, 0, count, car_sun_shadow_offset);
			car_sun_shadow_offset += count;
		}
		glDisable(GL_POLYGON_OFFSET_FILL);

		glUseProgram(SP_gen_PCSS_mips);
		glBindVertexArray(tex_blit_VAO);
		glDepthFunc(GL_ALWAYS);
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_mip_gen_FBO);
		for (int i = 0; i < PCSS_MIP_LEVELS; i++)
		{
			glProgramUniform1i(SP_gen_PCSS_mips, glGetUniformLocation(SP_gen_PCSS_mips, "mip_level"), i);
			glViewport(0, 0, SHADOW_DAY_TEX_SIZE >> (i + 1), SHADOW_DAY_TEX_SIZE >> (i + 1));
			for (int j = 0; j < CSM_LEVELS; j++)
			{
				glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_day_tex, i + 1, j);
				glProgramUniform1i(SP_gen_PCSS_mips, glGetUniformLocation(SP_gen_PCSS_mips, "layer"), j);
				glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
			}
		}
		glDepthFunc(GL_LESS);

		glBindFramebuffer(GL_FRAMEBUFFER, render_FBO);
		glViewport(0, 0, window_width, window_height);
		GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);
		glUseProgram(SP_car_day);
		car_VAO.bindCurrent();
		glDrawElementsInstanced(GL_TRIANGLES, CAR_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
		glUseProgram(SP_highway_day);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glBindVertexArray(highway_VAO);
		glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_terrain_day);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		drawTerrainMesh(camera_frustum);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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

		DrawArraysIndirectCommand commands[NUM_TILE_LIGHT_SHADOW_LAYERS]{};
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			commands[i].instance_count = 1;
			commands[i].first = 3 * MAX_AVERAGE_TRIANGLES_PER_LIGHT * tileLightShadowLayerOffset(i);
		}
		glUseProgram(CSP_shadow_highway_night);
		glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
		highway_tile_light_shadow_triangles_SSBO.updateData(0, sizeof(TileLightShadowTriangleData::commands), commands);
		bridge_tile_light_shadow_triangles_SSBO.updateData(0, sizeof(TileLightShadowTriangleData::commands), commands);

		highway_tile_light_shadow_triangles_SSBO.bindCurrent(GL_DRAW_INDIRECT_BUFFER);
		highway_tile_light_shadow_triangles_SSBO.bindBaseCurrent(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, VERTEX_BUFFER_BINDING, highway_VBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INDEX_BUFFER_BINDING, highway_EBO);
		glProgramUniform1i(CSP_shadow_highway_night, glGetUniformLocation(CSP_shadow_highway_night, "num_triangles"), HIGHWAY_EBO_SIZE / 3);
		glDispatchCompute((HIGHWAY_EBO_SIZE / 3 + 63) / 64, 1, 1);

		bridge_tile_light_shadow_triangles_SSBO.bindCurrent(GL_DRAW_INDIRECT_BUFFER);
		bridge_tile_light_shadow_triangles_SSBO.bindBaseCurrent(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, VERTEX_BUFFER_BINDING, bridge_VBO);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, INDEX_BUFFER_BINDING, bridge_EBO);
		glProgramUniform1i(CSP_shadow_highway_night, glGetUniformLocation(CSP_shadow_highway_night, "num_triangles"), BRIDGE_EBO_SIZE / 3);
		glDispatchCompute((BRIDGE_EBO_SIZE / 3 + 63) / 64, 1, 1);

		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
		int car_tile_light_shadow_offset = 0;
		for (int i = 0; i < NUM_TILE_LIGHT_SHADOW_LAYERS; i++)
		{
			glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_night_tex, 0, i);

			glUseProgram(SP_shadow_highway_night);
			glBindVertexArray(attri_less_VAO);
			highway_tile_light_shadow_triangles_SSBO.bindCurrent(GL_DRAW_INDIRECT_BUFFER);
			highway_tile_light_shadow_triangles_SSBO.bindBaseCurrent(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, VERTEX_BUFFER_BINDING, highway_VBO);
			glDrawArraysIndirect(GL_TRIANGLES, reinterpret_cast<void*>(i * sizeof(DrawArraysIndirectCommand)));
			bridge_tile_light_shadow_triangles_SSBO.bindCurrent(GL_DRAW_INDIRECT_BUFFER);
			bridge_tile_light_shadow_triangles_SSBO.bindBaseCurrent(GL_SHADER_STORAGE_BUFFER, TILE_LIGHT_SHADOW_TRIANGLE_BUFFER_BINDING);
			glBindBufferBase(GL_SHADER_STORAGE_BUFFER, VERTEX_BUFFER_BINDING, bridge_VBO);
			glDrawArraysIndirect(GL_TRIANGLES, reinterpret_cast<void*>(i * sizeof(DrawArraysIndirectCommand)));
			glUseProgram(SP_shadow_car_night);
			car_shadow_night_VAO.bindCurrent();
			int count = car_tile_light_shadow_transform[i].size();
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, CAR_SHADOW_EBO_SIZE, GL_UNSIGNED_INT, 0, count, car_tile_light_shadow_offset);
			car_tile_light_shadow_offset += count;
		}
		for (int i = 0; i < 4; i++)
		{
			glDisable(GL_CLIP_DISTANCE0 + i);
		}
		glDisable(GL_POLYGON_OFFSET_FILL);

		glBindFramebuffer(GL_FRAMEBUFFER, render_FBO);
		glViewport(0, 0, window_width, window_height);
		GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_NONE, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);
		glUseProgram(SP_highway_night);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glBindVertexArray(highway_VAO);
		glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_terrain_night);
		drawTerrainMesh(camera_frustum);
		glUseProgram(SP_car_night);
		attachments[1] = GL_COLOR_ATTACHMENT1;
		glDrawBuffers(4, attachments);
		car_VAO.bindCurrent();
		glDrawElementsInstanced(GL_TRIANGLES, CAR_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
	}
	if (sun.light_dir_and_radius.z > -0.2f)
	{
		glUseProgram(SP_sun);
		GLuint attachments[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3 };
		glDrawBuffers(4, attachments);
		glBindVertexArray(sun_VAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, SUN_VBO_SIZE);
	}

	glDisable(GL_DEPTH_TEST);

	glBindVertexArray(tex_blit_VAO);
	glBindFramebuffer(GL_FRAMEBUFFER, taa_FBOs[frame_idx & 1]);
	glBindTextureUnit(0, render_texs[RENDER_TARGET_COLOR]);
	glBindTextureUnit(1, taa_texs[1 - (frame_idx & 1)].color);
	glBindTextureUnit(2, render_texs[RENDER_TARGET_BLOOM]);
	glBindTextureUnit(3, taa_texs[1 - (frame_idx & 1)].bloom);
	glBindTextureUnit(4, render_texs[RENDER_TARGET_ALBEDO]);
	glBindTextureUnit(5, taa_texs[1 - (frame_idx & 1)].albedo);
	glBindTextureUnit(6, taa_texs[frame_idx & 1].depth);
	glBindTextureUnit(7, taa_texs[1 - (frame_idx & 1)].depth);
	glBindTextureUnit(8, render_texs[RENDER_TARGET_DELTA_POSITION]);
	for (int i = 0; i < 4; i++)
	{
		glBindTextureUnit(9 + i, taa_texs[1 - (frame_idx & 1)].stats[i]);
	}
	glUseProgram(SP_TAA);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, bloom_FBOs[0]);
	glViewport(0, 0, bloom_buffer_width, BLOOM_BUFFER_HEIGHT);
	glBindTextureUnit(0, taa_texs[frame_idx & 1].bloom);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUseProgram(SP_tex_blit);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, bloom_FBOs[1]);
	glUseProgram(SP_gaussian_blur);
	glViewport(0, 0, BLOOM_BUFFER_HEIGHT, bloom_buffer_width);
	glBindTextureUnit(0, bloom_texs[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindFramebuffer(GL_FRAMEBUFFER, bloom_FBOs[0]);
	glViewport(0, 0, bloom_buffer_width, BLOOM_BUFFER_HEIGHT);
	glBindTextureUnit(0, bloom_texs[1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window_width, window_height);
	glBindTextureUnit(0, taa_texs[frame_idx & 1].color);
	glBindTextureUnit(1, bloom_texs[0]);
	glUseProgram(SP_buffer_to_screen);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	if (show_fps && fps > 0.0f)
	{
		char str[40];
		static int displayed_fps = lround(fps), displayed_tick_rate = lround(tick_rate);
		static float cumulative_fps_diff = 0.0f, cumulative_tick_rate_diff = 0.0f;
		if (fps > displayed_fps + 0.5f)
		{
			cumulative_fps_diff += fps - 0.5f - displayed_fps;
		}
		else if (fps < displayed_fps - 0.5f)
		{
			cumulative_fps_diff += fps + 0.5f - displayed_fps;
		}
		if (abs(cumulative_fps_diff) > 1.0f)
		{
			displayed_fps = lround(fps);
			cumulative_fps_diff = 0.0f;
		}
		if (tick_rate > displayed_tick_rate + 0.5f)
		{
			cumulative_tick_rate_diff += tick_rate - 0.5f - displayed_tick_rate;
		}
		else if (tick_rate < displayed_tick_rate - 0.5f)
		{
			cumulative_tick_rate_diff += tick_rate + 0.5f - displayed_tick_rate;
		}
		if (abs(cumulative_tick_rate_diff) > 1.0f)
		{
			displayed_tick_rate = lround(tick_rate);
			cumulative_tick_rate_diff = 0.0f;
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

	GPUTriple::Lock();
	GPUTriple::Swap();
	frame_idx++;
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

	float tan_half_fov_y = tan(0.5f * FOV_Y);
	float tan_half_fov_x = tan_half_fov_y * width / height;
	glProgramUniform2f(SP_TAA, glGetUniformLocation(SP_TAA, "tan_half_fov"), tan_half_fov_x, tan_half_fov_y);

	GLint max_tex_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
	bloom_buffer_width = std::min(BLOOM_BUFFER_HEIGHT * (width + 1) / (height + 1), max_tex_size);

	glDeleteTextures(4, render_texs);
	glCreateTextures(GL_TEXTURE_2D, 4, render_texs);
	for (int i = 0; i < 4; i++)
	{
		glTextureStorage2D(render_texs[i], 1, GL_RGBA16F, width, height);
		glNamedFramebufferTexture(render_FBO, GL_COLOR_ATTACHMENT0 + i, render_texs[i], 0);
	}

	glDeleteTextures(sizeof(taa_texs) / sizeof(GLuint), &taa_texs[0].color);
	glCreateTextures(GL_TEXTURE_2D, sizeof(taa_texs) / sizeof(GLuint), &taa_texs[0].color);
	for (int i = 0; i < 2; i++)
	{
		glTextureStorage2D(taa_texs[i].color, 1, GL_RGBA16F, width, height);
		glTextureParameteri(taa_texs[i].color, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(taa_texs[i].color, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(taa_FBOs[i], GL_COLOR_ATTACHMENT0, taa_texs[i].color, 0);
		glTextureStorage2D(taa_texs[i].bloom, 4, GL_RGBA16F, width, height);
		glTextureParameteri(taa_texs[i].bloom, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(taa_texs[i].bloom, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glNamedFramebufferTexture(taa_FBOs[i], GL_COLOR_ATTACHMENT1, taa_texs[i].bloom, 0);
		glTextureStorage2D(taa_texs[i].albedo, 1, GL_RGBA16F, width, height);
		glTextureParameteri(taa_texs[i].albedo, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(taa_texs[i].albedo, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glNamedFramebufferTexture(taa_FBOs[i], GL_COLOR_ATTACHMENT2, taa_texs[i].albedo, 0);
		glTextureStorage2D(taa_texs[i].depth, 1, GL_DEPTH_COMPONENT24, width, height);
		glTextureParameteri(taa_texs[i].depth, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTextureParameteri(taa_texs[i].depth, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		for (int j = 0; j < sizeof(TAATextures::stats) / sizeof(GLuint); j++)
		{
			glTextureStorage2D(taa_texs[i].stats[j], 1, GL_RGBA16F, width, height);
			glTextureParameteri(taa_texs[i].stats[j], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTextureParameteri(taa_texs[i].stats[j], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glNamedFramebufferTexture(taa_FBOs[i], GL_COLOR_ATTACHMENT3 + j, taa_texs[i].stats[j], 0);
		}
	}

	glDeleteTextures(2, bloom_texs);
	glCreateTextures(GL_TEXTURE_2D, 2, bloom_texs);
	glTextureStorage2D(bloom_texs[0], 1, GL_RGBA16F, bloom_buffer_width, BLOOM_BUFFER_HEIGHT);
	glTextureStorage2D(bloom_texs[1], 1, GL_RGBA16F, BLOOM_BUFFER_HEIGHT, bloom_buffer_width);
	for (int i = 0; i < 2; i++)
	{
		glTextureParameteri(bloom_texs[i], GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(bloom_texs[i], GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(bloom_texs[i], GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTextureParameteri(bloom_texs[i], GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glNamedFramebufferTexture(bloom_FBOs[i], GL_COLOR_ATTACHMENT0, bloom_texs[i], 0);
	}
}

static void onKey(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		switch (key)
		{
		case ' ':
			is_paused = !is_paused;
			return;
		case '=':
			if (simulate_speed < 1 << 5)
			{
				simulate_speed = simulate_speed << 1;
			}
			return;
		case '-':
			if (simulate_speed > 1)
			{
				simulate_speed = simulate_speed >> 1;
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
			if (mods == GLFW_MOD_ALT)
			{
				static int old_pos_x, old_pos_y, old_width, old_height;
				static bool is_full_screen = false;
				if (is_full_screen)
				{
					glfwSetWindowMonitor(window, nullptr, old_pos_x, old_pos_y, old_width, old_height, 0);
				}
				else
				{
					glfwGetWindowPos(window, &old_pos_x, &old_pos_y);
					glfwGetWindowSize(window, &old_width, &old_height);
					GLFWmonitor* monitor = glfwGetPrimaryMonitor();
					const GLFWvidmode* mode = glfwGetVideoMode(monitor);
					glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				}
				is_full_screen = !is_full_screen;
			}
			else
			{
				show_fps = !show_fps;
			}
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
	// vcomp 在加载时读取该变量，且忽略之后的 _putenv。延迟加载后，这里早于首次 OpenMP。
	SetEnvironmentVariableW(L"OMP_WAIT_POLICY", L"PASSIVE");
	ImmDisableIME(GetCurrentThreadId());

	if (!glfwInit())
	{
		return -1;
	}

	initLogic();
	std::thread logical_thread(logicalFrame);
	simulate_speed = 1000000;
	while (logical_time < 0.6 * DAY_PERIOD)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	simulate_speed = 1;

	int width = 2400;
	int height = 1350;
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
		stopLogic();
		glfwTerminate();
		logical_thread.join();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
	{
		stopLogic();
		glfwTerminate();
		logical_thread.join();
		return -1;
	}
	glfwSetWindowPos(window, 70, 40);
	glfwSwapInterval(0);

	init();
	onResize(window, width, height);

	glfwSetWindowSizeCallback(window, onResize);
	glfwSetKeyCallback(window, onKey);
	glfwSetMouseButtonCallback(window, onMouseButton);
	glfwSetScrollCallback(window, onMouseWheel);

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