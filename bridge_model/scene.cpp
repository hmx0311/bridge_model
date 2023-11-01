#include "scene.h"
#include "Car.h"
#include "common.h"
#include "mesh.h"

#include <cmath>

#include "glm\gtx\transform.hpp"
#include "glm\matrix.hpp"

using namespace glm;

GLuint HIGHWAY_TEX;

Lane* lanes[6];

constexpr vec3 groundColor(0.05f, 0.5f, 0.05f);
constexpr vec3 cementColor(0.2f, 0.2f, 0.2f);
constexpr vec3 roadColor(0.1f, 0.1f, 0.1f);

void initLanes()
{
	Lane* iter;
	Lane* nextLane;
	Lane* critialLanes[2];
	lanes[0] = new Lane(200000, 2.5f, (mat4&)(translate(vec3(-100000, -200, 0)) * rotate(-PI / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});

	lanes[1] = new Lane(93920, 2.5f, (mat4&)(translate(vec3(-100000, -520, 0)) * rotate(-PI / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter = lanes[1];
	nextLane = new Lane(27160, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(78920, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	critialLanes[0] = nextLane;
	iter = lanes[1];
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-6080, -29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(6080, 29480, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, 28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(0, -28120, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(3000, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(-5.0f / 13) * 2800, 1.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, 0));
			transform *= rotate(-s / 2800, vec3(0, 0, 1));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(asin(61.0f / 1861) * 37220 * 560 / 600, 1.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, 37220 - 37220 * cos(s * 600 / (560 * 37220))));
			transform *= rotate(-7444 * sin(s * 600 / (560 * 37220)) / 600, vec3(0, 0, 1));
			transform *= rotate(-s * 600 / (560 * 37220), vec3(12, -5, 0));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(6556.4215f, 1.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, s / 6556.4215f * 305 * (1 - 456.0f / 1860)));
			transform *= rotate(-s * (acos(-12.0f / 13) - 244.0f / 600) / 6556.4215f, vec3(0, 0, 1));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(2280.0f * 1861 / 1860, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s * 1860 / 1861, s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(2 * asin(61.0f / 1861) * 37220, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -140, -36895));
			transform *= rotate(-s / 37220, vec3(1, 0, 0));
			transform *= translate(vec3(0, 140, 36895));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(9305, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s * 1860 / 1861, -s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(asin(61.0f / 1861) * 37220, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, 11600, 37220));
			transform *= rotate(-s / 37220, vec3(-1, 0, 0));
			transform *= translate(vec3(0, -11600, -37220));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(88400, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s, 0));
			return transform;
		});
	iter->setNextLane(nextLane);

	lanes[2] = new Lane(200000, 2.5f, (mat4&)(translate(vec3(100000, 200, 0)) * rotate(PI / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});

	lanes[3] = new Lane(78440, 2.5f, (mat4&)(translate(vec3(100000, 520, 0)) * rotate(PI / 2, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter = lanes[3];
	nextLane = new Lane(43120, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(78440, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	critialLanes[1] = nextLane;
	iter = lanes[3];
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(21560, 29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-21560, -29480, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(15480, -28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-15480, 28120, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(3000, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(11960 * PI / 2, 1.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(12480, 12800, 0));
			transform *= rotate(-s / 11960, vec3(0, 0, 1));
			transform *= translate(vec3(-12480, -12800, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(87200, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, s, 0));
			return transform;
		});
	iter->setNextLane(nextLane);

	lanes[4] = new Lane(88400, 2.5f, (mat4&)(translate(vec3(-200, 100000, 0)) * rotate(PI, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s, 0));
			return transform;
		});
	iter = lanes[4];
	nextLane = new Lane(asin(61.0f / 1861) * 37220, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, 11600, 37220));
			transform *= rotate(s / 37220, vec3(-1, 0, 0));
			transform *= translate(vec3(0, -11600, -37220));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(9305, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s * 1860 / 1861, s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(2 * asin(61.0f / 1861) * 37220, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -140, -36895));
			transform *= rotate(s / 37220, vec3(1, 0, 0));
			transform *= translate(vec3(0, 140, 36895));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(2280.0f * 1861 / 1860, 2.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s * 1860 / 1861, -s * 61 / 1861));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(7491.97f, 1.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, -s / 7491.97f * 305 * (1 - 456.0f / 1860)));
			transform *= rotate(s * (acos(-12.0f / 13) - 244.0f / 600) / 7491.97f, vec3(0, 0, 1));
			transform *= translate(vec3(-3000, 3640, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(asin(61.0f / 1861) * 37220 * 640 / 600, 1.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(3000, -3640, 37220 - 37220 * cos(asin(61.0f / 1861) - s * 600 / (640 * 37220))));
			transform *= rotate((1220.0f - 37220 * sin(asin(61.0f / 1861) - s * 600 / (640 * 37220))) / 3000, vec3(0, 0, 1));
			transform *= rotate(s * 600 / (640 * 37220), vec3(cos(asin(5.0f / 13) + 244.0f / 600), -sin(asin(5.0f / 13) + 244.0f / 600), 0));
			transform *= translate(vec3(-3000, 3640, -20));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(asin(12.0f / 13) * 6550, 1.0f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(12000, -7390, 0));
			transform *= rotate(-s / 6550, vec3(0, 0, 1));
			transform *= translate(vec3(-12000, 7390, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(3000, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(15000, 28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-15000, -28120, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(21080, -29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(-21080, 29480, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	nextLane->setNextLane(critialLanes[0]);

	lanes[5] = new Lane(87200, 2.5f, (mat4&)(translate(vec3(-520, 100000, 0)) * rotate(PI, vec3(0, 0, 1))), [](float s)->mat4
		{
			mat4 transform = translate(vec3(0, -s, 0));
			return transform;
		});
	iter = lanes[5];
	nextLane = new Lane(11960 * PI / 2, 1.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-12480, 12800, 0));
			transform *= rotate(-s / 11960, vec3(0, 0, 1));
			transform *= translate(vec3(12480, -12800, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(3000, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-s, 0, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-15480, -28120, 0));
			transform *= rotate(s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(15480, 28120, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	iter = nextLane;
	nextLane = new Lane(acos(180.0f / 181) * 28960, 2.5f, (mat4&)iter->transform(iter->length), [](float s)->mat4
		{
			mat4 transform = translate(vec3(-21560, 29480, 0));
			transform *= rotate(-s / 28960, vec3(0, 0, 1));
			transform *= translate(vec3(21560, -29480, 0));
			return transform;
		});
	iter->setNextLane(nextLane);
	nextLane->setNextLane(critialLanes[1]);
}

void initTex()
{
	vec3(*highwayTexData)[8192] = new vec3[4096][8192];
	for (int i = 0; i < 4096; i++)
	{
		for (int j = 0; j < 8192; j++)
		{
			highwayTexData[i][j] = roadColor;
		}
	}

	//accelerate lane1
	for (int i = 40; i < 80; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			for (int k = 0; k < 250; k++)
			{
				highwayTexData[k + 500 * j + 250][i] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
	}
	for (int i = 3840; i < 4096; i++)
	{
		for (int j = 50; j < 70; j++)
		{
			highwayTexData[i][j] = vec3(0.9f, 0.9f, 0.9f);
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
		highwayTexData[i][a] = vec3(color, color, color);
		color = 0.6f - 0.6f * (b - top);
		highwayTexData[i][b] = vec3(color, color, color);
		for (int j = a + 1; j < b; j++)
		{
			highwayTexData[i][j] = vec3(0.9f, 0.9f, 0.9f);
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
				highwayTexData[128 + 512 * j + k][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
	}
	for (int i = 314; i < 334; i++)
	{
		for (int j = 0; j < 4096; j++)
		{
			highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	offset += 1024;

	//single lane
	for (int i = 184; i < 200; i++)
	{
		for (int j = 0; j < 4096; j++)
		{
			highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
		}
	}
	for (int i = 824; i < 840; i++)
	{
		for (int j = 0; j < 4096; j++)
		{
			highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
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
				highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
		else if ((992 <= i && i < 1008) || (1040 <= i && i < 1056))
		{
			for (int j = 0; j < 4096; j++)
			{
				highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.1f);
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
				highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
		else if ((808 <= i && i < 824) || (2248 <= i && i < 2264))
		{
			for (int j = 616; j < 1432; j++)
			{
				highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
			for (int j = 2664; j < 3480; j++)
			{
				highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.9f);
			}
		}
		else if ((1504 <= i && i < 1520) || (1552 <= i && i < 1568))
		{
			for (int j = 0; j < 4096; j++)
			{
				highwayTexData[j][i + offset] = vec3(0.9f, 0.9f, 0.1f);
			}
		}
	}
	offset += 2048;

	for (int i = 3072; i < 3584; i++)
	{
		for (int j = 512; j < 1024; j++)
		{
			highwayTexData[i][j] = cementColor;
		}
	}
	for (int i = 3584; i < 4096; i++)
	{
		for (int j = 512; j < 1024; j++)
		{
			highwayTexData[i][j] = groundColor;
		}
	}

	GLfloat max_TexAni;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, &max_TexAni);
	glGenTextures(1, &HIGHWAY_TEX);
	glBindTexture(GL_TEXTURE_2D, HIGHWAY_TEX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 8192, 4096, 0, GL_RGB, GL_FLOAT, highwayTexData);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_TexAni);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 9);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	glGenerateMipmap(GL_TEXTURE_2D);
	delete[] highwayTexData;
}

void initScene()
{
	initTex();
	initLanes();
	buildMeshes();
}