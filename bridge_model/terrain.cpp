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
		bool half_level = false;
		uint8 stitching = 0;
		float min_height;
		float max_height;
		float max_curvature;
	};

	enum EDGE_STITCHING
	{
		STITCHING_LEFT = 0b0001,
		STITCHING_RIGHT = 0b0010,
		STITCHING_BOTTOM = 0b0100,
		STITCHING_TOP = 0b1000,
		STITCHING_HALF = 0b10000,
		STITCHING_MAX = 0b10001,
	};
}

constexpr int NUM_TILE_SIDE_VERTICES = TERRAIN_TILE_SIZE + 1;
constexpr int NUM_VERTICES_PER_TILE = NUM_TILE_SIDE_VERTICES * NUM_TILE_SIDE_VERTICES;

static constexpr int NUM_TILES_TO_LOD_PER_ROOT(int lod = NUM_TERRAIN_LOD)
{
	return ((1 << (2 * lod)) - 1) / 3;
}

static constexpr int TILE_IDX(int level, int x, int y)
{
	return NUM_TILES_TO_LOD_PER_ROOT(level) + x * (1 << level) + y;
}

static constexpr int NUM_INDICES(uint8 stitching)
{
	int num_indices;
	if (stitching & STITCHING_HALF)
	{
		num_indices = 3 * TERRAIN_TILE_SIZE * TERRAIN_TILE_SIZE;
		for (int i = 0; i < 4; i++)
		{
			if (stitching & (1 << i))
			{
				num_indices += 3 * TERRAIN_TILE_SIZE / 2;
			}
		}
	}
	else
	{
		num_indices = 3 * 2 * TERRAIN_TILE_SIZE * TERRAIN_TILE_SIZE;
		for (int i = 0; i < 4; i++)
		{
			if (stitching & (1 << i))
			{
				num_indices -= 3 * TERRAIN_TILE_SIZE / 2;
			}
		}
	}
	return num_indices;
}

static constexpr int INDICES_OFFSET(uint8 stitching = STITCHING_MAX)
{
	int total_num_indices = 0;
	for (int i = 0; i < stitching; i++)
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

	char* height_data = static_cast<char*>(LockResource(rc_data));

	for (int i = 0; i <= NUM_TERRAIN_GRID_Y; i++)
	{
		for (int j = 0; j <= NUM_TERRAIN_GRID_X; j++)
		{
			int idx = i * (NUM_TERRAIN_GRID_X + 1) + j;
			uint16 raw_height;
			memcpy(&raw_height, height_data + idx * sizeof(uint16), sizeof(uint16));
			float height = 0.01f * raw_height - 10.0f;
			positions[j][i] = vec3((-0.5f * NUM_TERRAIN_GRID_X + j) * TERRAIN_FINEST_VERTEX_SPACING, (-0.5f * NUM_TERRAIN_GRID_Y + i) * TERRAIN_FINEST_VERTEX_SPACING, height);
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

	int data_offset = NUM_TERRAIN_TILE_ROOTS_X * NUM_TERRAIN_TILE_ROOTS_Y * NUM_TILES_TO_LOD_PER_ROOT() - 1;
	for (int i = NUM_TERRAIN_TILE_ROOTS_X - 1; i >= 0; i--)
	{
		for (int j = NUM_TERRAIN_TILE_ROOTS_Y - 1; j >= 0; j--)
		{
			for (int k = NUM_TERRAIN_LOD - 1; k >= 0; k--)
			{
				for (int m = (1 << k) - 1; m >= 0; m--)
				{
					for (int n = (1 << k) - 1; n >= 0; n--)
					{
						vec3 tile_positions[NUM_TILE_SIDE_VERTICES][NUM_TILE_SIDE_VERTICES];
						vec3 tile_normals[NUM_TILE_SIDE_VERTICES][NUM_TILE_SIDE_VERTICES];
						vec2 tile_tex_coords[NUM_TILE_SIDE_VERTICES][NUM_TILE_SIDE_VERTICES];
						float min_height = FLT_MAX;
						float max_height = -FLT_MAX;
						float max_curvature = 0.0f;
						for (int p = 0; p < NUM_TILE_SIDE_VERTICES; p++)
						{
							for (int q = 0; q < NUM_TILE_SIDE_VERTICES; q++)
							{
								int stride = (1 << (NUM_TERRAIN_LOD - 1 - k));
								int x = i * TERRAIN_TILE_ROOT_SIZE + (m * TERRAIN_TILE_SIZE + p) * stride;
								int y = j * TERRAIN_TILE_ROOT_SIZE + (n * TERRAIN_TILE_SIZE + q) * stride;
								vec3 normal;
								float min_cos_angle = 1.0f;
								if (x == 0)
								{
									normal.x = -(positions[x + stride][y].z - positions[x][y].z) / (stride * TERRAIN_FINEST_VERTEX_SPACING);
								}
								else if (x == NUM_TERRAIN_GRID_X)
								{
									normal.x = -(positions[x][y].z - positions[x - stride][y].z) / (stride * TERRAIN_FINEST_VERTEX_SPACING);
								}
								else
								{
									normal.x = -(positions[x + stride][y].z - positions[x - stride][y].z) / (2 * stride * TERRAIN_FINEST_VERTEX_SPACING);
									vec3 v1 = positions[x][y] - positions[x - stride][y];
									vec3 v2 = positions[x + stride][y] - positions[x][y];
									min_cos_angle = std::min(min_cos_angle, dot(v1, v2) / sqrt(dot(v1, v1) * dot(v2, v2)));
								}
								if (y == 0)
								{
									normal.y = -(positions[x][y + stride].z - positions[x][y].z) / (stride * TERRAIN_FINEST_VERTEX_SPACING);
								}
								else if (y == NUM_TERRAIN_GRID_Y)
								{
									normal.y = -(positions[x][y].z - positions[x][y - stride].z) / (stride * TERRAIN_FINEST_VERTEX_SPACING);
								}
								else
								{
									normal.y = -(positions[x][y + stride].z - positions[x][y - stride].z) / (2 * stride * TERRAIN_FINEST_VERTEX_SPACING);
									vec3 v1 = positions[x][y] - positions[x][y - stride];
									vec3 v2 = positions[x][y + stride] - positions[x][y];
									min_cos_angle = std::min(min_cos_angle, dot(v1, v2) / sqrt(dot(v1, v1) * dot(v2, v2)));
								}
								normal.z = 1;
								normal = normalize(normal);
								tile_positions[p][q] = positions[x][y];
								tile_normals[p][q] = normal;
								tile_tex_coords[p][q] = vec2(0.09375f, 1.0f);
								min_height = std::min(min_height, positions[x][y].z);
								max_height = std::max(max_height, positions[x][y].z);
								max_curvature = std::max(max_curvature, sqrt((1.0f - min_cos_angle) / std::max(FLT_EPSILON, 1.0f + min_cos_angle)));
							}
						}
						(&tiles[0][0][0])[data_offset].min_height = min_height;
						(&tiles[0][0][0])[data_offset].max_height = max_height;
						(&tiles[0][0][0])[data_offset].max_curvature = max_curvature;
						glBufferSubData(GL_ARRAY_BUFFER, data_offset * NUM_VERTICES_PER_TILE * sizeof(vec3), NUM_VERTICES_PER_TILE * sizeof(vec3), tile_positions);
						glBufferSubData(GL_ARRAY_BUFFER, POSITION_SIZE + data_offset * NUM_VERTICES_PER_TILE * sizeof(vec3), NUM_VERTICES_PER_TILE * sizeof(vec3), tile_normals);
						glBufferSubData(GL_ARRAY_BUFFER, POSITION_SIZE + NORMAL_SIZE + data_offset * NUM_VERTICES_PER_TILE * sizeof(vec2), NUM_VERTICES_PER_TILE * sizeof(vec2), tile_tex_coords);
						data_offset--;
					}
				}
			}
		}
	}

	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			for (int k = 0; k < NUM_TERRAIN_LOD - 1; k++)
			{
				for (int m = 0; m < (1 << k); m++)
				{
					for (int n = 0; n < (1 << k); n++)
					{
						float max_child_curvature = 0.0f;
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m, 2 * n)].max_curvature);
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m + 1, 2 * n)].max_curvature);
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m, 2 * n + 1)].max_curvature);
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m + 1, 2 * n + 1)].max_curvature);
						tiles[i][j][TILE_IDX(k, m, n)].max_curvature = max_child_curvature;
					}
				}
			}
		}
	}

	for (int i = NUM_TERRAIN_TILE_ROOTS_X - 1; i >= 0; i--)
	{
		for (int j = NUM_TERRAIN_TILE_ROOTS_Y - 1; j >= 0; j--)
		{
			for (int k = NUM_TERRAIN_LOD - 3; k >= 0; k--)
			{
				for (int m = (1 << k) - 1; m >= 0; m--)
				{
					for (int n = (1 << k) - 1; n >= 0; n--)
					{
						float max_child_curvature = 0.0f;
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m, 2 * n)].max_curvature);
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m + 1, 2 * n)].max_curvature);
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m, 2 * n + 1)].max_curvature);
						max_child_curvature = std::max(max_child_curvature, tiles[i][j][TILE_IDX(k + 1, 2 * m + 1, 2 * n + 1)].max_curvature);
						tiles[i][j][TILE_IDX(k, m, n)].max_curvature = std::max(tiles[i][j][TILE_IDX(k, m, n)].max_curvature, 0.5f * max_child_curvature);
					}
				}
			}
		}
	}

	glGenBuffers(1, &terrain_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrain_EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, INDICES_OFFSET() * sizeof(int), nullptr, GL_STATIC_DRAW);

	for (int i = 0; i < 16; i++)
	{
		int indices[NUM_INDICES(0)];
		int idx = 0;
		int j = 0;
		if (i & STITCHING_LEFT)
		{
			for (int k = 0; k < TERRAIN_TILE_SIZE; k += 2)
			{
				if (k != 0 || !(i & STITCHING_BOTTOM))
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
				indices[idx + 1] = k + 2 + (k == TERRAIN_TILE_SIZE - 2 && i & STITCHING_TOP ? 2 * NUM_TILE_SIDE_VERTICES : NUM_TILE_SIDE_VERTICES);
				indices[idx + 2] = k + 2;
				idx += 3;
			}
			j++;
		}
		for (; j < TERRAIN_TILE_SIZE - (i & STITCHING_RIGHT) / STITCHING_RIGHT; j++)
		{
			int offset = j * NUM_TILE_SIDE_VERTICES;
			for (int k = 0; k < TERRAIN_TILE_SIZE; k++)
			{
				if (k == 0 && i & STITCHING_BOTTOM)
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
				if (k == TERRAIN_TILE_SIZE - 1 && i & STITCHING_TOP)
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
		if (i & STITCHING_RIGHT)
		{
			constexpr int offset = (TERRAIN_TILE_SIZE - 1) * NUM_TILE_SIDE_VERTICES;
			for (int k = 0; k < TERRAIN_TILE_SIZE; k += 2)
			{
				indices[idx + 0] = offset + k + (k == 0 && i & STITCHING_BOTTOM ? -NUM_TILE_SIDE_VERTICES : 0);
				indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES;
				indices[idx + 2] = offset + k + 1;
				idx += 3;
				indices[idx + 0] = offset + k + 1;
				indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES;
				indices[idx + 2] = offset + k + NUM_TILE_SIDE_VERTICES + 2;
				idx += 3;
				if (k != TERRAIN_TILE_SIZE - 2 || !(i & STITCHING_TOP))
				{
					indices[idx + 0] = offset + k + 1;
					indices[idx + 1] = offset + k + NUM_TILE_SIDE_VERTICES + 2;
					indices[idx + 2] = offset + k + 2;
					idx += 3;
				}
			}
		}
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, INDICES_OFFSET(i) * sizeof(int), NUM_INDICES(i) * sizeof(int), indices);
	}

	{
		int indices[NUM_INDICES(STITCHING_HALF)];
		int idx = 0;
		for (int i = 0; i < TERRAIN_TILE_SIZE; i += 2)
		{
			for (int j = 0; j < TERRAIN_TILE_SIZE; j += 2)
			{
				indices[idx + 0] = i * NUM_TILE_SIDE_VERTICES + j;
				indices[idx + 1] = (i + 1) * NUM_TILE_SIDE_VERTICES + j + 1;
				indices[idx + 2] = i * NUM_TILE_SIDE_VERTICES + j + 2;
				idx += 3;
				indices[idx + 0] = i * NUM_TILE_SIDE_VERTICES + j;
				indices[idx + 1] = (i + 2) * NUM_TILE_SIDE_VERTICES + j;
				indices[idx + 2] = (i + 1) * NUM_TILE_SIDE_VERTICES + j + 1;
				idx += 3;
				indices[idx + 0] = (i + 1) * NUM_TILE_SIDE_VERTICES + j + 1;
				indices[idx + 1] = (i + 2) * NUM_TILE_SIDE_VERTICES + j + 2;
				indices[idx + 2] = i * NUM_TILE_SIDE_VERTICES + j + 2;
				idx += 3;
				indices[idx + 0] = (i + 1) * NUM_TILE_SIDE_VERTICES + j + 1;
				indices[idx + 1] = (i + 2) * NUM_TILE_SIDE_VERTICES + j;
				indices[idx + 2] = (i + 2) * NUM_TILE_SIDE_VERTICES + j + 2;
				idx += 3;
			}
		}
		glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, INDICES_OFFSET(STITCHING_HALF) * sizeof(int), NUM_INDICES(STITCHING_HALF) * sizeof(int), indices);
	}

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(0));
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(POSITION_SIZE));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, reinterpret_cast<void*>(POSITION_SIZE + NORMAL_SIZE));
	glEnableVertexAttribArray(2);
	glBindVertexArray(0);
}

void updateTerrainLOD(float lod_factor, const vec3& camera_position)
{
	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			vec2 root_offset = TERRAIN_TILE_ROOT_SIZE * TERRAIN_FINEST_VERTEX_SPACING * vec2(i - 0.5f * NUM_TERRAIN_TILE_ROOTS_X, j - 0.5f * NUM_TERRAIN_TILE_ROOTS_Y);
			Tile* root = tiles[i][j];
			[root_offset, root, lod_factor, &camera_position](this auto&& self, int lod, int tile_x, int tile_y)->void
				{
					Tile& tile = root[TILE_IDX(lod, tile_x, tile_y)];
					float tile_side_lenght = TERRAIN_TILE_SIZE * TERRAIN_FINEST_VERTEX_SPACING * (1 << (NUM_TERRAIN_LOD - 1 - lod));
					vec3 bound_min = vec3(root_offset + vec2(tile_side_lenght * tile_x, tile_side_lenght * tile_y), tile.min_height);
					vec3 bound_max = vec3(root_offset + vec2(tile_side_lenght * (tile_x + 1), tile_side_lenght * (tile_y + 1)), tile.max_height);
					vec3 v = clamp(camera_position, bound_min, bound_max) - camera_position;
					float distance2 = dot(v, v);
					float threshold2 = lod_factor * tile.max_curvature * TERRAIN_FINEST_VERTEX_SPACING * (1 << (NUM_TERRAIN_LOD - 1 - lod));
					threshold2 *= threshold2;
					tile.active = distance2 > threshold2 || lod == NUM_TERRAIN_LOD - 1;
					if (tile.active)
					{
						tile.stitching = distance2 > 2.0f * threshold2 ? STITCHING_HALF : 0;
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

	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			[](this auto&& check_lod, int root_x, int root_y, int lod, int tile_x, int tile_y)->void
				{
					Tile& tile = tiles[root_x][root_y][TILE_IDX(lod, tile_x, tile_y)];
					if (tile.active)
					{
						auto check_neighbor = [check_lod, root_x, root_y, lod, tile_x, tile_y](this auto&& self, int offset_x, int offset_y)->void
							{
								int lod_side_size = (1 << lod);
								int neighbor_root_x = root_x;
								int neighbor_root_y = root_y;
								int neighbor_tile_x = tile_x + offset_x;
								int neighbor_tile_y = tile_y + offset_y;
								if (neighbor_tile_x < 0)
								{
									neighbor_tile_x = lod_side_size - 1;
									neighbor_root_x--;
								}
								else if (neighbor_tile_x >= lod_side_size)
								{
									neighbor_tile_x = 0;
									neighbor_root_x++;
								}
								if (neighbor_tile_y < 0)
								{
									neighbor_tile_y = lod_side_size - 1;
									neighbor_root_y--;
								}
								else if (neighbor_tile_y >= lod_side_size)
								{
									neighbor_tile_y = 0;
									neighbor_root_y++;
								}
								if (neighbor_root_x < 0 || neighbor_root_x >= NUM_TERRAIN_TILE_ROOTS_X || neighbor_root_y < 0 || neighbor_root_y >= NUM_TERRAIN_TILE_ROOTS_Y)
								{
									return;
								}
								Tile* neighbor_root = tiles[neighbor_root_x][neighbor_root_y];
								int neighbor_lod = 0;
								for (neighbor_lod = 0; neighbor_lod <= lod; neighbor_lod++)
								{
									int lod_tile_x = neighbor_tile_x >> (lod - neighbor_lod);
									int lod_tile_y = neighbor_tile_y >> (lod - neighbor_lod);
									Tile& neighbor_tile = neighbor_root[TILE_IDX(neighbor_lod, lod_tile_x, lod_tile_y)];
									if (neighbor_tile.active)
									{
										if (lod == neighbor_lod)
										{
											return;
										}
										if (lod - neighbor_lod == 1)
										{
											if (neighbor_tile.stitching == STITCHING_HALF)
											{
												neighbor_tile.stitching = 0;
												check_lod(neighbor_root_x, neighbor_root_y, neighbor_lod, lod_tile_x, lod_tile_y);
											}
											return;
										}
										neighbor_tile.active = false;
										for (int i = 0; i < 2; i++)
										{
											for (int j = 0; j < 2; j++)
											{
												int x = 2 * lod_tile_x + i;
												int y = 2 * lod_tile_y + j;
												neighbor_root[TILE_IDX(neighbor_lod + 1, x, y)].active = true;
												neighbor_root[TILE_IDX(neighbor_lod + 1, x, y)].stitching = STITCHING_HALF;
											}
										}
										for (int i = 0; i < 2; i++)
										{
											for (int j = 0; j < 2; j++)
											{
												int x = 2 * lod_tile_x + i;
												int y = 2 * lod_tile_y + j;
												check_lod(neighbor_root_x, neighbor_root_y, neighbor_lod + 1, x, y);
											}
										}
									}
								}
								return;
							};
						check_neighbor(-1, 0);
						check_neighbor(1, 0);
						check_neighbor(0, -1);
						check_neighbor(0, 1);
					}
					else
					{
						check_lod(root_x, root_y, lod + 1, 2 * tile_x, 2 * tile_y);
						check_lod(root_x, root_y, lod + 1, 2 * tile_x + 1, 2 * tile_y);
						check_lod(root_x, root_y, lod + 1, 2 * tile_x, 2 * tile_y + 1);
						check_lod(root_x, root_y, lod + 1, 2 * tile_x + 1, 2 * tile_y + 1);
					}
				}(i, j, 0, 0, 0);
		}
	}
	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			[](this auto&& build_stitching, int root_x, int root_y, int lod, int tile_x, int tile_y)->void
				{
					Tile& tile = tiles[root_x][root_y][TILE_IDX(lod, tile_x, tile_y)];
					if (tile.active)
					{
						if (tile.stitching == STITCHING_HALF)
						{
							return;
						}
						auto check_tile_coarser = [root_x, root_y, lod, tile_x, tile_y](int offset_x, int offset_y)->bool
							{
								int lod_side_size = (1 << lod);
								int neighbor_root_x = root_x;
								int neighbor_root_y = root_y;
								int neighbor_tile_x = tile_x + offset_x;
								int neighbor_tile_y = tile_y + offset_y;
								if (neighbor_tile_x < 0)
								{
									neighbor_tile_x = lod_side_size - 1;
									neighbor_root_x--;
								}
								else if (neighbor_tile_x >= lod_side_size)
								{
									neighbor_tile_x = 0;
									neighbor_root_x++;
								}
								if (neighbor_tile_y < 0)
								{
									neighbor_tile_y = lod_side_size - 1;
									neighbor_root_y--;
								}
								else if (neighbor_tile_y >= lod_side_size)
								{
									neighbor_tile_y = 0;
									neighbor_root_y++;
								}
								if (neighbor_root_x < 0 || neighbor_root_x >= NUM_TERRAIN_TILE_ROOTS_X || neighbor_root_y < 0 || neighbor_root_y >= NUM_TERRAIN_TILE_ROOTS_Y)
								{
									return false;
								}
								Tile* neighbor_root = tiles[neighbor_root_x][neighbor_root_y];
								int neighbor_lod = 0;
								for (neighbor_lod = 0; neighbor_lod <= lod; neighbor_lod++)
								{
									int lod_tile_x = neighbor_tile_x >> (lod - neighbor_lod);
									int lod_tile_y = neighbor_tile_y >> (lod - neighbor_lod);
									Tile& neighbor_tile = neighbor_root[TILE_IDX(neighbor_lod, lod_tile_x, lod_tile_y)];
									if (neighbor_tile.active)
									{
										return lod > neighbor_lod || neighbor_tile.stitching == STITCHING_HALF;
									}
								}
								return false;
							};
						if (check_tile_coarser(-1, 0))
						{
							tile.stitching |= STITCHING_LEFT;
						}
						if (check_tile_coarser(1, 0))
						{
							tile.stitching |= STITCHING_RIGHT;
						}
						if (check_tile_coarser(0, -1))
						{
							tile.stitching |= STITCHING_BOTTOM;
						}
						if (check_tile_coarser(0, 1))
						{
							tile.stitching |= STITCHING_TOP;
						}
					}
					else
					{
						build_stitching(root_x, root_y, lod + 1, 2 * tile_x, 2 * tile_y);
						build_stitching(root_x, root_y, lod + 1, 2 * tile_x + 1, 2 * tile_y);
						build_stitching(root_x, root_y, lod + 1, 2 * tile_x, 2 * tile_y + 1);
						build_stitching(root_x, root_y, lod + 1, 2 * tile_x + 1, 2 * tile_y + 1);
					}
				}(i, j, 0, 0, 0);
		}
	}
}

constexpr int MAX_DRAW_TILES = NUM_TERRAIN_TILE_ROOTS_X * NUM_TERRAIN_TILE_ROOTS_Y * (1 << (NUM_TERRAIN_LOD - 1)) * (1 << (NUM_TERRAIN_LOD - 1));
GLsizei count[MAX_DRAW_TILES];
void* indicies[MAX_DRAW_TILES];
GLint basevertex[MAX_DRAW_TILES];

void drawTerrainMesh()
{
	glBindVertexArray(terrain_VAO);
	GLsizei tile_cnt = 0;
	for (int i = 0; i < NUM_TERRAIN_TILE_ROOTS_X; i++)
	{
		for (int j = 0; j < NUM_TERRAIN_TILE_ROOTS_Y; j++)
		{
			Tile* root = tiles[i][j];
			[&tile_cnt, root](this auto&& self, int lod, int tile_x, int tile_y) -> void
				{
					Tile& tile = root[TILE_IDX(lod, tile_x, tile_y)];
					if (tile.active || lod == NUM_TERRAIN_LOD - 1)
					{
						count[tile_cnt] = NUM_INDICES(tile.stitching);
						indicies[tile_cnt] = reinterpret_cast<void*>(INDICES_OFFSET(tile.stitching) * sizeof(int));
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
	glMultiDrawElementsBaseVertex(GL_TRIANGLES, count, GL_UNSIGNED_INT, indicies, tile_cnt, basevertex);

}
