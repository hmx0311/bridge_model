#ifndef COMMON_DEFINES_H
#define COMMON_DEFINES_H

#ifdef __cplusplus
#include "glm.hpp"
using glm::uint;
using glm::ivec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::findMSB;

#define INLINE inline

#define UNIFORM_BUFFER_BEGIN(type, position) struct type
#define STORAGE_BUFFER_BEGIN(type, position) struct type
#define BUFFER_END(name) ;
#else
#define alignas(x)
#define INLINE
#define constexpr
#define UNIFORM_BUFFER_BEGIN(type, position) layout(std140, binding = position) uniform type##Block##position
#define STORAGE_BUFFER_BEGIN(type, position) layout(std430, binding = position) buffer type##Block##position
#define BUFFER_END(name) name;
#endif // __cplusplus

#endif // !COMMON_DEFINES_H
