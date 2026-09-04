#include <Windows.h>

#include "terrain.h"

#include <memory>

#include "resource.h"

using namespace glm;

namespace
{
	struct Tile
	{
		bool active = false;
		float min_height;
		float max_height;
	};

	enum TILE_EDGE
	{
		TILE_LEFT = 0b0001,
		TILE_RIGHT = 0b0010,
		TILE_BOTTOM = 0b0100,
		TILE_TOP = 0b1000,
	};
}

constexpr int NUM_TILE_SIDE_VERTICES = TERRAIN_TILE_SIZE + 1;
constexpr int NUM_VERTICES_PER_TILE = NUM_TILE_SIDE_VERTICES * NUM_TILE_SIDE_VERTICES;

static constexpr int NUM_TILES_TO_LOD_PER_ROOT(int lod = NUM_TERRAIN_LOD)
{
	return ((1 << (2 * lod)) - 1) / 3;
}

static constexpr int NUM_INDICES(int mask)
{
	int NUM_INDICES = 3 * 2 * TERRAIN_TILE_SIZE * TERRAIN_TILE_SIZE;
	for (int i = 0; i < 4; i++)
	{
		if (mask & (1 << i))
		{
			NUM_INDICES -= 3 * TERRAIN_TILE_SIZE / 2;
		}
	}
	return NUM_INDICES;
}

static constexpr int TOTAL_NUM_INDICES(int n = 16)
{
	int total_num_indices = 0;
	for (int i = 0; i < n; i++)
	{
		total_num_indices += NUM_INDICES(i);
	}
	return total_num_indices;
}

GLuint terrain_VAO, terrain_VBO, terrain_EBO;
Tile tiles[NUM_TERRAIN_TILE_ROOTS_X][NUM_TERRAIN_TILE_ROOTS_Y][NUM_TILES_TO_LOD_PER_ROOT()];

void buildTerrainMesh()
{
	constexpr int VERT_SIZE = (NUM_TERRAIN_GRID_X + 1) * (NUM_TERRAIN_GRID_Y + 1);
	HRSRC rc_info = FindResource(nullptr, MAKEINTRESOURCE(IDR_TERRAIN_HEIGHT), L"TEXTURE");
	if (rc_info == nullptr)
	{
		printf("ERROR: Can't Find Resource Text Altas\n");
		return;
	}
	HGLOBAL rc_data = LoadResource(nullptr, rc_info);
	if (SizeofResource(nullptr, rc_info) != VERT_SIZE * sizeof(uint16))
	{
		printf("ERROR: Invalid terrian height data size\n");
		return;
	}
	if (rc_data == nullptr)
	{
		printf("ERROR: Can't Load Resource Text Altas\n");
		return;
	}

	auto positions = std::make_unique<vec3[][NUM_TERRAIN_GRID_Y + 1]>(NUM_TERRAIN_GRID_X + 1);
	auto normals = std::make_unique<vec3[][NUM_TERRAIN_GRID_Y + 1]>(NUM_TERRAIN_GRID_X + 1);

	char* height_data = static_cast<char*>(LockResource(rc_data));

	for (int i = 0; i <= NUM_TERRAIN_GRID_Y; i++)
	{
		for (int j = 0; j <= NUM_TERRAIN_GRID_X; j++)
		{
			int idx = i * (NUM_TERRAIN_GRID_X + 1) + j;
			uint16 raw_height;
			memcpy(&raw_height, height_data + idx * sizeof(uint16), sizeof(uint16));
			float height = 0.001f * raw_height - 10.0f;
			positions[j][i] = vec3(-0.5f * NUM_TERRAIN_GRID_X + j, -0.5f * NUM_TERRAIN_GRID_Y + i, height);
		}
	}

	for (int i = 0; i <= NUM_TERRAIN_GRID_X; i++)
	{
		for (int j = 0; j <= NUM_TERRAIN_GRID_Y; j++)
		{
			int idx = i * (NUM_TERRAIN_GRID_X + 1) + j;
			vec3 x = positions[std::min(NUM_TERRAIN_GRID_X, i + 1)][j] - positions[std::max(0, i - 1)][j];
			vec3 y = positions[i][std::min(NUM_TERRAIN_GRID_Y, j + 1)] - positions[i][std::max(0, j - 1)];
			normals[i][j] = normalize(cross(x, y));
		}
	}

	constexpr int TOATAL_NUM_VERTICES = NUM_TERRAIN_TILE_ROOTS_X * NUM_TERRAIN_TILE_ROOTS_Y * NUM_TILES_TO_LOD_PER_ROOT() * NUM_TILE_SIDE_VERTICES * NUM_TILE_SIDE_VERTICES;
	constexpr int POSITION_SIZE = TOATAL_NUM_VERTICES * sizeof(vec3);
	constexpr int NORMAL_SIZE = TOATAL_NUM_VERTICES * sizeof(vec3);

	glGenVertexArrays(1, &terrain_VAO);
	glBindVertexArray(terrain_VAO);
	glGenBuffers(1, &terrain_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_VBO);
	glBufferData(GL_ARRAY_BUFFER, TOATAL_NUM_VERTICES * (sizeof(vec3) + sizeof(vec3) + sizeof(vec2)), nullptr, GL_STATIC_DRAW);

	int data_offset = 0;
	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			for (int k = 0; k < NUM_TERRAIN_LOD; k++)
			{
				for (int m = 0; m < (1 << k); m++)
				{
					for (int n = 0; n < (1 << k); n++)
					{
						vec3 tile_positions[NUM_TILE_SIDE_VERTICES][NUM_TILE_SIDE_VERTICES];
						vec3 tile_normals[NUM_TILE_SIDE_VERTICES][NUM_TILE_SIDE_VERTICES];
						vec2 tile_tex_coords[NUM_TILE_SIDE_VERTICES][NUM_TILE_SIDE_VERTICES];
						float min_height = FLT_MAX;
						float max_height = -FLT_MAX;
						for (int p = 0; p < NUM_TILE_SIDE_VERTICES; p++)
						{
							for (int q = 0; q < NUM_TILE_SIDE_VERTICES; q++)
							{
								int x = i * TERRAIN_TILE_ROOT_SIZE + (m * TERRAIN_TILE_SIZE + p) * (1 << (NUM_TERRAIN_LOD - 1 - k));
								int y = j * TERRAIN_TILE_ROOT_SIZE + (n * TERRAIN_TILE_SIZE + q) * (1 << (NUM_TERRAIN_LOD - 1 - k));
								tile_positions[p][q] = positions[x][y];
								tile_normals[p][q] = normals[x][y];
								tile_tex_coords[p][q] = vec2(0.09375f, 1.0f);
								min_height = std::min(min_height, positions[x][y].z);
								max_height = std::max(max_height, positions[x][y].z);
							}
						}
						(&tiles[0][0][0])[data_offset].min_height = min_height;
						(&tiles[0][0][0])[data_offset].max_height = max_height;
						glBufferSubData(GL_ARRAY_BUFFER, data_offset * NUM_VERTICES_PER_TILE * sizeof(vec3), NUM_VERTICES_PER_TILE * sizeof(vec3), tile_positions);
						glBufferSubData(GL_ARRAY_BUFFER, POSITION_SIZE + data_offset * NUM_VERTICES_PER_TILE * sizeof(vec3), NUM_VERTICES_PER_TILE * sizeof(vec3), tile_normals);
						glBufferSubData(GL_ARRAY_BUFFER, POSITION_SIZE + NORMAL_SIZE + data_offset * NUM_VERTICES_PER_TILE * sizeof(vec2), NUM_VERTICES_PER_TILE * sizeof(vec2), tile_tex_coords);
						data_offset++;
					}
				}
			}
		}
	}

	glGenBuffers(1, &terrain_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, TOTAL_NUM_INDICES() * sizeof(int), nullptr, GL_STATIC_DRAW);

	for (int i = 0; i < 16; i++)
	{
		int indices[NUM_INDICES(0)];
		int idx = 0;
		int j = 0;
		if (i & TILE_LEFT)
		{
			for (int k = 0; k < TERRAIN_TILE_SIZE; k += 2)
			{
				if (k != 0 || !(i & TILE_BOTTOM))
				{
					indices[idx + 0] = k;
					indices[idx + 1] = k + NUM_TILE_SIDE_VERTICES;
					indices[idx + 2] = k + NUM_TILE_SIDE_VERTICES + 1;
					idx += 3;
				}
				indices[idx + 0] = k;
				indices[idx + 1] = k + NUM_TILE_SIDE_VERTICES + 1;
				indices[idx + 2] = k + 2;
				idx += 3;
				indices[idx + 0] = k + NUM_TILE_SIDE_VERTICES + 1;
				indices[idx + 1] = k + 2 + (k == TERRAIN_TILE_SIZE - 2 && i & TILE_TOP ? 2 * NUM_TILE_SIDE_VERTICES : NUM_TILE_SIDE_VERTICES);
				indices[idx + 2] = k + 2;
				idx += 3;
			}
			j++;
		}
		for (; j < TERRAIN_TILE_SIZE - (i & TILE_RIGHT) / TILE_RIGHT; j++)
		{
			int offset = j * NUM_TILE_SIDE_VERTICES;
			for (int k = 0; k < TERRAIN_TILE_SIZE; k++)
			{
				if (k == 0 && i & TILE_BOTTOM)
				{
					if (j & 1)
					{
						indices[idx + 0] = offset - NUM_TILE_SIDE_VERTICES;
						indices[idx + 1] = offset + NUM_TILE_SIDE_VERTICES;
						indices[idx + 2] = offset + 1;
						idx += 3;
					}
				}
				else
				{
					indices[idx + 0] = offset + k;
					indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES;
					indices[idx + 2] = offset + k + (((j + k) & 1) ? 1 : NUM_TILE_SIDE_VERTICES + 1);
					idx += 3;
				}
				if (k == TERRAIN_TILE_SIZE - 1 && i & TILE_TOP)
				{
					if (!(j & 1))
					{
						indices[idx + 0] = offset + 2 * NUM_TILE_SIDE_VERTICES - 2;
						indices[idx + 1] = offset + 3 * NUM_TILE_SIDE_VERTICES - 1;
						indices[idx + 2] = offset + NUM_TILE_SIDE_VERTICES - 1;
						idx += 3;
					}
				}
				else
				{
					indices[idx + 0] = offset + k + (((j + k) & 1) ? NUM_TILE_SIDE_VERTICES : 0);
					indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES + 1;
					indices[idx + 2] = offset + k + 1;
					idx += 3;
				}
			}
		}
		if (i & TILE_RIGHT)
		{
			constexpr int offset = (TERRAIN_TILE_SIZE - 1) * NUM_TILE_SIDE_VERTICES;
			for (int k = 0; k < TERRAIN_TILE_SIZE; k += 2)
			{
				indices[idx + 0] = offset + k + (k == 0 && i & TILE_BOTTOM ? -NUM_TILE_SIDE_VERTICES : 0);
				indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES;
				indices[idx + 2] = offset + k + 1;
				idx += 3;
				indices[idx + 0] = offset + k;
				indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES + 1;
				indices[idx + 2] = offset + k + 2;
				idx += 3;
				if (k != TERRAIN_TILE_SIZE - 2 || !(i & TILE_TOP))
				{
					indices[idx + 0] = offset + k + NUM_TILE_SIDE_VERTICES + 1;
					indices[idx + 1] = offset + k + 2 + (k == TERRAIN_TILE_SIZE - 2 && i & TILE_TOP ? 2 * NUM_TILE_SIDE_VERTICES : NUM_TILE_SIDE_VERTICES);
					indices[idx + 2] = offset + k + 2;
					idx += 3;
				}
			}
		}
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, TOTAL_NUM_INDICES(i) * sizeof(int), NUM_INDICES(i) * sizeof(int), indices);
	}


	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)POSITION_SIZE);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)(POSITION_SIZE + NORMAL_SIZE));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void updateTerrainLOD(float lod_factor, const vec3& camera_position)
{
	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			vec2 root_offset = vec2((i - 0.5f * NUM_TERRAIN_TILE_ROOTS_X) * TERRAIN_TILE_ROOT_SIZE, (j - 0.5f * NUM_TERRAIN_TILE_ROOTS_Y) * TERRAIN_TILE_ROOT_SIZE);
			Tile* root = tiles[i][j];
			[root_offset, root, lod_factor, &camera_position](this auto&& self, int lod, int tile_x, int tile_y) -> void
				{
					Tile& tile = root[NUM_TILES_TO_LOD_PER_ROOT(lod) + tile_x * (1 << lod) + tile_y];
					if (lod == NUM_TERRAIN_LOD - 1)
					{
						tile.active = true;
						return;
					}
					float tile_side_lenght = TERRAIN_TILE_SIZE * (1 << (NUM_TERRAIN_LOD - 1 - lod));
					vec3 bound_min = vec3(root_offset + vec2(tile_side_lenght * tile_x, tile_side_lenght * tile_y), tile.min_height);
					vec3 bound_max = vec3(root_offset + vec2(tile_side_lenght * (tile_x + 1), tile_side_lenght * (tile_y + 1)), tile.max_height);
					float distance = length(clamp(camera_position, bound_min, bound_max) - camera_position);
					if (distance > lod_factor * tile_side_lenght)
					{
						tile.active = true;
					}
					else
					{
						tile.active = false;
						self(lod + 1, 2 * tile_x, 2 * tile_y);
						self(lod + 1, 2 * tile_x + 1, 2 * tile_y);
						self(lod + 1, 2 * tile_x, 2 * tile_y + 1);
						self(lod + 1, 2 * tile_x + 1, 2 * tile_y + 1);
					}
				}(0, 0, 0);
		}
	}
}

void drawTerrainMesh()
{
	glBindVertexArray(terrain_VAO);
	constexpr int MAX_DRAW_TILES = NUM_TERRAIN_TILE_ROOTS_X * NUM_TERRAIN_TILE_ROOTS_Y * (1 << (NUM_TERRAIN_LOD - 1)) * (1 << (NUM_TERRAIN_LOD - 1));
	GLsizei count[MAX_DRAW_TILES];
	void* indicies[MAX_DRAW_TILES];
	GLint basevertex[MAX_DRAW_TILES];
	int tile_cnt = 0;
	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			Tile* root = tiles[i][j];
			[&count, &indicies, &basevertex, &tile_cnt, root](this auto&& self, int lod, int tile_x, int tile_y) -> void
				{
					Tile& tile = root[NUM_TILES_TO_LOD_PER_ROOT(lod) + tile_x * (1 << lod) + tile_y];
					if (tile.active || lod == NUM_TERRAIN_LOD - 1)
					{
						count[tile_cnt] = NUM_INDICES(0);
						indicies[tile_cnt] = (void*)0;
						basevertex[tile_cnt] = (&tile - &tiles[0][0][0]) * NUM_VERTICES_PER_TILE;
						tile_cnt++;
					}
					else
					{
						self(lod + 1, 2 * tile_x, 2 * tile_y);
						self(lod + 1, 2 * tile_x + 1, 2 * tile_y);
						self(lod + 1, 2 * tile_x, 2 * tile_y + 1);
						self(lod + 1, 2 * tile_x + 1, 2 * tile_y + 1);
					}
				}(0, 0, 0);
		}
	}
	printf("%d\n", tile_cnt);
	glMultiDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_INT, indicies, tile_cnt, basevertex);

}
