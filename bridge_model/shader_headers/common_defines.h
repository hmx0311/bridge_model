#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#ifdef __cplusplus
#include "glm.hpp"
using glm::ivec2;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::findMSB;

#define INLINE inline

#define UNIFORM_BUFFER(type, position) struct type
#define STORAGE_BUFFER(type, position) struct type
#define BUFFER_NAME(name) ;
#else
#define alignas(x)
#define INLINE
#define constexpr
#define UNIFORM_BUFFER(type, position) layout(std140, binding = position) uniform type##Block##position
#define STORAGE_BUFFER(type, position) layout(std430, binding = position) buffer type##Block##position
#define BUFFER_NAME(name) name;
#endif // __cplusplus

#define RENDER_TARGET_COLOR				0
#define RENDER_TARGET_BLOOM				1
#define RENDER_TARGET_DELTA_POSITION	2
#define RENDER_TARGET_ALBEDO			3

#endif // !COMMON_DEFINES_H
