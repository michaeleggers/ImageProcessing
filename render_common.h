#ifndef _RENDER_COMMON_H_
#define _RENDER_COMMON_H_

#define GLM_FORCE_RADIANS
#include "dependencies/glm/glm.hpp"
#include "dependencies/glm/ext.hpp"

struct Vertex {
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec2 uv;
};

struct Rect {
	Vertex v1, v2, v3, v4;
};

#endif