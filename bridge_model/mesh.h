#pragma once

#include "glew.h"
#include "glm\glm.hpp"

constexpr int groundEBOsize = 3906;
constexpr int bridgeEBOsize = 18966;
constexpr int carEBOsize = 2496;
constexpr int carShadowEBOsize = 1566;
constexpr int sunVBOsize = 80;

extern GLuint groundVAO, bridgeVAO;
extern GLuint carVAO, carShadowVAO, carMatVBO, carColorVBO;
extern GLuint sunVAO;

extern const glm::vec3 carBoundray[8];

void buildMeshes();