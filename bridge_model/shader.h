#pragma once
#include "glad/glad.h"

GLuint loadShader(const char* shader_name, GLenum type);

GLuint linkShaderProgram(GLuint vert, GLuint frag, GLuint geom = 0);