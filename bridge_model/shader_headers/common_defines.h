#ifdef __cplusplus
using glm::vec3;
using glm::mat4;
#endif

#ifdef __cplusplus
#define DECLARE_UNIFORM(name, position) struct name
#else
#define DECLARE_UNIFORM(name, position) layout(std140, binding = position) uniform name
#endif
