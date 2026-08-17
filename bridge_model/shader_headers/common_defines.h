#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#ifdef __cplusplus
#include "glm.hpp"
using glm::ivec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;

#define DECLARE_UNIFORM(type, name, position) \
	constexpr int name##_binding = position;
#define DECLARE_BUFFER(type, name, position) \
	constexpr int name##_binding = position;
#else
#define alignas(x)
#define DECLARE_UNIFORM(type, name, position) layout(std140, binding = position) uniform name##_block { type name; }
#define DECLARE_BUFFER(type, name, position) layout(std430, binding = position) buffer name##_block { type name; }
#endif // __cplusplus

#endif // !COMMON_DEFINES_H
