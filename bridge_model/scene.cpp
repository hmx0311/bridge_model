#include "scene.h"

#include "glad/glad.h"
#include "gtx/transform.hpp"

#include "common.h"
#include "resource.h"
#include "terrain.h"
#include "mesh.h"
#include "shader.h"

#include "shader_headers/camera_defines.h"

using namespace glm;

StaticQuadTree<SenceHeight, NUM_SCENE_GRID_LEVELS, NUM_SCENE_GRID_ROOTS_X, NUM_SCENE_GRID_ROOTS_Y> g_scene_quad_tree;

constexpr float HEIGHT_RANGE[2] = { -10.0f, 646.0f };
constexpr float MAX_HEIGHT_OFFSET = 2.0f;

void initScene()
{
	GLuint SP_height_map;
	{
		GLuint VS_height_map = loadShader(SHADER_NAME(IDR_VS_HIGHWAY), GL_VERTEX_SHADER);
		GLuint FS_height_map = loadShader(SHADER_NAME(IDR_FS_SHADOW), GL_FRAGMENT_SHADER);
		SP_height_map = linkShaderProgram(VS_height_map, FS_height_map);
		glDeleteShader(VS_height_map);
		glDeleteShader(FS_height_map);
	}

	GLuint height_map_FBO;
	glGenFramebuffers(1, &height_map_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, height_map_FBO);

	GLuint height_map_tex;
	glGenTextures(1, &height_map_tex);
	glBindTexture(GL_TEXTURE_2D, height_map_tex);
	glTextureStorage2D(height_map_tex, 1, GL_DEPTH_COMPONENT32, SCENE_GRID_SIZE_X, SCENE_GRID_SIZE_Y);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, height_map_tex, 0);
	glDrawBuffer(GL_NONE);

	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SCENE_GRID_SIZE_X, SCENE_GRID_SIZE_Y);
	CameraData camera;
	camera.projection = ortho(SCENE_GRID_AREA.x, SCENE_GRID_AREA.z, SCENE_GRID_AREA.y, SCENE_GRID_AREA.w, 0.0f, HEIGHT_RANGE[1] - HEIGHT_RANGE[0]);
	camera.view = lookAt(vec3(0, 0, HEIGHT_RANGE[1]), vec3(0, 0, 0), vec3(0, 1, 0));
	camera.inv_view = inverse(camera.view);
	mat4 height_mat = ortho(SCENE_GRID_AREA.x, SCENE_GRID_AREA.z, SCENE_GRID_AREA.y, SCENE_GRID_AREA.w, 0.0f, HEIGHT_RANGE[1] - HEIGHT_RANGE[0]) *
		lookAt(vec3(0, 0, HEIGHT_RANGE[1]), vec3(0, 0, 0), vec3(0, 1, 0));
	glNamedBufferSubData(scene_UBO, 0, sizeof(CameraData), &camera);
	glUseProgram(SP_height_map);
	updateTerrainLOD(1e10, vec3(0, 0, HEIGHT_RANGE[1]));
	glClear(GL_DEPTH_BUFFER_BIT);
	drawTerrainMesh();
	glBindVertexArray(bridge_VAO);
	glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
	glBindVertexArray(highway_VAO);
	glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	auto max_depth_data = std::make_unique<GLfloat[][SCENE_GRID_SIZE_X]>(SCENE_GRID_SIZE_Y);
	glGetTextureImage(height_map_tex, 0, GL_DEPTH_COMPONENT, GL_FLOAT, SCENE_GRID_SIZE_X * SCENE_GRID_SIZE_Y * sizeof(GLfloat), max_depth_data.get());
	glDepthFunc(GL_GREATER);
	glClearDepth(0.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	drawTerrainMesh();
	glBindVertexArray(bridge_VAO);
	glDrawElements(GL_TRIANGLES, BRIDGE_EBO_SIZE, GL_UNSIGNED_INT, 0);
	glBindVertexArray(highway_VAO);
	glDrawElements(GL_TRIANGLES, HIGHWAY_EBO_SIZE, GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
	auto min_depth_data = std::make_unique<GLfloat[][SCENE_GRID_SIZE_X]>(SCENE_GRID_SIZE_Y);
	glGetTextureImage(height_map_tex, 0, GL_DEPTH_COMPONENT, GL_FLOAT, SCENE_GRID_SIZE_X * SCENE_GRID_SIZE_Y * sizeof(GLfloat), min_depth_data.get());
	glDepthFunc(GL_LESS);
	glClearDepth(1.0);
	glDeleteTextures(1, &height_map_tex);
	glDeleteFramebuffers(1, &height_map_FBO);
	glDeleteProgram(SP_height_map);
#pragma omp parallel
	{
#pragma omp for
		for (int i = 0; i < SCENE_GRID_SIZE_Y; i++)
		{
			for (int j = 0; j < SCENE_GRID_SIZE_X; j++)
			{
				float min_height = HEIGHT_RANGE[1];
				float max_height = HEIGHT_RANGE[0];
				for (int m = std::max(0, i - 1); m <= std::min(SCENE_GRID_SIZE_Y - 1, i + 1); m++)
				{
					for (int n = std::max(0, j - 1); n <= std::min(SCENE_GRID_SIZE_X - 1, j + 1); n++)
					{
						min_height = std::min(min_height, HEIGHT_RANGE[0] + (1.0f - min_depth_data[m][n]) * (HEIGHT_RANGE[1] - HEIGHT_RANGE[0]));
						max_height = std::max(max_height, HEIGHT_RANGE[0] + (1.0f - max_depth_data[m][n]) * (HEIGHT_RANGE[1] - HEIGHT_RANGE[0]));
					}
				}
				g_scene_quad_tree[NUM_SCENE_GRID_LEVELS - 1][j][i].min_height = min_height;
				g_scene_quad_tree[NUM_SCENE_GRID_LEVELS - 1][j][i].max_height = max_height + MAX_HEIGHT_OFFSET;
			}
		}
		for (int k = NUM_SCENE_GRID_LEVELS - 2; k >= 0; k--)
		{
#pragma omp for
			for (int i = 0; i < g_scene_quad_tree.LevelSizeX(k); i++)
			{
				for (int j = 0; j < g_scene_quad_tree.LevelSizeY(k); j++)
				{
					float min_height = g_scene_quad_tree[k + 1][2 * i][2 * j].min_height;
					float max_height = g_scene_quad_tree[k + 1][2 * i][2 * j].max_height;
					min_height = std::min(min_height, g_scene_quad_tree[k + 1][2 * i + 1][2 * j].min_height);
					max_height = std::max(max_height, g_scene_quad_tree[k + 1][2 * i + 1][2 * j].max_height);
					min_height = std::min(min_height, g_scene_quad_tree[k + 1][2 * i][2 * j + 1].min_height);
					max_height = std::max(max_height, g_scene_quad_tree[k + 1][2 * i][2 * j + 1].max_height);
					min_height = std::min(min_height, g_scene_quad_tree[k + 1][2 * i + 1][2 * j + 1].min_height);
					max_height = std::max(max_height, g_scene_quad_tree[k + 1][2 * i + 1][2 * j + 1].max_height);
					g_scene_quad_tree[k][i][j].min_height = min_height;
					g_scene_quad_tree[k][i][j].max_height = max_height;
				}
			}
		}
	}
}