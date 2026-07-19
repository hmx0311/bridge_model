#include "scene.h"

#include "gtx\transform.hpp"

#include "mesh.h"

using namespace glm;

GLuint highway_tex;

constexpr vec3 GROUND_COLOR(0.05f, 0.5f, 0.05f);
constexpr vec3 CEMENT_COLOR(0.2f, 0.2f, 0.2f);
constexpr vec3 ROAD_COLOR(0.1f, 0.1f, 0.1f);

void initTex()
{
	vec3(*highway_tex_data)[8192] = new vec3[4096][8192];
	for (int i = 0; i < 4096; i++)
	{
		for (int j = 0; j < 8192; j++)
		{
			highway_tex_data[i][j] = ROAD_COLOR;
		}
	}

	//accelerate lane1
	for (int i = 40; i < 80; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 250; k++)
			{
				highway_tex_data[k + 500 * j + 250][i] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
	}
	for (int i = 3840; i < 4096; i++)
	{
		for (int j = 50; j < 70; j++)
		{
			highway_tex_data[i][j] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	for (int i = 0, flag = 0; i < 3840; i++)
	{
		float top, bottom;
		if (flag == 0)
		{
			bottom = sqrtf(5912 * 5912 - (i + 0.5f) * (i + 0.5f)) - 5220;
			if ((bottom + 5220) / (i + 0.5f) < 35.0f / 12.0f)
			{
				flag++;
				bottom = 5980 - sqrtf(5928 * 5928 - (3840 - i - 0.5f) * (3840 - i - 0.5f));
			}
			top = sqrtf(5928 * 5928 - (i + 0.5f) * (i + 0.5f)) - 5220;
		}
		else if (flag == 1)
		{
			bottom = 5980 - sqrtf(5928 * 5928 - (3840 - i - 0.5f) * (3840 - i - 0.5f));
			top = sqrtf(5928 * 5928 - (i + 0.5f) * (i + 0.5f)) - 5220;
			if ((top + 5220) / (i + 0.5f) < 35.0f / 12.0f)
			{
				flag++;
				top = 5980 - sqrtf(5912 * 5912 - (3840 - i - 0.5f) * (3840 - i - 0.5f));
			}
		}
		else
		{
			bottom = 5980 - sqrtf(5928 * 5928 - (3840 - i - 0.5f) * (3840 - i - 0.5f));
			top = 5980 - sqrtf(5912 * 5912 - (3840 - i - 0.5f) * (3840 - i - 0.5f));
		}
		bottom -= 0.5f;
		top -= 0.5f;
		int a = lround(bottom), b = lround(top);
		float color = 0.6f + 0.6f * (a - bottom);
		highway_tex_data[i][a] = vec3(color, color, color);
		color = 0.6f - 0.6f * (b - top);
		highway_tex_data[i][b] = vec3(color, color, color);
		for (int j = a + 1; j < b; j++)
		{
			highway_tex_data[i][j] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	int offset = 1024;

	//accelerate lane2
	for (int i = 944; i < 984; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			for (int k = 0; k < 256; k++)
			{
				highway_tex_data[128 + 512 * j + k][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
	}
	for (int i = 314; i < 334; i++)
	{
		for (int j = 0; j < 4096; j++)
		{
			highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	offset += 1024;

	//single lane
	for (int i = 184; i < 200; i++)
	{
		for (int j = 0; j < 4096; j++)
		{
			highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	for (int i = 824; i < 840; i++)
	{
		for (int j = 0; j < 4096; j++)
		{
			highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	offset += 1024;

	//two lanes
	for (int i = 0; i < 2048; i++)
	{
		if ((296 <= i && i < 312) || (936 <= i && i < 952) || (1096 <= i && i < 1112) || (1736 <= i && i < 1752))
		{
			for (int j = 0; j < 4096; j++)
			{
				highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
		else if ((992 <= i && i < 1008) || (1040 <= i && i < 1056))
		{
			for (int j = 0; j < 4096; j++)
			{
				highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.1f);
			}
		}
	}
	offset += 2048;

	//four lanes
	for (int i = 0; i < 3072; i++)
	{
		if ((168 <= i && i < 184) || (1448 <= i && i < 1464) || (1608 <= i && i < 1624) || (2888 <= i && i < 2904))
		{
			for (int j = 0; j < 4096; j++)
			{
				highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
		else if ((808 <= i && i < 824) || (2248 <= i && i < 2264))
		{
			for (int j = 616; j < 1432; j++)
			{
				highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
			for (int j = 2664; j < 3480; j++)
			{
				highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
		else if ((1504 <= i && i < 1520) || (1552 <= i && i < 1568))
		{
			for (int j = 0; j < 4096; j++)
			{
				highway_tex_data[j][i + offset] = vec3(0.9f, 0.9f, 0.1f);
			}
		}
	}
	offset += 2048;

	for (int i = 3072; i < 3584; i++)
	{
		for (int j = 512; j < 1024; j++)
		{
			highway_tex_data[i][j] = CEMENT_COLOR;
		}
	}
	for (int i = 3584; i < 4096; i++)
	{
		for (int j = 512; j < 1024; j++)
		{
			highway_tex_data[i][j] = GROUND_COLOR;
		}
	}

	GLfloat max_TexAni;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_TexAni);
	glGenTextures(1, &highway_tex);
	glBindTexture(GL_TEXTURE_2D, highway_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8192, 4096, 0, GL_RGB, GL_FLOAT, highway_tex_data);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_TexAni);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] highway_tex_data;
}

void initScene()
{
	initTex();
	buildMeshes();
}