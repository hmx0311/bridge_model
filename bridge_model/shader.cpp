#include "shader.h"

#include <memory>
#include <vector>
#include <string>

#include "common.h"

static const char* version_str = "#version 430 core\n";

GLuint loadShader(const char* shader_name, GLenum type)
{
	auto shader_source = getResource(shader_name, "SHADER");
	if (shader_source.first == nullptr)
	{
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
	sizes.push_back(shader_source.second);
	sources.push_back(static_cast<const char*>(shader_source.first));
	source_names.push_back({ shader_name, static_cast<int>(strlen(shader_name)) });
	for (int i = 0; i < source_names.size(); i++)
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
						auto [header_soucre, size] = getResource(std::string(source + name_begin, name_len).c_str(), "SHADER_HEADER");
						if (header_soucre == nullptr)
						{
							printf("\nERROR: Can't get include resource \"%.*s\" in %.*s line %d\n", name_len, source + name_begin, source_names[i].len, source_names[i].name, line_cnt);
							return 0;
						}
						sizes.insert(sizes.begin() + i + 1, size);
						sources.insert(sources.begin() + i + 1, static_cast<const char*>(header_soucre));
						source_names.insert(source_names.begin() + i + 1, { source + name_begin, name_len });
					}
					len -= j;
					source += j;
					sizes[i] = len;
					sources[i] = source;
					j = 0;
					continue;
				}
				else if (source[j] == '#' && j + sizeof("#extension") - 1 < len && strncmp(source + j, "#extension", sizeof("#extension") - 1) == 0)
				{
					j += sizeof("#extension") - 1;
					while (j < len && source[j++] != '\n');
					sizes.insert(sizes.begin() + source_names.size(), j);
					sources.insert(sources.begin() + source_names.size(), source);
					len -= j;
					source += j;
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
		int total_lines = 1;
		char last = '\n';
		std::reverse(source_names.begin(), source_names.end());
		for (int i = 0; i < sources.size(); i++)
		{
			const char* source = sources[i];
			int size = sizes[i];
			if (size < 0)
			{
				size = INT_MAX;
			}
			int file_lines = 1;
			if (i >= sources.size() - source_names.size())
			{
				auto [source_begin, _] = i == sources.size() - 1 ? getResource(shader_name, "SHADER") :
					getResource(std::string(source_names[i + source_names.size() - sources.size()].name, source_names[i + source_names.size() - sources.size()].len).c_str(), "SHADER_HEADER");
				for (const char* p = static_cast<const char*>(source_begin); p != source; p++)
				{
					if (*p == '\n')
					{
						file_lines++;
					}
				}
			}
			for (int j = 0; j < size && source[j] != '\0'; j++)
			{
				if (last == '\n')
				{
					printf("%d\t%d\t", total_lines++, file_lines++);
				}
				last = source[j];
				putchar(source[j]);
			}
		}
		int len;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
		auto log = std::make_unique<char[]>(len);
		glGetShaderInfoLog(shader, len, nullptr, log.get());
		printf("%s", log.get());
	}
	return shader;
}

GLuint linkShaderProgram(GLuint vert, GLuint frag)
{
	GLint status;
	GLuint program = glCreateProgram();
	glAttachShader(program, vert);
	glAttachShader(program, frag);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("\nERROR: Shader Program %d Link Error\n", program);
		int len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		auto log = std::make_unique<char[]>(len);
		glGetProgramInfoLog(program, len, nullptr, log.get());
		printf("%s", log.get());
	}
	glDetachShader(program, vert);
	glDetachShader(program, frag);
	return program;
}

GLuint loadComputeProgram(const char* shader_name)
{
	GLuint compute_shader = loadShader(shader_name, GL_COMPUTE_SHADER);
	GLint status;
	GLuint program = glCreateProgram();
	glAttachShader(program, compute_shader);
	glLinkProgram(program);
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		printf("\nERROR: Shader Program %d Link Error\n", program);
		int len;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
		auto log = std::make_unique<char[]>(len);
		glGetProgramInfoLog(program, len, nullptr, log.get());
		printf("%s", log.get());
	}
	glDetachShader(program, compute_shader);
	glDeleteShader(compute_shader);
	return program;
}
