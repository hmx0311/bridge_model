#include "mesh.h"
#include "common.h"

GLuint groundVAO, groundVBO, groundEBO;
GLuint bridgeVAO, bridgeVBO, bridgeEBO;
GLuint carVAO, carVBO, carEBO, carMatVBO, carColorVBO;
GLuint carShadowVAO, carShadowVBO, carShadowEBO;
GLuint sunVAO, sunVBO;

using namespace glm;

void buildGroundMesh()
{
	constexpr int vertSize = 1334;
	vec3 positions[vertSize];
	vec3 normals[vertSize];
	vec2 texCoords[vertSize];
	GLuint indices[groundEBOsize];

	positions[0] = vec3(-100000, -100000, 0);
	positions[1] = vec3(100000, -100000, 0);
	positions[2] = vec3(100000, 100000, 0);
	positions[3] = vec3(-100000, 100000, 0);
	for (int i = 0; i < 4; i++)
	{
		texCoords[i] = vec2(0.09375f, 1.0f);
	}

	positions[4] = vec3(-100000, -720, 0);
	texCoords[4] = vec2(0.63671875f, -100000.0f / 2048);
	positions[5] = vec3(100000, -720, 0);
	texCoords[5] = vec2(0.63671875f, 100000.0f / 2048);
	positions[6] = vec3(100000, 720, 0);
	texCoords[6] = vec2(0.98828125f, 100000.0f / 2048);
	positions[7] = vec3(-100000, 720, 0);
	texCoords[7] = vec2(0.98828125f, -100000.0f / 2048);

	positions[8] = vec3(-720, 12800, 0);
	texCoords[8] = vec2(0.63671875f, 12800.0f / 2048);
	positions[9] = vec3(720, 12800, 0);
	texCoords[9] = vec2(0.98828125f, 12800.0f / 2048);
	positions[10] = vec3(720, 100000, 0);
	texCoords[10] = vec2(0.98828125f, 100000.0f / 2048);
	positions[11] = vec3(-720, 100000, 0);
	texCoords[11] = vec2(0.63671875f, 100000.0f / 2048);
	
	positions[12] = vec3(376, 12800, 0);
	texCoords[12] = vec2(0.59179687f, 0.5859375f);
	positions[13] = vec3(-376, 12800, 0);
	texCoords[13] = vec2(0.4082031f, 0.5859375f);
	positions[14] = vec3(-376, 11600, 0);
	texCoords[14] = vec2(0.4082031f, 0.0f);
	positions[15] = vec3(376, 11600, 0);
	texCoords[15] = vec2(0.59179687f, 0.0f);
	
	positions[16] = vec3(-12480, 1040, 0);
	texCoords[16] = vec2(0.15478516f, 3.03125f);
	positions[17] = vec3(-18480, 1040, 0);
	texCoords[17] = vec2(0.15478516f, 0.03125f);
	positions[18] = vec3(-18480, 650, 0);
	texCoords[18] = vec2(0.25f, 0.03125f);
	positions[19] = vec3(-12480, 650, 0);
	texCoords[19] = vec2(0.25f, 3.03125f);

	positions[20] = vec3(18480, 1040, 0);
	texCoords[20] = vec2(0.15478516f, 0.03125f);
	positions[21] = vec3(12480, 1040, 0);
	texCoords[21] = vec2(0.15478516f, 3.03125f);
	positions[22] = vec3(12480, 650, 0);
	texCoords[22] = vec2(0.25f, 3.03125f);
	positions[23] = vec3(18480, 650, 0);
	texCoords[23] = vec2(0.25f, 0.03125f);

	positions[24] = vec3(-3000, -1040, 0);
	texCoords[24] = vec2(0.15478516f, 0.03125f);
	positions[25] = vec3(3000, -1040, 0);
	texCoords[25] = vec2(0.15478516f, 3.03125f);
	positions[26] = vec3(3000, -650, 0);
	texCoords[26] = vec2(0.25f, 3.03125f);
	positions[27] = vec3(-3000, -650, 0);
	texCoords[27] = vec2(0.25f, 0.03125f);

	positions[28] = vec3(12000, -1040, 0);
	texCoords[28] = vec2(0.15478516f, 3.03125f);
	positions[29] = vec3(18000, -1040, 0);
	texCoords[29] = vec2(0.15478516f, 0.03125f);
	positions[30] = vec3(18000, -650, 0);
	texCoords[30] = vec2(0.25f, 0.03125f);
	positions[31] = vec3(12000, -650, 0);
	texCoords[31] = vec2(0.25f, 3.03125f);
	for (int i = 0; i < 8; i++)
	{
		indices[6 * i] = 4 * i;
		indices[6 * i + 1] = 4 * i + 1;
		indices[6 * i + 2] = 4 * i + 2;
		indices[6 * i + 3] = 4 * i + 0;
		indices[6 * i + 4] = 4 * i + 2;
		indices[6 * i + 5] = 4 * i + 3;
	}
	int iVert = 32;
	int iIdx = 48;

	float theta = asin(12.0f / 37);
	int n = theta * sqrtf(2960) + 3;
	float dtheta = theta / n;
	n++;
	positions[iVert] = vec3(-18480, 650, 0);
	texCoords[iVert] = vec2(0.0f, 0.0f);
	positions[iVert + 2 * n] = vec3(-20400, 650, 0);
	texCoords[iVert + 2 * n] = vec2(0.0f, 1920.0f / 2048);
	positions[iVert + 2 * n + 1] = vec3(18480, 650, 0);
	texCoords[iVert + 2 * n + 1] = vec2(0.0f, 0.0f);
	positions[iVert + 4 * n + 1] = vec3(20400, 650, 0);
	texCoords[iVert + 4 * n + 1] = vec2(0.0f, 1920.0f / 2048);
	positions[iVert + 4 * n + 2] = vec3(-3000, -650, 0);
	texCoords[iVert + 4 * n + 2] = vec2(0.0f, 0.0f);
	positions[iVert + 6 * n + 2] = vec3(-4920, -650, 0);
	texCoords[iVert + 6 * n + 2] = vec2(0.0f, 1920.0f / 2048);
	positions[iVert + 6 * n + 3] = vec3(18000, -650, 0);
	texCoords[iVert + 6 * n + 3] = vec2(0.0f, 0.0f);
	positions[iVert + 8 * n + 3] = vec3(19920, -650, 0);
	texCoords[iVert + 8 * n + 3] = vec2(0.0f, 1920.0f / 2048);
	for (int i = 0; i < n; i++)
	{
		float x = 2960 * sin(i * dtheta);
		float y = -2570 + 2960 * cos(i * dtheta);
		positions[iVert + 1 + i] = vec3(-18480 - x, 650 + y, 0);
		texCoords[iVert + 1 + i] = vec2(y / 4096.0f, x / 2048.0f);
		positions[iVert + 2 * n + 2 + i] = vec3(18480 + x, 650 + y, 0);
		texCoords[iVert + 2 * n + 2 + i] = vec2(y / 4096.0f, x / 2048.0f);
		positions[iVert + 4 * n + 3 + i] = vec3(-3000 - x, -650 - y, 0);
		texCoords[iVert + 4 * n + 3 + i] = vec2(y / 4096.0f, x / 2048.0f);
		positions[iVert + 6 * n + 4 + i] = vec3(18000 + x, -650 - y, 0);
		texCoords[iVert + 6 * n + 4 + i] = vec2(y / 4096.0f, x / 2048.0f);
	}
	for (int i = 1; i < n; i++)
	{
		float x = 1920 - 2960 * sin(theta - i * dtheta);
		float y = 3030 - 2960 * cos(theta - i * dtheta);
		positions[iVert + n + i] = vec3(-18480 - x, 650 + y, 0);
		texCoords[iVert + n + i] = vec2(y / 4096.0f, x / 2048.0f);
		positions[iVert + 3 * n + 1 + i] = vec3(18480 + x, 650 + y, 0);
		texCoords[iVert + 3 * n + 1 + i] = vec2(y / 4096.0f, x / 2048.0f);
		positions[iVert + 5 * n + 2 + i] = vec3(-3000 - x, -650 - y, 0);
		texCoords[iVert + 5 * n + 2 + i] = vec2(y / 4096.0f, x / 2048.0f);
		positions[iVert + 7 * n + 3 + i] = vec3(18000 + x, -650 - y, 0);
		texCoords[iVert + 7 * n + 3 + i] = vec2(y / 4096.0f, x / 2048.0f);
	}
	for (int i = 0; i < 2 * n - 1; i++)
	{
		indices[iIdx + 3 * i] = iVert;
		indices[iIdx + 3 * i + 1] = iVert + i + 1;
		indices[iIdx + 3 * i + 2] = iVert + i + 2;
		indices[iIdx + 3 * (2 * n - 1 + i)] = iVert + 2 * n + 1;
		indices[iIdx + 3 * (2 * n - 1 + i) + 1] = iVert + 2 * n + 1 + i + 2;
		indices[iIdx + 3 * (2 * n - 1 + i) + 2] = iVert + 2 * n + 1 + i + 1;
		indices[iIdx + 3 * (4 * n - 2 + i)] = iVert + 4 * n + 2;
		indices[iIdx + 3 * (4 * n - 2 + i) + 1] = iVert + 4 * n + 2 + i + 2;
		indices[iIdx + 3 * (4 * n - 2 + i) + 2] = iVert + 4 * n + 2 + i + 1;
		indices[iIdx + 3 * (6 * n - 3 + i)] = iVert + 6 * n + 3;
		indices[iIdx + 3 * (6 * n - 3 + i) + 1] = iVert + 6 * n + 3 + i + 1;
		indices[iIdx + 3 * (6 * n - 3 + i) + 2] = iVert + 6 * n + 3 + i + 2;
	}
	iVert += 4 * (2 * n + 1);
	iIdx += 3 * 4 * (2 * n - 1);

	theta = PI / 2;
	n = theta * sqrtf(12160) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float sinTheta = sin(i * dtheta);
		float cosTheta = cos(i * dtheta);
		constexpr float O[2] = { -12480, 12800 };
		float R = 12160;
		float r = 11760;
		float X = O[0] + R * cosTheta, Y = O[1] - R * sinTheta;
		float x = O[0] + r * cosTheta, y = O[1] - r * sinTheta;
		if (i < 0.07f * n)
		{
			if (i < 0.025f * n)
			{

				R -= 37.0f;
			}
			else
			{
				R -= (X + 365) / cosTheta;
			}
			X = O[0] + R * cosTheta;
			Y = O[1] - R * sinTheta;
		}
		else if (i > 0.93f * n)
		{
			if (i > 0.973f * n)
			{
				R -= 37.0f;
			}
			else
			{
				R -= (685 - Y) / sinTheta;
			}
			X = O[0] + R * cosTheta;
			Y = O[1] - R * sinTheta;
		}
		positions[iVert + 2 * i] = vec3(X, Y, 0);
		texCoords[iVert + 2 * i] = vec2(0.3125f + (R - 11960) / 4096, 11960 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 2 * i + 1] = vec3(x, y, 0);
		texCoords[iVert + 2 * i + 1] = vec2(0.263671875f, 11960 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 2 * n + 2 * i] = vec3(-x, y, 0);
		texCoords[iVert + 2 * n + 2 * i] = vec2(0.263671875f, 11960 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 2 * n + 2 * i + 1] = vec3(-X, Y, 0);
		texCoords[iVert + 2 * n + 2 * i + 1] = vec2(0.3125f + (R - 11960) / 4096, 11960 * theta / (2048 * (n - 1)) * i);
	}
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[iIdx + 6 * j] = iVert + 2 * j;
			indices[iIdx + 6 * j + 1] = iVert + 2 * j + 1;
			indices[iIdx + 6 * j + 2] = iVert + 2 * j + 3;
			indices[iIdx + 6 * j + 3] = iVert + 2 * j;
			indices[iIdx + 6 * j + 4] = iVert + 2 * j + 3;
			indices[iIdx + 6 * j + 5] = iVert + 2 * j + 2;
		}
		iVert += 2 * n;
		iIdx += 6 * (n - 1);
	}

	theta = acos(-5.0f / 13);
	n = theta * sqrtf(3000) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{

		float sinTheta = sin(i * dtheta);
		float cosTheta = cos(i * dtheta);
		constexpr float O[2] = { 3000, -3640 };
		float R = 3000;
		float r = 2600;
		float X = O[0] + R * sinTheta, Y = O[1] + R * cosTheta;
		float x = O[0] + r * sinTheta, y = O[1] + r * cosTheta;
		if (i < 0.11f * n)
		{
			if (i < 0.034f * n)
			{
				R -= 37.0f;
			}
			else
			{
				R -= (685 + Y) / cosTheta;
			}
			X = O[0] + R * sinTheta;
			Y = O[1] + R * cosTheta;
		}
		positions[iVert + 2 * i] = vec3(X, Y, 0);
		texCoords[iVert + 2 * i] = vec2(0.3125f + (R - 2800) / 4096, 2800 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 2 * i + 1] = vec3(x, y, 0);
		texCoords[iVert + 2 * i + 1] = vec2(0.263671875f, 2800 * theta / (2048 * (n - 1)) * i);
	}
	for (int j = 0; j < n - 1; j++)
	{
		indices[iIdx + 6 * j] = iVert + 2 * j;
		indices[iIdx + 6 * j + 1] = iVert + 2 * j + 1;
		indices[iIdx + 6 * j + 2] = iVert + 2 * j + 3;
		indices[iIdx + 6 * j + 3] = iVert + 2 * j;
		indices[iIdx + 6 * j + 4] = iVert + 2 * j + 3;
		indices[iIdx + 6 * j + 5] = iVert + 2 * j + 2;
	}
	iVert += 2 * n;
	iIdx += 6 * (n - 1);

	theta = asin(12.0f / 13);
	n = theta * sqrtf(6750) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float sinTheta = sin(i * dtheta);
		float cosTheta = cos(i * dtheta);
		constexpr float O[2] = { 12000, -7390 };
		float R = 6750;
		float r = 6350;
		float X = O[0] - R * sinTheta, Y = O[1] + R * cosTheta;
		float x = O[0] - r * sinTheta, y = O[1] + r * cosTheta;
		if (i < 0.11f * n)
		{
			if (i < 0.049f * n)
			{
				R -= 37.0f;
			}
			else
			{
				R -= (685 + Y) / cosTheta;
			}
			X = O[0] - R * sinTheta;
			Y = O[1] + R * cosTheta;
		}
		positions[iVert + 2 * i] = vec3(x, y, 0);
		texCoords[iVert + 2 * i] = vec2(0.263671875f, 6550 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 2 * i + 1] = vec3(X, Y, 0);
		texCoords[iVert + 2 * i + 1] = vec2(0.3125f + (R - 6550) / 4096, 6550 * theta / (2048 * (n - 1)) * i);
	}
	for (int j = 0; j < n - 1; j++)
	{
		indices[iIdx + 6 * j] = iVert + 2 * j;
		indices[iIdx + 6 * j + 1] = iVert + 2 * j + 1;
		indices[iIdx + 6 * j + 2] = iVert + 2 * j + 3;
		indices[iIdx + 6 * j + 3] = iVert + 2 * j;
		indices[iIdx + 6 * j + 4] = iVert + 2 * j + 3;
		indices[iIdx + 6 * j + 5] = iVert + 2 * j + 2;
	}
	iVert += 2 * n;
	iIdx += 6 * (n - 1);

	for (int i = 0; i < iVert; i++)
	{
		normals[i] = vec3(0, 0, 1);
	}

	glGenVertexArrays(1, &groundVAO);
	glBindVertexArray(groundVAO);
	glGenBuffers(1, &groundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals) + sizeof(texCoords), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(texCoords), texCoords);
	glGenBuffers(1, &groundEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groundEBO);
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
	constexpr int verticesSize = 6422;
	vec3 positions[verticesSize];
	vec3 normals[verticesSize];
	vec2 texCoords[verticesSize];
	GLuint indices[bridgeEBOsize];

	int iVert = 8;
	int iIdx = 12;
	positions[0] = vec3(-376, 11600, 0);
	positions[1] = vec3(-400, 11600, 0);
	positions[2] = vec3(-400, 11600, 50);
	positions[3] = vec3(-376, 11600, 50);
	positions[4] = vec3(400, 11600, 0);
	positions[5] = vec3(376, 11600, 0);
	positions[6] = vec3(376, 11600, 50);
	positions[7] = vec3(400, 11600, 50);
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
	positions[iVert] = vec3(-400, 11600, 0);
	normals[iVert] = vec3(-1, 0, 0);
	positions[iVert + n + 1] = vec3(-400, 10380, 0);
	normals[iVert + n + 1] = vec3(-1, 0, 0);
	positions[iVert + n + 2] = vec3(-376, 11600, 0);
	normals[iVert + n + 2] = vec3(1, 0, 0);
	positions[iVert + 2 * n + 3] = vec3(-376, 10380, 0);
	normals[iVert + 2 * n + 3] = vec3(1, 0, 0);
	positions[iVert + 2 * n + 4] = vec3(376, 11600, 0);
	normals[iVert + 2 * n + 4] = vec3(-1, 0, 0);
	positions[iVert + 3 * n + 5] = vec3(376, 10380, 0);
	normals[iVert + 3 * n + 5] = vec3(-1, 0, 0);
	positions[iVert + 3 * n + 6] = vec3(400, 11600, 0);
	normals[iVert + 3 * n + 6] = vec3(1, 0, 0);
	positions[iVert + 4 * n + 7] = vec3(400, 10380, 0);
	normals[iVert + 4 * n + 7] = vec3(1, 0, 0);
	for (int i = 0; i < n; i++)
	{
		float costheta = cos(i * dtheta);
		float sintheta = sin(i * dtheta);
		float y = 11600 - 37220 * sintheta;
		float z = 37220 - 37220 * costheta + 50;
		positions[iVert + 1 + i] = vec3(-400, y, z);
		normals[iVert + 1 + i] = vec3(-1, 0, 0);
		positions[iVert + n + 3 + i] = vec3(-376, y, z);
		normals[iVert + n + 3 + i] = vec3(1, 0, 0);
		positions[iVert + 2 * n + 5 + i] = vec3(376, y, z);
		normals[iVert + 2 * n + 5 + i] = vec3(-1, 0, 0);
		positions[iVert + 3 * n + 7 + i] = vec3(400, y, z);
		normals[iVert + 3 * n + 7 + i] = vec3(1, 0, 0);
		positions[iVert + 4 * n + 8 + i] = vec3(-400, y, z);
		normals[iVert + 4 * n + 8 + i] = vec3(0, sintheta, costheta);
		positions[iVert + 5 * n + 8 + i] = vec3(-376, y, z);
		normals[iVert + 5 * n + 8 + i] = vec3(0, sintheta, costheta);
		positions[iVert + 6 * n + 8 + i] = vec3(376, y, z);
		normals[iVert + 6 * n + 8 + i] = vec3(0, sintheta, costheta);
		positions[iVert + 7 * n + 8 + i] = vec3(400, y, z);
		normals[iVert + 7 * n + 8 + i] = vec3(0, sintheta, costheta);
		positions[iVert + 8 * n + 8 + i] = vec3(-376, y, z - 50);
		normals[iVert + 8 * n + 8 + i] = vec3(0, sintheta, costheta);
		texCoords[iVert + 8 * n + 8 + i] = vec2(0.4082031f, 37220 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 9 * n + 8 + i] = vec3(376, y, z - 50);
		normals[iVert + 9 * n + 8 + i] = vec3(0, sintheta, costheta);
		texCoords[iVert + 9 * n + 8 + i] = vec2(0.59179687f, 37220 * theta / (2048 * (n - 1)) * i);
	}
	for (int i = 0; i < n; i++)
	{
		indices[iIdx + 3 * i] = iVert;
		indices[iIdx + 3 * i + 1] = iVert + i + 2;
		indices[iIdx + 3 * i + 2] = iVert + i + 1;
		indices[iIdx + 3 * (n + i)] = iVert + n + 2;
		indices[iIdx + 3 * (n + i) + 1] = iVert + n + 2 + i + 1;
		indices[iIdx + 3 * (n + i) + 2] = iVert + n + 2 + i + 2;
		indices[iIdx + 3 * (2 * n + i)] = iVert + 2 * (n + 2);
		indices[iIdx + 3 * (2 * n + i) + 1] = iVert + 2 * (n + 2) + i + 2;
		indices[iIdx + 3 * (2 * n + i) + 2] = iVert + 2 * (n + 2) + i + 1;
		indices[iIdx + 3 * (3 * n + i)] = iVert + 3 * (n + 2);
		indices[iIdx + 3 * (3 * n + i) + 1] = iVert + 3 * (n + 2) + i + 1;
		indices[iIdx + 3 * (3 * n + i) + 2] = iVert + 3 * (n + 2) + i + 2;
	}
	iIdx += 3 * 4 * n;
	iVert += 4 * n + 8;
	for (int i = 0; i < n - 1; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			indices[iIdx + 6 * (n - 1) * j + 6 * i] = iVert + 2 * n * j + i;
			indices[iIdx + 6 * (n - 1) * j + 6 * i + 1] = iVert + 2 * n * j + n + i + 1;
			indices[iIdx + 6 * (n - 1) * j + 6 * i + 2] = iVert + 2 * n * j + n + i;
			indices[iIdx + 6 * (n - 1) * j + 6 * i + 3] = iVert + 2 * n * j + i;
			indices[iIdx + 6 * (n - 1) * j + 6 * i + 4] = iVert + 2 * n * j + i + 1;
			indices[iIdx + 6 * (n - 1) * j + 6 * i + 5] = iVert + 2 * n * j + n + i + 1;
		}
	}
	iVert += 4 * n;
	iIdx += 3 * 6 * (n - 1);
	for (int i = 0; i < iVert; i++)
	{
		texCoords[i] = vec2(0.09375f, 0.8125f);
	}
	iVert += 2 * n;

	positions[iVert] = vec3(-400, 10380, 70);
	positions[iVert + 1] = vec3(-400, 10380, 0);
	positions[iVert + 2] = vec3(-400, 1080, 305);
	positions[iVert + 3] = vec3(-400, 1080, 375);
	normals[iVert] = vec3(-1, 0, 0);
	normals[iVert + 1] = vec3(-1, 0, 0);
	normals[iVert + 2] = vec3(-1, 0, 0);
	normals[iVert + 3] = vec3(-1, 0, 0);
	positions[iVert + 4] = vec3(-376, 10380, 20);
	positions[iVert + 5] = vec3(-376, 10380, 70);
	positions[iVert + 6] = vec3(-376, 1080, 375);
	positions[iVert + 7] = vec3(-376, 1080, 325);
	normals[iVert + 4] = vec3(1, 0, 0);
	normals[iVert + 5] = vec3(1, 0, 0);
	normals[iVert + 6] = vec3(1, 0, 0);
	normals[iVert + 7] = vec3(1, 0, 0);
	positions[iVert + 8] = vec3(376, 10380, 70);
	positions[iVert + 9] = vec3(376, 10380, 20);
	positions[iVert + 10] = vec3(376, 1080, 325);
	positions[iVert + 11] = vec3(376, 1080, 375);
	normals[iVert + 8] = vec3(-1, 0, 0);
	normals[iVert + 9] = vec3(-1, 0, 0);
	normals[iVert + 10] = vec3(-1, 0, 0);
	normals[iVert + 11] = vec3(-1, 0, 0);
	positions[iVert + 12] = vec3(400, 10380, 0);
	positions[iVert + 13] = vec3(400, 10380, 70);
	positions[iVert + 14] = vec3(400, 1080, 375);
	positions[iVert + 15] = vec3(400, 1080, 305);
	normals[iVert + 12] = vec3(1, 0, 0);
	normals[iVert + 13] = vec3(1, 0, 0);
	normals[iVert + 14] = vec3(1, 0, 0);
	normals[iVert + 15] = vec3(1, 0, 0);
	positions[iVert + 16] = vec3(-400, 10380, 0);
	positions[iVert + 17] = vec3(400, 10380, 0);
	positions[iVert + 18] = vec3(400, 1080, 305);
	positions[iVert + 19] = vec3(-400, 1080, 305);
	normals[iVert + 16] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	normals[iVert + 17] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	normals[iVert + 18] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	normals[iVert + 19] = vec3(0, -61.0f / 1861, -1860.0f / 1861);
	positions[iVert + 20] = vec3(-376, 10380, 70);
	positions[iVert + 21] = vec3(-400, 10380, 70);
	positions[iVert + 22] = vec3(-400, 1080, 375);
	positions[iVert + 23] = vec3(-376, 1080, 375);
	normals[iVert + 20] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 21] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 22] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 23] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	positions[iVert + 24] = vec3(400, 10380, 70);
	positions[iVert + 25] = vec3(376, 10380, 70);
	positions[iVert + 26] = vec3(376, 1080, 375);
	positions[iVert + 27] = vec3(400, 1080, 375);
	normals[iVert + 24] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 25] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 26] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 27] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	for (int i = 0; i < 28; i++)
	{
		texCoords[iVert + i] = vec2(0.09375f, 0.8125f);
	}
	positions[iVert + 28] = vec3(-376, 10380, 20);
	positions[iVert + 29] = vec3(-376, 1080, 325);
	positions[iVert + 30] = vec3(376, 1080, 325);
	positions[iVert + 31] = vec3(376, 10380, 20);
	normals[iVert + 28] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 29] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 30] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 31] = vec3(0, 61.0f / 1861, 1860.0f / 1861);
	texCoords[iVert + 28] = vec2(0.4082031f, 0.0f);
	texCoords[iVert + 29] = vec2(0.4082031f, 1861.0f / 2048);
	texCoords[iVert + 30] = vec2(0.59179687f, 1861.0f / 2048);
	texCoords[iVert + 31] = vec2(0.59179687f, 0.0f);
	for (int i = 0; i < 8; i++)
	{
		indices[iIdx + 6 * i] = iVert + 4 * i;
		indices[iIdx + 6 * i + 1] = iVert + 4 * i + 1;
		indices[iIdx + 6 * i + 2] = iVert + 4 * i + 2;
		indices[iIdx + 6 * i + 3] = iVert + 4 * i;
		indices[iIdx + 6 * i + 4] = iVert + 4 * i + 2;
		indices[iIdx + 6 * i + 5] = iVert + 4 * i + 3;
	}
	iVert += 32;
	iIdx += 48;

	theta = asin(61.0f / 1861);
	n = 2 * theta * sqrtf(37220) + 3;
	dtheta = 2 * theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float costheta = cos(i * dtheta - theta);
		float sintheta = sin(i * dtheta - theta);
		float y = -140 - 37220 * sintheta;
		float z = 37220 * costheta - 36875;
		positions[iVert + i] = vec3(376, y, z);
		normals[iVert + i] = vec3(-1, 0, 0);
		positions[iVert + n + i] = vec3(376, y, z + 50);
		normals[iVert + n + i] = vec3(-1, 0, 0);
		positions[iVert + 2 * n + i] = vec3(376, y, z + 50);
		normals[iVert + 2 * n + i] = vec3(0, -sintheta, costheta);
		positions[iVert + 3 * n + i] = vec3(400, y, z + 50);
		normals[iVert + 3 * n + i] = vec3(0, -sintheta, costheta);
		positions[iVert + 4 * n + i] = vec3(400, y, z + 50);
		normals[iVert + 4 * n + i] = vec3(1, 0, 0);
		positions[iVert + 5 * n + i] = vec3(400, y, z - 20);
		normals[iVert + 5 * n + i] = vec3(1, 0, 0);
		positions[iVert + 6 * n + i] = vec3(400, y, z - 20);
		normals[iVert + 6 * n + i] = vec3(0, sintheta, -costheta);
		positions[iVert + 7 * n + i] = vec3(-400, y, z - 20);
		normals[iVert + 7 * n + i] = vec3(0, sintheta, -costheta);
		positions[iVert + 8 * n + i] = vec3(-400, y, z - 20);
		normals[iVert + 8 * n + i] = vec3(-1, 0, 0);
		positions[iVert + 9 * n + i] = vec3(-400, y, z + 50);
		normals[iVert + 9 * n + i] = vec3(-1, 0, 0);
		positions[iVert + 10 * n + i] = vec3(-400, y, z + 50);
		normals[iVert + 10 * n + i] = vec3(0, -sintheta, costheta);
		positions[iVert + 11 * n + i] = vec3(-376, y, z + 50);
		normals[iVert + 11 * n + i] = vec3(0, -sintheta, costheta);
		positions[iVert + 12 * n + i] = vec3(-376, y, z + 50);
		normals[iVert + 12 * n + i] = vec3(1, 0, 0);
		positions[iVert + 13 * n + i] = vec3(-376, y, z);
		normals[iVert + 13 * n + i] = vec3(1, 0, 0);
		positions[iVert + 14 * n + i] = vec3(-376, y, z);
		normals[iVert + 14 * n + i] = vec3(0, -sintheta, costheta);
		texCoords[iVert + 14 * n + i] = vec2(0.4082031f, 37220 * 2 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 15 * n + i] = vec3(376, y, z);
		normals[iVert + 15 * n + i] = vec3(0, -sintheta, costheta);
		texCoords[iVert + 15 * n + i] = vec2(0.59179687f, 37220 * 2 * theta / (2048 * (n - 1)) * i);
	}
	for (int i = 0; i < 14 * n; i++)
	{
		texCoords[iVert + i] = vec2(0.09375f, 0.8125f);
	}
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[iIdx + 6 * (i * (n - 1) + j)] = iVert + 2 * (i * n) + j;
			indices[iIdx + 6 * (i * (n - 1) + j) + 1] = iVert + 2 * (i * n) + j + n + 1;
			indices[iIdx + 6 * (i * (n - 1) + j) + 2] = iVert + 2 * (i * n) + j + n;
			indices[iIdx + 6 * (i * (n - 1) + j) + 3] = iVert + 2 * (i * n) + j;
			indices[iIdx + 6 * (i * (n - 1) + j) + 4] = iVert + 2 * (i * n) + j + 1;
			indices[iIdx + 6 * (i * (n - 1) + j) + 5] = iVert + 2 * (i * n) + j + n + 1;
		}
	}
	iVert += 16 * n;
	iIdx += 6 * 8 * (n - 1);

	positions[iVert] = vec3(-400, -1360, 375);
	positions[iVert + 1] = vec3(-400, -1360, 305);
	positions[iVert + 2] = vec3(-400, -3640, 305 - 2280.0f * 61 / 1860);
	positions[iVert + 3] = vec3(-400, -3640, 375 - 2280.0f * 61 / 1860);
	normals[iVert] = vec3(-1, 0, 0);
	normals[iVert + 1] = vec3(-1, 0, 0);
	normals[iVert + 2] = vec3(-1, 0, 0);
	normals[iVert + 3] = vec3(-1, 0, 0);
	positions[iVert + 4] = vec3(-376, -1360, 325);
	positions[iVert + 5] = vec3(-376, -1360, 375);
	positions[iVert + 6] = vec3(-376, -3640, 375 - 2280.0f * 61 / 1860);
	positions[iVert + 7] = vec3(-376, -3640, 325 - 2280.0f * 61 / 1860);
	normals[iVert + 4] = vec3(1, 0, 0);
	normals[iVert + 5] = vec3(1, 0, 0);
	normals[iVert + 6] = vec3(1, 0, 0);
	normals[iVert + 7] = vec3(1, 0, 0);
	positions[iVert + 8] = vec3(376, -1360, 375);
	positions[iVert + 9] = vec3(376, -1360, 325);
	positions[iVert + 10] = vec3(376, -3640, 325 - 2280.0f * 61 / 1860);
	positions[iVert + 11] = vec3(376, -3640, 375 - 2280.0f * 61 / 1860);
	normals[iVert + 8] = vec3(-1, 0, 0);
	normals[iVert + 9] = vec3(-1, 0, 0);
	normals[iVert + 10] = vec3(-1, 0, 0);
	normals[iVert + 11] = vec3(-1, 0, 0);
	positions[iVert + 12] = vec3(400, -1360, 305);
	positions[iVert + 13] = vec3(400, -1360, 375);
	positions[iVert + 14] = vec3(400, -3640, 375 - 2280.0f * 61 / 1860);
	positions[iVert + 15] = vec3(400, -3640, 305 - 2280.0f * 61 / 1860);
	normals[iVert + 12] = vec3(1, 0, 0);
	normals[iVert + 13] = vec3(1, 0, 0);
	normals[iVert + 14] = vec3(1, 0, 0);
	normals[iVert + 15] = vec3(1, 0, 0);
	positions[iVert + 16] = vec3(-400, -1360, 305);
	positions[iVert + 17] = vec3(400, -1360, 305);
	positions[iVert + 18] = vec3(400, -3640, 305 - 2280.0f * 61 / 1860);
	positions[iVert + 19] = vec3(-400, -3640, 305 - 2280.0f * 61 / 1860);
	normals[iVert + 16] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	normals[iVert + 17] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	normals[iVert + 18] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	normals[iVert + 19] = vec3(0, 61.0f / 1861, -1860.0f / 1861);
	positions[iVert + 20] = vec3(-376, -1360, 375);
	positions[iVert + 21] = vec3(-400, -1360, 375);
	positions[iVert + 22] = vec3(-400, -3640, 375 - 2280.0f * 61 / 1860);
	positions[iVert + 23] = vec3(-376, -3640, 375 - 2280.0f * 61 / 1860);
	normals[iVert + 20] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 21] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 22] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 23] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	positions[iVert + 24] = vec3(400, -1360, 375);
	positions[iVert + 25] = vec3(376, -1360, 375);
	positions[iVert + 26] = vec3(376, -3640, 375 - 2280.0f * 61 / 1860);
	positions[iVert + 27] = vec3(400, -3640, 375 - 2280.0f * 61 / 1860);
	normals[iVert + 24] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 25] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 26] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 27] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	for (int i = 0; i < 28; i++)
	{
		texCoords[iVert + i] = vec2(0.09375f, 0.8125f);
	}
	positions[iVert + 28] = vec3(-376, -1360, 325);
	positions[iVert + 29] = vec3(-376, -3640, 325 - 2280.0f * 61 / 1860);
	positions[iVert + 30] = vec3(376, -3640, 325 - 2280.0f * 61 / 1860);
	positions[iVert + 31] = vec3(376, -1360, 325);
	normals[iVert + 28] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 29] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 30] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	normals[iVert + 31] = vec3(0, -61.0f / 1861, 1860.0f / 1861);
	texCoords[iVert + 28] = vec2(0.4082031f, 0.0f);
	texCoords[iVert + 29] = vec2(0.4082031f, 2280.0f * 1861 / 1860 / 2048);
	texCoords[iVert + 30] = vec2(0.59179687f, 2280.0f * 1861 / 1860 / 2048);
	texCoords[iVert + 31] = vec2(0.59179687f, 0.0f);
	for (int i = 0; i < 8; i++)
	{
		indices[iIdx + 6 * i] = iVert + 4 * i;
		indices[iIdx + 6 * i + 1] = iVert + 4 * i + 1;
		indices[iIdx + 6 * i + 2] = iVert + 4 * i + 2;
		indices[iIdx + 6 * i + 3] = iVert + 4 * i;
		indices[iIdx + 6 * i + 4] = iVert + 4 * i + 2;
		indices[iIdx + 6 * i + 5] = iVert + 4 * i + 3;
	}
	iVert += 32;
	iIdx += 48;

	theta = acos(-12.0f / 13) - 244.0f / 600;
	n = theta * sqrtf(3400) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float costheta = cos(i * dtheta);
		float sintheta = sin(i * dtheta);
		float z = float(n - 1 - i) / (n - 1) * 7020 * 61 / 1860 + 20;
		positions[iVert + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z);
		normals[iVert + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + n + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z + 50);
		normals[iVert + n + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + 2 * n + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z + 50);
		normals[iVert + 2 * n + i] = vec3(61 * sintheta / 1861, -61 * costheta / 1861, 1860.0f / 1861);
		positions[iVert + 3 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, z + 50);
		normals[iVert + 3 * n + i] = vec3(61 * sintheta / 1861, -61 * costheta / 1861, 1860.0f / 1861);
		positions[iVert + 4 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, z + 50);
		normals[iVert + 4 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 5 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, z - 20);
		normals[iVert + 5 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 6 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, z - 20);
		normals[iVert + 6 * n + i] = vec3(-61 * sintheta / 1861, 61 * costheta / 1861, -1860.0f / 1861);
		positions[iVert + 7 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, z - 20);
		normals[iVert + 7 * n + i] = vec3(-61 * sintheta / 1861, 61 * costheta / 1861, -1860.0f / 1861);
		positions[iVert + 8 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, z - 20);
		normals[iVert + 8 * n + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + 9 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, z + 50);
		normals[iVert + 9 * n + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + 10 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, z + 50);
		normals[iVert + 10 * n + i] = vec3(61 * sintheta / 1861, -61 * costheta / 1861, 1860.0f / 1861);
		positions[iVert + 11 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z + 50);
		normals[iVert + 11 * n + i] = vec3(61 * sintheta / 1861, -61 * costheta / 1861, 1860.0f / 1861);
		positions[iVert + 12 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z + 50);
		normals[iVert + 12 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 13 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z);
		normals[iVert + 13 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 14 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z);
		normals[iVert + 14 * n + i] = vec3(61 * sintheta / 1861, -61 * costheta / 1861, 1860.0f / 1861);
		texCoords[iVert + 14 * n + i] = vec2(0.4082031f, 3000 * theta * 1861 / 1860 / (2048 * (n - 1)) * i);
		positions[iVert + 15 * n + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z);
		normals[iVert + 15 * n + i] = vec3(61 * sintheta / 1861, -61 * costheta / 1861, 1860.0f / 1861);
		texCoords[iVert + 15 * n + i] = vec2(0.59179687f, 3000 * theta * 1861 / 1860 / (2048 * (n - 1)) * i);
	}
	for (int i = 0; i < 14 * n; i++)
	{
		texCoords[iVert + i] = vec2(0.09375f, 0.8125f);
	}
	for (int i = 0; i < 8; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[iIdx + 6 * (i * (n - 1) + j)] = iVert + 2 * (i * n) + j;
			indices[iIdx + 6 * (i * (n - 1) + j) + 1] = iVert + 2 * (i * n) + j + n + 1;
			indices[iIdx + 6 * (i * (n - 1) + j) + 2] = iVert + 2 * (i * n) + j + n;
			indices[iIdx + 6 * (i * (n - 1) + j) + 3] = iVert + 2 * (i * n) + j;
			indices[iIdx + 6 * (i * (n - 1) + j) + 4] = iVert + 2 * (i * n) + j + 1;
			indices[iIdx + 6 * (i * (n - 1) + j) + 5] = iVert + 2 * (i * n) + j + n + 1;
		}
	}
	iVert += 16 * n;
	iIdx += 6 * 8 * (n - 1);

	theta = 244.0f / 600;
	n = theta * sqrtf(3400) + 3;
	dtheta = theta / n;
	n++;
	for (int i = 0; i < n; i++)
	{
		float costheta = -cos(theta - i * dtheta + acos(12.0f / 13));
		float sintheta = sin(theta - i * dtheta + acos(12.0f / 13));
		float z = 37220 - sqrtf(37220 * 37220 - (float(n - 1 - i) * (n - 1 - i) / ((n - 1) * (n - 1)) * 1220 * 1220));
		positions[iVert + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z);
		normals[iVert + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + n + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z + 50);
		normals[iVert + n + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + 2 * n + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z + 50);
		normals[iVert + 2 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sintheta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * costheta / 37220, 1 - z / 37220);
		positions[iVert + 3 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, z + 50);
		normals[iVert + 3 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sintheta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * costheta / 37220, 1 - z / 37220);
		positions[iVert + 4 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, z + 50);
		normals[iVert + 4 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 5 * n + i] = vec3(3000 - 2600 * costheta, -3640 - 2600 * sintheta, 0);
		normals[iVert + 5 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 6 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, 0);
		normals[iVert + 6 * n + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + 7 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, z + 50);
		normals[iVert + 7 * n + i] = vec3(-costheta, -sintheta, 0);
		positions[iVert + 8 * n + i] = vec3(3000 - 3400 * costheta, -3640 - 3400 * sintheta, z + 50);
		normals[iVert + 8 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sintheta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * costheta / 37220, 1 - z / 37220);
		positions[iVert + 9 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z + 50);
		normals[iVert + 9 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sintheta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * costheta / 37220, 1 - z / 37220);
		positions[iVert + 10 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z + 50);
		normals[iVert + 10 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 11 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z);
		normals[iVert + 11 * n + i] = vec3(costheta, sintheta, 0);
		positions[iVert + 12 * n + i] = vec3(3000 - 3376 * costheta, -3640 - 3376 * sintheta, z);
		normals[iVert + 12 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sintheta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * costheta / 37220, 1 - z / 37220);
		texCoords[iVert + 12 * n + i] = vec2(0.4082031f, 3000 * theta / (2048 * (n - 1)) * i);
		positions[iVert + 13 * n + i] = vec3(3000 - 2624 * costheta, -3640 - 2624 * sintheta, z);
		normals[iVert + 13 * n + i] = vec3(float(n - 1 - i) / (n - 1) * 1220 * sintheta / 37220, -float(n - 1 - i) / (n - 1) * 1220 * costheta / 37220, 1 - z / 37220);
		texCoords[iVert + 13 * n + i] = vec2(0.59179687f, 3000 * theta / (2048 * (n - 1)) * i);
	}
	for (int i = 0; i < 12 * n; i++)
	{
		texCoords[iVert + i] = vec2(0.09375f, 0.8125f);
	}
	for (int i = 0; i < 7; i++)
	{
		for (int j = 0; j < n - 1; j++)
		{
			indices[iIdx + 6 * (i * (n - 1) + j)] = iVert + 2 * (i * n) + j;
			indices[iIdx + 6 * (i * (n - 1) + j) + 1] = iVert + 2 * (i * n) + j + n + 1;
			indices[iIdx + 6 * (i * (n - 1) + j) + 2] = iVert + 2 * (i * n) + j + n;
			indices[iIdx + 6 * (i * (n - 1) + j) + 3] = iVert + 2 * (i * n) + j;
			indices[iIdx + 6 * (i * (n - 1) + j) + 4] = iVert + 2 * (i * n) + j + 1;
			indices[iIdx + 6 * (i * (n - 1) + j) + 5] = iVert + 2 * (i * n) + j + n + 1;
		}
	}
	iVert += 14 * n;
	iIdx += 6 * 7 * (n - 1);

	positions[iVert + 0] = vec3(3000 + 3400 * 12.0f / 13, -3640 - 3400 * 5.0f / 13, 0);
	positions[iVert + 1] = vec3(3000 + 3376 * 12.0f / 13, -3640 - 3376 * 5.0f / 13, 0);
	positions[iVert + 2] = vec3(3000 + 3376 * 12.0f / 13, -3640 - 3376 * 5.0f / 13, 50);
	positions[iVert + 3] = vec3(3000 + 3400 * 12.0f / 13, -3640 - 3400 * 5.0f / 13, 50);
	positions[iVert + 4] = vec3(3000 + 2624 * 12.0f / 13, -3640 - 2624 * 5.0f / 13, 0);
	positions[iVert + 5] = vec3(3000 + 2600 * 12.0f / 13, -3640 - 2600 * 5.0f / 13, 0);
	positions[iVert + 6] = vec3(3000 + 2600 * 12.0f / 13, -3640 - 2600 * 5.0f / 13, 50);
	positions[iVert + 7] = vec3(3000 + 2624 * 12.0f / 13, -3640 - 2624 * 5.0f / 13, 50);
	for (int i = 0; i < 8; i++)
	{
		normals[iVert + i] = vec3(5.0f / 13, 12.0f / 13, 0);
		texCoords[iVert + i] = vec2(0.09375f, 0.8125f);
	}
	indices[iIdx] = iVert;
	indices[iIdx + 1] = iVert + 1;
	indices[iIdx + 2] = iVert + 2;
	indices[iIdx + 3] = iVert;
	indices[iIdx + 4] = iVert + 2;
	indices[iIdx + 5] = iVert + 3;
	indices[iIdx + 6] = iVert + 4;
	indices[iIdx + 7] = iVert + 5;
	indices[iIdx + 8] = iVert + 6;
	indices[iIdx + 9] = iVert + 4;
	indices[iIdx + 10] = iVert + 6;
	indices[iIdx + 11] = iVert + 7;
	iVert += 8;
	iIdx += 12;

	n = 2 * PI * sqrtf(40) + 3;
	dtheta = 2 * PI / n;
	for (int i = 0; i < 4; i++)
	{
		vec3 pillarPos[10] = { {-300 + 200 * i, 8860,  65},
							{-300 + 200 * i, 6860, 130},
							{-300 + 200 * i, 4860, 195},
							{-300 + 200 * i, 2860, 260},
							{-300 + 200 * i, 860, 325},
							{-300 + 200 * i, -1140, 325},
							{-300 + 200 * i, -3140, 260},
							{3000 - (3300 - i * 200) * cos(300.0f / 600), -3640 - (3300 - i * 200) * sin(300.0f / 600), 195},
							{3000 - (3300 - i * 200) * cos(700.0f / 600), -3640 - (3300 - i * 200) * sin(700.0f / 600), 130},
							{3000 - (3300 - i * 200) * cos(1100.0f / 600), -3640 - (3300 - i * 200) * sin(1100.0f / 600), 65} };
		for (int j = 0; j < 10; j++)
		{
			for (int k = 0; k < n; k++)
			{
				constexpr float R = 40;
				float costheta = cos(k * dtheta);
				float sintheta = sin(k * dtheta);
				float x = pillarPos[j].x + R * costheta;
				float y = pillarPos[j].y + R * sintheta;
				positions[iVert + 2 * k] = vec3(x, y, 0);
				positions[iVert + 2 * k + 1] = vec3(x, y, pillarPos[j].z);
				normals[iVert + 2 * k] = vec3(costheta, sintheta, 0);
				normals[iVert + 2 * k + 1] = vec3(costheta, sintheta, 0);
				texCoords[iVert + 2 * k] = vec2(0.09375f, 0.8125f);
				texCoords[iVert + 2 * k + 1] = vec2(0.09375f, 0.8125f);
				indices[iIdx + 6 * k] = iVert + 2 * k;
				indices[iIdx + 6 * k + 1] = iVert + (2 * k + 3) % (2 * n);
				indices[iIdx + 6 * k + 2] = iVert + 2 * k + 1;
				indices[iIdx + 6 * k + 3] = iVert + 2 * k;
				indices[iIdx + 6 * k + 4] = iVert + (2 * k + 2) % (2 * n);
				indices[iIdx + 6 * k + 5] = iVert + (2 * k + 3) % (2 * n);
			}
			iVert += 2 * n;
			iIdx += 6 * n;
		}
	}

	glGenVertexArrays(1, &bridgeVAO);
	glBindVertexArray(bridgeVAO);
	glGenBuffers(1, &bridgeVBO);
	glBindBuffer(GL_ARRAY_BUFFER, bridgeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals) + sizeof(texCoords), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(texCoords), texCoords);
	glGenBuffers(1, &bridgeEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bridgeEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)sizeof(positions));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

#define X1 40
#define X2 80
#define X3 60
#define X4 70

#define Y0 190
#define Y1 105
#define Y2 30
#define Y3 170

#define Z0 60
#define Z1 80
#define Z2 75
#define Z3 90
#define Z4 20
#define Z5 140

const vec3 carBoundray[8] = { { X2, Y0, Z5 }, { -X2, Y0, Z5 }, { X2, -Y0, Z5 }, { -X2, -Y0, Z5 }, { X2, Y0, 0 }, { -X2, Y0, 0 }, { X2, -Y0, 0 }, { -X2, -Y0, 0 } };

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
	constexpr int verticesSize = 884;
	constexpr float hubColor[3] = { 0.3f,0.3f,0.3f };
	constexpr float tyreColor[3] = { 0.03f,0.03f,0.03f };
	glProgramUniform3f(spCarDay, glGetUniformLocation(spCarDay, "materials[0].ambientAndDiffuse"), 0.08f, 0.08f, 0.12f);
	glProgramUniform3f(spCarDay, glGetUniformLocation(spCarDay, "materials[0].specular"), 1.0f, 1.0f, 1.0f);
	glProgramUniform1i(spCarDay, glGetUniformLocation(spCarDay, "materials[0].shininess"), 256);
	glProgramUniform3fv(spCarDay, glGetUniformLocation(spCarDay, "materials[1].ambientAndDiffuse"), 1, hubColor);
	glProgramUniform3fv(spCarDay, glGetUniformLocation(spCarDay, "materials[1].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(spCarDay, glGetUniformLocation(spCarDay, "materials[1].shininess"), 0);
	glProgramUniform3fv(spCarDay, glGetUniformLocation(spCarDay, "materials[2].ambientAndDiffuse"), 1, tyreColor);
	glProgramUniform3fv(spCarDay, glGetUniformLocation(spCarDay, "materials[2].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(spCarDay, glGetUniformLocation(spCarDay, "materials[2].shininess"), 0);
	glProgramUniform3f(spCarDay, glGetUniformLocation(spCarDay, "materials[3].ambientAndDiffuse"), 0.6f, 0.6f, 0.6f);
	glProgramUniform3f(spCarDay, glGetUniformLocation(spCarDay, "materials[3].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(spCarDay, glGetUniformLocation(spCarDay, "materials[3].shininess"), 32);
	glProgramUniform3f(spCarDay, glGetUniformLocation(spCarDay, "materials[4].ambientAndDiffuse"), 0.6f, 0.04f, 0.04f);
	glProgramUniform3f(spCarDay, glGetUniformLocation(spCarDay, "materials[4].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(spCarDay, glGetUniformLocation(spCarDay, "materials[4].shininess"), 32);

	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[0].ambientAndDiffuse"), 0.08f, 0.08f, 0.12f);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[0].specular"), 1.0f, 1.0f, 1.0f);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[0].shininess"), 256);
	glProgramUniform3fv(spCarNight, glGetUniformLocation(spCarNight, "materials[1].ambientAndDiffuse"), 1, hubColor);
	glProgramUniform3fv(spCarNight, glGetUniformLocation(spCarNight, "materials[1].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[1].shininess"), 0);
	glProgramUniform3fv(spCarNight, glGetUniformLocation(spCarNight, "materials[2].ambientAndDiffuse"), 1, tyreColor);
	glProgramUniform3fv(spCarNight, glGetUniformLocation(spCarNight, "materials[2].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[2].shininess"), 0);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[3].ambientAndDiffuse"), 0.6f, 0.6f, 0.6f);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[3].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[3].shininess"), 32);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[4].ambientAndDiffuse"), 0.6f, 0.04f, 0.04f);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[4].specular"), 0.5f, 0.5f, 0.5f);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[4].shininess"), 32);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[5].ambientAndDiffuse"), 5.0f, 5.0f, 4.5f);
	glProgramUniform3fv(spCarNight, glGetUniformLocation(spCarNight, "materials[5].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[5].shininess"), 0);
	glProgramUniform3f(spCarNight, glGetUniformLocation(spCarNight, "materials[6].ambientAndDiffuse"), 1.5f, 0.1f, 0.1f);
	glProgramUniform3fv(spCarNight, glGetUniformLocation(spCarNight, "materials[6].specular"), 1, COLOR_BLACK);
	glProgramUniform1i(spCarNight, glGetUniformLocation(spCarNight, "materials[6].shininess"), 0);

	vec3 positions[verticesSize] = { {	 POINT0},	{POINT1},	{POINT2},	{POINT3},	//×óÇ°µÆ
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
	vec3 normals[verticesSize];
	int materialIdxs[verticesSize];
	GLuint indices[carEBOsize]{};

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
		materialIdxs[i] = 3;
	}
	for (int i = 8; i < 16; i++)
	{
		materialIdxs[i] = 4;
	}
	for (int i = 16; i < 56; i++)
	{
		materialIdxs[i] = -1;
	}
	for (int i = 56; i < 72; i++)
	{
		materialIdxs[i] = 0;
	}
	for (int i = 0; i < 18; i++)
	{
		for (int j = 0; j < 6; j++)
		{
			constexpr GLuint temp[] = { 0, 1, 2, 0, 2, 3 };
			indices[6 * i + j] = temp[j] + 4 * i;
		}
	}
	int iVert = 72;
	int iIdx = 108;

	int n = 2 * PI * sqrtf(30) + 3;
	float dtheta = 2 * PI / n;
	for (int i = 0; i < n; i++)
	{
		float cosTheta = cos(i * dtheta);
		float sinTheta = sin(i * dtheta);
		constexpr float wheelPos[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[iVert + j * 5 * n + i] = vec3(82 * wheelPos[j][0], 130 * wheelPos[j][1] + 18 * wheelPos[j][0] * cosTheta, 30 + 18 * sinTheta);
			normals[iVert + j * 5 * n + i] = vec3(wheelPos[j][0], 0, 0);
			materialIdxs[iVert + j * 5 * n + i] = 1;

			positions[iVert + (j * 5 + 1) * n + 2 * i] = vec3(82 * wheelPos[j][0], 130 * wheelPos[j][1] + 18 * wheelPos[j][0] * cosTheta, 30 + 18 * sinTheta);
			normals[iVert + (j * 5 + 1) * n + 2 * i] = vec3(wheelPos[j][0], 0, 0);
			materialIdxs[iVert + (j * 5 + 1) * n + 2 * i] = 2;

			positions[iVert + (j * 5 + 1) * n + 2 * i + 1] = vec3(82 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * cosTheta, 30 + 30 * sinTheta);
			normals[iVert + (j * 5 + 1) * n + 2 * i + 1] = vec3(wheelPos[j][0], 0, 0);
			materialIdxs[iVert + (j * 5 + 1) * n + 2 * i + 1] = 2;

			positions[iVert + (j * 5 + 3) * n + 2 * i] = vec3(82 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * cosTheta, 30 + 30 * sinTheta);
			normals[iVert + (j * 5 + 3) * n + 2 * i] = vec3(0, wheelPos[j][0] * cosTheta, sinTheta);
			materialIdxs[iVert + (j * 5 + 3) * n + 2 * i] = 2;

			positions[iVert + (j * 5 + 3) * n + 2 * i + 1] = vec3(60 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * cosTheta, 30 + 30 * sinTheta);
			normals[iVert + (j * 5 + 3) * n + 2 * i + 1] = vec3(0, wheelPos[j][0] * cosTheta, sinTheta);
			materialIdxs[iVert + (j * 5 + 3) * n + 2 * i + 1] = 2;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[iIdx + 3 * ((5 * n - 2) * i + j)] = iVert + i * 5 * n;
			indices[iIdx + 3 * ((5 * n - 2) * i + j) + 1] = iVert + i * 5 * n + j + 1;
			indices[iIdx + 3 * ((5 * n - 2) * i + j) + 2] = iVert + i * 5 * n + j + 2;
		}
		for (int j = 0; j < n; j++)
		{
			indices[iIdx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j)] = iVert + (i * 5 + 1) * n + 2 * j;
			indices[iIdx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 1] = iVert + (i * 5 + 1) * n + 2 * j + 1;
			indices[iIdx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 2] = iVert + (i * 5 + 1) * n + (2 * j + 2) % (2 * n);
			indices[iIdx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 3] = iVert + (i * 5 + 1) * n + (2 * j + 2) % (2 * n);
			indices[iIdx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 4] = iVert + (i * 5 + 1) * n + 2 * j + 1;
			indices[iIdx + 3 * ((5 * n - 2) * i + n - 2 + 2 * j) + 5] = iVert + (i * 5 + 1) * n + (2 * j + 3) % (2 * n);

			indices[iIdx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j)] = iVert + (i * 5 + 3) * n + 2 * j;
			indices[iIdx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 1] = iVert + (i * 5 + 3) * n + 2 * j + 1;
			indices[iIdx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 2] = iVert + (i * 5 + 3) * n + (2 * j + 2) % (2 * n);
			indices[iIdx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 3] = iVert + (i * 5 + 3) * n + (2 * j + 2) % (2 * n);
			indices[iIdx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 4] = iVert + (i * 5 + 3) * n + 2 * j + 1;
			indices[iIdx + 3 * ((5 * n - 2) * i + 3 * n - 2 + 2 * j) + 5] = iVert + (i * 5 + 3) * n + (2 * j + 3) % (2 * n);
		}
	}
	iVert += 4 * 5 * n;
	iIdx += 4 * 3 * (n - 2 + 4 * n);

	n = (2 * acos(1.0f / 5) / dtheta) + 1;
	float theta = n * dtheta / 2;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cosTheta = cos(theta - i * dtheta);
		float sinTheta = sin(theta - i * dtheta);
		constexpr float wheelPos[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[iVert + j * n + i] = vec3(60 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * sinTheta, 30 - 30 * cosTheta);
			normals[iVert + j * n + i] = vec3(-wheelPos[j][0], 0, 0);
			materialIdxs[iVert + j * n + i] = 2;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[iIdx + 3 * ((n - 2) * i + j)] = iVert + i * n;
			indices[iIdx + 3 * ((n - 2) * i + j) + 1] = iVert + i * n + j + 1;
			indices[iIdx + 3 * ((n - 2) * i + j) + 2] = iVert + i * n + j + 2;
		}
	}
	iVert += 4 * n;
	iIdx += 4 * 3 * (n - 2);

	glGenVertexArrays(1, &carVAO);
	glBindVertexArray(carVAO);
	glGenBuffers(1, &carVBO);
	glBindBuffer(GL_ARRAY_BUFFER, carVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals) + sizeof(materialIdxs), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), sizeof(materialIdxs), materialIdxs);
	glGenBuffers(1, &carEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, carEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)sizeof(positions));
	glEnableVertexAttribArray(1);
	glVertexAttribIPointer(2, 1, GL_INT, sizeof(int), (void*)(sizeof(positions) + sizeof(normals)));
	glEnableVertexAttribArray(2);

	glGenBuffers(1, &carMatVBO);
	glBindBuffer(GL_ARRAY_BUFFER, carMatVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_CAR_COUNT * sizeof(mat4), nullptr, GL_STREAM_DRAW);
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

	glGenBuffers(1, &carColorVBO);
	glBindBuffer(GL_ARRAY_BUFFER, carColorVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_CAR_COUNT * sizeof(vec3), nullptr, GL_STREAM_DRAW);
	glEnableVertexAttribArray(7);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glVertexAttribDivisor(7, 1);

	glBindVertexArray(0);
}

void buildCarShadowMesh()
{
	constexpr int verticesSize = 560;

	vec3 positions[verticesSize] = { {	 POINT16},	{POINT17},	{POINT18},	{POINT19},	//Ç°
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
	vec3 normals[verticesSize]{};
	GLuint indices[carShadowEBOsize]{};

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
			constexpr GLuint temp[] = { 0, 1, 2, 0, 2, 3 };
			indices[6 * i + j] = temp[j] + 4 * i;
		}
	}
	int iVert = 44;
	int iIdx = 66;

	int n = 2 * PI * sqrtf(30) + 3;
	float dtheta = 2 * PI / n;
	for (int i = 0; i < n; i++)
	{
		float cosTheta = cos(i * dtheta);
		float sinTheta = sin(i * dtheta);
		constexpr float wheelPos[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[iVert + j * 3 * n + i] = vec3(82 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * cosTheta, 30 + 30 * sinTheta);
			normals[iVert + j * 3 * n + i] = vec3(wheelPos[j][0], 0, 0);

			positions[iVert + (j * 3 + 1) * n + 2 * i] = vec3(82 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * cosTheta, 30 + 30 * sinTheta);
			normals[iVert + (j * 3 + 1) * n + 2 * i] = vec3(0, wheelPos[j][0] * cosTheta, sinTheta);

			positions[iVert + (j * 3 + 1) * n + 2 * i + 1] = vec3(60 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * cosTheta, 30 + 30 * sinTheta);
			normals[iVert + (j * 3 + 1) * n + 2 * i + 1] = vec3(0, wheelPos[j][0] * cosTheta, sinTheta);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[iIdx + 3 * ((3 * n - 2) * i + j)] = iVert + i * 3 * n;
			indices[iIdx + 3 * ((3 * n - 2) * i + j) + 1] = iVert + i * 3 * n + j + 1;
			indices[iIdx + 3 * ((3 * n - 2) * i + j) + 2] = iVert + i * 3 * n + j + 2;
		}
		for (int j = 0; j < n; j++)
		{
			indices[iIdx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j)] = iVert + (i * 3 + 1) * n + 2 * j;
			indices[iIdx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 1] = iVert + (i * 3 + 1) * n + 2 * j + 1;
			indices[iIdx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 2] = iVert + (i * 3 + 1) * n + (2 * j + 2) % (2 * n);
			indices[iIdx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 3] = iVert + (i * 3 + 1) * n + (2 * j + 2) % (2 * n);
			indices[iIdx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 4] = iVert + (i * 3 + 1) * n + 2 * j + 1;
			indices[iIdx + 3 * ((3 * n - 2) * i + n - 2 + 2 * j) + 5] = iVert + (i * 3 + 1) * n + (2 * j + 3) % (2 * n);
		}
	}
	iVert += 4 * 3 * n;
	iIdx += 4 * 3 * (n - 2 + 2 * n);

	n = (2 * acos(1.0f / 5) / dtheta) + 1;
	float theta = n * dtheta / 2;
	n++;
	for (int i = 0; i < n; i++)
	{
		float cosTheta = cos(theta - i * dtheta);
		float sinTheta = sin(theta - i * dtheta);
		constexpr float wheelPos[4][2] = { {1,1},{-1,1},{1,-1},{-1,-1} };
		for (int j = 0; j < 4; j++)
		{
			positions[iVert + j * n + i] = vec3(60 * wheelPos[j][0], 130 * wheelPos[j][1] + 30 * wheelPos[j][0] * sinTheta, 30 - 30 * cosTheta);
			normals[iVert + j * n + i] = vec3(-wheelPos[j][0], 0, 0);
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < n - 2; j++)
		{
			indices[iIdx + 3 * ((n - 2) * i + j)] = iVert + i * n;
			indices[iIdx + 3 * ((n - 2) * i + j) + 1] = iVert + i * n + j + 1;
			indices[iIdx + 3 * ((n - 2) * i + j) + 2] = iVert + i * n + j + 2;
		}
	}
	iVert += 4 * n;
	iIdx += 4 * 3 * (n - 2);

	glGenVertexArrays(1, &carShadowVAO);
	glBindVertexArray(carShadowVAO);
	glGenBuffers(1, &carShadowVBO);
	glBindBuffer(GL_ARRAY_BUFFER, carShadowVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions) + sizeof(normals), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(positions), sizeof(normals), normals);
	glGenBuffers(1, &carShadowEBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, carShadowEBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)sizeof(positions));
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, carMatVBO);
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
	vec3 positions[sunVBOsize];
	int n = sunVBOsize;
	float dtheta = 2 * PI / n;
	for (int i = 0; i < n; i++)
	{
		positions[i] = vec3(400 * cos(i * dtheta), -400 * sin(i * dtheta), 0);
	}

	glGenVertexArrays(1, &sunVAO);
	glBindVertexArray(sunVAO);
	glGenBuffers(1, &sunVBO);
	glBindBuffer(GL_ARRAY_BUFFER, sunVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(positions), nullptr, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(positions), positions);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void*)0);
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