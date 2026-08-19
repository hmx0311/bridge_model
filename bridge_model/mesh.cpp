#include "mesh.h"

#include "ext/scalar_constants.hpp"

#include "common.h"

#include "shader_headers/scene_constances.h"

GLuint ground_VAO, ground_VBO, ground_EBO;
GLuint bridge_VAO, bridge_VBO, bridge_EBO;
GLuint car_VAO, car_VBO, car_EBO, car_transform_VBO, car_color_VBO;
GLuint car_shadow_VAO, car_shadow_VBO, car_shadow_EBO;
GLuint sun_VAO, sun_VBO;

using namespace glm;

void buildGroundMesh()
{
	constexpr int VERT_SIZE = 1334;
	vec3 positions[VERT_SIZE];
	vec3 normals[VERT_SIZE];
	vec2 tex_coords[VERT_SIZE];
	GLuint indices[GROUND_EBO_SIZE];

	positions[0] = vec3(-1050.0f, -950.0f, 0);
	positions[1] = vec3(1050.0f, -950.0f, 0);
	positions[2] = vec3(1050.0f, 950.0f, 0);
	positions[3] = vec3(-1050.0f, 950.0f, 0);
	for (int i = 0; i < 4; i++)
	{
		tex_coords[i] = vec2(0.09375f, 1.0f);
	}

	positions[4] = vec3(-1050.0f, -7.2f, 0);
	tex_coords[4] = vec2(0.63671875f, -1050.0f / 20.48f);
	positions[5] = vec3(1050.0f, -7.2f, 0);
	tex_coords[5] = vec2(0.63671875f, 1050.0f / 20.48f);
	positions[6] = vec3(1050.0f, 7.2f, 0);
	tex_coords[6] = vec2(0.98828125f, 1050.0f / 20.48f);
	positions[7] = vec3(-1050.0f, 7.2f, 0);
	tex_coords[7] = vec2(0.98828125f, -1050.0f / 20.48f);

	positions[8] = vec3(-7.2f, 128.0f, 0);
	tex_coords[8] = vec2(0.63671875f, 128.0f / 20.48f);
	positions[9] = vec3(7.2f, 128.0f, 0);
	tex_coords[9] = vec2(0.98828125f, 128.0f / 20.48f);
	positions[10] = vec3(7.2f, 950.0f, 0);
	tex_coords[10] = vec2(0.98828125f, 950.0f / 20.48f);
	positions[11] = vec3(-7.2f, 950.0f, 0);
	tex_coords[11] = vec2(0.63671875f, 950.0f / 20.48f);

	positions[12] = vec3(3.76f, 128.0f, 0);
	tex_coords[12] = vec2(0.59179687f, 0.5859375f);
	positions[13] = vec3(-3.76f, 128.0f, 0);
	tex_coords[13] = vec2(0.4082031f, 0.5859375f);
	positions[14] = vec3(-3.76f, 116.0f, 0);
	tex_coords[14] = vec2(0.4082031f, 0.0f);
	positions[15] = vec3(3.76f, 116.0f, 0);
	tex_coords[15] = vec2(0.59179687f, 0.0f);

	positions[16] = vec3(-124.8f, 10.4f, 0);
	tex_coords[16] = vec2(0.15478516f, 3.03125f);
	positions[17] = vec3(-184.8f, 10.4f, 0);
	tex_coords[17] = vec2(0.15478516f, 0.03125f);
	positions[18] = vec3(-184.8f, 6.5f, 0);
	tex_coords[18] = vec2(0.25f, 0.03125f);
	positions[19] = vec3(-124.8f, 6.5f, 0);
	tex_coords[19] = vec2(0.25f, 3.03125f);

	positions[20] = vec3(184.8f, 10.4f, 0);
	tex_coords[20] = vec2(0.15478516f, 0.03125f);
	positions[21] = vec3(124.8f, 10.4f, 0);
	tex_coords[21] = vec2(0.15478516f, 3.03125f);
	positions[22] = vec3(124.8f, 6.5f, 0);
	tex_coords[22] = vec2(0.25f, 3.03125f);
	positions[23] = vec3(184.8f, 6.5f, 0);
	tex_coords[23] = vec2(0.25f, 0.03125f);

	positions[24] = vec3(-30.0f, -10.4f, 0);
	tex_coords[24] = vec2(0.15478516f, 0.03125f);
	positions[25] = vec3(30.0f, -10.4f, 0);
	tex_coords[25] = vec2(0.15478516f, 3.03125f);
	positions[26] = vec3(30.0f, -6.5f, 0);
	tex_coords[26] = vec2(0.25f, 3.03125f);
	positions[27] = vec3(-30.0f, -6.5f, 0);
	tex_coords[27] = vec2(0.25f, 0.03125f);

	positions[28] = vec3(120.0f, -10.4f, 0);
	tex_coords[28] = vec2(0.15478516f, 3.03125f);
	positions[29] = vec3(180.0f, -10.4f, 0);
	tex_coords[29] = vec2(0.15478516f, 0.03125f);
	positions[30] = vec3(180.0f, -6.5f, 0);
	tex_coords[30] = vec2(0.25f, 0.03125f);
	positions[31] = vec3(120.0f, -6.5f, 0);
	tex_coords[31] = vec2(0.25f, 3.03125f);
	for (int i = 0; i < 8; i++)
	{
		indices[6 * i] = 4 * i;
		indices[6 * i + 1] = 4 * i + 1;
		indices[6 * i + 2] = 4 * i + 2;
		indices[6 * i + 3] = 4 * i + 0;
		indices[6 * i + 4] = 4 * i + 2;
		indices[6 * i + 5] = 4 * i + 3;
	}
	int i_vert = 32;
	int i_idx = 48;

	float theta = asin(12.0f / 37);
	int n = theta * sqrtf(2960) + 3;
	float dtheta = theta / n;
	n++;
	positions[i_vert] = vec3(-184.8f, 6.5f, 0);
	tex_coords[i_vert] = vec2(0.0f, 0.0f);
	positions[i_vert + 2 * n] = vec3(-204.0f, 6.5f, 0);
	tex_coords[i_vert + 2 * n] = vec2(0.0f, 19.2f / 20.48f);
	positions[i_vert + 2 * n + 1] = vec3(184.8f, 6.5f, 0);
	tex_coords[i_vert + 2 * n + 1] = vec2(0.0f, 0.0f);
	positions[i_vert + 4 * n + 1] = vec3(204.0f, 6.5f, 0);
	tex_coords[i_vert + 4 * n + 1] = vec2(0.0f, 19.2f / 20.48f);
	positions[i_vert + 4 * n + 2] = vec3(-30.0f, -6.5f, 0);
	tex_coords[i_vert + 4 * n + 2] = vec2(0.0f, 0.0f);
	positions[i_vert + 6 * n + 2] = vec3(-49.2f, -6.5f, 0);
	tex_coords[i_vert + 6 * n + 2] = vec2(0.0f, 19.2f / 20.48f);
	positions[i_vert + 6 * n + 3] = vec3(180.0f, -6.5f, 0);
	tex_coords[i_vert + 6 * n + 3] = vec2(0.0f, 0.0f);
	positions[i_vert + 8 * n + 3] = vec3(199.2f, -6.5f, 0);
	tex_coords[i_vert + 8 * n + 3] = vec2(0.0f, 19.2f / 20.48f);
	for (int i = 0; i < n; i++)
	{
		float x = 29.6f * sin(i * dtheta);
		float y = -25.7f + 29.6f * cos(i * dtheta);
		positions[i_vert + 1 + i] = vec3(-184.8f - x, 6.5f + y, 0);
		tex_coords[i_vert + 1 + i] = vec2(y / 40.96f, x / 20.48f);
		positions[i_vert + 2 * n + 2 + i] = vec3(184.8f + x, 6.5f + y, 0);
		tex_coords[i_vert + 2 * n + 2 + i] = vec2(y / 40.96f, x / 20.48f);
		positions[i_vert + 4 * n + 3 + i] = vec3(-30.0f - x, -6.5f - y, 0);
		tex_coords[i_vert + 4 * n + 3 + i] = vec2(y / 40.96f, x / 20.48f);
		positions[i_vert + 6 * n + 4 + i] = vec3(180.0f + x, -6.5f - y, 0);
		tex_coords[i_vert + 6 * n + 4 + i] = vec2(y / 40.96f, x / 20.48f);
	}
	for (int i = 1; i < n; i++)
	{
		float x = 19.2f - 29.6f * sin(theta - i * dtheta);
		float y = 30.3f - 29.6f * cos(theta - i * dtheta);
		positions[i_vert + n + i] = vec3(-184.8f - x, 6.5f + y, 0);
		tex_coords[i_vert + n + i] = vec2(y / 40.96f, x / 20.48f);
		positions[i_vert + 3 * n + 1 + i] = vec3(184.8f + x, 6.5f + y, 0);
		tex_coords[i_vert + 3 * n + 1 + i] = vec2(y / 40.96f, x / 20.48f);
		positions[i_vert + 5 * n + 2 + i] = vec3(-30.0f - x, -6.5f - y, 0);
		tex_coords[i_vert + 5 * n + 2 + i] = vec2(y / 40.96f, x / 20.48f);
		positions[i_vert + 7 * n + 3 + i] = vec3(180.0f + x, -6.5f - y, 0);
		tex_coords[i_vert + 7 * n + 3 + i] = vec2(y / 40.96f, x / 20.48f);
	}
	for (int i = 0; i < 2 * n - 1; i++)
	{
		indices[i_idx + 3 * i] = i_vert;
		indices[i_idx + 3 * i + 1] = i_vert + i + 1;
		indices[i_idx + 3 * i + 2] = i_vert + i + 2;
		indices[i_idx + 3 * (2 * n - 1 + i)] = i_vert + 2 * n + 1;
		indices[i_idx + 3 * (2 * n - 1 + i) + 1] = i_vert + 2 * n + 1 + i + 2;
		indices[i_idx + 3 * (2 * n - 1 + i) + 2] = i_vert + 2 * n + 1 + i + 1;
		indices[i_idx + 3 * (4 * n - 2 + i)] = i_vert + 4 * n + 2;
		indices[i_idx + 3 * (4 * n - 2 + i) + 1] = i_vert + 4 * n + 2 + i + 2;
		indices[i_idx + 3 * (4 * n - 2 + i) + 2] = i_vert + 4 * n + 2 + i + 1;
		indices[i_idx + 3 * (6 * n - 3 + i)] = i_vert + 6 * n + 3;
		indices[i_idx + 3 * (6 * n - 3 + i) + 1] = i_vert + 6 * n + 3 + i + 1;
		indices[i_idx + 3 * (6 * n - 3 + i) + 2] = i_vert + 6 * n + 3 + i + 2;
	}
	i_vert += 4 * (2 * n + 1);
	i_idx += 3 * 4 * (2 * n - 1);

	theta = pi<float>() / 2;
	n = theta * sqrtf(12160) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float sin_theta = sin(i * dtheta);
		float cos_theta = cos(i * dtheta);
		constexpr float O[2] = { -124.8f, 128.0f };
		float R = 121.6f;
		float r = 117.6f;
		float X = O[0] + R * cos_theta, Y = O[1] - R * sin_theta;
		float x = O[0] + r * cos_theta, y = O[1] - r * sin_theta;
		if (i < 0.07f * n)
		{
			if (i < 0.025f * n)
			{

				R -= 0.37f;
			}
			else
			{
				R -= (X + 3.65f) / cos_theta;
			}
			X = O[0] + R * cos_theta;
			Y = O[1] - R * sin_theta;
		}
		else if (i > 0.93f * n)
		{
			if (i > 0.973f * n)
			{
				R -= 0.37f;
			}
			else
			{
				R -= (6.85f - Y) / sin_theta;
			}
			X = O[0] + R * cos_theta;
			Y = O[1] - R * sin_theta;
		}
		positions[i_vert + 2 * i] = vec3(X, Y, 0);
		tex_coords[i_vert + 2 * i] = vec2(0.3125f + (R - 119.6f) / 40.96f, 119.6f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 2 * i + 1] = vec3(x, y, 0);
		tex_coords[i_vert + 2 * i + 1] = vec2(0.263671875f, 119.6f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 2 * n + 2 * i] = vec3(-x, y, 0);
		tex_coords[i_vert + 2 * n + 2 * i] = vec2(0.263671875f, 119.6f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 2 * n + 2 * i + 1] = vec3(-X, Y, 0);
		tex_coords[i_vert + 2 * n + 2 * i + 1] = vec2(0.3125f + (R - 119.6f) / 40.96f, 119.6f * theta / (20.48f * (n - 1)) * i);
	}
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[i_idx + 6 * j] = i_vert + 2 * j;
			indices[i_idx + 6 * j + 1] = i_vert + 2 * j + 1;
			indices[i_idx + 6 * j + 2] = i_vert + 2 * j + 3;
			indices[i_idx + 6 * j + 3] = i_vert + 2 * j;
			indices[i_idx + 6 * j + 4] = i_vert + 2 * j + 3;
			indices[i_idx + 6 * j + 5] = i_vert + 2 * j + 2;
		}
		i_vert += 2 * n;
		i_idx += 6 * (n - 1);
	}

	theta = acos(-5.0f / 13);
	n = theta * sqrtf(3000) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{

		float sin_theta = sin(i * dtheta);
		float cos_theta = cos(i * dtheta);
		constexpr float O[2] = { 30.0f, -36.4f };
		float R = 30.0f;
		float r = 26.0f;
		float X = O[0] + R * sin_theta, Y = O[1] + R * cos_theta;
		float x = O[0] + r * sin_theta, y = O[1] + r * cos_theta;
		if (i < 0.11f * n)
		{
			if (i < 0.034f * n)
			{
				R -= 0.37f;
			}
			else
			{
				R -= (6.85f + Y) / cos_theta;
			}
			X = O[0] + R * sin_theta;
			Y = O[1] + R * cos_theta;
		}
		positions[i_vert + 2 * i] = vec3(X, Y, 0);
		tex_coords[i_vert + 2 * i] = vec2(0.3125f + (R - 28.0f) / 40.96f, 28.0f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 2 * i + 1] = vec3(x, y, 0);
		tex_coords[i_vert + 2 * i + 1] = vec2(0.263671875f, 28.0f * theta / (20.48f * (n - 1)) * i);
	}
	for (int j = 0; j < n - 1; j++)
	{
		indices[i_idx + 6 * j] = i_vert + 2 * j;
		indices[i_idx + 6 * j + 1] = i_vert + 2 * j + 1;
		indices[i_idx + 6 * j + 2] = i_vert + 2 * j + 3;
		indices[i_idx + 6 * j + 3] = i_vert + 2 * j;
		indices[i_idx + 6 * j + 4] = i_vert + 2 * j + 3;
		indices[i_idx + 6 * j + 5] = i_vert + 2 * j + 2;
	}
	i_vert += 2 * n;
	i_idx += 6 * (n - 1);

	theta = asin(12.0f / 13);
	n = theta * sqrtf(6750) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float sin_theta = sin(i * dtheta);
		float cos_theta = cos(i * dtheta);
		constexpr float O[2] = { 120.0f, -73.9f };
		float R = 67.5f;
		float r = 63.5f;
		float X = O[0] - R * sin_theta, Y = O[1] + R * cos_theta;
		float x = O[0] - r * sin_theta, y = O[1] + r * cos_theta;
		if (i < 0.11f * n)
		{
			if (i < 0.049f * n)
			{
				R -= 0.37f;
			}
			else
			{
				R -= (6.85f + Y) / cos_theta;
			}
			X = O[0] - R * sin_theta;
			Y = O[1] + R * cos_theta;
		}
		positions[i_vert + 2 * i] = vec3(x, y, 0);
		tex_coords[i_vert + 2 * i] = vec2(0.263671875f, 65.5f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 2 * i + 1] = vec3(X, Y, 0);
		tex_coords[i_vert + 2 * i + 1] = vec2(0.3125f + (R - 65.5f) / 40.96f, 65.5f * theta / (20.48f * (n - 1)) * i);
	}
	for (int j = 0; j < n - 1; j++)
	{
		indices[i_idx + 6 * j] = i_vert + 2 * j;
		indices[i_idx + 6 * j + 1] = i_vert + 2 * j + 1;
		indices[i_idx + 6 * j + 2] = i_vert + 2 * j + 3;
		indices[i_idx + 6 * j + 3] = i_vert + 2 * j;
		indices[i_idx + 6 * j + 4] = i_vert + 2 * j + 3;
		indices[i_idx + 6 * j + 5] = i_vert + 2 * j + 2;
	}
	i_vert += 2 * n;
	i_idx += 6 * (n - 1);

	for (int i = 0; i < i_vert; i++)
	{
		normals[i] = vec3(0, 0, 1);
	}

	glGenVertexArrays(1, &ground_VAO);
	glBindVertexArray(ground_VAO);
	glGenBuffers(1, &ground_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, ground_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals) + sizeof(tex_coords), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(tex_coords), tex_coords);
	glGenBuffers(1, &ground_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ground_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(positions));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void buildBridgeMesh()
{
	constexpr int VERTICES_SIZE = 6422;
	vec3 positions[VERTICES_SIZE];
	vec3 normals[VERTICES_SIZE];
	vec2 tex_coords[VERTICES_SIZE];
	GLuint indices[BRIDGE_EBO_SIZE];

	int i_vert = 8;
	int i_idx = 12;
	positions[0] = vec3(-3.76f, 116.0f, 0);
	positions[1] = vec3(-4.0f, 116.0f, 0);
	positions[2] = vec3(-4.0f, 116.0f, 0.5f);
	positions[3] = vec3(-3.76f, 116.0f, 0.5f);
	positions[4] = vec3(4.0f, 116.0f, 0);
	positions[5] = vec3(3.76f, 116.0f, 0);
	positions[6] = vec3(3.76f, 116.0f, 0.5f);
	positions[7] = vec3(4.0f, 116.0f, 0.5f);
	for (int i = 0; i < 8; i++)
	{
		normals[i] = vec3(0, 1, 0);
	}
	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 0;
	indices[4] = 2;
	indices[5] = 3;
	indices[6] = 4;
	indices[7] = 5;
	indices[8] = 6;
	indices[9] = 4;
	indices[10] = 6;
	indices[11] = 7;
	float theta = asin(61.0f / 1861);
	int n = theta * sqrtf(37220) + 3;
	float dtheta = theta / n;
	n++;
	positions[i_vert] = vec3(-4.0f, 116.0f, 0);
	normals[i_vert] = vec3(-1, 0, 0);
	positions[i_vert + n + 1] = vec3(-4.0f, 103.8f, 0);
	normals[i_vert + n + 1] = vec3(-1, 0, 0);
	positions[i_vert + n + 2] = vec3(-3.76f, 116.0f, 0);
	normals[i_vert + n + 2] = vec3(1, 0, 0);
	positions[i_vert + 2 * n + 3] = vec3(-3.76f, 103.8f, 0);
	normals[i_vert + 2 * n + 3] = vec3(1, 0, 0);
	positions[i_vert + 2 * n + 4] = vec3(3.76f, 116.0f, 0);
	normals[i_vert + 2 * n + 4] = vec3(-1, 0, 0);
	positions[i_vert + 3 * n + 5] = vec3(3.76f, 103.8f, 0);
	normals[i_vert + 3 * n + 5] = vec3(-1, 0, 0);
	positions[i_vert + 3 * n + 6] = vec3(4.0f, 116.0f, 0);
	normals[i_vert + 3 * n + 6] = vec3(1, 0, 0);
	positions[i_vert + 4 * n + 7] = vec3(4.0f, 103.8f, 0);
	normals[i_vert + 4 * n + 7] = vec3(1, 0, 0);
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(i * dtheta);
		float sin_theta = sin(i * dtheta);
		float y = 116.0f - 372.2f * sin_theta;
		float z = 372.2f - 372.2f * cos_theta + 0.5f;
		positions[i_vert + 1 + i] = vec3(-4.0f, y, z);
		normals[i_vert + 1 + i] = vec3(-1, 0, 0);
		positions[i_vert + n + 3 + i] = vec3(-3.76f, y, z);
		normals[i_vert + n + 3 + i] = vec3(1, 0, 0);
		positions[i_vert + 2 * n + 5 + i] = vec3(3.76f, y, z);
		normals[i_vert + 2 * n + 5 + i] = vec3(-1, 0, 0);
		positions[i_vert + 3 * n + 7 + i] = vec3(4.0f, y, z);
		normals[i_vert + 3 * n + 7 + i] = vec3(1, 0, 0);
		positions[i_vert + 4 * n + 8 + i] = vec3(-4.0f, y, z);
		normals[i_vert + 4 * n + 8 + i] = vec3(0, sin_theta, cos_theta);
		positions[i_vert + 5 * n + 8 + i] = vec3(-3.76f, y, z);
		normals[i_vert + 5 * n + 8 + i] = vec3(0, sin_theta, cos_theta);
		positions[i_vert + 6 * n + 8 + i] = vec3(3.76f, y, z);
		normals[i_vert + 6 * n + 8 + i] = vec3(0, sin_theta, cos_theta);
		positions[i_vert + 7 * n + 8 + i] = vec3(4.0f, y, z);
		normals[i_vert + 7 * n + 8 + i] = vec3(0, sin_theta, cos_theta);
		positions[i_vert + 8 * n + 8 + i] = vec3(-3.76f, y, z - 0.5f);
		normals[i_vert + 8 * n + 8 + i] = vec3(0, sin_theta, cos_theta);
		tex_coords[i_vert + 8 * n + 8 + i] = vec2(0.4082031f, 372.2f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 9 * n + 8 + i] = vec3(3.76f, y, z - 0.5f);
		normals[i_vert + 9 * n + 8 + i] = vec3(0, sin_theta, cos_theta);
		tex_coords[i_vert + 9 * n + 8 + i] = vec2(0.59179687f, 372.2f * theta / (20.48f * (n - 1)) * i);
	}
	for (int i = 0; i < n; i++)
	{
		indices[i_idx + 3 * i] = i_vert;
		indices[i_idx + 3 * i + 1] = i_vert + i + 2;
		indices[i_idx + 3 * i + 2] = i_vert + i + 1;
		indices[i_idx + 3 * (n + i)] = i_vert + n + 2;
		indices[i_idx + 3 * (n + i) + 1] = i_vert + n + 2 + i + 1;
		indices[i_idx + 3 * (n + i) + 2] = i_vert + n + 2 + i + 2;
		indices[i_idx + 3 * (2 * n + i)] = i_vert + 2 * (n + 2);
		indices[i_idx + 3 * (2 * n + i) + 1] = i_vert + 2 * (n + 2) + i + 2;
		indices[i_idx + 3 * (2 * n + i) + 2] = i_vert + 2 * (n + 2) + i + 1;
		indices[i_idx + 3 * (3 * n + i)] = i_vert + 3 * (n + 2);
		indices[i_idx + 3 * (3 * n + i) + 1] = i_vert + 3 * (n + 2) + i + 1;
		indices[i_idx + 3 * (3 * n + i) + 2] = i_vert + 3 * (n + 2) + i + 2;
	}
	i_idx += 3 * 4 * n;
	i_vert += 4 * n + 8;
	for (int i = 0; i < n - 1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			indices[i_idx + 6 * (n - 1) * j + 6 * i] = i_vert + 2 * n * j + i;
			indices[i_idx + 6 * (n - 1) * j + 6 * i + 1] = i_vert + 2 * n * j + n + i + 1;
			indices[i_idx + 6 * (n - 1) * j + 6 * i + 2] = i_vert + 2 * n * j + n + i;
			indices[i_idx + 6 * (n - 1) * j + 6 * i + 3] = i_vert + 2 * n * j + i;
			indices[i_idx + 6 * (n - 1) * j + 6 * i + 4] = i_vert + 2 * n * j + i + 1;
			indices[i_idx + 6 * (n - 1) * j + 6 * i + 5] = i_vert + 2 * n * j + n + i + 1;
		}
	}
	i_vert += 4 * n;
	i_idx += 3 * 6 * (n - 1);
	for (int i = 0; i < i_vert; i++)
	{
		tex_coords[i] = vec2(0.09375f, 0.8125f);
	}
	i_vert += 2 * n;

	positions[i_vert] = vec3(-4.0f, 103.8f, 0.7f);
	positions[i_vert + 1] = vec3(-4.0f, 103.8f, 0.0f);
	positions[i_vert + 2] = vec3(-4.0f, 10.8f, 3.05f);
	positions[i_vert + 3] = vec3(-4.0f, 10.8f, 3.75f);
	normals[i_vert] = vec3(-1, 0, 0);
	normals[i_vert + 1] = vec3(-1, 0, 0);
	normals[i_vert + 2] = vec3(-1, 0, 0);
	normals[i_vert + 3] = vec3(-1, 0, 0);
	positions[i_vert + 4] = vec3(-3.76f, 103.8f, 0.2f);
	positions[i_vert + 5] = vec3(-3.76f, 103.8f, 0.7f);
	positions[i_vert + 6] = vec3(-3.76f, 10.8f, 3.75f);
	positions[i_vert + 7] = vec3(-3.76f, 10.8f, 3.25f);
	normals[i_vert + 4] = vec3(1, 0, 0);
	normals[i_vert + 5] = vec3(1, 0, 0);
	normals[i_vert + 6] = vec3(1, 0, 0);
	normals[i_vert + 7] = vec3(1, 0, 0);
	positions[i_vert + 8] = vec3(3.76f, 103.8f, 0.7f);
	positions[i_vert + 9] = vec3(3.76f, 103.8f, 0.2f);
	positions[i_vert + 10] = vec3(3.76f, 10.8f, 3.25f);
	positions[i_vert + 11] = vec3(3.76f, 10.8f, 3.75f);
	normals[i_vert + 8] = vec3(-1, 0, 0);
	normals[i_vert + 9] = vec3(-1, 0, 0);
	normals[i_vert + 10] = vec3(-1, 0, 0);
	normals[i_vert + 11] = vec3(-1, 0, 0);
	positions[i_vert + 12] = vec3(4.0f, 103.8f, 0.0f);
	positions[i_vert + 13] = vec3(4.0f, 103.8f, 0.7f);
	positions[i_vert + 14] = vec3(4.0f, 10.8f, 3.75f);
	positions[i_vert + 15] = vec3(4.0f, 10.8f, 3.05f);
	normals[i_vert + 12] = vec3(1, 0, 0);
	normals[i_vert + 13] = vec3(1, 0, 0);
	normals[i_vert + 14] = vec3(1, 0, 0);
	normals[i_vert + 15] = vec3(1, 0, 0);
	positions[i_vert + 16] = vec3(-4.0f, 103.8f, 0.0f);
	positions[i_vert + 17] = vec3(4.0f, 103.8f, 0.0f);
	positions[i_vert + 18] = vec3(4.0f, 10.8f, 3.05f);
	positions[i_vert + 19] = vec3(-4.0f, 10.8f, 3.05f);
	normals[i_vert + 16] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	normals[i_vert + 17] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	normals[i_vert + 18] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	normals[i_vert + 19] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	positions[i_vert + 20] = vec3(-3.76f, 103.8f, 0.7f);
	positions[i_vert + 21] = vec3(-4.0f, 103.8f, 0.7f);
	positions[i_vert + 22] = vec3(-4.0f, 10.8f, 3.75f);
	positions[i_vert + 23] = vec3(-3.76f, 10.8f, 3.75f);
	normals[i_vert + 20] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 21] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 22] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 23] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	positions[i_vert + 24] = vec3(4.0f, 103.8f, 0.7f);
	positions[i_vert + 25] = vec3(3.76f, 103.8f, 0.7f);
	positions[i_vert + 26] = vec3(3.76f, 10.8f, 3.75f);
	positions[i_vert + 27] = vec3(4.0f, 10.8f, 3.75f);
	normals[i_vert + 24] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 25] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 26] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 27] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	for (int i = 0; i < 28; i++)
	{
		tex_coords[i_vert + i] = vec2(0.09375f, 0.8125f);
	}
	positions[i_vert + 28] = vec3(-3.76f, 103.8f, 0.2f);
	positions[i_vert + 29] = vec3(-3.76f, 10.8f, 3.25f);
	positions[i_vert + 30] = vec3(3.76f, 10.8f, 3.25f);
	positions[i_vert + 31] = vec3(3.76f, 103.8f, 0.2f);
	normals[i_vert + 28] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 29] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 30] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 31] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	tex_coords[i_vert + 28] = vec2(0.4082031f, 0.0f);
	tex_coords[i_vert + 29] = vec2(0.4082031f, 18.61f / 20.48f);
	tex_coords[i_vert + 30] = vec2(0.59179687f, 18.61f / 20.48f);
	tex_coords[i_vert + 31] = vec2(0.59179687f, 0.0f);
	for (int i = 0; i < 8; i++)
	{
		indices[i_idx + 6 * i] = i_vert + 4 * i;
		indices[i_idx + 6 * i + 1] = i_vert + 4 * i + 1;
		indices[i_idx + 6 * i + 2] = i_vert + 4 * i + 2;
		indices[i_idx + 6 * i + 3] = i_vert + 4 * i;
		indices[i_idx + 6 * i + 4] = i_vert + 4 * i + 2;
		indices[i_idx + 6 * i + 5] = i_vert + 4 * i + 3;
	}
	i_vert += 32;
	i_idx += 48;

	theta = asin(61.0f / 1861);
	n = 2 * theta * sqrtf(37220) + 3;
	dtheta = 2 * theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(i * dtheta - theta);
		float sin_theta = sin(i * dtheta - theta);
		float y = -1.40f - 372.2f * sin_theta;
		float z = 372.2f * cos_theta - 368.75f;
		positions[i_vert + i] = vec3(3.76f, y, z);
		normals[i_vert + i] = vec3(-1, 0, 0);
		positions[i_vert + n + i] = vec3(3.76f, y, z + 0.5f);
		normals[i_vert + n + i] = vec3(-1, 0, 0);
		positions[i_vert + 2 * n + i] = vec3(3.76f, y, z + 0.5f);
		normals[i_vert + 2 * n + i] = vec3(0, -sin_theta, cos_theta);
		positions[i_vert + 3 * n + i] = vec3(4.0f, y, z + 0.5f);
		normals[i_vert + 3 * n + i] = vec3(0, -sin_theta, cos_theta);
		positions[i_vert + 4 * n + i] = vec3(4.0f, y, z + 0.5f);
		normals[i_vert + 4 * n + i] = vec3(1, 0, 0);
		positions[i_vert + 5 * n + i] = vec3(4.0f, y, z - 0.2f);
		normals[i_vert + 5 * n + i] = vec3(1, 0, 0);
		positions[i_vert + 6 * n + i] = vec3(4.0f, y, z - 0.2f);
		normals[i_vert + 6 * n + i] = vec3(0, sin_theta, -cos_theta);
		positions[i_vert + 7 * n + i] = vec3(-4.0f, y, z - 0.2f);
		normals[i_vert + 7 * n + i] = vec3(0, sin_theta, -cos_theta);
		positions[i_vert + 8 * n + i] = vec3(-4.0f, y, z - 0.2f);
		normals[i_vert + 8 * n + i] = vec3(-1, 0, 0);
		positions[i_vert + 9 * n + i] = vec3(-4.0f, y, z + 0.5f);
		normals[i_vert + 9 * n + i] = vec3(-1, 0, 0);
		positions[i_vert + 10 * n + i] = vec3(-4.0f, y, z + 0.5f);
		normals[i_vert + 10 * n + i] = vec3(0, -sin_theta, cos_theta);
		positions[i_vert + 11 * n + i] = vec3(-3.76f, y, z + 0.5f);
		normals[i_vert + 11 * n + i] = vec3(0, -sin_theta, cos_theta);
		positions[i_vert + 12 * n + i] = vec3(-3.76f, y, z + 0.5f);
		normals[i_vert + 12 * n + i] = vec3(1, 0, 0);
		positions[i_vert + 13 * n + i] = vec3(-3.76f, y, z);
		normals[i_vert + 13 * n + i] = vec3(1, 0, 0);
		positions[i_vert + 14 * n + i] = vec3(-3.76f, y, z);
		normals[i_vert + 14 * n + i] = vec3(0, -sin_theta, cos_theta);
		tex_coords[i_vert + 14 * n + i] = vec2(0.4082031f, 372.2f * 2 * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 15 * n + i] = vec3(3.76f, y, z);
		normals[i_vert + 15 * n + i] = vec3(0, -sin_theta, cos_theta);
		tex_coords[i_vert + 15 * n + i] = vec2(0.59179687f, 372.2f * 2 * theta / (20.48f * (n - 1)) * i);
	}
	for (int i = 0; i < 14 * n; i++)
	{
		tex_coords[i_vert + i] = vec2(0.09375f, 0.8125f);
	}
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[i_idx + 6 * (i * (n - 1) + j)] = i_vert + 2 * (i * n) + j;
			indices[i_idx + 6 * (i * (n - 1) + j) + 1] = i_vert + 2 * (i * n) + j + n + 1;
			indices[i_idx + 6 * (i * (n - 1) + j) + 2] = i_vert + 2 * (i * n) + j + n;
			indices[i_idx + 6 * (i * (n - 1) + j) + 3] = i_vert + 2 * (i * n) + j;
			indices[i_idx + 6 * (i * (n - 1) + j) + 4] = i_vert + 2 * (i * n) + j + 1;
			indices[i_idx + 6 * (i * (n - 1) + j) + 5] = i_vert + 2 * (i * n) + j + n + 1;
		}
	}
	i_vert += 16 * n;
	i_idx += 6 * 8 * (n - 1);

	positions[i_vert] = vec3(-4.0f, -13.6f, 3.75f);
	positions[i_vert + 1] = vec3(-4.0f, -13.6f, 3.05f);
	positions[i_vert + 2] = vec3(-4.0f, -36.4f, 3.05f - 22.8f * 61 / 1860);
	positions[i_vert + 3] = vec3(-4.0f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	normals[i_vert] = vec3(-1, 0, 0);
	normals[i_vert + 1] = vec3(-1, 0, 0);
	normals[i_vert + 2] = vec3(-1, 0, 0);
	normals[i_vert + 3] = vec3(-1, 0, 0);
	positions[i_vert + 4] = vec3(-3.76f, -13.6f, 3.25f);
	positions[i_vert + 5] = vec3(-3.76f, -13.6f, 3.75f);
	positions[i_vert + 6] = vec3(-3.76f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	positions[i_vert + 7] = vec3(-3.76f, -36.4f, 3.25f - 22.8f * 61 / 1860);
	normals[i_vert + 4] = vec3(1, 0, 0);
	normals[i_vert + 5] = vec3(1, 0, 0);
	normals[i_vert + 6] = vec3(1, 0, 0);
	normals[i_vert + 7] = vec3(1, 0, 0);
	positions[i_vert + 8] = vec3(3.76f, -13.6f, 3.75f);
	positions[i_vert + 9] = vec3(3.76f, -13.6f, 3.25f);
	positions[i_vert + 10] = vec3(3.76f, -36.4f, 3.25f - 22.8f * 61 / 1860);
	positions[i_vert + 11] = vec3(3.76f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	normals[i_vert + 8] = vec3(-1, 0, 0);
	normals[i_vert + 9] = vec3(-1, 0, 0);
	normals[i_vert + 10] = vec3(-1, 0, 0);
	normals[i_vert + 11] = vec3(-1, 0, 0);
	positions[i_vert + 12] = vec3(4.0f, -13.6f, 3.05f);
	positions[i_vert + 13] = vec3(4.0f, -13.6f, 3.75f);
	positions[i_vert + 14] = vec3(4.0f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	positions[i_vert + 15] = vec3(4.0f, -36.4f, 3.05f - 22.8f * 61 / 1860);
	normals[i_vert + 12] = vec3(1, 0, 0);
	normals[i_vert + 13] = vec3(1, 0, 0);
	normals[i_vert + 14] = vec3(1, 0, 0);
	normals[i_vert + 15] = vec3(1, 0, 0);
	positions[i_vert + 16] = vec3(-4.0f, -13.6f, 3.05f);
	positions[i_vert + 17] = vec3(4.0f, -13.6f, 3.05f);
	positions[i_vert + 18] = vec3(4.0f, -36.4f, 3.05f - 22.8f * 61 / 1860);
	positions[i_vert + 19] = vec3(-4.0f, -36.4f, 3.05f - 22.8f * 61 / 1860);
	normals[i_vert + 16] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	normals[i_vert + 17] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	normals[i_vert + 18] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	normals[i_vert + 19] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	positions[i_vert + 20] = vec3(-3.76f, -13.6f, 3.75f);
	positions[i_vert + 21] = vec3(-4.0f, -13.6f, 3.75f);
	positions[i_vert + 22] = vec3(-4.0f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	positions[i_vert + 23] = vec3(-3.76f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	normals[i_vert + 20] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 21] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 22] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 23] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	positions[i_vert + 24] = vec3(4.0f, -13.6f, 3.75f);
	positions[i_vert + 25] = vec3(3.76f, -13.6f, 3.75f);
	positions[i_vert + 26] = vec3(3.76f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	positions[i_vert + 27] = vec3(4.0f, -36.4f, 3.75f - 22.8f * 61 / 1860);
	normals[i_vert + 24] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 25] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 26] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 27] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	for (int i = 0; i < 28; i++)
	{
		tex_coords[i_vert + i] = vec2(0.09375f, 0.8125f);
	}
	positions[i_vert + 28] = vec3(-3.76f, -13.6f, 3.25f);
	positions[i_vert + 29] = vec3(-3.76f, -36.4f, 3.25f - 22.8f * 61 / 1860);
	positions[i_vert + 30] = vec3(3.76f, -36.4f, 3.25f - 22.8f * 61 / 1860);
	positions[i_vert + 31] = vec3(3.76f, -13.6f, 3.25f);
	normals[i_vert + 28] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 29] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 30] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[i_vert + 31] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	tex_coords[i_vert + 28] = vec2(0.4082031f, 0.0f);
	tex_coords[i_vert + 29] = vec2(0.4082031f, 22.8f * 18.61f / 18.6f / 20.48f);
	tex_coords[i_vert + 30] = vec2(0.59179687f, 22.8f * 18.61f / 18.6f / 20.48f);
	tex_coords[i_vert + 31] = vec2(0.59179687f, 0.0f);
	for (int i = 0; i < 8; i++)
	{
		indices[i_idx + 6 * i] = i_vert + 4 * i;
		indices[i_idx + 6 * i + 1] = i_vert + 4 * i + 1;
		indices[i_idx + 6 * i + 2] = i_vert + 4 * i + 2;
		indices[i_idx + 6 * i + 3] = i_vert + 4 * i;
		indices[i_idx + 6 * i + 4] = i_vert + 4 * i + 2;
		indices[i_idx + 6 * i + 5] = i_vert + 4 * i + 3;
	}
	i_vert += 32;
	i_idx += 48;

	theta = acos(-12.0f / 13) - 244.0f / 600;
	n = theta * sqrtf(3400) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(i * dtheta);
		float sin_theta = sin(i * dtheta);
		float z = float(n - 1 - i) / (n - 1) * 70.2f * 61 / 1860 + 0.2f;
		positions[i_vert + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z);
		normals[i_vert + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + n + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z + 0.5f);
		normals[i_vert + n + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + 2 * n + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z + 0.5f);
		normals[i_vert + 2 * n + i] = vec3(61 * sin_theta / 1861, -61 * cos_theta / 1861, 1860.0f / 1861);
		positions[i_vert + 3 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, z + 0.5f);
		normals[i_vert + 3 * n + i] = vec3(61 * sin_theta / 1861, -61 * cos_theta / 1861, 1860.0f / 1861);
		positions[i_vert + 4 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, z + 0.5f);
		normals[i_vert + 4 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 5 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, z - 0.2f);
		normals[i_vert + 5 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 6 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, z - 0.2f);
		normals[i_vert + 6 * n + i] = vec3(-61 * sin_theta / 1861, 61 * cos_theta / 1861, -1860.0f / 1861);
		positions[i_vert + 7 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, z - 0.2f);
		normals[i_vert + 7 * n + i] = vec3(-61 * sin_theta / 1861, 61 * cos_theta / 1861, -1860.0f / 1861);
		positions[i_vert + 8 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, z - 0.2f);
		normals[i_vert + 8 * n + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + 9 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, z + 0.5f);
		normals[i_vert + 9 * n + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + 10 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, z + 0.5f);
		normals[i_vert + 10 * n + i] = vec3(61 * sin_theta / 1861, -61 * cos_theta / 1861, 1860.0f / 1861);
		positions[i_vert + 11 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z + 0.5f);
		normals[i_vert + 11 * n + i] = vec3(61 * sin_theta / 1861, -61 * cos_theta / 1861, 1860.0f / 1861);
		positions[i_vert + 12 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z + 0.5f);
		normals[i_vert + 12 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 13 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z);
		normals[i_vert + 13 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 14 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z);
		normals[i_vert + 14 * n + i] = vec3(61 * sin_theta / 1861, -61 * cos_theta / 1861, 1860.0f / 1861);
		tex_coords[i_vert + 14 * n + i] = vec2(0.4082031f, 30.0f * theta * 18.61f / 18.6f / (20.48f * (n - 1)) * i);
		positions[i_vert + 15 * n + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z);
		normals[i_vert + 15 * n + i] = vec3(61 * sin_theta / 1861, -61 * cos_theta / 1861, 1860.0f / 1861);
		tex_coords[i_vert + 15 * n + i] = vec2(0.59179687f, 30.0f * theta * 18.61f / 18.6f / (20.48f * (n - 1)) * i);
	}
	for (int i = 0; i < 14 * n; i++)
	{
		tex_coords[i_vert + i] = vec2(0.09375f, 0.8125f);
	}
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[i_idx + 6 * (i * (n - 1) + j)] = i_vert + 2 * (i * n) + j;
			indices[i_idx + 6 * (i * (n - 1) + j) + 1] = i_vert + 2 * (i * n) + j + n + 1;
			indices[i_idx + 6 * (i * (n - 1) + j) + 2] = i_vert + 2 * (i * n) + j + n;
			indices[i_idx + 6 * (i * (n - 1) + j) + 3] = i_vert + 2 * (i * n) + j;
			indices[i_idx + 6 * (i * (n - 1) + j) + 4] = i_vert + 2 * (i * n) + j + 1;
			indices[i_idx + 6 * (i * (n - 1) + j) + 5] = i_vert + 2 * (i * n) + j + n + 1;
		}
	}
	i_vert += 16 * n;
	i_idx += 6 * 8 * (n - 1);

	theta = 244.0f / 600;
	n = theta * sqrtf(3400) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = -cos(theta - i * dtheta + acos(12.0f / 13));
		float sin_theta = sin(theta - i * dtheta + acos(12.0f / 13));
		float z = 372.2f - sqrtf(372.2f * 372.2f - (float(n - 1 - i) * (n - 1 - i) / ((n - 1) * (n - 1)) * 12.2f * 12.2f));
		positions[i_vert + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z);
		normals[i_vert + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + n + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z + 0.5f);
		normals[i_vert + n + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + 2 * n + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z + 0.5f);
		normals[i_vert + 2 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sin_theta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * cos_theta / 37220, 1 - z / 372.2f);
		positions[i_vert + 3 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, z + 0.5f);
		normals[i_vert + 3 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sin_theta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * cos_theta / 37220, 1 - z / 372.2f);
		positions[i_vert + 4 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, z + 0.5f);
		normals[i_vert + 4 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 5 * n + i] = vec3(30.0f - 26.0f * cos_theta, -36.4f - 26.0f * sin_theta, 0);
		normals[i_vert + 5 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 6 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, 0);
		normals[i_vert + 6 * n + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + 7 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, z + 0.5f);
		normals[i_vert + 7 * n + i] = vec3(-cos_theta, -sin_theta, 0);
		positions[i_vert + 8 * n + i] = vec3(30.0f - 34.0f * cos_theta, -36.4f - 34.0f * sin_theta, z + 0.5f);
		normals[i_vert + 8 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sin_theta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * cos_theta / 37220, 1 - z / 372.2f);
		positions[i_vert + 9 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z + 0.5f);
		normals[i_vert + 9 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sin_theta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * cos_theta / 37220, 1 - z / 372.2f);
		positions[i_vert + 10 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z + 0.5f);
		normals[i_vert + 10 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 11 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z);
		normals[i_vert + 11 * n + i] = vec3(cos_theta, sin_theta, 0);
		positions[i_vert + 12 * n + i] = vec3(30.0f - 33.76f * cos_theta, -36.4f - 33.76f * sin_theta, z);
		normals[i_vert + 12 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sin_theta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * cos_theta / 37220, 1 - z / 372.2f);
		tex_coords[i_vert + 12 * n + i] = vec2(0.4082031f, 30.0f * theta / (20.48f * (n - 1)) * i);
		positions[i_vert + 13 * n + i] = vec3(30.0f - 26.24f * cos_theta, -36.4f - 26.24f * sin_theta, z);
		normals[i_vert + 13 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sin_theta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * cos_theta / 37220, 1 - z / 372.2f);
		tex_coords[i_vert + 13 * n + i] = vec2(0.59179687f, 30.0f * theta / (20.48f * (n - 1)) * i);
	}
	for (int i = 0; i < 12 * n; i++)
	{
		tex_coords[i_vert + i] = vec2(0.09375f, 0.8125f);
	}
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[i_idx + 6 * (i * (n - 1) + j)] = i_vert + 2 * (i * n) + j;
			indices[i_idx + 6 * (i * (n - 1) + j) + 1] = i_vert + 2 * (i * n) + j + n + 1;
			indices[i_idx + 6 * (i * (n - 1) + j) + 2] = i_vert + 2 * (i * n) + j + n;
			indices[i_idx + 6 * (i * (n - 1) + j) + 3] = i_vert + 2 * (i * n) + j;
			indices[i_idx + 6 * (i * (n - 1) + j) + 4] = i_vert + 2 * (i * n) + j + 1;
			indices[i_idx + 6 * (i * (n - 1) + j) + 5] = i_vert + 2 * (i * n) + j + n + 1;
		}
	}
	i_vert += 14 * n;
	i_idx += 6 * 7 * (n - 1);

	positions[i_vert + 0] = vec3(30.0f + 34.0f * 12.0f / 13, -36.4f - 34.0f * 5.0f / 13, 0);
	positions[i_vert + 1] = vec3(30.0f + 33.76f * 12.0f / 13, -36.4f - 33.76f * 5.0f / 13, 0);
	positions[i_vert + 2] = vec3(30.0f + 33.76f * 12.0f / 13, -36.4f - 33.76f * 5.0f / 13, 0.5f);
	positions[i_vert + 3] = vec3(30.0f + 34.0f * 12.0f / 13, -36.4f - 34.0f * 5.0f / 13, 0.5f);
	positions[i_vert + 4] = vec3(30.0f + 26.24f * 12.0f / 13, -36.4f - 26.24f * 5.0f / 13, 0);
	positions[i_vert + 5] = vec3(30.0f + 26.0f * 12.0f / 13, -36.4f - 26.0f * 5.0f / 13, 0);
	positions[i_vert + 6] = vec3(30.0f + 26.0f * 12.0f / 13, -36.4f - 26.0f * 5.0f / 13, 0.5f);
	positions[i_vert + 7] = vec3(30.0f + 26.24f * 12.0f / 13, -36.4f - 26.24f * 5.0f / 13, 0.5f);
	for (int i = 0; i < 8; i++)
	{
		normals[i_vert + i] = vec3(5.0f / 13, 12.0f / 13, 0);
		tex_coords[i_vert + i] = vec2(0.09375f, 0.8125f);
	}
	indices[i_idx] = i_vert;
	indices[i_idx + 1] = i_vert + 1;
	indices[i_idx + 2] = i_vert + 2;
	indices[i_idx + 3] = i_vert;
	indices[i_idx + 4] = i_vert + 2;
	indices[i_idx + 5] = i_vert + 3;
	indices[i_idx + 6] = i_vert + 4;
	indices[i_idx + 7] = i_vert + 5;
	indices[i_idx + 8] = i_vert + 6;
	indices[i_idx + 9] = i_vert + 4;
	indices[i_idx + 10] = i_vert + 6;
	indices[i_idx + 11] = i_vert + 7;
	i_vert += 8;
	i_idx += 12;

	n = 2 * pi<float>() * sqrtf(40) + 3;
	dtheta = 2 * pi<float>() / n;
	for (int i = 0; i < 4; i++)
	{
		vec3 pillar_pos[10] = { { -3.0f + 2.0f * i, 88.6f,  0.65f},
							{ -3.0f + 2.0f * i, 68.6f, 1.3f },
							{ -3.0f + 2.0f * i, 48.6f, 1.95f },
							{ -3.0f + 2.0f * i, 28.6f, 2.6f },
							{ -3.0f + 2.0f * i, 8.6f, 3.25f},
							{ -3.0f + 2.0f * i, -11.4f, 3.25f },
							{ -3.0f + 2.0f * i, -31.4f, 2.6f },
							{ 30.0f - (33.0f - i * 2.0f) * cos(300.0f / 600), -36.4f - (33.0f - i * 2.0f) * sin(300.0f / 600), 1.95f },
							{ 30.0f - (33.0f - i * 2.0f) * cos(700.0f / 600), -36.4f - (33.0f - i * 2.0f) * sin(700.0f / 600), 1.3f },
							{ 30.0f - (33.0f - i * 2.0f) * cos(1100.0f / 600), -36.4f - (33.0f - i * 2.0f) * sin(1100.0f / 600), 0.65f } };
		for (int j = 0; j < 10; j++)
		{
			for (int k = 0; k < n; k++)
			{
				constexpr float R = 0.4f;
				float cos_theta = cos(k * dtheta);
				float sin_theta = sin(k * dtheta);
				float x = pillar_pos[j].x + R * cos_theta;
				float y = pillar_pos[j].y + R * sin_theta;
				positions[i_vert + 2 * k] = vec3(x, y, 0);
				positions[i_vert + 2 * k + 1] = vec3(x, y, pillar_pos[j].z);
				normals[i_vert + 2 * k] = vec3(cos_theta, sin_theta, 0);
				normals[i_vert + 2 * k + 1] = vec3(cos_theta, sin_theta, 0);
				tex_coords[i_vert + 2 * k] = vec2(0.09375f, 0.8125f);
				tex_coords[i_vert + 2 * k + 1] = vec2(0.09375f, 0.8125f);
				indices[i_idx + 6 * k] = i_vert + 2 * k;
				indices[i_idx + 6 * k + 1] = i_vert + (2 * k + 3) % (2 * n);
				indices[i_idx + 6 * k + 2] = i_vert + 2 * k + 1;
				indices[i_idx + 6 * k + 3] = i_vert + 2 * k;
				indices[i_idx + 6 * k + 4] = i_vert + (2 * k + 2) % (2 * n);
				indices[i_idx + 6 * k + 5] = i_vert + (2 * k + 3) % (2 * n);
			}
			i_vert += 2 * n;
			i_idx += 6 * n;
		}
	}

	glGenVertexArrays(1, &bridge_VAO);
	glBindVertexArray(bridge_VAO);
	glGenBuffers(1, &bridge_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, bridge_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals) + sizeof(tex_coords), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(tex_coords), tex_coords);
	glGenBuffers(1, &bridge_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bridge_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(positions));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

#define X1 0.4f
#define X2 0.8f
#define X3 0.6f
#define X4 0.7f

#define Y0 1.9f
#define Y1 1.05f
#define Y2 0.3f
#define Y3 1.7f

#define Z0 0.6f
#define Z1 0.8f
#define Z2 0.75f
#define Z3 0.9f
#define Z4 0.2f
#define Z5 1.4f

const vec3 car_boundray[8] = { { X2, Y0, Z5 }, { -X2, Y0, Z5 }, { X2, -Y0, Z5 }, { -X2, -Y0, Z5 }, { X2, Y0, 0 }, { -X2, Y0, 0 }, { X2, -Y0, 0 }, { -X2, -Y0, 0 } };

#define POINT0 -X1,Y0,Z0
#define POINT1 -X2,Y0,Z0
#define POINT2  -X2,Y0,Z1
#define POINT3 -X1,Y0,Z1
#define POINT4 X2,Y0,Z0
#define POINT5 X1,Y0,Z0
#define POINT6 X1,Y0,Z1
#define POINT7 X2,Y0,Z1
#define POINT8 -X2,-Y0,Z2
#define POINT9 -X3,-Y0,Z2
#define POINT10 -X3,-Y0,Z3
#define POINT11	-X2,-Y0,Z3
#define POINT12	X3,-Y0,Z2
#define POINT13	X2,-Y0,Z2
#define POINT14	X2,-Y0,Z3
#define POINT15	X3,-Y0,Z3
#define POINT16	-X2,Y0,Z3
#define POINT17	X2,Y0,Z3
#define POINT18	X2,Y0,Z4
#define POINT19	-X2,Y0,Z4
#define POINT20	X2,-Y0,Z4
#define POINT21	-X2,-Y0,Z4
#define POINT22	-X2,Y1,Z3
#define POINT23	X2,Y1,Z3
#define POINT24	X4,Y2,Z5
#define POINT25	-X4,Y2,Z5
#define POINT26	-X4,-Y3,Z5
#define POINT27	X4,-Y3,Z5

void buildCarMesh()
{
	constexpr int VERTICES_SIZE = 884;
	constexpr float HUB_COLOR[3] = { 0.3f,0.3f,0.3f };
	constexpr float TYRE_COLOR[3] = { 0.03f,0.03f,0.03f };
	glProgramUniform3f(SP_car_day, glGetUniformLocation(SP_car_day, "materials[0].albedo"), 0.08f, 0.08f, 0.12f);
	glProgramUniform3f(SP_car_day, glGetUniformLocation(SP_car_day, "materials[0].specular"), 1.0f, 1.0f, 1.0f);
	glProgramUniform1i(SP_car_day, glGetUniformLocation(SP_car_day, "materials[0].shininess"), 256);
	glProgramUniform3fv(SP_car_day, glGetUniformLocation(SP_car_day, "materials[1].albedo"), 1, HUB_COLOR);
	glProgramUniform3fv(SP_car_day, glGetUniformLocation(SP_car_day, "materials[1].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(SP_car_day, glGetUniformLocation(SP_car_day, "materials[1].shininess"), 0);
	glProgramUniform3fv(SP_car_day, glGetUniformLocation(SP_car_day, "materials[2].albedo"), 1, TYRE_COLOR);
	glProgramUniform3fv(SP_car_day, glGetUniformLocation(SP_car_day, "materials[2].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(SP_car_day, glGetUniformLocation(SP_car_day, "materials[2].shininess"), 0);
	glProgramUniform3f(SP_car_day, glGetUniformLocation(SP_car_day, "materials[3].albedo"), 0.6f, 0.6f, 0.6f);
	glProgramUniform3f(SP_car_day, glGetUniformLocation(SP_car_day, "materials[3].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(SP_car_day, glGetUniformLocation(SP_car_day, "materials[3].shininess"), 32);
	glProgramUniform3f(SP_car_day, glGetUniformLocation(SP_car_day, "materials[4].albedo"), 0.6f, 0.04f, 0.04f);
	glProgramUniform3f(SP_car_day, glGetUniformLocation(SP_car_day, "materials[4].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(SP_car_day, glGetUniformLocation(SP_car_day, "materials[4].shininess"), 32);

	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[0].albedo"), 0.08f, 0.08f, 0.12f);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[0].specular"), 1.0f, 1.0f, 1.0f);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[0].shininess"), 256);
	glProgramUniform3fv(SP_car_night, glGetUniformLocation(SP_car_night, "materials[1].albedo"), 1, HUB_COLOR);
	glProgramUniform3fv(SP_car_night, glGetUniformLocation(SP_car_night, "materials[1].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[1].shininess"), 0);
	glProgramUniform3fv(SP_car_night, glGetUniformLocation(SP_car_night, "materials[2].albedo"), 1, TYRE_COLOR);
	glProgramUniform3fv(SP_car_night, glGetUniformLocation(SP_car_night, "materials[2].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[2].shininess"), 0);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[3].albedo"), 0.6f, 0.6f, 0.6f);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[3].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[3].shininess"), 32);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[4].albedo"), 0.6f, 0.04f, 0.04f);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[4].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[4].shininess"), 32);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[5].albedo"), 5.0f, 5.0f, 4.5f);
	glProgramUniform3fv(SP_car_night, glGetUniformLocation(SP_car_night, "materials[5].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[5].shininess"), 0);
	glProgramUniform3f(SP_car_night, glGetUniformLocation(SP_car_night, "materials[6].albedo"), 1.5f, 0.1f, 0.1f);
	glProgramUniform3fv(SP_car_night, glGetUniformLocation(SP_car_night, "materials[6].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(SP_car_night, glGetUniformLocation(SP_car_night, "materials[6].shininess"), 0);

	vec3 positions[VERTICES_SIZE] = { {	 POINT0},	{POINT1},	{POINT2},	{POINT3},	//×óÇ°µÆ
										{POINT4},	{POINT5},	{POINT6},	{POINT7},	//ÓÒÇ°µÆ
										{POINT8},	{POINT9},	{POINT10},	{POINT11},	//×óºóµÆ
										{POINT12},	{POINT13},	{POINT14},	{POINT15},	//ÓÒºóµÆ
										{POINT7},	{POINT2},	{POINT16},	{POINT17},	//Ç°ÉÏ
										{POINT5},	{POINT0},	{POINT3},	{POINT6},	//Ç°ÖÐ
										{POINT18},	{POINT19},	{POINT1},	{POINT4},	//Ç°ÏÂ
										{POINT18},	{POINT20},	{POINT21},	{POINT19},	//µ×
										{POINT9},	{POINT12},	{POINT15},	{POINT10},	//ºóÉÏ
										{POINT21},	{POINT20},	{POINT13},	{POINT8},	//ºóÏÂ
										{POINT19},	{POINT21},	{POINT11},	{POINT16},	//×ó
										{POINT20},	{POINT18},	{POINT17},	{POINT14},	//ÓÒ
										{POINT17},	{POINT16},	{POINT22},	{POINT23},	//ÒýÇæ¸Ç
										{POINT24},	{POINT25},	{POINT26},	{POINT27},	//¶¥
										{POINT23},	{POINT22},	{POINT25},	{POINT24},	//Ç°²£Á§
										{POINT11},	{POINT14},	{POINT27},	{POINT26},	//ºó²£Á§
										{POINT22},	{POINT11},	{POINT26},	{POINT25},	//×ó²£Á§
										{POINT14},	{POINT23},	{POINT24},	{POINT27} };//ÓÒ²£Á§
	vec3 normals[VERTICES_SIZE];
	int material_idxs[VERTICES_SIZE];
	GLuint indices[CAR_EBO_SIZE]{};

	for (int i = 0; i < 4; i++)
	{
		normals[i] = vec3(0, 1, 0);
		normals[4 + i] = vec3(0, 1, 0);
		normals[8 + i] = vec3(0, -1, 0);
		normals[12 + i] = vec3(0, -1, 0);
		normals[16 + i] = vec3(0, 1, 0);
		normals[20 + i] = vec3(0, 1, 0);
		normals[24 + i] = vec3(0, 1, 0);
		normals[28 + i] = vec3(0, 0, -1);
		normals[32 + i] = vec3(0, -1, 0);
		normals[36 + i] = vec3(0, -1, 0);
		normals[40 + i] = vec3(-1, 0, 0);
		normals[44 + i] = vec3(1, 0, 0);
		normals[48 + i] = vec3(0, 0, 1);
		normals[52 + i] = vec3(0, 0, 1);
		normals[56 + i] = normalize(vec3(0, 2, 3));
		normals[60 + i] = normalize(vec3(0, -5, 2));
		normals[64 + i] = normalize(vec3(-5, 0, 1));
		normals[68 + i] = normalize(vec3(5, 0, 1));
	}

	for (int i = 0; i < 8; i++)
	{
		material_idxs[i] = 3;
	}
	for (int i = 8; i < 16; i++)
	{
		material_idxs[i] = 4;
	}
	for (int i = 16; i < 56; i++)
	{
		material_idxs[i] = -1;
	}
	for (int i = 56; i < 72; i++)
	{
		material_idxs[i] = 0;
	}
	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			constexpr GLuint TEMP[] = { 0, 1, 2, 0, 2, 3 };
			indices[6 * i + j] = TEMP[j] + 4 * i;
		}
	}
	int i_vert = 72;
	int i_idx = 108;

	int n = 2 * pi<float>() * sqrtf(30) + 3;
	float dtheta = 2 * pi<float>() / n;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(i * dtheta);
		float sin_theta = sin(i * dtheta);
		constexpr float WHEEL_POS[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[i_vert + j * 5 * n + i] = vec3(0.82f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.18f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.18f * sin_theta);
			normals[i_vert + j * 5 * n + i] = vec3(WHEEL_POS[j][0], 0, 0);
			material_idxs[i_vert + j * 5 * n + i] = 1;

			positions[i_vert + (j * 5 + 1) * n + 2 * i] = vec3(0.82f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.18f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.18f * sin_theta);
			normals[i_vert + (j * 5 + 1) * n + 2 * i] = vec3(WHEEL_POS[j][0], 0, 0);
			material_idxs[i_vert + (j * 5 + 1) * n + 2 * i] = 2;

			positions[i_vert + (j * 5 + 1) * n + 2 * i + 1] = vec3(0.82f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.3f * sin_theta);
			normals[i_vert + (j * 5 + 1) * n + 2 * i + 1] = vec3(WHEEL_POS[j][0], 0, 0);
			material_idxs[i_vert + (j * 5 + 1) * n + 2 * i + 1] = 2;

			positions[i_vert + (j * 5 + 3) * n + 2 * i] = vec3(0.82f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.3f * sin_theta);
			normals[i_vert + (j * 5 + 3) * n + 2 * i] = vec3(0, WHEEL_POS[j][0] * cos_theta, sin_theta);
			material_idxs[i_vert + (j * 5 + 3) * n + 2 * i] = 2;

			positions[i_vert + (j * 5 + 3) * n + 2 * i + 1] = vec3(0.6f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.3f * sin_theta);
			normals[i_vert + (j * 5 + 3) * n + 2 * i + 1] = vec3(0, WHEEL_POS[j][0] * cos_theta, sin_theta);
			material_idxs[i_vert + (j * 5 + 3) * n + 2 * i + 1] = 2;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[i_idx + 3 * ((5 * n - 2) * i + j)] = i_vert + i * 5 * n;
			indices[i_idx + 3 * ((5 * n - 2) * i + j) + 1] = i_vert + i * 5 * n + j + 1;
			indices[i_idx + 3 * ((5 * n - 2) * i + j) + 2] = i_vert + i * 5 * n + j + 2;
		}
		for (int j = 0; j < n; j++)
		{
			indices[i_idx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j)] = i_vert + (i * 5 + 1) * n + 2 * j;
			indices[i_idx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 1] = i_vert + (i * 5 + 1) * n + 2 * j + 1;
			indices[i_idx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 2] = i_vert + (i * 5 + 1) * n + (2 * j + 2) % (2 * n);
			indices[i_idx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 3] = i_vert + (i * 5 + 1) * n + (2 * j + 2) % (2 * n);
			indices[i_idx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 4] = i_vert + (i * 5 + 1) * n + 2 * j + 1;
			indices[i_idx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 5] = i_vert + (i * 5 + 1) * n + (2 * j + 3) % (2 * n);

			indices[i_idx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j)] = i_vert + (i * 5 + 3) * n + 2 * j;
			indices[i_idx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 1] = i_vert + (i * 5 + 3) * n + 2 * j + 1;
			indices[i_idx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 2] = i_vert + (i * 5 + 3) * n + (2 * j + 2) % (2 * n);
			indices[i_idx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 3] = i_vert + (i * 5 + 3) * n + (2 * j + 2) % (2 * n);
			indices[i_idx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 4] = i_vert + (i * 5 + 3) * n + 2 * j + 1;
			indices[i_idx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 5] = i_vert + (i * 5 + 3) * n + (2 * j + 3) % (2 * n);
		}
	}
	i_vert += 4 * 5 * n;
	i_idx += 4 * 3 * (n - 2 + 4 * n);

	n = (2 * acos(1.0f / 5) / dtheta) + 1;
	float theta = n * dtheta / 2;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(theta - i * dtheta);
		float sin_theta = sin(theta - i * dtheta);
		constexpr float WHEEL_POS[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[i_vert + j * n + i] = vec3(0.6f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * sin_theta, 0.3f - 0.3f * cos_theta);
			normals[i_vert + j * n + i] = vec3(-WHEEL_POS[j][0], 0, 0);
			material_idxs[i_vert + j * n + i] = 2;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[i_idx + 3 * ((n - 2) * i + j)] = i_vert + i * n;
			indices[i_idx + 3 * ((n - 2) * i + j) + 1] = i_vert + i * n + j + 1;
			indices[i_idx + 3 * ((n - 2) * i + j) + 2] = i_vert + i * n + j + 2;
		}
	}
	i_vert += 4 * n;
	i_idx += 4 * 3 * (n - 2);

	glGenVertexArrays(1, &car_VAO);
	glBindVertexArray(car_VAO);
	glGenBuffers(1, &car_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, car_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals) + sizeof(material_idxs), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(material_idxs), material_idxs);
	glGenBuffers(1, &car_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)sizeof(positions));
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(int), (void*)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &car_transform_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, car_transform_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_CAR_CNT * sizeof(mat4), nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)0);
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(sizeof(vec4)));
	glVertexAttribDivisor(4, 1);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(2 * sizeof(vec4)));
	glVertexAttribDivisor(5, 1);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(3 * sizeof(vec4)));
	glVertexAttribDivisor(6, 1);

	glGenBuffers(1, &car_color_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, car_color_VBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_CAR_CNT * sizeof(vec3), nullptr, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
}

void buildCarShadowMesh()
{
	constexpr int VERTICES_SIZE = 560;

	vec3 positions[VERTICES_SIZE] = { {	 POINT16},	{POINT17},	{POINT18},	{POINT19},	//Ç°
										{POINT18},	{POINT20},	{POINT21},	{POINT19},	//µ×
										{POINT21},	{POINT20},	{POINT14},	{POINT11},	//ºó
										{POINT19},	{POINT21},	{POINT11},	{POINT16},	//×ó
										{POINT20},	{POINT18},	{POINT17},	{POINT14},	//ÓÒ
										{POINT17},	{POINT16},	{POINT22},	{POINT23},	//ÒýÇæ¸Ç
										{POINT24},	{POINT25},	{POINT26},	{POINT27},	//¶¥
										{POINT23},	{POINT22},	{POINT25},	{POINT24},	//Ç°²£Á§
										{POINT11},	{POINT14},	{POINT27},	{POINT26},	//ºó²£Á§
										{POINT22},	{POINT11},	{POINT26},	{POINT25},	//×ó²£Á§
										{POINT14},	{POINT23},	{POINT24},	{POINT27} };//ÓÒ²£Á§
	vec3 normals[VERTICES_SIZE]{};
	GLuint indices[CAR_SHADOW_EBO_SIZE]{};

	for (int i = 0; i < 4; i++)
	{
		normals[i] = vec3(0, 1, 0);
		normals[4 + i] = vec3(0, 0, -1);
		normals[8 + i] = vec3(0, -1, 0);
		normals[12 + i] = vec3(-1, 0, 0);
		normals[16 + i] = vec3(1, 0, 0);
		normals[20 + i] = vec3(0, 0, 1);
		normals[24 + i] = vec3(0, 0, 1);
		normals[28 + i] = normalize(vec3(0, 2, 3));
		normals[32 + i] = normalize(vec3(0, -5, 2));
		normals[36 + i] = normalize(vec3(-5, 0, 1));
		normals[40 + i] = normalize(vec3(5, 0, 1));
	}

	for (int i = 0; i < 11; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			constexpr GLuint TEMP[] = { 0, 1, 2, 0, 2, 3 };
			indices[6 * i + j] = TEMP[j] + 4 * i;
		}
	}
	int i_vert = 44;
	int i_idx = 66;

	int n = 2 * pi<float>() * sqrtf(30) + 3;
	float dtheta = 2 * pi<float>() / n;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(i * dtheta);
		float sin_theta = sin(i * dtheta);
		constexpr float WHEEL_POS[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[i_vert + j * 3 * n + i] = vec3(0.82f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.3f * sin_theta);
			normals[i_vert + j * 3 * n + i] = vec3(WHEEL_POS[j][0], 0, 0);

			positions[i_vert + (j * 3 + 1) * n + 2 * i] = vec3(0.82f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.3f * sin_theta);
			normals[i_vert + (j * 3 + 1) * n + 2 * i] = vec3(0, WHEEL_POS[j][0] * cos_theta, sin_theta);

			positions[i_vert + (j * 3 + 1) * n + 2 * i + 1] = vec3(0.6f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * cos_theta, 0.3f + 0.3f * sin_theta);
			normals[i_vert + (j * 3 + 1) * n + 2 * i + 1] = vec3(0, WHEEL_POS[j][0] * cos_theta, sin_theta);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[i_idx + 3 * ((3 * n - 2) * i + j)] = i_vert + i * 3 * n;
			indices[i_idx + 3 * ((3 * n - 2) * i + j) + 1] = i_vert + i * 3 * n + j + 1;
			indices[i_idx + 3 * ((3 * n - 2) * i + j) + 2] = i_vert + i * 3 * n + j + 2;
		}
		for (int j = 0; j < n; j++)
		{
			indices[i_idx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j)] = i_vert + (i * 3 + 1) * n + 2 * j;
			indices[i_idx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 1] = i_vert + (i * 3 + 1) * n + 2 * j + 1;
			indices[i_idx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 2] = i_vert + (i * 3 + 1) * n + (2 * j + 2) % (2 * n);
			indices[i_idx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 3] = i_vert + (i * 3 + 1) * n + (2 * j + 2) % (2 * n);
			indices[i_idx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 4] = i_vert + (i * 3 + 1) * n + 2 * j + 1;
			indices[i_idx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 5] = i_vert + (i * 3 + 1) * n + (2 * j + 3) % (2 * n);
		}
	}
	i_vert += 4 * 3 * n;
	i_idx += 4 * 3 * (n - 2 + 2 * n);

	n = (2 * acos(1.0f / 5) / dtheta) + 1;
	float theta = n * dtheta / 2;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cos_theta = cos(theta - i * dtheta);
		float sin_theta = sin(theta - i * dtheta);
		constexpr float WHEEL_POS[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[i_vert + j * n + i] = vec3(0.6f * WHEEL_POS[j][0], 1.3f * WHEEL_POS[j][1] + 0.3f * WHEEL_POS[j][0] * sin_theta, 0.3f - 0.3f * cos_theta);
			normals[i_vert + j * n + i] = vec3(-WHEEL_POS[j][0], 0, 0);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[i_idx + 3 * ((n - 2) * i + j)] = i_vert + i * n;
			indices[i_idx + 3 * ((n - 2) * i + j) + 1] = i_vert + i * n + j + 1;
			indices[i_idx + 3 * ((n - 2) * i + j) + 2] = i_vert + i * n + j + 2;
		}
	}
	i_vert += 4 * n;
	i_idx += 4 * 3 * (n - 2);

	glGenVertexArrays(1, &car_shadow_VAO);
	glBindVertexArray(car_shadow_VAO);
	glGenBuffers(1, &car_shadow_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, car_shadow_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glGenBuffers(1, &car_shadow_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, car_shadow_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)sizeof(positions));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, car_transform_VBO);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)0);
	glVertexAttribDivisor(3, 1);
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(sizeof(vec4)));
	glVertexAttribDivisor(4, 1);
	glEnableVertexAttribArray(5);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(2 * sizeof(vec4)));
	glVertexAttribDivisor(5, 1);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*)(3 * sizeof(vec4)));
	glVertexAttribDivisor(6, 1);

	glBindVertexArray(0);
}

void buildSunMesh()
{
	vec2 positions[SUN_VBO_SIZE];
	int n = SUN_VBO_SIZE;
	float dtheta = 2 * pi<float>() / n;
	for (int i = 0; i < n; i++)
	{
		positions[i] = vec2(4.0f * cos(i * dtheta), -4.0f * sin(i * dtheta));
	}

	glGenVertexArrays(1, &sun_VAO);
	glBindVertexArray(sun_VAO);
	glGenBuffers(1, &sun_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, sun_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vec2), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void buildMeshes()
{
	buildGroundMesh();
	buildBridgeMesh();
	buildCarMesh();
	buildCarShadowMesh();
	buildSunMesh();
}