#pragma comment(lib,"imm32.lib")

#include "resource.h"
#include "scene.h"
#include "mesh.h"
#include "common.h"
#include "Car.h"
#include "logical_frame.h"

#include "glew.h"
#include "freeglut.h"
#include "gtx/transform.hpp"

#include <time.h>
#include <thread>

using namespace glm;

#define SHADOW_TEX_SIZE 4096
#define MAX_CSM_RATIO 3.6f
#define CSM_LEVELS 4
GLuint shadowFBO;
GLuint shadowTex;

#define FOVY (PI / 4)
#define VIEW_Z_NEAR 90.0f
#define VIEW_Z_FAR 90000.0f
#define FOCUS_HEIGHT 200
#define MIN_VIEW_DISTANCE 200
#define MAX_VIEW_DISTANCE 25000
#define MIN_SHADOW_FAR 369.5f

GLint windowWidth, windowHeight;

float aimAzimuth = 0.3f, aimRelativeDepression = 0.1f, aimViewDistance = 15000;
float azimuth = aimAzimuth, relativeDepression = aimRelativeDepression, viewDistance = aimViewDistance;
vec3 focus(0);
char focusMoveDir = 0;
bool needUpdateView = true;
clock_t lastFrameTime;

#define MAX_HEIGHT 500
#define HEIGHT_MAP_SIZE 256
constexpr RECT HEIGHT_MAP_AREA = { -10240, 12800, 10240, -7680 };
float heightMap[HEIGHT_MAP_SIZE][HEIGHT_MAP_SIZE];

GLuint multiSampleRenderFBO;
GLuint multiSampleRenderRBO[2];
GLuint depthRBO;

GLuint renderFBO;
GLuint renderTex;

#define BLOOM_BUFFER_HEIGHT 512
int bloomBufferWidth;
GLuint bloomFBOs[2];
GLuint bloomTexs[2];

GLuint texBlitVAO, texMappingVBO;


/*
* binding = 0
*	mat4 projectionMat
*	mat4 viewMat
*	mat4 invViewMat
*
* binding = 1
*	vec4 sunDirection
*	vec4 sunAmbient
*	vec4 sunDiffuseAndSpecular
*	vec4 skyColor
*/
GLuint sceneUBO;
GLintptr sceneUBOoffset1 = 256;

/*
* binding = 2
*	mat4 shadowMat[CSMLevels];
*	mat4 shadowTexMat[CSMLevels];
*/
GLuint shadowUBO;

#define LIGHT_MAP_SIZE 128
#define LIGHT_MAP_GRID_LENGTH (CAR_LIGHT_RANGE / 2)
// binding = 3
GLuint carLightMapSSBO;
// binding = 4
GLuint carLightPosUBO;
// binding = 5
GLuint carLightShadowMatUBO;
// binding = 6
GLuint carLightingSSBO;

struct
{
	mat4 projMat;
	mat4 viewMat;
	mat4 invViewMat;
}view;
float horizonY;
vec3 CSMAreas[CSM_LEVELS][12];
struct
{
	mat4 mat[CSM_LEVELS];
	mat4 texMat[CSM_LEVELS];
}sunShadow;
float carLightMapGridDistanceToView[LIGHT_MAP_SIZE][LIGHT_MAP_SIZE];
ivec2 carLightMapGridDistanceOrder[LIGHT_MAP_SIZE * LIGHT_MAP_SIZE];
int numActiveCarLightMapGrids = 0;
const mat4 carLightShadowProjMat = perspective(2 * acos(CAR_LIGHT_V_COS_ANGLE - 0.001f), CAR_LIGHT_ASPECT, 50.0f, 50.0f + 2 * LIGHT_MAP_GRID_LENGTH);

int carLightMap[LIGHT_MAP_SIZE][LIGHT_MAP_SIZE][2];
vec4 carLightPos[2 * MAX_CAR_COUNT];
mat4 carLightMats[2 * MAX_CAR_COUNT];
int carLightings[MAX_CAR_COUNT][24];

GLuint spHighwayDay;
GLuint spHighwayNight;
GLuint spCarDay;
GLuint spCarNight;
GLuint spSun;
GLuint spShadowHighwayDay;
GLuint spShadowHighwayNight;
GLuint spShadowCarDay;
GLuint spShadowCarNight;
GLuint spTexBlit;
GLuint spGaussianBlur;
GLuint spBuffer2Screen;

std::mt19937 rdEng;

GLuint loadShader(GLuint shaderID, GLenum type)
{
	HRSRC rc = FindResource(nullptr, MAKEINTRESOURCE(shaderID), L"SHADER");
	if (rc == nullptr)
	{
		printf("ERROR: Can't Load Resource %d\n", shaderID);
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
		printf("ERROR: Shader %d Compilation Error\n", shaderID);
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
	GLuint vsHighway = loadShader(IDR_VS_HIGHWAY, GL_VERTEX_SHADER);
	GLuint fsHighwayDay = loadShader(IDR_FS_HIGHWAY_DAY, GL_FRAGMENT_SHADER);
	GLuint fsHighwayNight = loadShader(IDR_FS_HIGHWAY_NIGHT, GL_FRAGMENT_SHADER);
	spHighwayDay = linkShaderProgram(vsHighway, fsHighwayDay);
	spHighwayNight = linkShaderProgram(vsHighway, fsHighwayNight);
	glDeleteShader(vsHighway);
	glDeleteShader(fsHighwayDay);
	glDeleteShader(fsHighwayNight);
	glProgramUniform1f(spHighwayDay, glGetUniformLocation(spHighwayDay, "fogDensity"), 0.000016f);
	glProgramUniform1f(spHighwayNight, glGetUniformLocation(spHighwayNight, "fogDensity"), 0.000016f);

	GLuint vsCar = loadShader(IDR_VS_CAR, GL_VERTEX_SHADER);
	GLuint fsCarDay = loadShader(IDR_FS_CAR_DAY, GL_FRAGMENT_SHADER);
	GLuint fsCarNight = loadShader(IDR_FS_CAR_NIGHT, GL_FRAGMENT_SHADER);
	spCarDay = linkShaderProgram(vsCar, fsCarDay);
	spCarNight = linkShaderProgram(vsCar, fsCarNight);
	glDeleteShader(vsCar);
	glDeleteShader(fsCarDay);
	glDeleteShader(fsCarNight);
	glProgramUniform1f(spCarDay, glGetUniformLocation(spCarDay, "fogDensity"), 0.000016f);
	glProgramUniform1f(spCarNight, glGetUniformLocation(spCarNight, "fogDensity"), 0.000016f);

	GLuint vsSun = loadShader(IDR_VS_SUN, GL_VERTEX_SHADER);
	GLuint fsSun = loadShader(IDR_FS_SUN, GL_FRAGMENT_SHADER);
	spSun = linkShaderProgram(vsSun, fsSun);
	glDeleteShader(vsSun);
	glDeleteShader(fsSun);

	GLuint vsShadowHighwayDay = loadShader(IDR_VS_SHADOW_HIGHWAY_DAY, GL_VERTEX_SHADER);
	GLuint vsShadowHighwayNight = loadShader(IDR_VS_SHADOW_HIGHWAY_NIGHT, GL_VERTEX_SHADER);
	GLuint vsShadowCarDay = loadShader(IDR_VS_SHADOW_CAR_DAY, GL_VERTEX_SHADER);
	GLuint vsShadowCarNight = loadShader(IDR_VS_SHADOW_CAR_NIGHT, GL_VERTEX_SHADER);
	GLuint fsShadow = loadShader(IDR_FS_SHADOW, GL_FRAGMENT_SHADER);
	GLuint gsShadowDay = loadShader(IDR_GS_SHADOW_DAY, GL_GEOMETRY_SHADER);
	GLuint gsShadowHighwayNight = loadShader(IDR_GS_SHADOW_HIGHWAY_NIGHT, GL_GEOMETRY_SHADER);
	GLuint gsShadowCarNight = loadShader(IDR_GS_SHADOW_CAR_NIGHT, GL_GEOMETRY_SHADER);
	spShadowHighwayDay = linkShaderProgram(vsShadowHighwayDay, fsShadow, gsShadowDay);
	spShadowHighwayNight = linkShaderProgram(vsShadowHighwayNight, fsShadow, gsShadowHighwayNight);
	spShadowCarDay = linkShaderProgram(vsShadowCarDay, fsShadow, gsShadowDay);
	spShadowCarNight = linkShaderProgram(vsShadowCarNight, fsShadow, gsShadowCarNight);
	glDeleteShader(vsShadowHighwayDay);
	glDeleteShader(vsShadowHighwayNight);
	glDeleteShader(gsShadowHighwayNight);
	glDeleteShader(vsShadowCarDay);
	glDeleteShader(gsShadowDay);
	glDeleteShader(vsShadowCarNight);
	glDeleteShader(gsShadowCarNight);
	glDeleteShader(fsShadow);

	GLuint vsTexBlit = loadShader(IDR_VS_TEX_BLIT, GL_VERTEX_SHADER);
	GLuint fsTexBlit = loadShader(IDR_FS_TEX_BLIT, GL_FRAGMENT_SHADER);
	GLuint fsGaussianBlur = loadShader(IDR_FS_GAUSSIAN_BLUR, GL_FRAGMENT_SHADER);
	GLuint fsBuffer2Screen = loadShader(IDR_FS_BUFFER_TO_SCREEN, GL_FRAGMENT_SHADER);
	spTexBlit = linkShaderProgram(vsTexBlit, fsTexBlit);
	spGaussianBlur = linkShaderProgram(vsTexBlit, fsGaussianBlur);
	spBuffer2Screen = linkShaderProgram(vsTexBlit, fsBuffer2Screen);
	glDeleteShader(vsTexBlit);
	glDeleteShader(fsTexBlit);
	glDeleteShader(fsGaussianBlur);
	glDeleteShader(fsBuffer2Screen);
}

void buildHeightMap()
{
	glEnable(GL_DEPTH_TEST);
	constexpr int heightBorder = 3;
	constexpr int filterRadius = 3;
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
	glClear(GL_DEPTH_BUFFER_BIT);
	mat4 heightMat = ortho(float(HEIGHT_MAP_AREA.left), float(HEIGHT_MAP_AREA.right), float(HEIGHT_MAP_AREA.bottom), float(HEIGHT_MAP_AREA.top), 0.0f, float(MAX_HEIGHT)) *
		lookAt(vec3(0, 0, MAX_HEIGHT), vec3(0, 0, 0), vec3(0, 1, 0));
	vec4 v(FLT_MAX);
	glNamedBufferSubData(sceneUBO, sceneUBOoffset1, sizeof(vec4), &v);
	glNamedBufferSubData(shadowUBO, 0, sizeof(mat4), &heightMat);
	glUseProgram(spShadowHighwayDay);
	glBindVertexArray(bridgeVAO);
	glDrawElements(GL_TRIANGLES, bridgeEBOsize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(groundVAO);
	glDrawElements(GL_TRIANGLES, groundEBOsize, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	GLfloat(*depthData)[SHADOW_TEX_SIZE][SHADOW_TEX_SIZE] = new GLfloat[CSM_LEVELS][SHADOW_TEX_SIZE][SHADOW_TEX_SIZE];
	glGetTextureImage(shadowTex, 0, GL_DEPTH_COMPONENT, GL_FLOAT, CSM_LEVELS * SHADOW_TEX_SIZE * SHADOW_TEX_SIZE * sizeof(GLfloat), depthData);
	float(*heightMapBuffer)[HEIGHT_MAP_SIZE][HEIGHT_MAP_SIZE] = new float[2][HEIGHT_MAP_SIZE][HEIGHT_MAP_SIZE];
	constexpr int filterSize = 2 * filterRadius + 1;
	int heightFilter[filterSize][filterSize];
	heightFilter[0][0] = 1;
	for (int i = 1; i < filterSize; i++)
	{
		heightFilter[0][i] = heightFilter[0][i - 1] * (filterSize - i) / i;
	}
	for (int i = 1; i < filterSize; i++)
	{
		heightFilter[i][0] = heightFilter[i - 1][0] * (filterSize - i) / i;
		for (int j = 1; j < filterSize; j++)
		{
			heightFilter[i][j] = heightFilter[i][0] * heightFilter[0][j];
		}
	}
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < HEIGHT_MAP_SIZE; i++)
		{
			for (int j = 0; j < HEIGHT_MAP_SIZE; j++)
			{
				float minDepth = 1.0f;
				for (int p = 0; p < SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE; p++)
				{
					for (int q = 0; q < SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE; q++)
					{
						if (minDepth > depthData[0][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * i + p][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * j + q])
						{
							minDepth = depthData[0][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * i + p][SHADOW_TEX_SIZE / HEIGHT_MAP_SIZE * j + q];
						}
					}
				}
				heightMapBuffer[0][i][j] = (1 - minDepth) * MAX_HEIGHT;
			}
		}
#pragma omp for
		for (int i = 0; i < HEIGHT_MAP_SIZE; i++)
		{
			for (int j = 0; j < HEIGHT_MAP_SIZE; j++)
			{
				float maxHeight = 0.0f;
				for (int p = -heightBorder; p <= heightBorder; p++)
				{
					if (0 <= i + p && i + p < HEIGHT_MAP_SIZE)
					{
						for (int q = -heightBorder; q <= heightBorder; q++)
						{
							if (0 <= j + q && j + q < HEIGHT_MAP_SIZE && maxHeight < heightMapBuffer[0][i + p][j + q])
							{
								maxHeight = heightMapBuffer[0][i + p][j + q];
							}
						}
					}
				}
				heightMapBuffer[1][i][j] = maxHeight;
			}
		}
#pragma omp for
		for (int i = 0; i < HEIGHT_MAP_SIZE; i++)
		{
			for (int j = 0; j < HEIGHT_MAP_SIZE; j++)
			{
				float height = 0;
				int factor = 0;
				for (int p = -filterRadius; p <= filterRadius; p++)
				{
					if (0 <= i + p && i + p < HEIGHT_MAP_SIZE)
					{
						for (int q = -filterRadius; q <= filterRadius; q++)
						{
							if (0 <= j + q && j + q < HEIGHT_MAP_SIZE)
							{
								height += heightFilter[p + filterRadius][q + filterRadius] * heightMapBuffer[1][i + p][j + q];
								factor += heightFilter[p + filterRadius][q + filterRadius];
							}
						}
					}
				}
				height /= factor;
				heightMap[i][j] = height;
			}
		}
	}
	delete[] depthData;
	delete[] heightMapBuffer;
}

void init()
{
	glewInit();
	printf("%s\n", (char*)glGetString(GL_VERSION));
	printf("%s\n", (char*)glGetString(GL_SHADING_LANGUAGE_VERSION));
	initShader();
	std::random_device rd;
	rdEng.seed(rd());

	for (int i = 0; i < LIGHT_MAP_SIZE; i++)
	{
		for (int j = 0; j < LIGHT_MAP_SIZE; j++)
		{
			carLightMapGridDistanceOrder[i * LIGHT_MAP_SIZE + j] = ivec2(i, j);
		}
	}

	int UBOOffsetAlignment;
	glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &UBOOffsetAlignment);

	sceneUBOoffset1 = ((sizeof(view) - 1) / UBOOffsetAlignment + 1) * UBOOffsetAlignment;
	glGenBuffers(1, &sceneUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, sceneUBO);
	glBufferData(GL_UNIFORM_BUFFER, sceneUBOoffset1 + 4 * sizeof(vec4), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 0, sceneUBO, 0, sizeof(view));
	glBindBufferRange(GL_UNIFORM_BUFFER, 1, sceneUBO, sceneUBOoffset1, 4 * sizeof(vec4));

	glGenBuffers(1, &shadowUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, shadowUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(sunShadow), nullptr, GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 2, shadowUBO, 0, sizeof(sunShadow));
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenBuffers(1, &carLightMapSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, carLightMapSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(carLightMap), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 3, carLightMapSSBO, 0, sizeof(carLightMap));
	glGenBuffers(1, &carLightPosUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, carLightPosUBO);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(carLightPos), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 4, carLightPosUBO, 0, sizeof(carLightPos));
	glGenBuffers(1, &carLightShadowMatUBO);
	glBindBuffer(GL_UNIFORM_BUFFER, carLightShadowMatUBO);
	glBufferData(GL_UNIFORM_BUFFER, 2 * MAX_CAR_COUNT * sizeof(mat4), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, 5, carLightShadowMatUBO, 0, 2 * MAX_CAR_COUNT * sizeof(mat4));
	glGenBuffers(1, &carLightingSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, carLightingSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(carLightings), nullptr, GL_STREAM_DRAW);
	glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 6, carLightingSSBO, 0, sizeof(carLightings));

	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glGenFramebuffers(1, &multiSampleRenderFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, multiSampleRenderFBO);
	glGenRenderbuffers(2, multiSampleRenderRBO);
	for (int i = 0; i < 2; i++)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, multiSampleRenderRBO[i]);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, multiSampleRenderRBO[i]);
	}
	glGenRenderbuffers(1, &depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRBO);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);

	glGenFramebuffers(1, &renderFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, renderFBO);
	glGenTextures(1, &renderTex);
	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 3);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, renderTex, 0);

	glGenFramebuffers(2, bloomFBOs);
	glGenTextures(2, bloomTexs);
	for (int i = 0; i < 2; i++)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, bloomFBOs[i]);
		glBindTexture(GL_TEXTURE_2D, bloomTexs[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, bloomTexs[i], 0);
	}

	vec2 screenCroods[4] = { { 1, 1 },{ -1, 1 }, { -1, -1 }, { 1, -1 } };
	glGenVertexArrays(1, &texBlitVAO);
	glBindVertexArray(texBlitVAO);
	glGenBuffers(1, &texMappingVBO);
	glBindBuffer(GL_ARRAY_BUFFER, texMappingVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screenCroods), screenCroods, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D_ARRAY, shadowTex);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE, CSM_LEVELS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowTex, 0);
	glBindTextureUnit(1, shadowTex);

	initScene();
	buildHeightMap();
	initLogic();
}

void drawGraphics()
{
	clock_t time = clock();
	uint32_t frameTime = time - lastFrameTime;
	lastFrameTime = time;
	LogicalData& logical_data = getLatestLogicalData();
	bool isViewUpdated = needUpdateView;
	if (needUpdateView)
	{
		needUpdateView = false;
		if (azimuth != aimAzimuth)
		{
			constexpr float rotateSpeed = 0.01f;
			float maxRotateAngle = rotateSpeed * frameTime;
			if (abs(azimuth - aimAzimuth) > maxRotateAngle)
			{
				if (azimuth > aimAzimuth)
				{
					azimuth -= maxRotateAngle;
				}
				else
				{
					azimuth += maxRotateAngle;
				}
				needUpdateView = true;
			}
			else
			{
				aimAzimuth -= int(aimAzimuth / (2 * PI)) * (2 * PI);
				azimuth = aimAzimuth;
			}
		}
		if (relativeDepression != aimRelativeDepression)
		{
			constexpr float rotateSpeed = 0.005f;
			float maxRotateAngle = rotateSpeed * frameTime;
			if (abs(relativeDepression - aimRelativeDepression) > maxRotateAngle)
			{
				if (relativeDepression > aimRelativeDepression)
				{
					relativeDepression -= maxRotateAngle;
				}
				else
				{
					relativeDepression += maxRotateAngle;
				}
				needUpdateView = true;
			}
			else
			{
				relativeDepression = aimRelativeDepression;
			}
		}
		if (viewDistance != aimViewDistance)
		{
			constexpr float zoomSpeed = 0.005f;
			float maxZoomDistance = zoomSpeed * viewDistance * frameTime;
			if (abs(viewDistance - aimViewDistance) > maxZoomDistance)
			{
				if (viewDistance > aimViewDistance)
				{
					viewDistance -= maxZoomDistance;
				}
				else
				{
					viewDistance += maxZoomDistance;
				}
				needUpdateView = true;
			}
			else
			{
				viewDistance = aimViewDistance;
			}
		}
		if (focusMoveDir != 0)
		{
			float moveSpeed = 0.001f * viewDistance + 1.0f;
			float moveDistance = moveSpeed * frameTime;
			switch (focusMoveDir)
			{
			case 'a':
				focus.x -= cos(azimuth) * moveDistance;
				focus.y -= sin(azimuth) * moveDistance;
				break;
			case 'd':
				focus.x += cos(azimuth) * moveDistance;
				focus.y += sin(azimuth) * moveDistance;
				break;
			case 'w':
				focus.x -= sin(azimuth) * moveDistance;
				focus.y += cos(azimuth) * moveDistance;
				break;
			case 's':
				focus.x += sin(azimuth) * moveDistance;
				focus.y -= cos(azimuth) * moveDistance;
				break;
			}
			focus.x = clamp(focus.x, -30000.0f, 30000.0f);
			focus.y = clamp(focus.y, -10000.0f, 20000.0f);
			needUpdateView = true;
		}

		float depression = PI / 2 * (1 - FLT_EPSILON) * (1 - ((1 - 0.3f * viewDistance / MAX_VIEW_DISTANCE) * (1 - relativeDepression)));
		vec3 viewDir(-cos(depression) * sin(azimuth), cos(depression) * cos(azimuth), -sin(depression));
		focus.z = FOCUS_HEIGHT;
		vec3 eye = focus - viewDistance * viewDir;
		if (eye.z < FOCUS_HEIGHT + MAX_HEIGHT)
		{
			vec2 heightMapCoord((eye.x - HEIGHT_MAP_AREA.left) / (HEIGHT_MAP_AREA.right - HEIGHT_MAP_AREA.left) * HEIGHT_MAP_SIZE - 0.5f,
				(eye.y - HEIGHT_MAP_AREA.bottom) / (HEIGHT_MAP_AREA.top - HEIGHT_MAP_AREA.bottom) * HEIGHT_MAP_SIZE - 0.5f);
			if (0 < heightMapCoord.x && heightMapCoord.x < HEIGHT_MAP_SIZE - 1 && 0 < heightMapCoord.y && heightMapCoord.y < HEIGHT_MAP_SIZE - 1)
			{
				float heightOffset = heightMap[int(heightMapCoord.y)][int(heightMapCoord.x)] * (int(heightMapCoord.x) + 1 - heightMapCoord.x) * (int(heightMapCoord.y) + 1 - heightMapCoord.y) +
					heightMap[int(heightMapCoord.y)][int(heightMapCoord.x) + 1] * (heightMapCoord.x - int(heightMapCoord.x)) * (int(heightMapCoord.y) + 1 - heightMapCoord.y) +
					heightMap[int(heightMapCoord.y) + 1][int(heightMapCoord.x)] * (int(heightMapCoord.x) + 1 - heightMapCoord.x) * (heightMapCoord.y - int(heightMapCoord.y)) +
					heightMap[int(heightMapCoord.y) + 1][int(heightMapCoord.x) + 1] * (heightMapCoord.x - int(heightMapCoord.x)) * (heightMapCoord.y - int(heightMapCoord.y));
				heightOffset *= 1 - (eye.z - FOCUS_HEIGHT) / (MAX_HEIGHT);
				eye.z += heightOffset;
				focus.z += heightOffset;
			}
		}
		view.viewMat = lookAt(eye, focus, vec3(-sin(azimuth), cos(azimuth), 0));
		view.invViewMat = inverse(view.viewMat);
		vec3 topView(0, tanf(FOVY / 2), -1);
		vec3 bottomView(0, -topView.y, -1);
		vec3 viewX = vec3(topView.y / windowHeight * windowWidth, 0, 0);
		topView = mat3(view.invViewMat) * topView;
		bottomView = mat3(view.invViewMat) * bottomView;
		viewX = mat3(view.invViewMat) * viewX;
		horizonY = (tan(depression - acos(EARTH_RADIUS / (focus.z + EARTH_RADIUS))) / tan(FOVY / 2) + 1) * windowHeight / 2;

		if (eye.z > MAX_HEIGHT)
		{
			float shadowNear = fmax(-(eye.z - MAX_HEIGHT) / bottomView.z, VIEW_Z_NEAR);
			float shadowFar = fmax(shadowNear, MIN_SHADOW_FAR);
			float bottomFar = -eye.z / bottomView.z;
			float topNear = VIEW_Z_FAR;
			float topFar = VIEW_Z_FAR;
			if (topView.z < 0)
			{
				topNear = fmin(-(eye.z - MAX_HEIGHT) / topView.z, VIEW_Z_FAR);
				topFar = fmin(-eye.z / topView.z, VIEW_Z_FAR);
			}
			float CSMRatio = fmin(pow(topFar / shadowFar, 1.0f / CSM_LEVELS), MAX_CSM_RATIO);
			vec3 shadowHexa[6];
			shadowHexa[4] = shadowNear * bottomView;
			shadowHexa[5] = shadowHexa[4];
			for (int i = 0; i < CSM_LEVELS; i++)
			{
				shadowFar *= CSMRatio;
				shadowHexa[0] = shadowHexa[4];
				shadowHexa[1] = shadowHexa[5];
				if (shadowNear < bottomFar && bottomFar < shadowFar)
				{
					shadowHexa[2] = bottomFar * bottomView;
				}
				else
				{
					shadowHexa[2] = shadowHexa[0];
				}
				if (shadowNear < topNear && topNear < shadowFar)
				{
					shadowHexa[3] = topNear * topView;
				}
				else
				{
					shadowHexa[3] = shadowHexa[0];
				}
				if (topFar < shadowFar)
				{
					shadowHexa[4] = topView * topFar;
					shadowHexa[5] = shadowHexa[4];
				}
				else
				{
					if (bottomFar < shadowFar)
					{
						shadowHexa[4] = shadowFar * bottomView - (eye.z + shadowFar * bottomView.z) / (viewDir.z - bottomView.z) * (viewDir - bottomView);
					}
					else
					{
						shadowHexa[4] = shadowFar * bottomView;
					}
					if (topNear > shadowFar)
					{
						shadowHexa[5] = shadowFar * topView + (eye.z + shadowFar * topView.z - MAX_HEIGHT) / (topView.z - viewDir.z) * (viewDir - topView);
					}
					else
					{
						shadowHexa[5] = shadowFar * topView;
					}
				}
				shadowNear = shadowFar;
				for (int j = 0; j < 6; j++)
				{
					float distance = dot(shadowHexa[j], viewDir);
					CSMAreas[i][2 * j] = eye + shadowHexa[j] + distance * viewX;
					CSMAreas[i][2 * j + 1] = eye + shadowHexa[j] - distance * viewX;
				}
			}
		}
		else
		{
			float shadowNear = VIEW_Z_NEAR;
			float bottom = -eye.z / bottomView.z;
			float shadowFar = MIN_SHADOW_FAR;
			if (topView.z > 0)
			{
				float top = fmin((MAX_HEIGHT - eye.z) / topView.z, VIEW_Z_FAR);
				float CSMRatio = fmin(pow(VIEW_Z_FAR / shadowFar, 1.0f / CSM_LEVELS), MAX_CSM_RATIO);
				vec3 shadowHexa[6];
				shadowHexa[4] = shadowNear * bottomView;
				if (top < shadowNear)
				{
					shadowHexa[5] = shadowNear * topView - (eye.z + shadowNear * topView.z - MAX_HEIGHT) / (viewDir.z - topView.z) * (viewDir - topView);
				}
				else
				{
					shadowHexa[5] = shadowNear * topView;
				}
				for (int i = 0; i < CSM_LEVELS; i++)
				{
					shadowFar *= CSMRatio;
					shadowHexa[0] = shadowHexa[4];
					shadowHexa[1] = shadowHexa[5];
					if (shadowNear < bottom && bottom < shadowFar)
					{
						shadowHexa[2] = bottom * bottomView;
					}
					else
					{
						shadowHexa[2] = shadowHexa[0];
					}
					if (shadowNear < top && top < shadowFar)
					{
						shadowHexa[3] = top * topView;
					}
					else
					{
						shadowHexa[3] = shadowHexa[0];
					}
					if (bottom < shadowFar)
					{
						shadowHexa[4] = shadowFar * bottomView - (eye.z + shadowFar * bottomView.z) / (viewDir.z - bottomView.z) * (viewDir - bottomView);
					}
					else
					{
						shadowHexa[4] = shadowFar * bottomView;
					}
					if (top < shadowFar)
					{
						shadowHexa[5] = shadowFar * topView - (eye.z + shadowFar * topView.z - MAX_HEIGHT) / (viewDir.z - topView.z) * (viewDir - topView);
					}
					else
					{
						shadowHexa[5] = shadowFar * topView;
					}
					shadowNear = shadowFar;
					for (int j = 0; j < 6; j++)
					{
						float distance = dot(shadowHexa[j], viewDir);
						CSMAreas[i][2 * j] = eye + shadowHexa[j] + distance * viewX;
						CSMAreas[i][2 * j + 1] = eye + shadowHexa[j] - distance * viewX;
					}
				}
			}
			else
			{
				float top = VIEW_Z_FAR;
				if (topView.z < 0)
				{
					top = fmin(-eye.z / topView.z, VIEW_Z_FAR);
				}
				float CSMRatio = fmin(pow(top / shadowFar, 1.0f / CSM_LEVELS), MAX_CSM_RATIO);
				vec3 shadowHexa[6];
				shadowHexa[4] = shadowNear * bottomView;
				shadowHexa[5] = shadowNear * topView;
				for (int i = 0; i < CSM_LEVELS; i++)
				{
					shadowFar *= CSMRatio;
					shadowHexa[0] = shadowHexa[4];
					shadowHexa[1] = shadowHexa[5];
					if (shadowNear < bottom && bottom < shadowFar)
					{
						shadowHexa[2] = bottom * bottomView;
					}
					else
					{
						shadowHexa[2] = shadowHexa[0];
					}
					shadowHexa[3] = shadowHexa[0];
					if (top < shadowFar)
					{
						shadowHexa[4] = topView * top;
						shadowHexa[5] = shadowHexa[4];
					}
					else
					{
						if (bottom < shadowFar)
						{
							shadowHexa[4] = shadowFar * bottomView - (eye.z + shadowFar * bottomView.z) / (viewDir.z - bottomView.z) * (viewDir - bottomView);
						}
						else
						{
							shadowHexa[4] = shadowFar * bottomView;
						}
						shadowHexa[5] = shadowFar * topView;
					}
					shadowNear = shadowFar;
					for (int j = 0; j < 6; j++)
					{
						float distance = dot(shadowHexa[j], viewDir);
						CSMAreas[i][2 * j] = eye + shadowHexa[j] + distance * viewX;
						CSMAreas[i][2 * j + 1] = eye + shadowHexa[j] - distance * viewX;
					}
				}
			}
		}

		mat4 projAndViewMat = view.projMat * view.viewMat;
		bool isLightGridVisible[LIGHT_MAP_SIZE][LIGHT_MAP_SIZE];
		for (int i = 0; i < LIGHT_MAP_SIZE; i++)
		{
			float offsetX = (-LIGHT_MAP_SIZE / 2 + i) * LIGHT_MAP_GRID_LENGTH;
			for (int j = 0; j < LIGHT_MAP_SIZE; j++)
			{
				float offsetY = (-LIGHT_MAP_SIZE / 2 + j) * LIGHT_MAP_GRID_LENGTH;
				constexpr vec4 gridAABB[8] = { vec4(-LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, 0, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, 0, 1),
					vec4(-LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, 0, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, 0, 1),
					vec4(-LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, -LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1),
					vec4(-LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1), vec4(2 * LIGHT_MAP_GRID_LENGTH, 2 * LIGHT_MAP_GRID_LENGTH, MAX_HEIGHT, 1) };
				int left = 0, right = 0, bottom = 0, top = 0, back = 0, front = 0;
				for (vec4 vert : gridAABB)
				{
					vert.x += offsetX;
					vert.y += offsetY;
					vert = projAndViewMat * vert;
					left += vert.x < -vert.w;
					right += vert.x > vert.w;
					bottom += vert.y < -vert.w;
					top += vert.y > vert.w;
					back += vert.z < -vert.w;
					front += vert.z > vert.w;
				}
				isLightGridVisible[i][j] = left != 8 && right != 8 && bottom != 8 && top != 8 && back != 8 && front != 8;
			}
		}
		numActiveCarLightMapGrids = 0;
		for (int i = 0; i < LIGHT_MAP_SIZE; i++)
		{
			float offsetX = (-LIGHT_MAP_SIZE / 2 + 0.5f + i) * LIGHT_MAP_GRID_LENGTH;
			for (int j = 0; j < LIGHT_MAP_SIZE; j++)
			{
				float offsetY = (-LIGHT_MAP_SIZE / 2 + 0.5f + j) * LIGHT_MAP_GRID_LENGTH;
				if (isLightGridVisible[i][j])
				{
					vec2 distance = vec2(eye) - vec2(offsetX, offsetY);
					carLightMapGridDistanceToView[i][j] = dot(distance, distance);
					numActiveCarLightMapGrids++;
				}
				else
				{
					carLightMapGridDistanceToView[i][j] = FLT_MAX;
				}
			}
		}
		std::sort(carLightMapGridDistanceOrder, &carLightMapGridDistanceOrder[LIGHT_MAP_SIZE * LIGHT_MAP_SIZE], [](ivec2 a, ivec2 b)->bool
			{
				return carLightMapGridDistanceToView[a.x][a.y] < carLightMapGridDistanceToView[b.x][b.y];
			});
	}

	struct
	{
		vec4 dir;
		vec4 ambient;
		vec4 diffuseAndSpecular;
		vec4 skyColor;
	}sun;
	sun.dir = vec4(logical_data.sunDir, 0);
	sun.ambient = vec4(vec3(0.01f), 0);
	constexpr float atomosphere = EARTH_RADIUS + 2e6f;
	float absorbFactor = -1e-7f * (-EARTH_RADIUS * sun.dir.z + sqrt(atomosphere * atomosphere - EARTH_RADIUS * EARTH_RADIUS * (1 - sun.dir.z * sun.dir.z)));
	sun.diffuseAndSpecular = vec4(0.5f * vec3(exp(0.2f * absorbFactor), exp(0.3f * absorbFactor), exp(1.1f * absorbFactor)), 0);
	sun.skyColor = vec4(0.01f, 0.015f, 0.055f, 0);
	if (sun.dir.z > -0.2f)
	{
		sun.ambient += vec4(vec3((sun.dir.z + 0.2f) * 0.1f), 0);
		sun.skyColor += vec4((sun.dir.z + 0.2f) * vec3(0.2f, 0.3f, 1.1f), 0);
	}

	int numVisibleCars;
	int numVisibleLightOnCars;
	int numVisibleCarLights;

	if (sun.dir.z > 0)
	{
		numVisibleCars = logical_data.numCars;
		mat4 sunMat = lookAt(vec3(0.0f), -vec3(sun.dir), vec3(-sun.dir.x, -sun.dir.y, sun.dir.z));
		float zFar = FLT_MAX;
		float slope = sqrt(1.0f / (sun.dir.z * sun.dir.z) - 1.0f);
		float maxX[CSM_LEVELS], minX[CSM_LEVELS], maxY[CSM_LEVELS], minY[CSM_LEVELS], zFars[CSM_LEVELS];
		for (int i = CSM_LEVELS - 1; i >= 0; i--)
		{
			minX[i] = FLT_MAX, maxX[i] = -FLT_MAX, minY[i] = FLT_MAX, maxY[i] = -FLT_MAX;
			for (int j = 0; j < 12; j++)
			{
				vec3 point = vec3(sunMat * vec4(CSMAreas[i][j], 1.0f));
				if (point.z < zFar)
				{
					zFar = point.z;
				}
				if (point.x < minX[i])
				{
					minX[i] = point.x;
				}
				if (point.x > maxX[i])
				{
					maxX[i] = point.x;
				}
				if (point.y < minY[i])
				{
					minY[i] = point.y;
				}
				if (point.y > maxY[i])
				{
					maxY[i] = point.y;
				}
			}
			zFar = fmax(-maxY[i] * slope, zFar);
			zFars[i] = zFar;
		}
		for (int i = 0; i < CSM_LEVELS - 3; i += 4)
		{
			float xMin = FLT_MAX, xMax = -FLT_MAX, yMin = FLT_MAX, yMax = -FLT_MAX, furthestZFar = FLT_MAX;
			for (int j = 0; j < 4; j++)
			{
				if (zFars[4 * i + j] < furthestZFar)
				{
					furthestZFar = zFars[4 * i + j];
				}
				if (minX[4 * i + j] < xMin)
				{
					xMin = minX[4 * i + j];
				}
				if (maxX[4 * i + j] > xMax)
				{
					xMax = maxX[4 * i + j];
				}
				if (minY[4 * i + j] < yMin)
				{
					yMin = minY[4 * i + j];
				}
				if (maxY[4 * i + j] > yMax)
				{
					yMax = maxY[4 * i + j];
				}
			}
			if (4 * (maxX[4 * i] - minX[4 * i]) * (maxY[4 * i] - minY[4 * i]) > (xMax - xMin) * (yMax - yMin))
			{
				minX[4 * i] = xMin;
				maxX[4 * i] = (xMin + xMax) / 2;
				minY[4 * i] = yMin;
				maxY[4 * i] = (xMin + yMax) / 2;
				zFars[4 * i] = furthestZFar;
				minX[4 * i + 1] = (xMin + xMax) / 2;
				maxX[4 * i + 1] = xMax;
				minY[4 * i + 1] = yMin;
				maxY[4 * i + 1] = (xMin + yMax) / 2;
				zFars[4 * i + 1] = furthestZFar;
				minX[4 * i + 2] = xMin;
				maxX[4 * i + 2] = (xMin + xMax) / 2;
				minY[4 * i + 2] = (xMin + yMax) / 2;
				maxY[4 * i + 2] = yMax;
				zFars[4 * i + 2] = furthestZFar;
				minX[4 * i + 3] = (xMin + xMax) / 2;
				maxX[4 * i + 3] = xMax;
				minY[4 * i + 3] = (xMin + yMax) / 2;
				maxY[4 * i + 3] = yMax;
				zFars[4 * i + 3] = furthestZFar;
			}
		}
		for (int i = 0; i < CSM_LEVELS; i++)
		{
			float zNear = fmin(MAX_HEIGHT / sun.dir.z - minY[i] * slope, 5 * VIEW_Z_FAR + zFars[i]);
			constexpr float texPadding = (1 / (1 - 2 * 0.02f) - 1) / 2;
			mat4 shadowMat = ortho(minX[i] - texPadding * (maxX[i] - minX[i]), maxX[i] + texPadding * (maxX[i] - minX[i]),
				minY[i] - texPadding * (maxY[i] - minY[i]), maxY[i] + texPadding * (maxY[i] - minY[i]),
				-zNear, -zFars[i]);
			shadowMat = shadowMat * sunMat;
			sunShadow.mat[i] = shadowMat;
			sunShadow.texMat[i] = mat4(
				0.5f, 0.0f, 0.0f, 0.0f,
				0.0f, 0.5f, 0.0f, 0.0f,
				0.0f, 0.0f, 0.5f, 0.0f,
				0.5f, 0.5f, 0.5f, 1.0f) * shadowMat;
		}
	}
	else
	{
		numVisibleCars = 0;
		numVisibleLightOnCars = 0;
		numVisibleCarLights = 0;
		memset(carLightMap, 0, sizeof(carLightMap));
		struct CarLightInfo
		{
			vec4 pos;
			vec4 dir;
			int* lightMapGrid;
		};
		std::vector<CarLightInfo>carLightInfos;
		for (int i = 0, j = logical_data.numLightOnCars; i < j; i++)
		{
			mat4& modelMat = logical_data.carModelMat[i];
			ivec2 lightMapIdx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(modelMat[3]) + 0.5f * LIGHT_MAP_SIZE);
			if (carLightMapGridDistanceToView[lightMapIdx.x][lightMapIdx.y] == FLT_MAX)
			{
				for (j--; i < j; j--)
				{
					mat4& backModelMat = logical_data.carModelMat[j];
					ivec2 backLightMapIdx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(backModelMat[3]) + 0.5f * LIGHT_MAP_SIZE);
					if (carLightMapGridDistanceToView[backLightMapIdx.x][backLightMapIdx.y] < FLT_MAX)
					{
						std::swap(modelMat, backModelMat);
						std::swap(logical_data.carColor[i], logical_data.carColor[j]);
						break;
					}
				}
				if (i == j)
				{
					break;
				}
			}
			numVisibleCars++;
			numVisibleLightOnCars++;
		}
		for (int i = logical_data.numLightOnCars, j = logical_data.numCars; i < j; i++)
		{
			mat4& modelMat = logical_data.carModelMat[i];
			ivec2 lightMapIdx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(modelMat[3]) + 0.5f * LIGHT_MAP_SIZE);
			if (carLightMapGridDistanceToView[lightMapIdx.x][lightMapIdx.y] == FLT_MAX)
			{
				for (j--; i < j; j--)
				{
					mat4& backModelMat = logical_data.carModelMat[j];
					ivec2 backLightMapIdx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(backModelMat[3]) + 0.5f * LIGHT_MAP_SIZE);
					if (carLightMapGridDistanceToView[backLightMapIdx.x][backLightMapIdx.y] < FLT_MAX)
					{
						std::swap(modelMat, backModelMat);
						std::swap(logical_data.carColor[i], logical_data.carColor[j]);
						break;
					}
				}
				if (i == j)
				{
					break;
				}
			}
			numVisibleCars++;
		}
		int numCarLights = 2 * logical_data.numLightOnCars;
		for (int i = 0; i < numCarLights; i++)
		{
			constexpr float posOffset = 50.0f / LIGHT_MAP_GRID_LENGTH + 1.0f;
			vec4 lightPos = logical_data.carLightPos[i];
			vec4 lightDir = logical_data.carLightDir[i];
			ivec2 lightMapIdx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(lightPos) + posOffset * vec2(lightDir) + 0.5f * LIGHT_MAP_SIZE);
			if (carLightMapGridDistanceToView[lightMapIdx.x][lightMapIdx.y] < FLT_MAX)
			{
				carLightInfos.emplace_back(CarLightInfo{ lightPos, lightDir, carLightMap[lightMapIdx.x][lightMapIdx.y] });
				carLightMap[lightMapIdx.x][lightMapIdx.y][0]++;
			}
		}
		for (int i = 0; i < numActiveCarLightMapGrids; i++)
		{
			ivec2& idx = carLightMapGridDistanceOrder[i];
			carLightMap[idx.x][idx.y][1] = numVisibleCarLights;
			numVisibleCarLights += carLightMap[idx.x][idx.y][0];
			carLightMap[idx.x][idx.y][0] = carLightMap[idx.x][idx.y][1];
		}
		for (CarLightInfo& carLightInfo : carLightInfos)
		{
			int idx = carLightInfo.lightMapGrid[1]++;
			carLightPos[idx] = carLightInfo.pos;
			carLightMats[idx] = carLightShadowProjMat * lookAt(vec3(carLightInfo.pos), vec3(carLightInfo.pos) + vec3(carLightInfo.dir), vec3(0.0f, 0.0f, 1.0f));
		}
		for (int i = 0; i < numVisibleCars; i++)
		{
			mat4& modelMat = logical_data.carModelMat[i < numVisibleLightOnCars ? i : i + logical_data.numLightOnCars - numVisibleLightOnCars];
			ivec2 posIdx = ivec2(1.0f / LIGHT_MAP_GRID_LENGTH * vec2(modelMat[3]) + LIGHT_MAP_SIZE / 2.0f);
			int numLighting = 0;
			for (int j = -1; j < 2; j++)
			{
				for (int k = -1; k < 2; k++)
				{
					ivec2 idx = posIdx + ivec2(j, k);
					for (int p = carLightMap[idx.x][idx.y][0]; p < carLightMap[idx.x][idx.y][1]; p++)
					{
						mat4& carLightMat = carLightMats[p];
						for (const vec3& vertex : carBoundray)
						{
							vec4 v = carLightMat * modelMat * vec4(vertex, 1.0f);
							v.x /= v.w;
							v.y /= v.w;
							if (-v.w < v.z && v.z < v.w && v.x * v.x + v.y * v.y < 1.0f)
							{
								numLighting++;
								carLightings[i][numLighting] = p;
								break;
							}
						}
					}
				}
			}
			carLightings[i][0] = numLighting;
		}
	}

	if (isViewUpdated)
	{
		glNamedBufferSubData(sceneUBO, offsetof(decltype(view), viewMat), sizeof(view.viewMat) + sizeof(view.invViewMat), &view.viewMat);
		glProgramUniform1f(spSun, glGetUniformLocation(spSun, "horizionY"), horizonY);
	}

	glNamedBufferSubData(sceneUBO, sceneUBOoffset1, 4 * sizeof(vec4), &sun);
	if (sun.dir.z > 0)
	{
		glNamedBufferSubData(shadowUBO, 0, sizeof(sunShadow), &sunShadow);
		glNamedBufferSubData(carMatVBO, 0, numVisibleCars * sizeof(mat4), logical_data.carModelMat);
		glNamedBufferSubData(carColorVBO, 0, numVisibleCars * sizeof(vec3), logical_data.carColor);
	}
	else
	{
		glNamedBufferSubData(carLightMapSSBO, 0, sizeof(carLightMap), carLightMap);
		glNamedBufferSubData(carLightPosUBO, 0, numVisibleCarLights * sizeof(vec4), carLightPos);
		glNamedBufferSubData(carLightShadowMatUBO, 0, numVisibleCarLights * sizeof(mat4), carLightMats);
		glNamedBufferSubData(carLightingSSBO, 0, numVisibleCars * sizeof(carLightings[0]), carLightings);
		glNamedBufferSubData(carMatVBO, 0, numVisibleLightOnCars * sizeof(mat4), logical_data.carModelMat);
		glNamedBufferSubData(carColorVBO, 0, numVisibleLightOnCars * sizeof(vec3), logical_data.carColor);
		glNamedBufferSubData(carMatVBO, numVisibleLightOnCars * sizeof(mat4), (numVisibleCars - numVisibleLightOnCars) * sizeof(mat4), &logical_data.carModelMat[logical_data.numLightOnCars]);
		glNamedBufferSubData(carColorVBO, numVisibleLightOnCars * sizeof(vec3), (numVisibleCars - numVisibleLightOnCars) * sizeof(vec3), &logical_data.carColor[logical_data.numLightOnCars]);
		glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "numLightOnCars"), numVisibleLightOnCars);
	}
	if (sun.dir.z > -0.2f)
	{
		mat3 sunMat = rotate(acos(sun.dir.z), vec3(-sun.dir.y, sun.dir.x, 0));
		glProgramUniformMatrix3fv(spSun, glGetUniformLocation(spSun, "sunMat"), 1, GL_FALSE, (GLfloat*)&sunMat);
	}

	glBindTextureUnit(0, HIGHWAY_TEX);
	glBindTextureUnit(1, shadowTex);
	glEnable(GL_DEPTH_TEST);
	if (sun.dir.z > 0)
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
		glUseProgram(spShadowHighwayDay);
		glBindVertexArray(bridgeVAO);
		glDrawElements(GL_TRIANGLES, bridgeEBOsize, GL_UNSIGNED_INT, 0);
		glUseProgram(spShadowCarDay);
		glBindVertexArray(carShadowVAO);
		glDrawElementsInstanced(GL_TRIANGLES, carShadowEBOsize, GL_UNSIGNED_INT, 0, numVisibleCars);
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, multiSampleRenderFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&sun.skyColor);
		glClearBufferfv(GL_COLOR, 1, (GLfloat*)&COLOR_BLACK);
		glViewport(0, 0, windowWidth, windowHeight);
		glUseProgram(spHighwayDay);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindVertexArray(groundVAO);
		glDisable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, groundEBOsize, GL_UNSIGNED_INT, 0);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(bridgeVAO);
		glDrawElements(GL_TRIANGLES, bridgeEBOsize, GL_UNSIGNED_INT, 0);
		glUseProgram(spCarDay);
		glBindVertexArray(carVAO);
		glDrawElementsInstanced(GL_TRIANGLES, carEBOsize, GL_UNSIGNED_INT, 0, numVisibleCars);
	}
	else
	{
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glDisable(GL_CULL_FACE);
		for (int i = 0; i < 4; i++)
		{
			glEnable(GL_CLIP_DISTANCE0 + i);
		}
		glViewport(0, 0, SHADOW_TEX_SIZE, SHADOW_TEX_SIZE);
		glUseProgram(spShadowHighwayNight);
		glBindVertexArray(bridgeVAO);
		glDrawElements(GL_TRIANGLES, bridgeEBOsize, GL_UNSIGNED_INT, 0);
		glUseProgram(spShadowCarNight);
		glBindVertexArray(carShadowVAO);
		glDrawElementsInstanced(GL_TRIANGLES, carShadowEBOsize, GL_UNSIGNED_INT, 0, numVisibleCars);
		for (int i = 0; i < 4; i++)
		{
			glDisable(GL_CLIP_DISTANCE0 + i);
		}
		glEnable(GL_CULL_FACE);

		glBindFramebuffer(GL_FRAMEBUFFER, multiSampleRenderFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glClearBufferfv(GL_COLOR, 0, (GLfloat*)&sun.skyColor);
		glClearBufferfv(GL_COLOR, 1, (GLfloat*)&COLOR_BLACK);
		glViewport(0, 0, windowWidth, windowHeight);
		glUseProgram(spHighwayNight);
		glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBindVertexArray(groundVAO);
		glDisable(GL_DEPTH_TEST);
		glDrawElements(GL_TRIANGLES, groundEBOsize, GL_UNSIGNED_INT, 0);
		glEnable(GL_DEPTH_TEST);
		glBindVertexArray(bridgeVAO);
		glDrawElements(GL_TRIANGLES, bridgeEBOsize, GL_UNSIGNED_INT, 0);
		glUseProgram(spCarNight);
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glBindVertexArray(carVAO);
		glDrawElementsInstanced(GL_TRIANGLES, carEBOsize, GL_UNSIGNED_INT, 0, numVisibleCars);
	}
	if (sun.dir.z > -0.2f)
	{
		glUseProgram(spSun);
		GLuint attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
		glDrawBuffers(2, attachments);
		glBindVertexArray(sunVAO);
		glDrawArrays(GL_TRIANGLE_FAN, 0, sunVBOsize);
	}

	glDisable(GL_DEPTH_TEST);
	glReadBuffer(GL_COLOR_ATTACHMENT1);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloomFBOs[0]);
	glBindVertexArray(texBlitVAO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, bloomBufferWidth, BLOOM_BUFFER_HEIGHT);
	glBindTextureUnit(0, renderTex);
	glGenerateMipmap(GL_TEXTURE_2D);
	glUseProgram(spTexBlit);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloomFBOs[1]);
	glUseProgram(spGaussianBlur);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, BLOOM_BUFFER_HEIGHT, bloomBufferWidth);
	glBindTextureUnit(0, bloomTexs[0]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, bloomFBOs[0]);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, bloomBufferWidth, BLOOM_BUFFER_HEIGHT);
	glBindTextureUnit(0, bloomTexs[1]);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, multiSampleRenderFBO);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, renderFBO);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0, windowWidth, windowHeight, 0, 0, windowWidth, windowHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, windowWidth, windowHeight);
	glEnable(GL_FRAMEBUFFER_SRGB);
	glBindTextureUnit(0, renderTex);
	glBindTextureUnit(1, bloomTexs[0]);
	glUseProgram(spBuffer2Screen);
	glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	glDisable(GL_FRAMEBUFFER_SRGB);

	glBindVertexArray(0);
	glUseProgram(0);

	static float fps = 60;
	fps = (fps + 1) / (frameTime * 0.001f + 1.0f);
	glWindowPos2i(10, 10);
	char str[40];
	sprintf_s(str, 40, "fps: %d|%d", int(fps), int(tickRate));
	glutBitmapString(GLUT_BITMAP_8_BY_13, (unsigned char*)str);
	glutSwapBuffers();
}

void lineSegment()
{
	glutIdleFunc(drawGraphics);
}

void winReshapeFunc(GLint width, GLint height)
{
	windowWidth = width;
	windowHeight = height;
	needUpdateView = true;

	GLint maxTexSize;
	glGetIntegerv(GL_MAX_TEXTURE_SIZE, &maxTexSize);
	bloomBufferWidth = fmin(BLOOM_BUFFER_HEIGHT * (width + 1) / (height + 1), maxTexSize);
	view.projMat = perspective(FOVY, float(width) / height, VIEW_Z_NEAR, VIEW_Z_FAR);
	glNamedBufferSubData(sceneUBO, 0, sizeof(view.projMat), &view.projMat);

	int MSAA_level = 8;
	for (int i = 0; i < 2; i++)
	{
		glBindRenderbuffer(GL_RENDERBUFFER, multiSampleRenderRBO[i]);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_level, GL_RGB16F, width, height);
	}
	glBindRenderbuffer(GL_RENDERBUFFER, depthRBO);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_level, GL_DEPTH_COMPONENT, width, height);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindTexture(GL_TEXTURE_2D, renderTex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, bloomTexs[0]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, bloomBufferWidth, BLOOM_BUFFER_HEIGHT, 0, GL_RGB, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, bloomTexs[1]);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, BLOOM_BUFFER_HEIGHT, bloomBufferWidth, 0, GL_RGB, GL_FLOAT, nullptr);
}

void winKeyboardFunc(GLubyte key, GLint x, GLint y)
{
	switch (key)
	{
	case ' ':
		isPaused = !isPaused;
		return;
	case '=':
		if (simulateSpeed < 5)
		{
			simulateSpeed++;
		}
		return;
	case '-':
		if (simulateSpeed > 1)
		{
			simulateSpeed--;
		}
		return;
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
		simulateSpeed = key - '0';
		return;
	case 'a':
	case 'd':
	case 'w':
	case 's':
		focusMoveDir = key;
		break;
	default:
		return;
	}
	needUpdateView = true;
}

void winKeyboardUpFunc(GLubyte key, GLint x, GLint y)
{
	switch (key)
	{
	case 'a':
	case 'd':
	case 'w':
	case 's':
		focusMoveDir = 0;
		break;
	}
}

void mouseWheel(GLint button, GLint dir, GLint x, GLint y)
{
	float viewDistance;
	if (dir > 0)
	{
		viewDistance = aimViewDistance * 0.91f;
		if (viewDistance < MIN_VIEW_DISTANCE)
		{
			viewDistance = MIN_VIEW_DISTANCE;
		}
	}
	else
	{
		viewDistance = aimViewDistance * 1.1f;
		if (viewDistance > MAX_VIEW_DISTANCE)
		{
			viewDistance = MAX_VIEW_DISTANCE;
		}
	}
	if (aimViewDistance != viewDistance)
	{
		aimViewDistance = viewDistance;
		needUpdateView = true;
	}
}

void rotateView(GLint xMouse, GLint yMouse)
{
	if (xMouse != windowWidth / 2 || yMouse != windowHeight / 2)
	{
		float azimuth = aimAzimuth + 0.001f * (windowWidth / 2 - xMouse);
		float relativeDepression = aimRelativeDepression - 0.001f * (windowHeight / 2 - yMouse);
		if (relativeDepression > 1)
		{
			relativeDepression = 1;
		}
		if (relativeDepression < 0)
		{
			relativeDepression = 0;
		}
		if (aimAzimuth != azimuth || aimRelativeDepression != relativeDepression)
		{
			aimAzimuth = azimuth;
			aimRelativeDepression = relativeDepression;
			needUpdateView = true;
		}
		SetCursorPos(glutGet(GLUT_WINDOW_X) + windowWidth / 2, glutGet(GLUT_WINDOW_Y) + windowHeight / 2);
	}
}

void winMouseFunc(GLint button, GLint action, GLint xMouse, GLint yMouse)
{
	if (button == GLUT_MIDDLE_BUTTON)
	{
		if (action == GLUT_DOWN)
		{
			SetCursorPos(glutGet(GLUT_WINDOW_X) + windowWidth / 2, glutGet(GLUT_WINDOW_Y) + windowHeight / 2);
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
	ImmDisableIME(GetCurrentThreadId());
	glutInit(&argc, argv);
	glutInitContextVersion(4, 6);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowPosition(10, 0);
	glutInitWindowSize(1900, 1000);
	glutCreateWindow("");
	init();
	std::thread logicalThread(logicalFrame);
	glutDisplayFunc(lineSegment);
	glutReshapeFunc(winReshapeFunc);
	glutKeyboardFunc(winKeyboardFunc);
	glutKeyboardUpFunc(winKeyboardUpFunc);
	glutMouseFunc(winMouseFunc);
	glutMouseWheelFunc(mouseWheel);
	lastFrameTime = clock();
	glutMainLoop();
}