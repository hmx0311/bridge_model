#pragma comment(lib,"imm32.lib")
//#include "common.h"
#include "resource.h"
#include "scene.h"
#include "mesh.h"
#include "Car.h"
#include "logical_frame.h"

#include "glew.h"
#include "freeglut.h"
#include "gtx/transform.hpp"

#include <thread>

using namespace glm;

#define SHADOW_TEX_SIZE 4096
#define MAX_CSM_RATIO 3.6f
#define CSM_LEVELS 4
GLuint shadow_FBO;
GLuint shadow_tex;

constexpr float FOVY = pi<float>() / 4;
constexpr float VIEW_Z_NEAR = 90.0f;
constexpr float VIEW_Z_FAR = 90000.0f;
constexpr float FOCUS_HEIGHT = 200;
constexpr float MIN_VIEW_DISTANCE = 200;
constexpr float MAX_VIEW_DISTANCE = 25000;
constexpr float MIN_SHADOW_FAR = 369.5f;
constexpr float FOG_DENSITY = 0.00003f;

GLint window_width, window_height;

float aim_azimuth = 0.3f, aim_relative_depression = 0.1f, aim_view_distance = 15000;
float azimuth = aim_azimuth, relative_depression = aim_relative_depression, view_distance = aim_view_distance;
vec3 focus(0);
uint32_t focus_move_dir = 0;
bool need_update_view = true;
uint64_t last_frame_time_us;

#define MAX_HEIGHT 500
constexpr int HEIGHT_MAP_SIZE = 256;
constexpr RECT HEIGHT_MAP_AREA = { -10240, 12800, 10240, -7680 };
float height_map[HEIGHT_MAP_SIZE][HEIGHT_MAP_SIZE];

GLuint multisample_render_FBO;
GLuint multisample_render_RBOs[2];
GLuint depth_RBO;

GLuint render_FBO;
GLuint render_tex;

#define BLOOM_BUFFER_HEIGHT 512
int bloom_buffer_width;
GLuint bloom_FBOs[2];
GLuint bloom_texs[2];

GLuint tex_blit_VAO, tex_mapping_VBO;


/*
* binding = 0
*	mat4 projection_mat
*	mat4 view_mat
*	mat4 inv_view_mat
*
* binding = 1
*	vec4 sun_direction
*	vec4 sun_ambient
*	vec4 sun_diffuse_and_specular
*	vec4 sky_color
*/
GLuint scene_UBO;
GLintptr scene_UBO_offset1;

/*
* binding = 2
*	mat4 shadow_mat[CSM_LEVELS];
*	mat4 shadow_tex_mat[CSM_LEVELS];
*/
GLuint shadow_UBO;

// binding = 3
GLuint car_light_map_SSBO;
// binding = 4
GLuint car_light_pos_UBO;
// binding = 5
GLuint car_light_shadow_mat_UBO;
// binding = 6
GLuint car_lighting_SSBO;

struct
{
	mat4 proj_mat;
	mat4 view_mat;
	mat4 inv_view_mat;
}view;
float horizon_y;
vec3 CSM_areas[CSM_LEVELS][12];
struct
{
	mat4 mat[CSM_LEVELS];
	mat4 tex_mat[CSM_LEVELS];
}sun_shadow;
constexpr ivec2 LIGHT_MAP_SIZE = ivec2(134, 122);
constexpr float LIGHT_MAP_GRID_LENGTH = (CAR_LIGHT_RANGE / 2);
constexpr int MAX_LIGHT_PER_CAR = 24;
const mat4 CAR_LIGHT_SHADOW_PROJ_MAT = perspective(2 * acos(CAR_LIGHT_V_COS_ANGLE - 0.001f), CAR_LIGHT_ASPECT, 50.0f, 50.0f + 2 * LIGHT_MAP_GRID_LENGTH);

int num_active_car_light_map_grids = 0;
float car_light_map_grid_distance2_to_view[LIGHT_MAP_SIZE.x][LIGHT_MAP_SIZE.y];
ivec2 car_light_map_grid_distance_order[LIGHT_MAP_SIZE.x * LIGHT_MAP_SIZE.y];

int car_light_map[LIGHT_MAP_SIZE.x][LIGHT_MAP_SIZE.y][2];
vec4 car_light_pos[2 * MAX_CAR_CNT];
mat4 car_light_mats[2 * MAX_CAR_CNT];
int car_lightings[MAX_CAR_CNT][MAX_LIGHT_PER_CAR];

GLuint SP_highway_day;
GLuint SP_highway_night;
GLuint SP_car_day;
GLuint SP_car_night;
GLuint SP_sun;
GLuint SP_shadow_highway_day;
GLuint SP_shadow_highway_night;
GLuint SP_shadow_car_day;
GLuint SP_shadow_car_night;
GLuint SP_tex_blit;
GLuint SP_gaussian_blur;
GLuint SP_buffer_to_screen;

GLuint loadShader(GLuint shader_id, GLenum type)
{
	HRSRC rc = FindResource(nullptr, MAKEINTRESOURCE(shader_id), L"SHADER");
	if (rc == nullptr)
	{
		printf("ERROR: Can't Load Resource %d\n", shader_id);
		return 0;
	}
	int size = SizeofResource(nullptr, rc);
	const char* data = (const char*)(LockResource(LoadResource(nullptr, rc)));

	GLint status;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, 1, &data, &size);
	glCompileShader(shader);

	// check for errors
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("ERROR: Shader %d Compilation Error\n", shader_id);
		printf("%s\n", data);
		char log[1024];
		int len;
		glGetShaderInfoLog(shader, 1024, &len, log);
		printf("%s", log);
	}

	return shader;
}

GLuint linkShaderProgram(GLuint vs, GLuint fs, GLuint gs = 0)
{
	GLint status;
	GLuint sp = glCreateProgram();
	glAttachShader(sp, vs);
	glAttachShader(sp, fs);
	if (gs != 0)
	{
		glAttachShader(sp, gs);
	}
	glLinkProgram(sp);
	glGetProgramiv(sp, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("ERROR: Shader Program %d Link Error\n", sp);
		char data[1024];
		int len;
		glGetProgramInfoLog(sp, 1024, &len, data);
		printf("\n%s", data);
	}
	glDetachShader(sp, vs);
	glDetachShader(sp, fs);
	if (gs != 0)
	{
		glDetachShader(sp, gs);
	}
	return sp;
}

void initShader()
{
	GLuint VS_highway = loadShader(IDR_VS_HIGHWAY, GL_VERTEX_SHADER);
	GLuint FS_highway_day = loadShader(IDR_FS_HIGHWAY_DAY, GL_FRAGMENT_SHADER);
	GLuint FS_highway_night = loadShader(IDR_FS_HIGHWAY_NIGHT, GL_FRAGMENT_SHADER);
	SP_highway_day = linkShaderProgram(VS_highway, FS_highway_day);
	SP_highway_night = linkShaderProgram(VS_highway, FS_highway_night);
	glDeleteShader(VS_highway);
	glDeleteShader(FS_highway_day);
	glDeleteShader(FS_highway_night);
	glProgramUniform1f(SP_highway_day, glGetUniformLocation(SP_highway_day, "fogDensity"), FOG_DENSITY);
	glProgramUniform1f(SP_highway_night, glGetUniformLocation(SP_highway_night, "fogDensity"), FOG_DENSITY);

	GLuint VS_car = loadShader(IDR_VS_CAR, GL_VERTEX_SHADER);
	GLuint FS_car_day = loadShader(IDR_FS_CAR_DAY, GL_FRAGMENT_SHADER);
	GLuint FS_car_night = loadShader(IDR_FS_CAR_NIGHT, GL_FRAGMENT_SHADER);
	SP_car_day = linkShaderProgram(VS_car, FS_car_day);
	SP_car_night = linkShaderProgram(VS_car, FS_car_night);
	glDeleteShader(VS_car);
	glDeleteShader(FS_car_day);
	glDeleteShader(FS_car_night);
	glProgramUniform1f(SP_car_day, glGetUniformLocation(SP_car_day, "fogDensity"), FOG_DENSITY);
	glProgramUniform1f(SP_car_night, glGetUniformLocation(SP_car_night, "fogDensity"), FOG_DENSITY);

	GLuint VS_sun = loadShader(IDR_VS_SUN, GL_VERTEX_SHADER);
	GLuint FS_sun = loadShader(IDR_FS_SUN, GL_FRAGMENT_SHADER);
	SP_sun = linkShaderProgram(VS_sun, FS_sun);
	glDeleteShader(VS_sun);
	glDeleteShader(FS_sun);

	GLuint VS_shadow_highway_day = loadShader(IDR_VS_SHADOW_HIGHWAY_DAY, GL_VERTEX_SHADER);
	GLuint VS_shadow_highway_night = loadShader(IDR_VS_SHADOW_HIGHWAY_NIGHT, GL_VERTEX_SHADER);
	GLuint VS_shadow_car_day = loadShader(IDR_VS_SHADOW_CAR_DAY, GL_VERTEX_SHADER);
	GLuint VS_shadow_car_night = loadShader(IDR_VS_SHADOW_CAR_NIGHT, GL_VERTEX_SHADER);
	GLuint FS_shadow = loadShader(IDR_FS_SHADOW, GL_FRAGMENT_SHADER);
	GLuint GS_shadow_day = loadShader(IDR_GS_SHADOW_DAY, GL_GEOMETRY_SHADER);
	GLuint GS_shadow_highway_night = loadShader(IDR_GS_SHADOW_HIGHWAY_NIGHT, GL_GEOMETRY_SHADER);
	GLuint GS_shadow_car_night = loadShader(IDR_GS_SHADOW_CAR_NIGHT, GL_GEOMETRY_SHADER);
	SP_shadow_highway_day = linkShaderProgram(VS_shadow_highway_day, FS_shadow, GS_shadow_day);
	SP_shadow_highway_night = linkShaderProgram(VS_shadow_highway_night, FS_shadow, GS_shadow_highway_night);
	SP_shadow_car_day = linkShaderProgram(VS_shadow_car_day, FS_shadow, GS_shadow_day);
	SP_shadow_car_night = linkShaderProgram(VS_shadow_car_night, FS_shadow, GS_shadow_car_night);
	glDeleteShader(VS_shadow_highway_day);
	glDeleteShader(VS_shadow_highway_night);
	glDeleteShader(GS_shadow_highway_night);
	glDeleteShader(VS_shadow_car_day);
	glDeleteShader(GS_shadow_day);
	glDeleteShader(VS_shadow_car_night);
	glDeleteShader(GS_shadow_car_night);
	glDeleteShader(FS_shadow);

	GLuint VS_tex_blit = loadShader(IDR_VS_TEX_BLIT, GL_VERTEX_SHADER);
	GLuint FS_tex_blit = loadShader(IDR_FS_TEX_BLIT, GL_FRAGMENT_SHADER);
	GLuint FS_gaussian_blur = loadShader(IDR_FS_GAUSSIAN_BLUR, GL_FRAGMENT_SHADER);
	GLuint FS_buffer_to_screen = loadShader(IDR_FS_BUFFER_TO_SCREEN, GL_FRAGMENT_SHADER);
	SP_tex_blit = linkShaderProgram(VS_tex_blit, FS_tex_blit);
	SP_gaussian_blur = linkShaderProgram(VS_tex_blit, FS_gaussian_blur);
	SP_buffer_to_screen = linkShaderProgram(VS_tex_blit, FS_buffer_to_screen);
	glDeleteShader(VS_tex_blit);
	glDeleteShader(FS_tex_blit);
	glDeleteShader(FS_gaussian_blur);
	glDeleteShader(FS_buffer_to_screen);
}

void buildHeightMap()
{
	glEnable(GL_DEPTH_TEST);
	constexpr int HEIGHT_BORDER = 3;
	constexpr int FILTER_RADIUS = 3;
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);
	glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
	glClear(GL_DEPTH_BUFFER_BIT);
	mat4 height_mat = ortho(float(HEIGHT_MAP_AREA.left), float(HEIGHT_MAP_AREA.right), float(HEIGHT_MAP_AREA.bottom), float(HEIGHT_MAP_AREA.top), 0.0f, float(MAX_HEIGHT)) *
		lookAt(vec3(0, 0, MAX_HEIGHT), vec3(0, 0, 0), vec3(0, 1, 0));
	vec4 v(FLT_MAX);
	glNamedBufferSubData(scene_UBO, scene_UBO_offset1, sizeof(vec4), &v);
	glNamedBufferSubData(shadow_UBO, 0, sizeof(mat4), &height_mat);
	glUseProgram(SP_shadow_highway_day);
	glBindVertexArray(bridge_VAO);
	glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
	glBindVertexArray(ground_VAO);
	glDrawElements(GL_TRIANGLES, GROUND_EBO_SIZE, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	GLfloat(*depth_data)[SHADOW_TEX_SIZE][SHADOW_TEX_SIZE] = new GLfloat[CSM_LEVELS][SHADOW_TEX_SIZE][SHADOW_TEX_SIZE];
	glGetTextureImage(shadow_tex, 0, GL_DEPTH_COMPONENT, GL_FLOAT, CSM_LEVELS * SHADOW_TEX_SIZE * SHADOW_TEX_SIZE * sizeof(GLfloat), depth_data);
	float(*height_map_buffer)[HEIGHT_MAP_SIZE][HEIGHT_MAP_SIZE] = new float[2][HEIGHT_MAP_SIZE][HEIGHT_MAP_SIZE];
	constexpr int FILTER_SIZE = 2 * FILTER_RADIUS + 1;
	int height_filter[FILTER_SIZE][FILTER_SIZE];
	height_filter[0][0] = 1;
	for (int i = 1; i < FILTER_SIZE; i++)
	{
		height_filter[0][i] = height_filter[0][i - 1] * (FILTER_SIZE - i) / i;
	}
	for (int i = 1; i < FILTER_SIZE; i++)
	{
		height_filter[i][0] = height_filter[i - 1][0] * (FILTER_SIZE - i) / i;
		for (int j = 1; j < FILTER_SIZE; j++)
		{
			height_filter[i][j] = height_filter[i][0] * height_filter[0][j];
		}
	}
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < HEIGHT_MAP_SIZE; i++)
		{
			for (int j = 0; j < HEIGHT_MAP_SIZE; j++)
			{
				float min_depth = 1.0f;
				for (int p = 0; p < SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE; p++)
				{
					for (int q = 0; q < SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE; q++)
					{
						if (min_depth > depth_data[0][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * i + p][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * j + q])
						{
							min_depth = depth_data[0][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * i + p][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * j + q];
						}
					}
				}
				height_map_buffer[0][i][j] = (1 - min_depth) * MAX_HEIGHT;
			}
		}
#pragma omp for
		for (int i = 0; i < HEIGHT_MAP_SIZE; i++)
		{
			for (int j = 0; j < HEIGHT_MAP_SIZE; j++)
			{
				float max_height = 0.0f;
				for (int p = -HEIGHT_BORDER; p <= HEIGHT_BORDER; p++)
				{
					if (0 <= i + p && i + p < HEIGHT_MAP_SIZE)
					{
						for (int q = -HEIGHT_BORDER; q <= HEIGHT_BORDER; q++)
						{
							if (0 <= j + q && j + q < HEIGHT_MAP_SIZE && max_height < height_map_buffer[0][i + p][j + q])
							{
								max_height = height_map_buffer[0][i + p][j + q];
							}
						}
					}
				}
				height_map_buffer[1][i][j] = max_height;
			}
		}
#pragma omp for
		for (int i = 0; i < HEIGHT_MAP_SIZE; i++)
		{
			for (int j = 0; j < HEIGHT_MAP_SIZE; j++)
			{
				float height = 0;
				int factor = 0;
				for (int p = -FILTER_RADIUS; p <= FILTER_RADIUS; p++)
				{
					if (0 <= i + p && i + p < HEIGHT_MAP_SIZE)
					{
						for (int q = -FILTER_RADIUS; q <= FILTER_RADIUS; q++)
						{
							if (0 <= j + q && j + q < HEIGHT_MAP_SIZE)
							{
								height += height_filter[p + FILTER_RADIUS][q + FILTER_RADIUS] * height_map_buffer[1][i + p][j + q];
								factor += height_filter[p + FILTER_RADIUS][q + FILTER_RADIUS];
							}
						}
					}
				}
				height /= factor;
				height_map[i][j] = height;
			}
		}
	}
	delete[] depth_data;
	delete[] height_map_buffer;
}

void init()
{
	glewInit();
	printf("%s\n", (char*)glGetString(GL_VERSION));
	printf("%s\n", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	initShader();

	for (int i = 0; i < LIGHT_MAP_SIZE.x; i++)
	{
		for (int j = 0; j < LIGHT_MAP_SIZE.y; j++)
		{
			car_light_map_grid_distance_order[i * LIGHT_MAP_SIZE.y + j] = ivec2(i, j);
		}
	}

	int UBO_offset_alignment;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UBO_offset_alignment);

	scene_UBO_offset1 = ((sizeof(view) - 1) / UBO_offset_alignment + 1) * UBO_offset_alignment;
	glGenBuffers(1, &scene_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, scene_UBO);
	glBufferData(GL_UNIFORM_BUFFER, scene_UBO_offset1 + 4 * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, scene_UBO, 0, sizeof(view));
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, scene_UBO, scene_UBO_offset1, 4 * sizeof(vec4));

	glGenBuffers(1, &shadow_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, shadow_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(sun_shadow), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, shadow_UBO, 0, sizeof(sun_shadow));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &car_light_map_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, car_light_map_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(car_light_map), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, car_light_map_SSBO, 0, sizeof(car_light_map));
	glGenBuffers(1, &car_light_pos_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, car_light_pos_UBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(car_light_pos), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 4, car_light_pos_UBO, 0, sizeof(car_light_pos));
	glGenBuffers(1, &car_light_shadow_mat_UBO);
	glBindBuffer(GL_UNIFORM_BUFFER, car_light_shadow_mat_UBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * MAX_CAR_CNT * sizeof(mat4), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 5, car_light_shadow_mat_UBO, 0, 2 * MAX_CAR_CNT * sizeof(mat4));
	glGenBuffers(1, &car_lighting_SSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, car_lighting_SSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(car_lightings), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 6, car_lighting_SSBO, 0, sizeof(car_lightings));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

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
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenFramebuffers(1, &shadow_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glGenTextures(1, &shadow_tex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_tex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, CSM_LEVELS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_tex, 0);
	glBindTextureUnit(1, shadow_tex);

	initScene();
	buildHeightMap();
	last_frame_time_us = getTimestampMicroseconds();
}

void drawGraphics()
{
	uint64_t time_us = getTimestampMicroseconds();
	uint32_t frame_dt_us = time_us - last_frame_time_us;
	last_frame_time_us = time_us;
	LogicalData& logical_data = getLatestLogicalData();
	bool is_view_updated = need_update_view;
	if (need_update_view)
	{
		need_update_view = false;
		if (azimuth != aim_azimuth)
		{
			constexpr float ROTATE_SPEED = 10.0f;
			float max_rotate_angle = ROTATE_SPEED * 1e-6 * frame_dt_us;
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
			float max_rotate_angle = ROTATE_SPEED * 1e-6f * frame_dt_us;
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
			float max_zoom_distance = ZOOM_SPEED * 1e-6f * frame_dt_us * view_distance;
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
				float move_speed = 1.0f * view_distance + 1000.0f;
				float move_distance = move_speed * 1e-6f * frame_dt_us;
				focus.x += dir.x * move_distance;
				focus.y += dir.y * move_distance;
				focus.x = clamp(focus.x, -30000.0f, 30000.0f);
				focus.y = clamp(focus.y, -10000.0f, 20000.0f);
				need_update_view = true;
			}
		}

		float depression = pi<float>() / 2 * (1 - FLT_EPSILON) * (1 - ((1 - 0.3f * view_distance / MAX_VIEW_DISTANCE) * (1 - relative_depression)));
		vec3 view_dir(-cos(depression) * sin(azimuth), cos(depression) * cos(azimuth), -sin(depression));
		focus.z = FOCUS_HEIGHT;
		vec3 eye = focus - view_distance * view_dir;
		if (eye.z < FOCUS_HEIGHT + MAX_HEIGHT)
		{
			vec2 height_map_coord((eye.x - HEIGHT_MAP_AREA.left) / (HEIGHT_MAP_AREA.right - HEIGHT_MAP_AREA.left) * HEIGHT_MAP_SIZE - 0.5f,
				(eye.y - HEIGHT_MAP_AREA.bottom) / (HEIGHT_MAP_AREA.top - HEIGHT_MAP_AREA.bottom) * HEIGHT_MAP_SIZE - 0.5f);
			if (0 < height_map_coord.x && height_map_coord.x < HEIGHT_MAP_SIZE - 1 && 0 < height_map_coord.y && height_map_coord.y < HEIGHT_MAP_SIZE - 1)
			{
				float height_offset = height_map[int(height_map_coord.y)][int(height_map_coord.x)] * (int(height_map_coord.x) + 1 - height_map_coord.x) * (int(height_map_coord.y) + 1 - height_map_coord.y) +
					height_map[int(height_map_coord.y)][int(height_map_coord.x) + 1] * (height_map_coord.x - int(height_map_coord.x)) * (int(height_map_coord.y) + 1 - height_map_coord.y) +
					height_map[int(height_map_coord.y) + 1][int(height_map_coord.x)] * (int(height_map_coord.x) + 1 - height_map_coord.x) * (height_map_coord.y - int(height_map_coord.y)) +
					height_map[int(height_map_coord.y) + 1][int(height_map_coord.x) + 1] * (height_map_coord.x - int(height_map_coord.x)) * (height_map_coord.y - int(height_map_coord.y));
				height_offset *= 1 - (eye.z - FOCUS_HEIGHT) / (MAX_HEIGHT);
				eye.z += height_offset;
				focus.z += height_offset;
			}
		}
		view.view_mat = lookAt(eye, focus, vec3(-sin(azimuth), cos(azimuth), 0));
		view.inv_view_mat = inverse(view.view_mat);
		vec3 top_view(0, tanf(FOVY / 2), -1);
		vec3 bottom_view(0, -top_view.y, -1);
		vec3 view_x = vec3(top_view.y / window_height * window_width, 0, 0);
		top_view = mat3(view.inv_view_mat) * top_view;
		bottom_view = mat3(view.inv_view_mat) * bottom_view;
		view_x = mat3(view.inv_view_mat) * view_x;
		horizon_y = (tan(depression - acos(EARTH_RADIUS / (focus.z + EARTH_RADIUS))) / tan(FOVY / 2) + 1) * window_height / 2;

		if (eye.z > MAX_HEIGHT)
		{
			float shadow_near = fmax(-(eye.z - MAX_HEIGHT) / bottom_view.z, VIEW_Z_NEAR);
			float shadow_far = fmax(shadow_near, MIN_SHADOW_FAR);
			float bottom_far = -eye.z / bottom_view.z;
			float top_near = VIEW_Z_FAR;
			float top_far = VIEW_Z_FAR;
			if (top_view.z < 0)
			{
				top_near = fmin(-(eye.z - MAX_HEIGHT) / top_view.z, VIEW_Z_FAR);
				top_far = fmin(-eye.z / top_view.z, VIEW_Z_FAR);
			}
			float CSM_ratio = fmin(pow(top_far / shadow_far, 1.0f / CSM_LEVELS), MAX_CSM_RATIO);
			vec3 shadow_hexa[6];
			shadow_hexa[4] = shadow_near * bottom_view;
			shadow_hexa[5] = shadow_hexa[4];
			for (int i = 0; i < CSM_LEVELS; i++)
			{
				shadow_far *= CSM_ratio;
				shadow_hexa[0] = shadow_hexa[4];
				shadow_hexa[1] = shadow_hexa[5];
				if (shadow_near < bottom_far && bottom_far < shadow_far)
				{
					shadow_hexa[2] = bottom_far * bottom_view;
				}
				else
				{
					shadow_hexa[2] = shadow_hexa[0];
				}
				if (shadow_near < top_near && top_near < shadow_far)
				{
					shadow_hexa[3] = top_near * top_view;
				}
				else
				{
					shadow_hexa[3] = shadow_hexa[0];
				}
				if (top_far < shadow_far)
				{
					shadow_hexa[4] = top_view * top_far;
					shadow_hexa[5] = shadow_hexa[4];
				}
				else
				{
					if (bottom_far < shadow_far)
					{
						shadow_hexa[4] = shadow_far * bottom_view - (eye.z + shadow_far * bottom_view.z) / (view_dir.z - bottom_view.z) * (view_dir - bottom_view);
					}
					else
					{
						shadow_hexa[4] = shadow_far * bottom_view;
					}
					if (top_near > shadow_far)
					{
						shadow_hexa[5] = shadow_far * top_view + (eye.z + shadow_far * top_view.z - MAX_HEIGHT) / (top_view.z - view_dir.z) * (view_dir - top_view);
					}
					else
					{
						shadow_hexa[5] = shadow_far * top_view;
					}
				}
				shadow_near = shadow_far;
				for (int j = 0; j < 6; j++)
				{
					float distance = dot(shadow_hexa[j], view_dir);
					CSM_areas[i][2 * j] = eye + shadow_hexa[j] + distance * view_x;
					CSM_areas[i][2 * j + 1] = eye + shadow_hexa[j] - distance * view_x;
				}
			}
		}
		else
		{
			float shadow_near = VIEW_Z_NEAR;
			float bottom = -eye.z / bottom_view.z;
			float shadow_far = MIN_SHADOW_FAR;
			if (top_view.z > 0)
			{
				float top = fmin((MAX_HEIGHT - eye.z) / top_view.z, VIEW_Z_FAR);
				float CSM_ratio = fmin(pow(VIEW_Z_FAR / shadow_far, 1.0f / CSM_LEVELS), MAX_CSM_RATIO);
				vec3 shadow_hexa[6];
				shadow_hexa[4] = shadow_near * bottom_view;
				if (top < shadow_near)
				{
					shadow_hexa[5] = shadow_near * top_view - (eye.z + shadow_near * top_view.z - MAX_HEIGHT) / (view_dir.z - top_view.z) * (view_dir - top_view);
				}
				else
				{
					shadow_hexa[5] = shadow_near * top_view;
				}
				for (int i = 0; i < CSM_LEVELS; i++)
				{
					shadow_far *= CSM_ratio;
					shadow_hexa[0] = shadow_hexa[4];
					shadow_hexa[1] = shadow_hexa[5];
					if (shadow_near < bottom && bottom < shadow_far)
					{
						shadow_hexa[2] = bottom * bottom_view;
					}
					else
					{
						shadow_hexa[2] = shadow_hexa[0];
					}
					if (shadow_near < top && top < shadow_far)
					{
						shadow_hexa[3] = top * top_view;
					}
					else
					{
						shadow_hexa[3] = shadow_hexa[0];
					}
					if (bottom < shadow_far)
					{
						shadow_hexa[4] = shadow_far * bottom_view - (eye.z + shadow_far * bottom_view.z) / (view_dir.z - bottom_view.z) * (view_dir - bottom_view);
					}
					else
					{
						shadow_hexa[4] = shadow_far * bottom_view;
					}
					if (top < shadow_far)
					{
						shadow_hexa[5] = shadow_far * top_view - (eye.z + shadow_far * top_view.z - MAX_HEIGHT) / (view_dir.z - top_view.z) * (view_dir - top_view);
					}
					else
					{
						shadow_hexa[5] = shadow_far * top_view;
					}
					shadow_near = shadow_far;
					for (int j = 0; j < 6; j++)
					{
						float distance = dot(shadow_hexa[j], view_dir);
						CSM_areas[i][2 * j] = eye + shadow_hexa[j] + distance * view_x;
						CSM_areas[i][2 * j + 1] = eye + shadow_hexa[j] - distance * view_x;
					}
				}
			}
			else
			{
				float top = VIEW_Z_FAR;
				if (top_view.z < 0)
				{
					top = fmin(-eye.z / top_view.z, VIEW_Z_FAR);
				}
				float CSM_ratio = fmin(pow(top / shadow_far, 1.0f / CSM_LEVELS), MAX_CSM_RATIO);
				vec3 shadow_hexa[6];
				shadow_hexa[4] = shadow_near * bottom_view;
				shadow_hexa[5] = shadow_near * top_view;
				for (int i = 0; i < CSM_LEVELS; i++)
				{
					shadow_far *= CSM_ratio;
					shadow_hexa[0] = shadow_hexa[4];
					shadow_hexa[1] = shadow_hexa[5];
					if (shadow_near < bottom && bottom < shadow_far)
					{
						shadow_hexa[2] = bottom * bottom_view;
					}
					else
					{
						shadow_hexa[2] = shadow_hexa[0];
					}
					shadow_hexa[3] = shadow_hexa[0];
					if (top < shadow_far)
					{
						shadow_hexa[4] = top_view * top;
						shadow_hexa[5] = shadow_hexa[4];
					}
					else
					{
						if (bottom < shadow_far)
						{
							shadow_hexa[4] = shadow_far * bottom_view - (eye.z + shadow_far * bottom_view.z) / (view_dir.z - bottom_view.z) * (view_dir - bottom_view);
						}
						else
						{
							shadow_hexa[4] = shadow_far * bottom_view;
						}
						shadow_hexa[5] = shadow_far * top_view;
					}
					shadow_near = shadow_far;
					for (int j = 0; j < 6; j++)
					{
						float distance = dot(shadow_hexa[j], view_dir);
						CSM_areas[i][2 * j] = eye + shadow_hexa[j] + distance * view_x;
						CSM_areas[i][2 * j + 1] = eye + shadow_hexa[j] - distance * view_x;
					}
				}
			}
		}

		mat4 proj_and_view_mat = view.proj_mat * view.view_mat;
		bool is_light_grid_visible[LIGHT_MAP_SIZE.x][LIGHT_MAP_SIZE.y];
		for (int i = 0; i < LIGHT_MAP_SIZE.x; i++)
		{
			float offset_x = (-LIGHT_MAP_SIZE.x / 2 + i) * LIGHT_MAP_GRID_LENGTH;
			for (int j = 0; j < LIGHT_MAP_SIZE.y; j++)
			{
				float offset_y = (-LIGHT_MAP_SIZE.y / 2 + j) * LIGHT_MAP_GRID_LENGTH;
				constexpr vec4 GRID_AABB[8] = { vec4(-LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, 0, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, 0, 1),
					vec4(-LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, 0, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, 0, 1),
					vec4(-LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1),
					vec4(-LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1) };
				int left = 0, right = 0, bottom = 0, top = 0, back = 0, front = 0;
				for (vec4 vert : GRID_AABB)
				{
					vert.x += offset_x;
					vert.y += offset_y;
					vert = proj_and_view_mat * vert;
					left += vert.x < -vert.w;
					right += vert.x > vert.w;
					bottom += vert.y < -vert.w;
					top += vert.y > vert.w;
					back += vert.z < -vert.w;
					front += vert.z > vert.w;
				}
				is_light_grid_visible[i][j] = left != 8 && right != 8 && bottom != 8 && top != 8 && back != 8 && front != 8;
			}
		}
		num_active_car_light_map_grids = 0;
		for (int i = 0; i < LIGHT_MAP_SIZE.x; i++)
		{
			float offset_x = (-LIGHT_MAP_SIZE.x / 2 + 0.5f + i) * LIGHT_MAP_GRID_LENGTH;
			for (int j = 0; j < LIGHT_MAP_SIZE.y; j++)
			{
				float offset_y = (-LIGHT_MAP_SIZE.y / 2 + 0.5f + j) * LIGHT_MAP_GRID_LENGTH;
				if (is_light_grid_visible[i][j])
				{
					vec2 distance = vec2(eye) - vec2(offset_x, offset_y);
					car_light_map_grid_distance2_to_view[i][j] = dot(distance, distance);
					num_active_car_light_map_grids++;
				}
				else
				{
					car_light_map_grid_distance2_to_view[i][j] = FLT_MAX;
				}
			}
		}
		std::sort(car_light_map_grid_distance_order, &car_light_map_grid_distance_order[LIGHT_MAP_SIZE.x * LIGHT_MAP_SIZE.y], [](ivec2 a, ivec2 b)->bool
			{
				return car_light_map_grid_distance2_to_view[a.x][a.y] < car_light_map_grid_distance2_to_view[b.x][b.y];
			});
	}

	struct
	{
		vec4 dir;
		vec4 ambient;
		vec4 diffuse_and_specular;
		vec4 sky_color;
	}sun;
	sun.dir = vec4(logical_data.sun_dir, 0);
	sun.ambient = vec4(vec3(0.01f), 0);
	constexpr float ATMOSPHERE = EARTH_RADIUS + 2e6f;
	float absorb_factor = -1e-7f * (-EARTH_RADIUS * sun.dir.z + sqrt(ATMOSPHERE * ATMOSPHERE - EARTH_RADIUS * EARTH_RADIUS * (1 - sun.dir.z * sun.dir.z)));
	sun.diffuse_and_specular = vec4(0.5f * vec3(exp(0.2f * absorb_factor), exp(0.3f * absorb_factor), exp(1.1f * absorb_factor)), 0);
	sun.sky_color = vec4(0.01f, 0.015f, 0.055f, 0);
	if (sun.dir.z > -0.2f)
	{
		sun.ambient += vec4(vec3((sun.dir.z + 0.2f) * 0.1f), 0);
		sun.sky_color += vec4((sun.dir.z + 0.2f) * vec3(0.2f, 0.3f, 1.1f), 0);
	}

	int num_visible_cars;
	int num_visible_light_on_cars;
	int num_visible_car_lights;

	if (sun.dir.z > 0)
	{
		num_visible_cars = logical_data.num_cars;
		mat4 sun_mat = lookAt(vec3(0.0f), -vec3(sun.dir), vec3(-sun.dir.x, -sun.dir.y, sun.dir.z));
		float z_far = FLT_MAX;
		float slope = sqrt(1.0f / (sun.dir.z * sun.dir.z) - 1.0f);
		float max_x[CSM_LEVELS], min_x[CSM_LEVELS], max_y[CSM_LEVELS], min_y[CSM_LEVELS], z_fars[CSM_LEVELS];
		for (int i = CSM_LEVELS - 1; i >= 0; i--)
		{
			min_x[i] = FLT_MAX, max_x[i] = -FLT_MAX, min_y[i] = FLT_MAX, max_y[i] = -FLT_MAX;
			for (int j = 0; j < 12; j++)
			{
				vec3 point = vec3(sun_mat * vec4(CSM_areas[i][j], 1.0f));
				if (point.z < z_far)
				{
					z_far = point.z;
				}
				if (point.x < min_x[i])
				{
					min_x[i] = point.x;
				}
				if (point.x > max_x[i])
				{
					max_x[i] = point.x;
				}
				if (point.y < min_y[i])
				{
					min_y[i] = point.y;
				}
				if (point.y > max_y[i])
				{
					max_y[i] = point.y;
				}
			}
			z_far = fmax(-max_y[i] * slope, z_far);
			z_fars[i] = z_far;
		}
		for (int i = 0; i < CSM_LEVELS - 3; i += 4)
		{
			float x_min = FLT_MAX, x_max = -FLT_MAX, y_min = FLT_MAX, y_max = -FLT_MAX, furthest_z_far = FLT_MAX;
			for (int j = 0; j < 4; j++)
			{
				if (z_fars[4 * i + j] < furthest_z_far)
				{
					furthest_z_far = z_fars[4 * i + j];
				}
				if (min_x[4 * i + j] < x_min)
				{
					x_min = min_x[4 * i + j];
				}
				if (max_x[4 * i + j] > x_max)
				{
					x_max = max_x[4 * i + j];
				}
				if (min_y[4 * i + j] < y_min)
				{
					y_min = min_y[4 * i + j];
				}
				if (max_y[4 * i + j] > y_max)
				{
					y_max = max_y[4 * i + j];
				}
			}
			if (4 * (max_x[4 * i] - min_x[4 * i]) * (max_y[4 * i] - min_y[4 * i]) > (x_max - x_min) * (y_max - y_min))
			{
				min_x[4 * i] = x_min;
				max_x[4 * i] = (x_min + x_max) / 2;
				min_y[4 * i] = y_min;
				max_y[4 * i] = (x_min + y_max) / 2;
				z_fars[4 * i] = furthest_z_far;
				min_x[4 * i + 1] = (x_min + x_max) / 2;
				max_x[4 * i + 1] = x_max;
				min_y[4 * i + 1] = y_min;
				max_y[4 * i + 1] = (x_min + y_max) / 2;
				z_fars[4 * i + 1] = furthest_z_far;
				min_x[4 * i + 2] = x_min;
				max_x[4 * i + 2] = (x_min + x_max) / 2;
				min_y[4 * i + 2] = (x_min + y_max) / 2;
				max_y[4 * i + 2] = y_max;
				z_fars[4 * i + 2] = furthest_z_far;
				min_x[4 * i + 3] = (x_min + x_max) / 2;
				max_x[4 * i + 3] = x_max;
				min_y[4 * i + 3] = (x_min + y_max) / 2;
				max_y[4 * i + 3] = y_max;
				z_fars[4 * i + 3] = furthest_z_far;
			}
		}
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			float z_near = fmin(MAX_HEIGHT / sun.dir.z - min_y[i] * slope, 5 * VIEW_Z_FAR + z_fars[i]);
			constexpr float TEX_PADDING = (1 / (1 - 2 * 0.02f) - 1) / 2;
			mat4 shadow_mat = ortho(min_x[i] - TEX_PADDING * (max_x[i] - min_x[i]), max_x[i] + TEX_PADDING * (max_x[i] - min_x[i]),
				min_y[i] - TEX_PADDING * (max_y[i] - min_y[i]), max_y[i] + TEX_PADDING * (max_y[i] - min_y[i]),
				-z_near, -z_fars[i]);
			shadow_mat = shadow_mat * sun_mat;
			sun_shadow.mat[i] = shadow_mat;
			sun_shadow.tex_mat[i] = mat4(
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
		memset(car_light_map, 0, sizeof(car_light_map));
		struct CarLightInfo
		{
			vec4 pos;
			vec4 dir;
			int* light_map_grid;
		};
		std::vector<CarLightInfo>car_light_infos;
		for (int i = 0, j = logical_data.num_light_on_cars; i < j; i++)
		{
			mat4& transform = logical_data.car_transform[i];
			ivec2 light_map_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(transform[3]) + 0.5f * vec2(LIGHT_MAP_SIZE));
			if (car_light_map_grid_distance2_to_view[light_map_idx.x][light_map_idx.y] == FLT_MAX)
			{
				for (j--; i < j; j--)
				{
					mat4& back_transform = logical_data.car_transform[j];
					ivec2 back_light_map_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(back_transform[3]) + 0.5f * vec2(LIGHT_MAP_SIZE));
					if (car_light_map_grid_distance2_to_view[back_light_map_idx.x][back_light_map_idx.y] < FLT_MAX)
					{
						std::swap(transform, back_transform);
						std::swap(logical_data.car_color[i], logical_data.car_color[j]);
						break;
					}
				}
				if (i == j)
				{
					break;
				}
			}
			num_visible_cars++;
			num_visible_light_on_cars++;
		}
		for (int i = logical_data.num_light_on_cars, j = logical_data.num_cars; i < j; i++)
		{
			mat4& transform = logical_data.car_transform[i];
			ivec2 light_map_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(transform[3]) + 0.5f * vec2(LIGHT_MAP_SIZE));
			if (car_light_map_grid_distance2_to_view[light_map_idx.x][light_map_idx.y] == FLT_MAX)
			{
				for (j--; i < j; j--)
				{
					mat4& back_transform = logical_data.car_transform[j];
					ivec2 back_light_map_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(back_transform[3]) + 0.5f * vec2(LIGHT_MAP_SIZE));
					if (car_light_map_grid_distance2_to_view[back_light_map_idx.x][back_light_map_idx.y] < FLT_MAX)
					{
						std::swap(transform, back_transform);
						std::swap(logical_data.car_color[i], logical_data.car_color[j]);
						break;
					}
				}
				if (i == j)
				{
					break;
				}
			}
			num_visible_cars++;
		}
		int num_car_lights = 2 * logical_data.num_light_on_cars;
		for (int i = 0; i < num_car_lights; i++)
		{
			constexpr float POS_OFFSET = 50.0f / LIGHT_MAP_GRID_LENGTH + 1.0f;
			vec4 light_pos = logical_data.car_light_pos[i];
			vec4 light_dir = logical_data.car_light_dir[i];
			ivec2 light_map_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(light_pos) + POS_OFFSET * vec2(light_dir) + 0.5f * vec2(LIGHT_MAP_SIZE));
			if (car_light_map_grid_distance2_to_view[light_map_idx.x][light_map_idx.y] < FLT_MAX)
			{
				car_light_infos.emplace_back(CarLightInfo{ light_pos, light_dir, car_light_map[light_map_idx.x][light_map_idx.y] });
				car_light_map[light_map_idx.x][light_map_idx.y][0]++;
			}
		}
		for (int i = 0; i < num_active_car_light_map_grids; i++)
		{
			ivec2& idx = car_light_map_grid_distance_order[i];
			car_light_map[idx.x][idx.y][1] = num_visible_car_lights;
			num_visible_car_lights += car_light_map[idx.x][idx.y][0];
			car_light_map[idx.x][idx.y][0] = car_light_map[idx.x][idx.y][1];
		}
		for (CarLightInfo& car_light_info : car_light_infos)
		{
			int idx = car_light_info.light_map_grid[1]++;
			car_light_pos[idx] = car_light_info.pos;
			car_light_mats[idx] = CAR_LIGHT_SHADOW_PROJ_MAT * lookAt(vec3(car_light_info.pos), vec3(car_light_info.pos) + vec3(car_light_info.dir), vec3(0.0f, 0.0f, 1.0f));
		}
		for (int i = 0; i < num_visible_cars; i++)
		{
			mat4& transform = logical_data.car_transform[i < num_visible_light_on_cars ? i : i + logical_data.num_light_on_cars - num_visible_light_on_cars];
			ivec2 pos_idx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(transform[3]) + 0.5f * vec2(LIGHT_MAP_SIZE));
			int num_lighting = 0;
			for (int j = -1; j < 2; j++)
			{
				for (int k = -1; k < 2; k++)
				{
					ivec2 idx = pos_idx + ivec2(j, k);
					for (int p = car_light_map[idx.x][idx.y][0]; p < car_light_map[idx.x][idx.y][1]; p++)
					{
						mat4& car_light_mat = car_light_mats[p];
						for (const vec3& vertex : car_boundray)
						{
							vec4 v = car_light_mat * transform * vec4(vertex, 1.0f);
							v.x /= v.w;
							v.y /= v.w;
							if (-v.w < v.z && v.z < v.w && v.x * v.x + v.y * v.y < 1.0f)
							{
								num_lighting++;
								car_lightings[i][num_lighting] = p;
								break;
							}
						}
					}
				}
			}
			car_lightings[i][0] = num_lighting;
		}
	}

	if (is_view_updated)
	{
		glNamedBufferSubData(scene_UBO, offsetof(decltype(view), view_mat), sizeof(view.view_mat) + sizeof(view.inv_view_mat), &view.view_mat);
		glProgramUniform1f(SP_sun, glGetUniformLocation(SP_sun, "horizionY"), horizon_y);
	}

	glNamedBufferSubData(scene_UBO, scene_UBO_offset1, 4 * sizeof(vec4), &sun);
	if (sun.dir.z > 0)
	{
		glNamedBufferSubData(shadow_UBO, 0, sizeof(sun_shadow), &sun_shadow);
		glNamedBufferSubData(car_transform_VBO, 0, num_visible_cars * sizeof(mat4), logical_data.car_transform);
		glNamedBufferSubData(car_color_VBO, 0, num_visible_cars * sizeof(vec3), logical_data.car_color);
	}
	else
	{
		glNamedBufferSubData(car_light_map_SSBO, 0, sizeof(car_light_map), car_light_map);
		glNamedBufferSubData(car_light_pos_UBO, 0, num_visible_car_lights * sizeof(vec4), car_light_pos);
		glNamedBufferSubData(car_light_shadow_mat_UBO, 0, num_visible_car_lights * sizeof(mat4), car_light_mats);
		glNamedBufferSubData(car_lighting_SSBO, 0, num_visible_cars * sizeof(car_lightings[0]), car_lightings);
		glNamedBufferSubData(car_transform_VBO, 0, num_visible_light_on_cars * sizeof(mat4), logical_data.car_transform);
		glNamedBufferSubData(car_color_VBO, 0, num_visible_light_on_cars * sizeof(vec3), logical_data.car_color);
		glNamedBufferSubData(car_transform_VBO, num_visible_light_on_cars * sizeof(mat4), (num_visible_cars - num_visible_light_on_cars) * sizeof(mat4), &logical_data.car_transform[logical_data.num_light_on_cars]);
		glNamedBufferSubData(car_color_VBO, num_visible_light_on_cars * sizeof(vec3), (num_visible_cars - num_visible_light_on_cars) * sizeof(vec3), &logical_data.car_color[logical_data.num_light_on_cars]);
		glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "numLightOnCars"), num_visible_light_on_cars);
	}
	if (sun.dir.z > -0.2f)
	{
		mat3 sun_mat = rotate(acos(sun.dir.z), vec3(-sun.dir.y, sun.dir.x, 0));
		glProgramUniformMatrix3fv(SP_sun, glGetUniformLocation(SP_sun, "sunMat"), 1, GL_FALSE, (GLfloat*)&sun_mat);
	}

	glBindTextureUnit(0, highway_tex);
	glBindTextureUnit(1, shadow_tex);
	glEnable(GL_DEPTH_TEST);
	if (sun.dir.z > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
		glUseProgram(SP_shadow_highway_day);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_shadow_car_day);
		glBindVertexArray(car_shadow_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, CAR_SHADOW_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, multisample_render_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&sun.sky_color);
		glClearBufferfv(GL_COLOR, 1, (GLfloat*)&COLOR_BLACK);
		glViewport(0, 0, window_width, window_height);
		glUseProgram(SP_highway_day);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindVertexArray(ground_VAO);
		glDisable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, GROUND_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_car_day);
		glBindVertexArray(car_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, CAR_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadow_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		for (int i = 0; i < 4; i++)
		{
			glEnable(GL_CLIP_DISTANCE0 + i);
		}
		glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
		glUseProgram(SP_shadow_highway_night);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_shadow_car_night);
		glBindVertexArray(car_shadow_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, CAR_SHADOW_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
		for (int i = 0; i < 4; i++)
		{
			glDisable(GL_CLIP_DISTANCE0 + i);
		}
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, multisample_render_FBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&sun.sky_color);
		glClearBufferfv(GL_COLOR, 1, (GLfloat*)&COLOR_BLACK);
		glViewport(0, 0, window_width, window_height);
		glUseProgram(SP_highway_night);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindVertexArray(ground_VAO);
		glDisable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, GROUND_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(bridge_VAO);
		glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
		glUseProgram(SP_car_night);
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glBindVertexArray(car_VAO);
		glDrawElementsInstanced(GL_TRIANGLES, CAR_EBO_SIZE, GL_UNSIGNED_INT, 0, num_visible_cars);
	}
	if (sun.dir.z > -0.2f)
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
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBindTextureUnit(0, render_tex);
	glBindTextureUnit(1, bloom_texs[0]);
	glUseProgram(SP_buffer_to_screen);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_FRAMEBUFFER_SRGB);

	glBindVertexArray(0);
	glUseProgram(0);

	static float fps = 60;
	fps = (fps + 1) / (frame_dt_us * 1e-6 + 1.0f);
	glWindowPos2i(10, 10);
	char str[40];
	sprintf_s(str, 40, "fps: %d|%d", int(fps), int(tick_rate));
	glutBitmapString(GLUT_BITMAP_8_BY_13, (unsigned char*)str);
	glutSwapBuffers();
}

void lineSegment()
{
	glutIdleFunc(drawGraphics);
}

void onReshape(GLint width, GLint height)
{
	window_width = width;
	window_height = height;
	need_update_view = true;

	GLint max_tex_size;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);
	bloom_buffer_width = fmin(BLOOM_BUFFER_HEIGHT * (width + 1) / (height + 1), max_tex_size);
	view.proj_mat = perspective(FOVY, float(width) / height, VIEW_Z_NEAR, VIEW_Z_FAR);
	glNamedBufferSubData(scene_UBO, 0, sizeof(view.proj_mat), &view.proj_mat);

	int MSAA_level = 8;
	for (int i = 0; i < 2; i++)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, multisample_render_RBOs[i]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_level, GL_RGB16F, width, height);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, depth_RBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_level, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, render_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, bloom_texs[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, bloom_buffer_width, BLOOM_BUFFER_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, bloom_texs[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, BLOOM_BUFFER_HEIGHT, bloom_buffer_width, 0, GL_RGB, GL_FLOAT, nullptr);
}

void onKeyDown(GLubyte key, GLint x, GLint y)
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
		simulate_speed = key - '0';
		return;
	case 'a':
		focus_move_dir |= 0b0001;
		break;
	case 'd':
		focus_move_dir |= 0b0010;
		break;
	case 'w':
		focus_move_dir |= 0b0100;
		break;
	case 's':
		focus_move_dir |= 0b1000;
		break;
	default:
		return;
	}
	need_update_view = true;
}

void onKeyUp(GLubyte key, GLint x, GLint y)
{
	switch (key)
	{
	case 'a':
		focus_move_dir &= ~(0b0001);
		break;
	case 'd':
		focus_move_dir &= ~(0b0010);
		break;
	case 'w':
		focus_move_dir &= ~(0b0100);
		break;
	case 's':
		focus_move_dir &= ~(0b1000);
		break;
	}
}

void onMouseWheel(GLint button, GLint dir, GLint x, GLint y)
{
	float view_distance;
	if (dir > 0)
	{
		view_distance = aim_view_distance * 0.91f;
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

void rotateView(GLint x_mouse, GLint y_mouse)
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
		SetCursorPos(glutGet(GLUT_WINDOW_X) + window_width / 2, glutGet(GLUT_WINDOW_Y) + window_height / 2);
	}
}

void onMouseButton(GLint button, GLint action, GLint x_mouse, GLint y_mouse)
{
	if (button == GLUT_MIDDLE_BUTTON)
	{
		if (action == GLUT_DOWN)
		{
			SetCursorPos(glutGet(GLUT_WINDOW_X) + window_width / 2, glutGet(GLUT_WINDOW_Y) + window_height / 2);
			glutSetCursor(GLUT_CURSOR_NONE);
			glutMotionFunc(rotateView);
		}
		else if (action == GLUT_UP)
		{
			glutMotionFunc(0);
			glutSetCursor(0);
		}
	}
}

int main(int argc, char** argv)
{
	initLogic();
	std::thread logicalThread(logicalFrame);
	ImmDisableIME(GetCurrentThreadId());
	glutInit(&argc, argv);
	//glutInitContextVersion(4, 6);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(2520, 1350);
	glutCreateWindow("");
	glutDisplayFunc(lineSegment);
	glutReshapeFunc(onReshape);
	glutKeyboardFunc(onKeyDown);
	glutKeyboardUpFunc(onKeyUp);
	glutMouseFunc(onMouseButton);
	glutMouseWheelFunc(onMouseWheel);
	init();
	glutMainLoop();
}