#include <Windows.h>

#include "shader.h"

#include <vector>
#include <string>

GLuint loadShader(const char* shader_name, GLenum type)
{
	HRSRC rc_info = FindResourceA(nullptr, shader_name, "SHADER");
	if (rc_info == nullptr)
	{
		printf("\nERROR: Can't Find Resource %s\n", shader_name);
		return 0;
	}
	std::vector<int> sizes;
	std::vector<const char*> sources;
	struct ShaderName
	{
		const char* name;
		int len;
	};
	std::vector<ShaderName> source_names;
	sizes.push_back(SizeofResource(nullptr, rc_info));
	HGLOBAL rc_data = LoadResource(nullptr, rc_info);
	if (rc_data == nullptr)
	{
		printf("\nERROR: Can't Load Resource %s\n", shader_name);
		return 0;
	}
	sources.push_back(static_cast<const char*>(LockResource(rc_data)));
	source_names.push_back({ shader_name, static_cast<int>(strlen(shader_name)) });
	for (int i = 0; i < sources.size(); i++)
	{
		const char* source = sources[i];
		int len = sizes[i];
		int j = 0;
		int comment_type = 0;
		int line_cnt = 1;
		while (j < len)
		{
			if (source[j] == '\n')
			{
				line_cnt++;
			}
			if (comment_type > 0)
			{
				if (comment_type == 1 && source[j] == '\n')
				{
					comment_type = 0;
				}
				else if (comment_type == 2 && source[j] == '*' && j + 1 < len && source[j + 1] == '/')
				{
					comment_type = 0;
					j++;
				}
			}
			else
			{
				if (source[j] == '#' && j + sizeof("#include") - 2 < len && strncmp(source + j, "#include", sizeof("#include") - 1) == 0)
				{
					j += sizeof("#include") - 2;
					while (true)
					{
						j++;
						if (j == len)
						{
							printf("\nERROR: Invalid include in shader %.*s line %d\n", source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
						if (source[j] == '\"')
						{
							break;
						}
						if (!isspace(source[j]) || source[j] == '\n')
						{
							printf("\nERROR: Invalid include in shader %.*s line %d\n", source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
					}
					int name_begin = j + 1;
					do
					{
						j++;
						if (j == len || source[j] == '\n')
						{
							printf("\nERROR: Invalid include in shader %.*s line %d\n", source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
					} while (source[j] != '\"');
					int name_len = j - name_begin;
					for (j++; j < len; j++)
					{
						if (source[j] == '\n')
						{
							break;
						}
						if (!isspace(source[j]))
						{
							printf("\nERROR: Invalid include in shader %.*s line %d\n", source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
					}
					bool included = false;
					for (int k = i; k < sources.size(); k++)
					{
						if (name_len == source_names[k].len && strncmp(source + name_begin, source_names[k].name, name_len) == 0)
						{
							included = true;
							break;
						}
					}
					if (!included)
					{
						HRSRC rc_info = FindResourceA(nullptr, std::string(source + name_begin, name_len).c_str(), "SHADER_HEADER");
						if (rc_info == nullptr)
						{
							printf("\nERROR: Can't find include resource \"%.*s\" in %.*s line %d\n", name_len, source + name_begin, source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
						sizes.insert(sizes.begin() + i + 1, SizeofResource(nullptr, rc_info));
						HGLOBAL rc_data = LoadResource(nullptr, rc_info);
						if (rc_data == nullptr)
						{
							printf("\nERROR: Can't load include resource \"%.*s\" in %.*s line %d\n", name_len, source + name_begin, source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
						sources.insert(sources.begin() + i + 1, static_cast<const char*>(LockResource(rc_data)));
						source_names.insert(source_names.begin() + i + 1, { source + name_begin, name_len });
					}
					len -= j;
					source = &source[j];
					sizes[i] = len;
					sources[i] = source;
					j = 0;
					continue;
				}
				else if (source[j] == '/' && j + 1 < len)
				{
					if (source[j + 1] == '/')
					{
						comment_type = 1;
						j++;
					}
					else if (source[j + 1] == '*')
					{
						comment_type = 2;
						j++;
					}
					else
					{
						break;
					}
				}
				else if (!isspace(source[j]))
				{
					break;
				}
			}
			j++;
		}
	}
	const char* version_str = "#version 430 core\n";
	sources.push_back(version_str);
	sizes.push_back(-1);
	std::reverse(sources.begin(), sources.end());
	std::reverse(sizes.begin(), sizes.end());
#if 1
	std::string cource_str;
	for (int i = 0; i < sources.size(); i++)
	{
		if (sizes[i] < 0)
		{
			cource_str.append(sources[i]);
		}
		else
		{
			cource_str.append(sources[i], sizes[i]);
		}
	}
	sources.clear();
	sources.push_back(cource_str.c_str());
	sizes.clear();
	sizes.push_back(-1);
#endif

	GLint status;
	GLuint shader = glCreateShader(type);
	glShaderSource(shader, sources.size(), sources.data(), sizes.data());
	glCompileShader(shader);

	// check for errors
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("\nERROR: Shader \"%s\" Compilation Error\n", shader_name);
		for (const char* source : sources)
		{
			printf("%s", source);
		}
		int len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		char* log = new char[len];
		glGetShaderInfoLog(shader, len, nullptr, log);
		printf("%s", log);
		delete[] log;
	}
	return shader;
}

GLuint linkShaderProgram(GLuint vert, GLuint frag, GLuint geom)
{
	GLint status;
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	if (geom != 0)
	{
		glAttachShader(program, geom);
	}
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("\nERROR: Shader Program %d Link Error\n", program);
		int len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		char* log = new char[len];
		glGetProgramInfoLog(program, len, nullptr, log);
		printf("%s", log);
		delete[] log;
	}
	glDetachShader(program, vert);
	glDetachShader(program, frag);
	if (geom != 0)
	{
		glDetachShader(program, geom);
	}
	return program;
}